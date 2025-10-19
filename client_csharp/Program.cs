using System;
using System.Windows.Forms;

namespace client_csharp
{
    class Program
    {
        [STAThread]
        static void Main()
        {
            Console.WriteLine("=== Game Cờ Caro 3x3 - Client ===");

            // 🔹 Nhập IP và Port từ bàn phím
            Console.Write("Nhập IP server (mặc định 127.0.0.1): ");
            string ip = Console.ReadLine();
            if (string.IsNullOrWhiteSpace(ip))
                ip = "127.0.0.1";

            Console.Write("Nhập port server (mặc định 8080): ");
            string portInput = Console.ReadLine();
            int port = 8080;
            if (!string.IsNullOrWhiteSpace(portInput))
            {
                if (!int.TryParse(portInput, out port))
                {
                    Console.WriteLine("⚠️ Port không hợp lệ, dùng mặc định 8080.");
                    port = 8080;
                }
            }

            // 🔹 Tạo client socket và kết nối
            ClientSocket client = new ClientSocket(ip, port);
            client.ConnectToServer();

            // 🔹 Khởi động giao diện (Form)
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new GameForm(client));
        }
    }
}
