#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <array>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socklen_t = int;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
using SOCKET = int;
const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
#define closesocket(s) close(s)
#endif

using namespace std;

// --- Server Config ---
constexpr int PORT = 5000;
constexpr int BUFFER_SIZE = 1024;
constexpr int BOARD_SIZE = 3;
constexpr int WIN_CONDITION = 3;

// --- Game Logic ---
enum class CellState { Empty, X, O };
enum class PlayerMark { X, O };

void send_message(SOCKET clientSocket, const string& message) {
    string msg = message + "\n";
    send(clientSocket, msg.c_str(), static_cast<int>(msg.length()), 0);
}

// --- GameRoom Class ---
class GameRoom {
    string id;
    array<SOCKET, 2> players{};
    int player_count = 0;
    vector<vector<CellState>> board;
    mutex room_mutex;
    SOCKET current_turn_player = INVALID_SOCKET;
    int moves_count = 0;

public:
    GameRoom(const string& roomId)
        : id(roomId), board(BOARD_SIZE, vector<CellState>(BOARD_SIZE, CellState::Empty)) {
        players.fill(INVALID_SOCKET);
    }
    string getId() const { return id; }
    SOCKET getPlayerX() const { return players[0]; }
    SOCKET getPlayerO() const { return players[1]; }
    SOCKET getCurrentTurn() const { return current_turn_player; }
    bool isFull() const { return player_count == 2; }

    bool addPlayer(SOCKET clientSocket) {
        lock_guard<mutex> lock(room_mutex);
        if (isFull()) return false;
        players[player_count++] = clientSocket;
        if (player_count == 2)
            current_turn_player = players[0];
        cout << "[GameRoom " << id << "] Player added: " << clientSocket << ". Total: " << player_count << endl;
        return true;
    }

    void removePlayer(SOCKET clientSocket) {
        lock_guard<mutex> lock(room_mutex);
        for (int i = 0; i < 2; ++i) {
            if (players[i] == clientSocket) {
                players[i] = INVALID_SOCKET;
                --player_count;
                break;
            }
        }
    }

    void broadcast(const string& message, SOCKET exclude = INVALID_SOCKET) {
        //lock_guard<mutex> lock(room_mutex);
        for (auto player : players)
            if (player != INVALID_SOCKET && player != exclude)
                send_message(player, message);
    }

    bool makeMove(SOCKET clientSocket, int x, int y) {
        lock_guard<mutex> lock(room_mutex);
        if (!isFull() || clientSocket != current_turn_player)
            return false;
        if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE || board[x][y] != CellState::Empty)
            return false;

        PlayerMark mark = (clientSocket == getPlayerX()) ? PlayerMark::X : PlayerMark::O;
        board[x][y] = (mark == PlayerMark::X) ? CellState::X : CellState::O;
        moves_count++;

        string update_msg = "UPDATE_BOARD " + to_string(x) + " " + to_string(y) + " " + ((mark == PlayerMark::X) ? "X" : "O");
        broadcast(update_msg);

        if (checkWin(x, y)) {
            broadcast("GAME_OVER WINNER " + string(1, (mark == PlayerMark::X) ? 'X' : 'O'));
            current_turn_player = INVALID_SOCKET;
            return true;
        }
        if (moves_count == BOARD_SIZE * BOARD_SIZE) {
            broadcast("GAME_OVER DRAW");
            current_turn_player = INVALID_SOCKET;
            return true;
        }

        current_turn_player = (current_turn_player == getPlayerX()) ? getPlayerO() : getPlayerX();
        send_message(current_turn_player, "YOUR_TURN");
        return true;
    }

private:
    bool checkWin(int x, int y) {
        CellState mark = board[x][y];
        if (mark == CellState::Empty) return false;
        const int dx[] = { 1, 0, 1, 1 }, dy[] = { 0, 1, 1, -1 };
        for (int dir = 0; dir < 4; ++dir) {
            int count = 1;
            for (int step = 1; step < WIN_CONDITION; ++step) {
                int nx = x + step * dx[dir], ny = y + step * dy[dir];
                if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == mark)
                    ++count;
                else break;
            }
            for (int step = 1; step < WIN_CONDITION; ++step) {
                int nx = x - step * dx[dir], ny = y - step * dy[dir];
                if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == mark)
                    ++count;
                else break;
            }
            if (count >= WIN_CONDITION) return true;
        }
        return false;
    }
};

