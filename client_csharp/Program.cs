using System;

namespace client_csharp
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("=== Game Cờ Caro 5x5 - Client ===");

            // Tạo và khởi động client socket
            ClientSocket client = new ClientSocket("127.0.0.1", 8080);
            client.ConnectToServer();

            // Tạo bàn cờ và bắt đầu game
            GameBoard board = new GameBoard(5, 5);
            board.DisplayBoard();

            // Bắt đầu vòng lặp chơi
            client.StartGameLoop(board);
        }
    }
}
