using System;
using System.Windows.Forms;

namespace client_csharp
{
    static class Program
    {
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            // Nếu GameForm nhận ClientSocket, hãy sửa lại cho phù hợp
            Application.Run(new GameForm());
        }
    }
}