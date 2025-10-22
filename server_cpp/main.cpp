#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <cstdlib>
#include <ctime>

// --- Thư viện Socket đa nền tảng ---
#ifdef _WIN32
    // Dành cho Windows (Winsock)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socklen_t = int;
#else
    // Dành cho Linux, macOS
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
using SOCKET = int;
const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
#define closesocket(s) close(s)
#endif

// --- Cấu hình Server ---
const int PORT = 5000;
const int BUFFER_SIZE = 1024;
const int BOARD_SIZE = 3;
const int WIN_CONDITION = 3; 

// --- Khai báo các lớp logic game ---
enum class CellState { Empty, X, O };
enum class PlayerMark { X, O };

// Hàm gửi tin nhắn (thêm \n)
void send_message(SOCKET clientSocket, const std::string& message) {
    std::string full_msg = message + "\n";
    send(clientSocket, full_msg.c_str(), full_msg.length(), 0);
}

// --- Lớp GameRoom ---
class GameRoom {
private:
    std::string id;
    std::vector<SOCKET> players; // Tối đa 2 players
    std::vector<std::vector<CellState>> board;
    std::mutex room_mutex;
    SOCKET current_turn_player = INVALID_SOCKET;
    int moves_count = 0;

public:
    GameRoom(const std::string& roomId) : id(roomId) {
        board.resize(BOARD_SIZE, std::vector<CellState>(BOARD_SIZE, CellState::Empty));
    }

    std::string getId() const { return id; }
    SOCKET getPlayerX() const { return players.empty() ? INVALID_SOCKET : players[0]; }
    SOCKET getPlayerO() const { return players.size() > 1 ? players[1] : INVALID_SOCKET; }
    SOCKET getCurrentTurn() const { return current_turn_player; }
    bool isFull() const { return players.size() >= 2; }

    bool addPlayer(SOCKET clientSocket) {
        std::lock_guard<std::mutex> lock(room_mutex);
        if (isFull()) return false;
        players.push_back(clientSocket);

        if (players.size() == 2) {
            // Khi đủ 2 người, bắt đầu game và xác định lượt đi
            current_turn_player = players[0]; // Player X đi trước
        }
        std::cout << "[GameRoom " << id << "] Player added: " << clientSocket << ". Total: " << players.size() << std::endl;
        return true;
    }

    void removePlayer(SOCKET clientSocket) {
        std::lock_guard<std::mutex> lock(room_mutex);
        players.erase(std::remove(players.begin(), players.end(), clientSocket), players.end());
    }

    // Gửi tin nhắn đến tất cả người chơi trong phòng
    void broadcast(const std::string& message, SOCKET exclude = INVALID_SOCKET) {
        std::lock_guard<std::mutex> lock(room_mutex);
        for (SOCKET player : players) {
            if (player != exclude) {
                send_message(player, message);
            }
        }
    }

    // Xử lý nước đi
    bool makeMove(SOCKET clientSocket, int x, int y) {
        std::lock_guard<std::mutex> lock(room_mutex);

        if (!isFull() || clientSocket != current_turn_player) {
            return false; // Chưa đủ người hoặc không phải lượt
        }
        if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE || board[x][y] != CellState::Empty) {
            return false; // Nước đi không hợp lệ
        }

        PlayerMark mark = (clientSocket == getPlayerX()) ? PlayerMark::X : PlayerMark::O;
        board[x][y] = (mark == PlayerMark::X) ? CellState::X : CellState::O;
        moves_count++;

        // 1. Thông báo nước đi hợp lệ cho cả hai người chơi
        std::string move_msg = "MOVE_OK " + std::to_string(x) + " " + std::to_string(y) + " " + ((mark == PlayerMark::X) ? "X" : "O");
        broadcast(move_msg);

        // 2. Kiểm tra thắng/thua
        if (checkWin(x, y)) {
            std::string winner_mark = (mark == PlayerMark::X) ? "X" : "O";
            broadcast("GAME_OVER WINNER " + winner_mark);
            // Reset game state or prepare for cleanup
            current_turn_player = INVALID_SOCKET;
            return true;
        }

