using System;
using System.Net.Sockets;

namespace client_csharp
{
    public class ClientSocket
    {
        private TcpClient client;
        private NetworkStream stream;
        private string serverIp;
        private int serverPort;

        // 🔹 Hàm khởi tạo 
        public ClientSocket(string ip, int port)
        {
            serverIp = ip;
            serverPort = port;
        }

        // 🔹 Hàm kết nối tới server 
        public void ConnectToServer()
        {
            try
            {
                Console.WriteLine($" Đang kết nối tới server {serverIp}:{serverPort} ...");

                client = new TcpClient();
                client.Connect(serverIp, serverPort);
                stream = client.GetStream();

                Console.WriteLine(" Đã kết nối thành công tới server!");
            }
            catch (SocketException ex)
            {
                Console.WriteLine($" Không thể kết nối tới server: {ex.Message}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($" Lỗi không xác định: {ex.Message}");
            }
        }

        //  Hàm ngắt kết nối (tùy chọn)
        public void Disconnect()
        {
            if (client != null)
            {
                stream?.Close();
                client.Close();
                Console.WriteLine("🔌 Đã ngắt kết nối với server.");
            }
        }

        
        // Gửi dữ liệu tới server
        public void SendData(string message)
        {
            // TODO: Viết logic gửi dữ liệu 
        }

        // Nhận dữ liệu từ server
        public string ReceiveData()
        {
            // TODO: Viết logic nhận dữ liệu 
            return "";
        }

       
        // Vòng lặp chơi (game loop)
        public void StartGameLoop()
        {
            // TODO: Viết logic game 
        }
    }
}
