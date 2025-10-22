using System;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;

namespace client_csharp
{
    [DebuggerDisplay($"{{{nameof(GetDebuggerDisplay)}(),nq}}")]
    public partial class GameForm : Form
    {
        private ClientSocket client;
        private Button[,] buttons = new Button[3, 3];
        private TextBox txtIp, txtPort, txtRoomId;
        private Button btnConnect, btnCreateRoom, btnJoinRoom, btnReset, btnExit;
        private Label lblStatus;

        private bool myTurn = false; // Đúng khi tới lượt mình
        private char myMark = 'X';   // Ký hiệu của mình ('X' hoặc 'O')

        // Constructor không tham số để khởi tạo từ Program.cs
        public GameForm() : this(null) { }

        public GameForm(ClientSocket clientSocket)
        {
            this.client = clientSocket;
            BuildUI();
            if (client != null)
                client.OnServerMessage += HandleServerMessage;
        }

        // ------------------ GIAO DIỆN ------------------
        private void BuildUI()
        {
            this.Text = "Cờ Caro 3x3 - Client";
            this.Size = new Size(420, 600);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.FormClosing += (s, e) => client?.Disconnect();

            // Nhập IP/port
            Label lblIp = new Label() { Text = "IP server:", Location = new Point(20, 15), AutoSize = true };
            txtIp = new TextBox() { Location = new Point(90, 12), Width = 100, Text = "127.0.0.1" };
            Label lblPort = new Label() { Text = "Port:", Location = new Point(200, 15), AutoSize = true };
            txtPort = new TextBox() { Location = new Point(250, 12), Width = 60, Text = "5000" };
            btnConnect = new Button() { Text = "Kết nối", Location = new Point(320, 10), Width = 70 };
            btnConnect.Click += BtnConnect_Click;

            // Tạo phòng / Vào phòng
            btnCreateRoom = new Button() { Text = "Tạo phòng", Location = new Point(20, 50), Width = 100, Enabled = false };
            btnCreateRoom.Click += BtnCreateRoom_Click;
            txtRoomId = new TextBox() { Location = new Point(140, 52), Width = 80, PlaceholderText = "Mã phòng" };
            btnJoinRoom = new Button() { Text = "Vào phòng", Location = new Point(240, 50), Width = 100, Enabled = false };
            btnJoinRoom.Click += BtnJoinRoom_Click;

            // Bàn cờ 3x3
            int startX = 60, startY = 110, size = 80;
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                {
                    buttons[i, j] = new Button();
                    buttons[i, j].SetBounds(startX + j * size, startY + i * size, size, size);
                    buttons[i, j].Font = new Font(FontFamily.GenericSansSerif, 24, FontStyle.Bold);
                    buttons[i, j].BackColor = Color.White;
                    buttons[i, j].Click += OnCellClick;
                    buttons[i, j].Enabled = false;
                    this.Controls.Add(buttons[i, j]);
                }

            // Nút Reset/Thoát
            btnReset = new Button() { Text = "Chơi lại", Location = new Point(60, 390), Width = 100, Enabled = false };
            btnReset.Click += (s, e) => { ResetBoard(); client?.SendData("RESET\n"); };
            btnExit = new Button() { Text = "Thoát", Location = new Point(200, 390), Width = 100 };
            btnExit.Click += (s, e) => { client?.Disconnect(); this.Close(); };

            // Label trạng thái
            lblStatus = new Label() { Text = "Trạng thái: Chưa kết nối", Location = new Point(20, 440), AutoSize = true };

            // Thêm control vào form
            this.Controls.Add(lblIp); this.Controls.Add(txtIp); this.Controls.Add(lblPort); this.Controls.Add(txtPort);
            this.Controls.Add(btnConnect); this.Controls.Add(btnCreateRoom); this.Controls.Add(txtRoomId); this.Controls.Add(btnJoinRoom);
            this.Controls.Add(btnReset); this.Controls.Add(btnExit); this.Controls.Add(lblStatus);
        }

        // ------------------ KẾT NỐI SERVER ------------------
        private void BtnConnect_Click(object sender, EventArgs e)
        {
            string ip = txtIp.Text.Trim();
            int port = int.TryParse(txtPort.Text.Trim(), out var p) ? p : 5000;
            client = new ClientSocket(ip, port);
            client.OnServerMessage += HandleServerMessage;
            client.ConnectToServer();
            lblStatus.Text = "Đã kết nối server, hãy tạo/vào phòng.";
            btnCreateRoom.Enabled = true;
            btnJoinRoom.Enabled = true;
        }