        // 3. Kiểm tra hòa
        if (moves_count == BOARD_SIZE * BOARD_SIZE) {
            broadcast("GAME_OVER DRAW");
            current_turn_player = INVALID_SOCKET;
            return true;
        }

        // 4. Chuyển lượt (FIXED LOGIC)
        SOCKET next_player = (current_turn_player == getPlayerX()) ? getPlayerO() : getPlayerX();
        current_turn_player = next_player;

        // Chỉ gửi "YOUR_TURN" đến người chơi có lượt tiếp theo
        send_message(next_player, "YOUR_TURN");

        return true;
    }

private:
    // Kiểm tra thắng (FIXED LOGIC)
    bool checkWin(int x, int y) {
        CellState mark = board[x][y];
        if (mark == CellState::Empty) return false;

        // Tọa độ các hướng để kiểm tra (ngang, dọc, chéo chính, chéo phụ)
        int dx[] = { 1, 0, 1, 1 };
        int dy[] = { 0, 1, 1, -1 };

        for (int i = 0; i < 4; ++i) {
            int count = 1;
            // Đếm theo hướng (dx[i], dy[i])
            for (int j = 1; j < WIN_CONDITION; ++j) {
                int nx = x + j * dx[i];
                int ny = y + j * dy[i];
                if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == mark) {
                    count++;
                }
                else {
                    break;
                }
            }
            // Đếm theo hướng ngược lại
            for (int j = 1; j < WIN_CONDITION; ++j) {
                int nx = x - j * dx[i];
                int ny = y - j * dy[i];
                if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == mark) {
                    count++;
                }
                else {
                    break;
                }
            }
            if (count >= WIN_CONDITION) return true;
        }
        return false;
    }
};

// --- Lớp RoomManager (Singleton) ---
class RoomManager {
private:
    std::unordered_map<std::string, std::shared_ptr<GameRoom>> rooms;
    std::unordered_map<SOCKET, std::string> client_to_room;
    std::mutex manager_mutex;

    RoomManager() {
        srand(time(NULL));
    }

public:
    static RoomManager* getInstance() {
        static RoomManager instance;
        return &instance;
    }

    std::string generateRoomId() {
        return std::to_string(rand() % 9000 + 1000); // ID 4 chữ số từ 1000-9999
    }

    

    // ✅ Kiểm tra server quá tải

    std::shared_ptr<GameRoom> createRoom(SOCKET clientSocket) {
        std::lock_guard<std::mutex> lock(manager_mutex);
        
        if (rooms.size() >= 10) { // giới hạn 10 phòng
        std::cout << "[WARN] Server full, cannot create more rooms." << std::endl;  
        send_message(clientSocket, "ERROR ServerFull");
        return nullptr;
        }

        std::string newId;
        do {
            newId = generateRoomId();
        } while (rooms.count(newId)); // Đảm bảo ID không trùng

        auto room = std::make_shared<GameRoom>(newId);
        rooms[newId] = room;
        room->addPlayer(clientSocket);
        client_to_room[clientSocket] = newId;
        return room;
    }

    bool joinRoom(const std::string& roomId, SOCKET clientSocket) {
        std::lock_guard<std::mutex> lock(manager_mutex);
        if (rooms.count(roomId) && rooms[roomId]->addPlayer(clientSocket)) {
            client_to_room[clientSocket] = roomId;

            auto room = rooms[roomId];
            // Nếu phòng đã đủ người, bắt đầu game
            if (room->isFull()) {
                room->broadcast("GAME_START " + room->getId() + " X:" + std::to_string(room->getPlayerX()) + " O:" + std::to_string(room->getPlayerO()));
                send_message(room->getCurrentTurn(), "YOUR_TURN");
            }
            return true;
        }
        return false;
    }

