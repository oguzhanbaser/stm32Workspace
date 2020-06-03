using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Net.Sockets;
using System.IO;

namespace tcpClient_test
{
    public partial class Form1 : Form
    {

        String ipStr, portStr;
        bool led1Toggle = false, led2Toggle = false, cnnToggle = false;

        public Form1()
        {
            InitializeComponent();
            timer1.Interval = 1000;
            connTxt.Text = "192.168.137.120:61";
        }

        private void cnnBtn_Click(object sender, EventArgs e)
        {
            if(!cnnToggle)
            {
                timer1.Start();             // start data request with interval
                cnnToggle = true;
                cnnStat.Text = "Bağlı";
            }
            else
            {
                timer1.Stop();             // start data request with interval
                cnnToggle = false;
                cnnStat.Text = "Bağlı Değil";
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            sendTcpData(0x10, 0x00);        // send data which ID is 0x10 to server and request ADC data
        }

        // send data to tcp server with ID and data
        private void sendTcpData(byte pId, byte pVal)
        {
            TcpClient tcpclnt = new TcpClient();                // create TCP client 

            // try to connect server
            try
            {
                // split ip str and port from string
                String []connVal = connTxt.Text.Split(':');
                ipStr = connVal[0];
                portStr = connVal[1];

                tcpclnt.ConnectAsync(ipStr, Convert.ToInt32(portStr)).Wait(1000);

                Stream stm = tcpclnt.GetStream();

                byte[] send_bytes = { 0xA5, 0x5A, pId, pVal };

                stm.Write(send_bytes, 0, send_bytes.Length);        // send data to tcp server

                byte[] recv_bytes = new byte[100];  
                int k = stm.Read(recv_bytes, 0, 100);               // recv data from tcp server

                // check length and first two bytes of recevived data 
                if (recv_bytes.Length > 2 && (recv_bytes[0] == 0xA5 && recv_bytes[1] == 0x5A))
                {
                    // if third data is 0x30 show it in gui
                    if (recv_bytes[2] == 0x30)
                    {
                        prgLbl.Text = Convert.ToString(recv_bytes[3]);
                        progressBar1.Value = recv_bytes[3];

                        prgLbl2.Text = Convert.ToString(recv_bytes[4]);
                        progressBar2.Value = recv_bytes[4];
                    }
                }

                // show all received data in listbox
                String recStr = ByteArrayToString(recv_bytes, k);
                listBox1.Items.Add(recStr);
                listBox1.SelectedIndex = listBox1.SelectedIndex + 1;

                // close connection after sending and receiving data
                tcpclnt.Close();
            }catch(Exception ex)
            {
                timer1.Stop();
                cnnStat.Text = "Bağlı Değil";
                MessageBox.Show("Bağlantı Zaman Aşımına Uğradı", "Bağlantı Hatası");
            }
        }

        // convert byte array to hex string
        public static string ByteArrayToString(byte[] ba, int pLen)
        {
            StringBuilder hex = new StringBuilder(pLen * 2);

            for(int i = 0; i < pLen; i++)
            {
                hex.AppendFormat("{0:x2} ", ba[i]);
            }
            return hex.ToString();
        }

        // send trackbar value to tcp server when mouseup event triggered
        private void trackBar1_MouseUp(object sender, MouseEventArgs e)
        {
            trackLbl.Text = trackBar1.Value.ToString();
            sendTcpData(0x30, Convert.ToByte(trackBar1.Value));
        }

        // send led1 toggle cmd with buttton1
        private void led1Btn_Click(object sender, EventArgs e)
        {
            led1Toggle = !led1Toggle;
            sendTcpData(0x01, Convert.ToByte(led1Toggle));
        }

        // send led2 toggle cmd with buttton2
        private void led2Btn_Click(object sender, EventArgs e)
        {
            led2Toggle = !led2Toggle;
            sendTcpData(0x02, Convert.ToByte(led2Toggle));
        }
    }
}
