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
        private bool isRunning = false;

        // Sự kiện gửi thông điệp đến Form (WinForms) hoặc Console
        public event Action<string>? OnServerMessage;

        // Lưu trạng thái bàn cờ (3x3) - có thể dùng cho giao diện
        public char[,] Board { get; } = new char[3, 3];

        // Trạng thái lượt đi của mình (chỉ dùng cho console)
        public bool IsMyTurn { get; private set; } = false;
        public char MyMark { get; private set; } = '?'; // 'X' hoặc 'O'
        public char OpponentMark { get; private set; } = '?';

        public ClientSocket(string ip, int port)
        {
            serverIp = ip;
            serverPort = port;
        }

        // Kết nối tới server
        public void ConnectToServer()
        {
            try
            {
                client = new TcpClient();
                client.Connect(serverIp, serverPort);
                stream = client.GetStream();
                isRunning = true;

                // Bắt đầu luồng nhận dữ liệu
                receiveThread = new Thread(ReceiveLoop)
                {
                    IsBackground = true
                };
                receiveThread.Start();

                OnServerMessage?.Invoke("CONNECTED");
            }
            catch (Exception ex)
            {
                OnServerMessage?.Invoke("ERROR Không thể kết nối: " + ex.Message);
            }
        }

        // Vòng lặp nhận dữ liệu
        private void ReceiveLoop()
        {
            var buffer = new byte[2048];
            StringBuilder leftover = new();
            try
            {
                while (isRunning)
                {
                    if (client == null || stream == null || !client.Connected) break;
                    int bytesRead = stream.Read(buffer, 0, buffer.Length);
                    if (bytesRead <= 0) break;
                    string data = leftover + Encoding.UTF8.GetString(buffer, 0, bytesRead);
                    int idx;
                    while ((idx = data.IndexOf('\n')) >= 0)
                    {
                        string msg = data[..idx].Trim('\r');
                        data = data[(idx + 1)..];
                        HandleServerMessage(msg);
                        OnServerMessage?.Invoke(msg);
                    }
                    leftover.Clear();
                    leftover.Append(data);
                }
            }
            catch (Exception ex)
            {
                OnServerMessage?.Invoke("ERROR Mất kết nối tới server: " + ex.Message);
            }
            finally
            {
                Disconnect();
            }
        }

        // Xử lý message từ server (cập nhật trạng thái, bàn cờ...)
        private void HandleServerMessage(string msg)
        {
            if (string.IsNullOrWhiteSpace(msg)) return;

            if (msg.StartsWith("UPDATE_BOARD"))
            {
                // UPDATE_BOARD x y X
                var parts = msg.Split(' ');
                if (parts.Length == 4 &&
                    int.TryParse(parts[1], out int x) &&
                    int.TryParse(parts[2], out int y))
                {
                    Board[x, y] = parts[3][0];
                }
            }
            else if (msg.StartsWith("GAME_START"))
            {
                // GAME_START X hoặc GAME_START O
                if (msg.Trim().EndsWith("X")) { MyMark = 'X'; OpponentMark = 'O'; }
                else if (msg.Trim().EndsWith("O")) { MyMark = 'O'; OpponentMark = 'X'; }
                else { MyMark = '?'; OpponentMark = '?'; }
                IsMyTurn = false; // Chờ đến khi nhận "YOUR_TURN" mới được đi
            }
            else if (msg.Contains("YOUR_TURN"))
            {
                IsMyTurn = true;
            }
            else if (msg.StartsWith("GAME_OVER"))
            {
                IsMyTurn = false;
            }
            else if (msg.Contains("OPPONENT_LEFT"))
            {
                IsMyTurn = false;
            }
        }

        // Gửi lệnh tới server
        public void SendData(string message)
        {
            try
            {
                if (client != null && stream != null && client.Connected)
                {
                    if (!message.EndsWith("\n")) message += "\n";
                    byte[] data = Encoding.UTF8.GetBytes(message);
                    stream.Write(data, 0, data.Length);
                }
                else
                {
                    OnServerMessage?.Invoke("ERROR Không thể gửi: chưa kết nối hoặc đã mất kết nối.");
                }
            }
            catch (Exception ex)
            {
                OnServerMessage?.Invoke("ERROR Lỗi gửi dữ liệu: " + ex.Message);
            }
        }

        // Bắt đầu vòng lặp game console (nếu cần)
        public void StartGameLoop()
        {
            Console.WriteLine("🎮 Bắt đầu chơi. Nhập lệnh (CREATE_ROOM, JOIN_ROOM <id>, MOVE x y, exit):");
            while (isRunning)
            {
                if (IsMyTurn)
                {
                    Console.Write("Nhập nước đi (x y): ");
                    string? input = Console.ReadLine();
                    if (string.IsNullOrWhiteSpace(input)) continue;
                    if (input.Trim().ToLower() == "exit")
                    {
                        Disconnect();
                        break;
                    }
                    SendData("MOVE " + input);
                    IsMyTurn = false;
                }
                else
                {
                    Thread.Sleep(100);
                }
            }
        }

        public void Disconnect()
        {
            isRunning = false;
            try
            {
                stream?.Close();
                client?.Close();
                OnServerMessage?.Invoke("DISCONNECTED");
            }
            catch { }
        }
    }
}