#include "CaroGame.h"
#include <stdexcept>
#include <cstring>    // Để dùng memset

// === (Nhiệm vụ 2) Khởi tạo bàn cờ 3x3 ===
CaroGame::CaroGame() {
    reset();
}

// === (Nhiệm vụ 6) Reset bàn cờ khi chơi lại ===
void CaroGame::reset() {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = CellState::Empty;
        }
    }
    currentPlayer = Player::X;
    currentState = GameState::Playing;
    moveCount = 0;
}

bool CaroGame::isMoveValid(int row, int col) const {
    if (currentState != GameState::Playing)
        return false;
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE)
        return false;
    if (board[row][col] != CellState::Empty)
        return false;

    return true;
}

bool CaroGame::makeMove(int row, int col) {
    if (!isMoveValid(row, col))
        return false;

    CellState mark = (currentPlayer == Player::X) ? CellState::X : CellState::O;
    board[row][col] = mark;
    moveCount++;

    updateGameState(row, col);

    if (currentState == GameState::Playing) {
        currentPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
    }

    return true;
}

void CaroGame::updateGameState(int lastRow, int lastCol) {
    if (checkWin(lastRow, lastCol)) {
        currentState = (currentPlayer == Player::X) ? GameState::X_Won : GameState::O_Won;
    } else if (moveCount == BOARD_SIZE * BOARD_SIZE) {
        currentState = GameState::Draw;
    }
}

bool CaroGame::checkWin(int x, int y) {
    CellState mark = board[x][y];
    if (mark == CellState::Empty) return false;

    constexpr int WIN_CONDITION = 3;
    int dx[] = { 1, 0, 1, 1 };
    int dy[] = { 0, 1, 1, -1 };

    for (int i = 0; i < 4; ++i) {
        int count = 1;
        for (int j = 1; j < WIN_CONDITION; ++j) {
            int nx = x + j * dx[i];
            int ny = y + j * dy[i];
            if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == mark) {
                count++;
            } else break;
        }
        for (int j = 1; j < WIN_CONDITION; ++j) {
            int nx = x - j * dx[i];
            int ny = y - j * dy[i];
            if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == mark) {
                count++;
            } else break;
        }
        if (count >= WIN_CONDITION) return true;
    }
    return false;
}

GameState CaroGame::getGameState() const {
    return currentState;
}

Player CaroGame::getCurrentPlayer() const {
    return currentPlayer;
}

CellState CaroGame::getCellState(int row, int col) const {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        throw std::out_of_range("Toa do getCellState khong hop le.");
    }
    return board[row][col];
}