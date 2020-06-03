namespace tcpClient_test
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.connBtn = new System.Windows.Forms.Button();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.led1Btn = new System.Windows.Forms.Button();
            this.led2Btn = new System.Windows.Forms.Button();
            this.prgLbl = new System.Windows.Forms.Label();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.connTxt = new System.Windows.Forms.TextBox();
            this.trackBar1 = new System.Windows.Forms.TrackBar();
            this.trackLbl = new System.Windows.Forms.Label();
            this.prgLbl2 = new System.Windows.Forms.Label();
            this.progressBar2 = new System.Windows.Forms.ProgressBar();
            this.listBox1 = new System.Windows.Forms.ListBox();
            this.cnnStat = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.trackBar1)).BeginInit();
            this.SuspendLayout();
            // 
            // connBtn
            // 
            this.connBtn.Location = new System.Drawing.Point(224, 12);
            this.connBtn.Name = "connBtn";
            this.connBtn.Size = new System.Drawing.Size(75, 23);
            this.connBtn.TabIndex = 0;
            this.connBtn.Text = "BAĞLAN";
            this.connBtn.UseVisualStyleBackColor = true;
            this.connBtn.Click += new System.EventHandler(this.cnnBtn_Click);
            // 
            // timer1
            // 
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // led1Btn
            // 
            this.led1Btn.Location = new System.Drawing.Point(10, 51);
            this.led1Btn.Name = "led1Btn";
            this.led1Btn.Size = new System.Drawing.Size(75, 23);
            this.led1Btn.TabIndex = 1;
            this.led1Btn.Text = "LED1";
            this.led1Btn.UseVisualStyleBackColor = true;
            this.led1Btn.Click += new System.EventHandler(this.led1Btn_Click);
            // 
            // led2Btn
            // 
            this.led2Btn.Location = new System.Drawing.Point(224, 51);
            this.led2Btn.Name = "led2Btn";
            this.led2Btn.Size = new System.Drawing.Size(75, 23);
            this.led2Btn.TabIndex = 2;
            this.led2Btn.Text = "LED2";
            this.led2Btn.UseVisualStyleBackColor = true;
            this.led2Btn.Click += new System.EventHandler(this.led2Btn_Click);
            // 
            // prgLbl
            // 
            this.prgLbl.AutoSize = true;
            this.prgLbl.Location = new System.Drawing.Point(270, 140);
            this.prgLbl.Name = "prgLbl";
            this.prgLbl.Size = new System.Drawing.Size(16, 13);
            this.prgLbl.TabIndex = 4;
            this.prgLbl.Text = "---";
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(14, 134);
            this.progressBar1.Margin = new System.Windows.Forms.Padding(2);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(241, 19);
            this.progressBar1.TabIndex = 5;
            // 
            // connTxt
            // 
            this.connTxt.Location = new System.Drawing.Point(10, 15);
            this.connTxt.Margin = new System.Windows.Forms.Padding(2);
            this.connTxt.Name = "connTxt";
            this.connTxt.Size = new System.Drawing.Size(193, 20);
            this.connTxt.TabIndex = 6;
            // 
            // trackBar1
            // 
            this.trackBar1.Location = new System.Drawing.Point(11, 85);
            this.trackBar1.Margin = new System.Windows.Forms.Padding(2);
            this.trackBar1.Maximum = 100;
            this.trackBar1.Name = "trackBar1";
            this.trackBar1.Size = new System.Drawing.Size(244, 45);
            this.trackBar1.TabIndex = 7;
            this.trackBar1.MouseUp += new System.Windows.Forms.MouseEventHandler(this.trackBar1_MouseUp);
            // 
            // trackLbl
            // 
            this.trackLbl.AutoSize = true;
            this.trackLbl.Location = new System.Drawing.Point(261, 93);
            this.trackLbl.Name = "trackLbl";
            this.trackLbl.Size = new System.Drawing.Size(13, 13);
            this.trackLbl.TabIndex = 8;
            this.trackLbl.Text = "--";
            // 
            // prgLbl2
            // 
            this.prgLbl2.AutoSize = true;
            this.prgLbl2.Location = new System.Drawing.Point(270, 176);
            this.prgLbl2.Name = "prgLbl2";
            this.prgLbl2.Size = new System.Drawing.Size(16, 13);
            this.prgLbl2.TabIndex = 9;
            this.prgLbl2.Text = "---";
            // 
            // progressBar2
            // 
            this.progressBar2.Location = new System.Drawing.Point(14, 172);
            this.progressBar2.Margin = new System.Windows.Forms.Padding(2);
            this.progressBar2.Name = "progressBar2";
            this.progressBar2.Size = new System.Drawing.Size(241, 19);
            this.progressBar2.TabIndex = 10;
            // 
            // listBox1
            // 
            this.listBox1.FormattingEnabled = true;
            this.listBox1.Location = new System.Drawing.Point(14, 210);
            this.listBox1.Name = "listBox1";
            this.listBox1.Size = new System.Drawing.Size(285, 147);
            this.listBox1.TabIndex = 11;
            // 
            // cnnStat
            // 
            this.cnnStat.AutoSize = true;
            this.cnnStat.Location = new System.Drawing.Point(133, 56);
            this.cnnStat.Name = "cnnStat";
            this.cnnStat.Size = new System.Drawing.Size(57, 13);
            this.cnnStat.TabIndex = 12;
            this.cnnStat.Text = "Bağlı Değil";
            this.cnnStat.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(310, 365);
            this.Controls.Add(this.cnnStat);
            this.Controls.Add(this.listBox1);
            this.Controls.Add(this.progressBar2);
            this.Controls.Add(this.prgLbl2);
            this.Controls.Add(this.trackLbl);
            this.Controls.Add(this.trackBar1);
            this.Controls.Add(this.connTxt);
            this.Controls.Add(this.progressBar1);
            this.Controls.Add(this.prgLbl);
            this.Controls.Add(this.led2Btn);
            this.Controls.Add(this.led1Btn);
            this.Controls.Add(this.connBtn);
            this.Name = "Form1";
            this.Text = "Form1";
            ((System.ComponentModel.ISupportInitialize)(this.trackBar1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button connBtn;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.Button led1Btn;
        private System.Windows.Forms.Button led2Btn;
        private System.Windows.Forms.Label prgLbl;
        private System.Windows.Forms.ProgressBar progressBar1;
        private System.Windows.Forms.TextBox connTxt;
        private System.Windows.Forms.TrackBar trackBar1;
        private System.Windows.Forms.Label trackLbl;
        private System.Windows.Forms.Label prgLbl2;
        private System.Windows.Forms.ProgressBar progressBar2;
        private System.Windows.Forms.ListBox listBox1;
        private System.Windows.Forms.Label cnnStat;
    }
}