        // ------------------ TẠO PHÒNG ------------------
        private void BtnCreateRoom_Click(object sender, EventArgs e)
        {
            if (client == null) { MessageBox.Show("Chưa kết nối server!"); return; }
            client.SendData("CREATE_ROOM\n");
        }

        // ------------------ VÀO PHÒNG ------------------
        private void BtnJoinRoom_Click(object sender, EventArgs e)
        {
            if (client == null) { MessageBox.Show("Chưa kết nối server!"); return; }
            string roomId = txtRoomId.Text.Trim();
            if (string.IsNullOrEmpty(roomId)) { MessageBox.Show("Nhập mã phòng!"); return; }
            client.SendData($"JOIN_ROOM {roomId}\n");
        }

        // ------------------ ĐÁNH CỜ ------------------
        private void OnCellClick(object sender, EventArgs e)
        {
            if (!myTurn) { MessageBox.Show("⏳ Chưa tới lượt của bạn!"); return; }
            Button btn = sender as Button;
            if (btn == null || btn.Text != "") return;
            int row = -1, col = -1;
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    if (buttons[i, j] == btn) { row = i; col = j; break; }
            btn.Text = myMark.ToString();
            btn.Enabled = false;
            myTurn = false;
            client.SendData($"MOVE {row} {col}\n");
            lblStatus.Text = "Đã đánh nước đi, chờ đối thủ...";
        }

        // ------------------ XỬ LÝ DỮ LIỆU SERVER ------------------
        private void HandleServerMessage(string message)
        {
            if (this.InvokeRequired) { this.Invoke(new Action(() => HandleServerMessage(message))); return; }
            if (string.IsNullOrWhiteSpace(message)) return;

            if (message.StartsWith("ROOM_CREATED"))
            {
                var parts = message.Split(' ');
                if (parts.Length > 1)
                {
                    txtRoomId.Text = parts[1];
                    lblStatus.Text = $"Đã tạo phòng: {parts[1]}";
                }
            }
            else if (message.StartsWith("JOINED_OK"))
            {
                lblStatus.Text = "Đã vào phòng, đợi đối thủ...";
            }
            else if (message.StartsWith("GAME_START"))
            {
                // GAME_START X hoặc GAME_START O
                if (message.Trim().EndsWith("X")) { myMark = 'X'; }
                else if (message.Trim().EndsWith("O")) { myMark = 'O'; }
                else myMark = '?';
                lblStatus.Text = $"Game bắt đầu! Bạn là {myMark}.";
                ResetBoard();
                myTurn = false; // Mặc định chưa được đi, chỉ khi nhận YOUR_TURN mới cho đi
                UpdateBoardEnable();
            }
            else if (message.StartsWith("UPDATE_BOARD"))
            {
                var parts = message.Split(' ');
                if (parts.Length == 4 &&
                    int.TryParse(parts[1], out int x) &&
                    int.TryParse(parts[2], out int y))
                {
                    string ch = parts[3];
                    buttons[x, y].Text = ch;
                    buttons[x, y].Enabled = false;
                }
            }
            else if (message.Contains("YOUR_TURN"))
            {
                myTurn = true;
                UpdateBoardEnable();
                lblStatus.Text = "🎯 Đến lượt bạn!";
            }
            else if (message.StartsWith("GAME_OVER"))
            {
                if (message.Contains("WINNER"))
                {
                    char winner = message[message.Length - 1];
                    if (winner == myMark) lblStatus.Text = "🎉 Bạn đã thắng!";
                    else lblStatus.Text = $"Người thắng: {winner}";
                }
                else if (message.Contains("DRAW"))
                {
                    lblStatus.Text = "🤝 Hai bên hoà!";
                }
                myTurn = false;
                UpdateBoardEnable();
            }
            else if (message.Contains("OPPONENT_LEFT"))
            {
                lblStatus.Text = "⚠️ Đối thủ đã thoát!";
                myTurn = false;
                UpdateBoardEnable();
            }
            else if (message.StartsWith("RESET"))
            {
                ResetBoard();
                lblStatus.Text = "Bàn cờ đã reset.";
            }
            else
            {
                lblStatus.Text = message;
            }
        }

        private void UpdateBoardEnable()
        {
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    buttons[i, j].Enabled = (buttons[i, j].Text == "" && myTurn);
        }

        private void ResetBoard()
        {
            foreach (Button btn in buttons)
            {
                btn.Text = "";
                btn.BackColor = Color.White;
                btn.Enabled = false;
            }
            btnReset.Enabled = true;
            myTurn = false;
        }

        private string GetDebuggerDisplay()
        {
            return ToString();
        }
    }
}