    void unregisterClient(SOCKET clientSocket) {
        std::lock_guard<std::mutex> lock(manager_mutex);
        if (client_to_room.count(clientSocket)) {
            std::string roomId = client_to_room[clientSocket];
            if (rooms.count(roomId)) {
                auto room = rooms[roomId];
                room->removePlayer(clientSocket);
                room->broadcast("OPPONENT_LEFT");

                // Nếu phòng không còn đủ người chơi, xóa phòng
                if (room->getPlayerX() == INVALID_SOCKET || room->getPlayerO() == INVALID_SOCKET) {
                    rooms.erase(roomId);
                    std::cout << "[RoomManager] Room deleted: " << roomId << std::endl;
                }
            }
            client_to_room.erase(clientSocket);
        }
    }

    std::shared_ptr<GameRoom> getRoomByClient(SOCKET clientSocket) {
        std::lock_guard<std::mutex> lock(manager_mutex);
        if (client_to_room.count(clientSocket) && rooms.count(client_to_room[clientSocket])) {
            return rooms[client_to_room[clientSocket]];
        }
        return nullptr;
    }
};

// --- Hàm xử lý client (ClientHandler) ---
void handle_client(SOCKET clientSocket);

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[ERROR] WSAStartup that bai." << std::endl;
        return 1;
    }
    std::cout << "[INFO] Khoi tao Winsock thanh cong." << std::endl;
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "[ERROR] Khong the tao socket." << std::endl;
        return 1;
    }
    std::cout << "[INFO] Tao socket server thanh cong." << std::endl;

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] Bind that bai." << std::endl;
        closesocket(serverSocket);
        return 1;
    }
    std::cout << "[BIND] Server da gan dia chi tai port " << PORT << std::endl;

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[ERROR] Listen that bai." << std::endl;
        closesocket(serverSocket);
        return 1;
    }
    std::cout << "[LISTENING] Server dang lang nghe ket noi..." << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "[WARNING] Accept that bai." << std::endl;
            continue;
        }

        char clientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
        std::cout << "[NEW CONNECTION] Chap nhan ket noi tu " << clientIp << ":" << ntohs(clientAddr.sin_port) << " (Socket: " << clientSocket << ")" << std::endl;

        std::thread clientThread(handle_client, clientSocket);
        clientThread.detach();
    }

    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

void handle_client(SOCKET clientSocket) {
    RoomManager* manager = RoomManager::getInstance();
    send_message(clientSocket, "WELCOME CaroServer 5x5");

    char buffer[BUFFER_SIZE];
    int bytesReceived;
    std::string leftover_data;

    do {
        bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            leftover_data += buffer;

            size_t newline_pos;
            while ((newline_pos = leftover_data.find('\n')) != std::string::npos) {
                std::string line = leftover_data.substr(0, newline_pos);
                leftover_data.erase(0, newline_pos + 1);

                if (line.back() == '\r') line.pop_back(); // Xử lý ký tự \r từ telnet/netcat

                if (line.empty()) continue;

                std::stringstream ss(line);
                std::string cmd;
                ss >> cmd;

                std::cout << "[Recv " << clientSocket << "] " << line << std::endl;

                if (cmd == "CREATE_ROOM") {
                    auto room = manager->createRoom(clientSocket);
                    if (room != nullptr)
                    send_message(clientSocket, "ROOM_CREATED " + room->getId());

                }
                else if (cmd == "JOIN_ROOM") {
                    std::string roomId;
                    ss >> roomId;
                    if (roomId.empty()) {
                        send_message(clientSocket, "ERROR MissingRoomId");
                    }
                    else if (manager->joinRoom(roomId, clientSocket)) {
                        send_message(clientSocket, "JOINED_OK " + roomId);
                    }
                    else {
                        send_message(clientSocket, "ERROR CannotJoin " + roomId);
                    }
                }
                else if (cmd == "MOVE") {
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
                }
                else {
                    send_message(clientSocket, "ERROR UnknownCommand " + cmd);
                }
            }
        }
    } while (bytesReceived > 0);

    std::cout << "[DISCONNECTED] Client (Socket: " << clientSocket << ") da ngat ket noi." << std::endl;
    manager->unregisterClient(clientSocket);
    closesocket(clientSocket);
}