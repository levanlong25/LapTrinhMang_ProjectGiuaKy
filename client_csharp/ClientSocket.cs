using System;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace client_csharp
{
    public class ClientSocket
    {
        private TcpClient? client;
        private NetworkStream? stream;
        private string serverIp;
        private int serverPort;
        private Thread? receiveThread;
        private bool isRunning = true;

        // 🔹 Sự kiện gửi thông điệp đến Form
        public event Action<string>? OnServerMessage;

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
                        if (client == null || stream == null || !client.Connected)
                            break;

                        byte[] buffer = new byte[1024];
                        int bytesRead = stream.Read(buffer, 0, buffer.Length);

                        if (bytesRead == 0)
                        {
                            Console.WriteLine("⚠️ Server đã đóng kết nối bất ngờ.");
                            Console.WriteLine("💔 Mất kết nối tới server. Vui lòng thử kết nối lại sau.");

                            Disconnect();
                            isRunning = false;
                            Environment.Exit(0);
                            break;
                        }

                        string message = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                        Console.WriteLine($"\n📩 Từ server: {message}");

                        HandleServerMessage(message);
                        OnServerMessage?.Invoke(message);
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

        // 🔹 Xử lý tất cả thông điệp từ server
        private void HandleServerMessage(string message)
        {
            var messages = message.Split('\n');
            foreach (var msg in messages)
            {
                if (string.IsNullOrWhiteSpace(msg)) continue;

                if (msg.Contains("OPPONENT_LEFT"))
                {
                    Console.WriteLine("⚠️ Đối thủ đã thoát khỏi phòng. Trận đấu bị gián đoạn.");
                    Console.WriteLine("👉 Bạn có muốn quay lại sảnh không? (y/n)");

                    Disconnect();
                    isRunning = false;

                    string choice = Console.ReadLine()?.Trim().ToLower();
                    if (choice == "y")
                    {
                        Console.WriteLine("🏠 Đang trở về sảnh...");
                    }
                    else
                    {
                        Console.WriteLine("👋 Cảm ơn bạn đã chơi!");
                        Environment.Exit(0);
                    }
                }
                else if (msg.Contains("ERROR ServerFull"))
                {
                    Console.WriteLine("🚫 Server đang quá tải, không thể tạo phòng mới. Vui lòng thử lại sau.");
                }
                else if (msg.Contains("GAME_OVER"))
                {
                    Console.WriteLine("🏁 Trò chơi đã kết thúc: " + msg);
                }
                else if (msg.Contains("YOUR_TURN"))
                {
                    Console.WriteLine("🎯 Đến lượt bạn đánh!");
                }
                else if (msg.Contains("MOVE_OK"))
                {
                    Console.WriteLine($"✅ Nước đi hợp lệ: {msg}");
                }
                else if (msg.Contains("WELCOME"))
                {
                    Console.WriteLine($"👋 Kết nối thành công: {msg}");
                }
                else
                {
                    Console.WriteLine($"ℹ️ Thông điệp từ server: {msg}");
                }
            }
        }

        // 🔹 Gửi dữ liệu tới server
        public void SendData(string message)
        {
            try
            {
                if (client != null && stream != null && client.Connected)
                {
                    byte[] data = Encoding.UTF8.GetBytes(message);
                    stream.Write(data, 0, data.Length);
                }
                else
                {
                    Console.WriteLine("⚠️ Không thể gửi: chưa kết nối hoặc đã mất kết nối.");
                }
            }
            catch (ObjectDisposedException)
            {
                Console.WriteLine("⚠️ Kết nối đã bị đóng. Không thể gửi dữ liệu.");
            }
            catch (SocketException ex)
            {
                Console.WriteLine($"⚠️ Lỗi mạng: {ex.Message}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"⚠️ Lỗi khi gửi dữ liệu: {ex.Message}");
            }
        }

        // 🔹 Nhận dữ liệu (nếu cần đọc tức thì)
        public string ReceiveData()
        {
            try
            {
                byte[] buffer = new byte[1024];
                if (stream == null) return string.Empty;
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

            while (isRunning)
            {
                string? input = Console.ReadLine();
                if (string.IsNullOrWhiteSpace(input))
                    continue;

                if (input.Trim().ToLower() == "exit")
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
