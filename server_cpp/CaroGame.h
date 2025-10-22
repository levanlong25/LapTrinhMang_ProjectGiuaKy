#pragma once

// Enum trạng thái ô cờ
enum class CellState {
    Empty,
    X,
    O
};

// Enum lượt người chơi
enum class Player {
    X,
    O
};

// Enum trạng thái ván cờ
enum class GameState {
    Playing,
    X_Won,
    O_Won,
    Draw
};

// === Định nghĩa class CaroGame ===
class CaroGame {
public:
    // Kích thước bàn cờ (3x3)
    static constexpr int BOARD_SIZE = 3;

    // Constructor: tự động gọi reset bàn cờ
    CaroGame();

    // Thực hiện một nước đi (row, col), trả về true nếu hợp lệ
    bool makeMove(int row, int col);

    // Đặt lại bàn cờ, bắt đầu ván mới
    void reset();

    // Lấy trạng thái game hiện tại (win/lose/draw/playing)
    GameState getGameState() const;

    // Lấy người chơi đang tới lượt
    Player getCurrentPlayer() const;

    // Lấy trạng thái một ô (Empty/X/O)
    CellState getCellState(int row, int col) const;

private:
    // Ma trận bàn cờ 3x3
    CellState board[BOARD_SIZE][BOARD_SIZE];

    Player currentPlayer;
    GameState currentState;
    int moveCount;

    // Kiểm tra nước đi hợp lệ
    bool isMoveValid(int row, int col) const;

    // Cập nhật trạng thái game sau mỗi nước đi
    void updateGameState(int lastRow, int lastCol);

    // Kiểm tra thắng/thua với ô vừa đánh (lastRow, lastCol)
    bool checkWin(int x, int y);
};