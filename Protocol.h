#pragma once
#include <string>
#include <vector>
#include <sstream>

/*
 * Giao thức Client-Server dạng text:
 * COMMAND param1 param2 ... \n
 *
 * --- Client gửi lên Server ---
 * CREATE_ROOM
 * JOIN_ROOM <room_id>
 * MOVE <x> <y>
 * EXIT_ROOM
 *
 * --- Server gửi xuống Client ---
 * ROOM_CREATED <room_id>          // Gửi cho người tạo phòng
 * WAITING_OPPONENT              // Gửi cho người tạo phòng
 * ERROR <error_message>           // Gửi khi có lỗi (vd: JOIN_ROOM thất bại)
 * GAME_START <you_are_char>       // Bắt đầu game, bạn là X hay O
 * UPDATE_BOARD <x> <y> <char>     // Cập nhật bàn cờ với nước đi của đối thủ
 * YOUR_TURN                       // Đến lượt bạn đi
 * GAME_OVER <result> <winner_char> // Kết thúc game. result = WINNER/DRAW
 * OPPONENT_LEFT                   // Đối thủ đã thoát
 */

 // Định nghĩa các lệnh dưới dạng string để dễ sử dụng
namespace Protocol {
    const std::string C_CREATE_ROOM = "CREATE_ROOM";
    const std::string C_JOIN_ROOM = "JOIN_ROOM";
    const std::string C_MOVE = "MOVE";

    const std::string S_ROOM_CREATED = "ROOM_CREATED";
    const std::string S_GAME_START = "GAME_START";
    const std::string S_UPDATE_BOARD = "UPDATE_BOARD";
    const std::string S_YOUR_TURN = "YOUR_TURN";
    const std::string S_GAME_OVER = "GAME_OVER";
    const std::string S_ERROR = "ERROR";
    const std::string S_OPPONENT_LEFT = "OPPONENT_LEFT";

    // Hàm helper để tạo message (Encode)
    template<typename... Args>
    std::string createMessage(const std::string& cmd, Args... args) {
        std::stringstream ss;
        ss << cmd;
        // Gói các tham số vào stringstream
        ((ss << " " << args), ...);
        ss << "\n"; // Quan trọng: kết thúc bằng newline
        return ss.str();
    }
}