// --- RoomManager Singleton ---
class RoomManager {
    unordered_map<string, shared_ptr<GameRoom>> rooms;
    unordered_map<SOCKET, string> client_to_room;
    mutex manager_mutex;
    RoomManager() { srand(static_cast<unsigned>(time(nullptr))); }
public:
    static RoomManager* getInstance() {
        static RoomManager instance;
        return &instance;
    }
    string generateRoomId() {
        return to_string(rand() % 9000 + 1000);
    }
    shared_ptr<GameRoom> createRoom(SOCKET clientSocket) {
        lock_guard<mutex> lock(manager_mutex);
        if (rooms.size() >= 10) {
            send_message(clientSocket, "ERROR ServerFull");
            return nullptr;
        }
        string newId;
        do { newId = generateRoomId(); } while (rooms.count(newId));
        auto room = make_shared<GameRoom>(newId);
        rooms[newId] = room;
        room->addPlayer(clientSocket);
        client_to_room[clientSocket] = newId;
        return room;
    }
    bool joinRoom(const string& roomId, SOCKET clientSocket) {
        lock_guard<mutex> lock(manager_mutex);
        if (rooms.count(roomId) && rooms[roomId]->addPlayer(clientSocket)) {
            client_to_room[clientSocket] = roomId;
            auto room = rooms[roomId];
            if (room->isFull()) {
                send_message(room->getPlayerX(), "GAME_START X");
                send_message(room->getPlayerO(), "GAME_START O");
                send_message(room->getPlayerX(), "YOUR_TURN");
            }
            return true;
        }
        return false;
    }
    void unregisterClient(SOCKET clientSocket) {
        lock_guard<mutex> lock(manager_mutex);
        if (client_to_room.count(clientSocket)) {
            string roomId = client_to_room[clientSocket];
            if (rooms.count(roomId)) {
                auto room = rooms[roomId];
                room->removePlayer(clientSocket);
                room->broadcast("OPPONENT_LEFT");
                if (room->getPlayerX() == INVALID_SOCKET || room->getPlayerO() == INVALID_SOCKET) {
                    rooms.erase(roomId);
                }
            }
            client_to_room.erase(clientSocket);
        }
    }
    shared_ptr<GameRoom> getRoomByClient(SOCKET clientSocket) {
        lock_guard<mutex> lock(manager_mutex);
        if (client_to_room.count(clientSocket) && rooms.count(client_to_room[clientSocket]))
            return rooms[client_to_room[clientSocket]];
        return nullptr;
    }
};

// --- Client handler ---
void handle_client(SOCKET clientSocket) {
    RoomManager* manager = RoomManager::getInstance();
    send_message(clientSocket, "WELCOME CaroServer 3x3");
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    string leftover;
    do {
        bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            leftover += buffer;
            size_t pos;
            while ((pos = leftover.find('\n')) != string::npos) {
                string line = leftover.substr(0, pos);
                leftover.erase(0, pos + 1);
                if (!line.empty() && line.back() == '\r') line.pop_back();
                if (line.empty()) continue;
                stringstream ss(line);
                string cmd;
                ss >> cmd;
                if (cmd == "CREATE_ROOM") {
                    auto room = manager->createRoom(clientSocket);
                    if (room) send_message(clientSocket, "ROOM_CREATED " + room->getId());
                } else if (cmd == "JOIN_ROOM") {
                    string roomId;
                    ss >> roomId;
                    if (roomId.empty()) {
                        send_message(clientSocket, "ERROR MissingRoomId");
                    } else if (manager->joinRoom(roomId, clientSocket)) {
                        send_message(clientSocket, "JOINED_OK " + roomId);
                    } else {
                        send_message(clientSocket, "ERROR CannotJoin " + roomId);
                    }
                } else if (cmd == "MOVE") {
                    int x, y;
                    if (!(ss >> x >> y)) {
                        send_message(clientSocket, "ERROR InvalidMoveFormat");
                        continue;
                    }
                    auto room = manager->getRoomByClient(clientSocket);
                    if (!room) {
                        send_message(clientSocket, "ERROR NotInRoom");
                        continue;
                    }
                    if (!room->makeMove(clientSocket, x, y)) {
                        send_message(clientSocket, "ERROR InvalidMoveOrTurn");
                    }
                } else {
                    send_message(clientSocket, "ERROR UnknownCommand " + cmd);
                }
            }
        }
    } while (bytesReceived > 0);
    manager->unregisterClient(clientSocket);
    closesocket(clientSocket);
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[ERROR] WSAStartup failed." << endl;
        return 1;
    }
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "[ERROR] Failed to create socket." << endl;
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "[ERROR] Bind failed." << endl;
        closesocket(serverSocket);
        return 1;
    }
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "[ERROR] Listen failed." << endl;
        closesocket(serverSocket);
        return 1;
    }
    cout << "[SERVER] Listening on port " << PORT << "..." << endl;

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "[WARNING] Accept failed." << endl;
            continue;
        }
        thread clientThread(handle_client, clientSocket);
        clientThread.detach();
    }

    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}