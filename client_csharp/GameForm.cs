// Code giao diện Windows Forms sẽ thêm ở đây

using System;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

namespace client_csharp
{
    [DebuggerDisplay($"{{{nameof(GetDebuggerDisplay)}(),nq}}")]
    public partial class GameForm : Form
    {
        private ClientSocket client;
        private Button[,] buttons = new Button[3, 3];
        private TextBox txtName;
        private ComboBox cbRoom;
        private Button btnReset, btnExit;
        private Label lblStatus;

        private bool myTurn = true; // Biến xác định lượt đi của người chơi

        public GameForm(ClientSocket clientSocket)
        {
            this.client = clientSocket;
           
            BuildUI();

            // 🔹 Lắng nghe phản hồi từ server (cập nhật UI khi có dữ liệu)
            client.OnServerMessage += HandleServerMessage;
        }

        // ------------------ GIAO DIỆN ------------------
        private void BuildUI()
        {
            this.Text = "Cờ Caro 3x3 - Client";
            this.Size = new Size(400, 520);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.FormClosing += (s, e) => client.Disconnect();

            // 🔹 Nhập tên người chơi
            Label lblName = new Label() { Text = "Tên người chơi:", Location = new Point(20, 20), AutoSize = true };
            txtName = new TextBox() { Location = new Point(140, 18), Width = 200, Text = "Player" };

            // 🔹 Chọn phòng (giả lập)
            Label lblRoom = new Label() { Text = "Phòng chơi:", Location = new Point(20, 55), AutoSize = true };
            cbRoom = new ComboBox() { Location = new Point(140, 50), Width = 200 };
            cbRoom.Items.AddRange(new string[] { "Phòng 1", "Phòng 2", "Phòng 3" });
            cbRoom.SelectedIndex = 0;

            // 🔹 Bàn cờ 3x3
            int startX = 60, startY = 100, size = 80;
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    buttons[i, j] = new Button();
                    buttons[i, j].SetBounds(startX + j * size, startY + i * size, size, size);
                    buttons[i, j].Font = new Font(FontFamily.GenericSansSerif, 20, FontStyle.Bold);
                    buttons[i, j].BackColor = Color.White;
                    buttons[i, j].Click += OnCellClick;
                    this.Controls.Add(buttons[i, j]);
                }
            }

            // 🔹 Nút Reset
            btnReset = new Button()
            {
                Text = "Chơi lại",
                Location = new Point(60, 370),
                Width = 100
            };
            btnReset.Click += (s, e) => ResetBoard();

            // 🔹 Nút Thoát
            btnExit = new Button()
            {
                Text = "Thoát",
                Location = new Point(200, 370),
                Width = 100
            };
            btnExit.Click += (s, e) => { client.Disconnect(); this.Close(); };

            // 🔹 Label hiển thị trạng thái
            lblStatus = new Label()
            {
                Text = "Trạng thái: Đang chờ...",
                Location = new Point(20, 420),
                AutoSize = true
            };

            // 🔹 Thêm tất cả vào form
            this.Controls.Add(lblName);
            this.Controls.Add(txtName);
            this.Controls.Add(lblRoom);
            this.Controls.Add(cbRoom);
            this.Controls.Add(btnReset);
            this.Controls.Add(btnExit);
            this.Controls.Add(lblStatus);
        }

        // ------------------ XỬ LÝ CLICK ------------------
        private void OnCellClick(object sender, EventArgs e)
        {
            if (!myTurn)
            {
                MessageBox.Show("⏳ Chưa tới lượt của bạn!");
                return;
            }

            Button btn = sender as Button;
            if (btn == null || btn.Text != "") return;

            // Đánh cờ
            btn.Text = "X";
            myTurn = false;
            lblStatus.Text = "Đã đánh nước đi, chờ đối thủ...";

            // Gửi nước đi lên server
            int row = -1, col = -1;
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    if (buttons[i, j] == btn)
                    {
                        row = i; col = j;
                        break;
                    }

            string move = $"MOVE {row} {col}\n";
            client.SendData(move);

        }

        // ------------------ XỬ LÝ DỮ LIỆU TỪ SERVER ------------------
        private void HandleServerMessage(string message)
        {
            if (this.InvokeRequired)
            {
                this.Invoke(new Action(() => HandleServerMessage(message)));
                return;
            }

            // 📩 Phản hồi khi đối thủ đi
            if (message.StartsWith("MOVE:"))
            {
                string[] parts = message.Replace("MOVE:", "").Split(',');
                string player = parts[0];
                int row = int.Parse(parts[1]);
                int col = int.Parse(parts[2]);

                buttons[row, col].Text = player;
                lblStatus.Text = $"Đối thủ ({player}) đã đi ô ({row},{col})";
                myTurn = true; // Giờ tới lượt mình
            }
            // 🏆 Khi có người thắng
            else if (message.StartsWith("WIN:"))
            {
                // WIN:X,0,0;0,1;0,2
                string[] parts = message.Replace("WIN:", "").Split(',');
                string player = parts[0];
                string[] cells = parts[1].Split(';');

                foreach (string cell in cells)
                {
                    string[] xy = cell.Split(',');
                    int r = int.Parse(xy[0]);
                    int c = int.Parse(xy[1]);
                    buttons[r, c].BackColor = Color.Yellow;
                }

                lblStatus.Text = $"🎉 Người chơi {player} thắng!";
                myTurn = false;
            }
            // 🔄 Khi server yêu cầu reset
            else if (message.StartsWith("RESET"))
            {
                ResetBoard();
                lblStatus.Text = "Bàn cờ đã được reset.";
            }
            // ℹ️ Các thông báo khác
            else
            {
                lblStatus.Text = message;
            }
        }

        // ------------------ RESET BÀN CỜ ------------------
        private void ResetBoard()
        {
            foreach (Button btn in buttons)
            {
                btn.Text = "";
                btn.BackColor = Color.White;
            }
            myTurn = true;
            lblStatus.Text = "Bàn cờ đã được reset, tới lượt bạn!";
        }

        private string GetDebuggerDisplay()
        {
            return ToString();
        }
    }
}
