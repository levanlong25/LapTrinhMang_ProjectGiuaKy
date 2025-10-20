#include "CaroGame.h"
#include <stdexcept> // Dùng cho getCellState
#include <iostream>  // Dùng để debug, có thể xóa sau

// === (Nhiệm vụ 2) Khởi tạo bàn cờ 3x3 ===
CaroGame::CaroGame() {
    // Gọi hàm reset khi một đối tượng CaroGame được tạo
    reset();
}

// === (Nhiệm vụ 6) Reset bàn cờ khi chơi lại ===
void CaroGame::reset() {
    // (Nhiệm vụ 2) Khởi tạo ma trận 3x3
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = CellState::Empty;
        }
    }
    // Theo luật, X luôn đi trước
    currentPlayer = Player::X;
    currentState = GameState::Playing;
    moveCount = 0; // Reset số nước đi
}

// === (Nhiệm vụ 4) Hàm kiểm tra hợp lệ nước đi (nội bộ) ===
bool CaroGame::isMoveValid(int row, int col) const {
    // 1. Kiểm tra game có đang diễn ra không
    if (currentState != GameState::Playing) {
        return false; // Game đã kết thúc
    }
    // 2. Kiểm tra tọa độ có nằm ngoài bàn cờ không
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return false; // Tọa độ không hợp lệ
    }
    // 3. Kiểm tra ô đó đã được đánh chưa
    if (board[row][col] != CellState::Empty) {
        return false; // Ô đã được đánh
    }

    // Nếu qua cả 3 kiểm tra -> nước đi hợp lệ
    return true;
}

// === (Nhiệm vụ 3) Hàm đặt quân cờ (move) ===
bool CaroGame::makeMove(int row, int col) {
    // (Nhiệm vụ 4) Gọi hàm kiểm tra hợp lệ
    if (!isMoveValid(row, col)) {
        return false; // Trả về false nếu nước đi không hợp lệ
    }

    // Nếu hợp lệ, tiến hành đặt cờ
    CellState mark = (currentPlayer == Player::X) ? CellState::X : CellState::O;
    board[row][col] = mark;
    moveCount++;

    // (Nhiệm vụ 5) Cập nhật trạng thái (kiểm tra thắng/hòa)
    updateGameState(row, col);

    // Nếu game chưa kết thúc, đổi lượt cho người chơi tiếp theo
    if (currentState == GameState::Playing) {
        currentPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
    }

    return true; // Trả về true báo hiệu nước đi thành công
}

// === (Nhiệm vụ 5) Hàm kiểm tra thắng/thua/hòa (nội bộ) ===
void CaroGame::updateGameState(int lastRow, int lastCol) {
    // 1. Kiểm tra thắng
    if (checkWin(lastRow, lastCol)) {
        // Gán trạng thái thắng cho người chơi vừa đi
        currentState = (currentPlayer == Player::X) ? GameState::X_Won : GameState::O_Won;
    }
    // 2. Nếu không thắng, kiểm tra hòa (đã đánh hết 9 ô)
    else if (moveCount == BOARD_SIZE * BOARD_SIZE) {
        currentState = GameState::Draw;
    }
    // 3. Nếu không, game vẫn tiếp tục (currentState giữ nguyên là Playing)
}

// Hàm con (lấy từ logic của main.cpp) để kiểm tra thắng
bool CaroGame::checkWin(int x, int y) {
    CellState mark = board[x][y];
    if (mark == CellState::Empty) return false;

    // (WIN_CONDITION = 3 cho cờ 3x3)
    const int WIN_CONDITION = 3;

    // Mảng 4 hướng: 1. Ngang, 2. Dọc, 3. Chéo chính, 4. Chéo phụ
    int dx[] = { 1, 0, 1, 1 };
    int dy[] = { 0, 1, 1, -1 };

    for (int i = 0; i < 4; ++i) {
        int count = 1; // Đếm cả ô vừa đánh
        // Đếm theo 1 chiều (ví dụ: sang phải)
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
        // Đếm theo chiều ngược lại (ví dụ: sang trái)
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
        // Kiểm tra đủ 3 ô
        if (count >= WIN_CONDITION) return true;
    }

    // Nếu không tìm thấy đường thắng nào
    return false;
}

// --- Các hàm hỗ trợ (Getters) ---

GameState CaroGame::getGameState() const {
    return currentState;
}

Player CaroGame::getCurrentPlayer() const {
    return currentPlayer;
}

CellState CaroGame::getCellState(int row, int col) const {
    // Kiểm tra tọa độ hợp lệ trước khi truy cập
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        throw std::out_of_range("Toa do getCellState khong hop le.");
    }
    return board[row][col];
}