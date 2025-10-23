ĐỒ ÁN MÔN HỌC LẬP TRÌNH MẠNG
Nhóm 4 - Game Cờ Caro (Tic Tac Toe) Multi Client-Server
Giới thiệu đề tài
Tên đề tài: Xây dựng game Cờ Caro (Tic Tac Toe) đa phòng, đa người chơi trên mạng LAN sử dụng kỹ thuật lập trình Socket.
Nhóm thực hiện: Nhóm 4
Ngôn ngữ sử dụng:
Server: C++
Client: C# WinForms
Mô tả đề tài
Đề tài xây dựng một hệ thống game Cờ Caro (Tic Tac Toe) cho phép nhiều người chơi kết nối từ nhiều máy tính khác nhau trong mạng LAN. Hệ thống sử dụng mô hình Multi Client-Server với các tính năng:

Server quản lý nhiều phòng chơi, mỗi phòng tối đa 2 người.
Người dùng có thể tạo phòng, tham gia phòng, chơi cờ theo lượt.
Server xử lý toàn bộ logic game và truyền thông tin giữa các client.
Giao diện client trực quan, dễ sử dụng bằng WinForms (C#).
Cấu trúc chương trình
Dự_án_Caro/
│
├── server_cpp/           # Chương trình server (C++)
│   └── main.cpp
│
├── client_csharp/        # Chương trình client (C# WinForms)
│   └── GameForm.cs
│   └── ClientSocket.cs
│   └── ... (các file C# khác)
│
└── README.md
Hướng dẫn chạy chương trình
1. Cài đặt môi trường
Server:
Máy chủ cần cài đặt trình biên dịch C++ (g++, MinGW hoặc Visual Studio).
Client:
Máy client cần cài đặt .NET SDK (khuyên dùng .NET 6 hoặc .NET 8).
2. Biên dịch và chạy SERVER (C++)
Trên Windows (MinGW):

cd server_cpp
g++ main.cpp -o server.exe -lws2_32 -std=c++11 -pthread
server.exe
Trên Linux:

cd server_cpp
g++ main.cpp -o server -std=c++11 -pthread
./server
3. Build và chạy CLIENT (C# WinForms)
cd client_csharp
dotnet build
cd bin/Debug/net8.0-windows/
client_csharp.exe
(Lặp lại bước này để mở nhiều client trên các máy khác nhau trong cùng mạng LAN)

4. Hướng dẫn sử dụng
Mở server trước.
Chạy client:
Nhập IP server (máy chủ), Port (mặc định 5000), bấm Kết nối.
Tạo phòng hoặc nhập mã phòng để vào phòng cùng bạn bè.
Đợi đủ 2 người, game bắt đầu.
Mỗi người chỉ được đánh khi đến lượt, kết quả hiển thị rõ ràng trên giao diện.
Thoát:
Có thể bấm "Thoát" hoặc đóng cửa sổ để rời phòng.

Tác giả & liên hệ
Nhóm 4 - Lập trình mạng

