using System;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace client_csharp
{
    public class ClientSocket
    {
        private TcpClient client;
        private NetworkStream stream;
        private string serverIp;
        private int serverPort;
        private Thread receiveThread;

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
                Console.WriteLine($"Đang kết nối tới server {serverIp}:{serverPort} ...");

                client = new TcpClient();
                client.Connect(serverIp, serverPort);
                stream = client.GetStream();

                Console.WriteLine("✅ Đã kết nối thành công tới server!");

                // 🔄 Bắt đầu luồng nhận dữ liệu
                StartReceiveLoop();
            }
            catch (SocketException ex)
            {
                Console.WriteLine($"❌ Không thể kết nối tới server: {ex.Message}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"⚠️ Lỗi không xác định: {ex.Message}");
            }
        }

        // 🔹 Bắt đầu luồng nhận dữ liệu từ server
        private void StartReceiveLoop()
        {
            receiveThread = new Thread(() =>
            {
                try
                {
                    while (true)
                    {
                        if (stream == null || !client.Connected) break;

                        byte[] buffer = new byte[1024];
                        int bytesRead = stream.Read(buffer, 0, buffer.Length);

                        if (bytesRead == 0)
                        {
                            Console.WriteLine("⚠️ Server đã đóng kết nối.");
                            break;
                        }

                        string message = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                        Console.WriteLine($"\n📩 Từ server: {message}");
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"⚠️ Lỗi khi nhận dữ liệu: {ex.Message}");
                }
            });

            receiveThread.IsBackground = true;
            receiveThread.Start();
        }

        // 🔹 Gửi dữ liệu tới server
        public void SendData(string message)
        {
            try
            {
                if (stream != null && client.Connected)
                {
                    byte[] data = Encoding.UTF8.GetBytes(message);
                    stream.Write(data, 0, data.Length);
                }
                else
                {
                    Console.WriteLine("⚠️ Không thể gửi: chưa kết nối hoặc đã mất kết nối.");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"⚠️ Lỗi khi gửi dữ liệu: {ex.Message}");
            }
        }

        // 🔹 Nhận dữ liệu (nếu cần đọc tức thì, không dùng thread)
        public string ReceiveData()
        {
            try
            {
                byte[] buffer = new byte[1024];
                int bytesRead = stream.Read(buffer, 0, buffer.Length);
                if (bytesRead > 0)
                {
                    return Encoding.UTF8.GetString(buffer, 0, bytesRead);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"⚠️ Lỗi nhận dữ liệu: {ex.Message}");
            }
            return string.Empty;
        }

        // 🔹 Vòng lặp game (console)
        public void StartGameLoop()
        {
            Console.WriteLine("🎮 Bắt đầu chơi. Nhập tin nhắn hoặc nước đi (vd: 0,2). Gõ 'exit' để thoát.");

            while (true)
            {
                string input = Console.ReadLine();
                if (input.ToLower() == "exit")
                {
                    Disconnect();
                    break;
                }

                SendData(input);
            }
        }

        // 🔹 Hàm ngắt kết nối
        public void Disconnect()
        {
            try
            {
                stream?.Close();
                client?.Close();
                Console.WriteLine("🔌 Đã ngắt kết nối với server.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"⚠️ Lỗi khi ngắt kết nối: {ex.Message}");
            }
        }
    }
}
