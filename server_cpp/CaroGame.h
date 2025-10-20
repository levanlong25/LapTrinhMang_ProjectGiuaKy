#pragma once
enum class CellState {
    Empty,
    X,
    O
};
enum class Player {
    X,
    O
};
enum class GameState {
    Playing,
    X_Won,
    O_Won,
    Draw
};

// === (Nhiệm vụ 1) Định nghĩa class CaroGame ===
class CaroGame {
public:
    // Đặt kích thước bàn cờ
    static const int BOARD_SIZE = 3;

    // === (Nhiệm vụ 2) Khởi tạo bàn cờ 3x3 ===
    // Constructor (Hàm dựng) sẽ gọi hàm reset() để khởi tạo
    CaroGame();

    // === (Nhiệm vụ 3) Hàm đặt quân cờ (move) ===
    /**
     * @brief Thực hiện một nước đi. Sẽ tự động gọi (Nhiệm vụ 4) và (Nhiệm vụ 5).
     * @return true nếu nước đi hợp lệ, false nếu không.
     */
    bool makeMove(int row, int col);

    // === (Nhiệm vụ 6) Reset bàn cờ khi chơi lại ===
    void reset();

    // --- Các hàm hỗ trợ để lớp GameRoom lấy thông tin ---

    // Lấy trạng thái game (Thắng/Thua/Hòa/Đang chơi)
    GameState getGameState() const;

    // Lấy lượt của người chơi hiện tại
    Player getCurrentPlayer() const;

    // Lấy trạng thái của một ô (dùng để gửi cho client)
    CellState getCellState(int row, int col) const;

private:
    // === (Nhiệm vụ 2) Ma trận 2 chiều 3x3 ===
    CellState board[BOARD_SIZE][BOARD_SIZE];

    Player currentPlayer;
    GameState currentState;
    int moveCount;

    // === (Nhiệm vụ 4) Hàm kiểm tra hợp lệ nước đi (nội bộ) ===
    bool isMoveValid(int row, int col) const;

    // === (Nhiệm vụ 5) Hàm kiểm tra thắng/thua/hòa (nội bộ) ===
    // Cập nhật trạng thái sau mỗi nước đi
    void updateGameState(int lastRow, int lastCol);
    // Hàm con để kiểm tra 3-ô-thẳng-hàng
    bool checkWin(int x, int y);
};