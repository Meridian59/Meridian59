namespace ClientPatcher
{
    partial class ClientPatchForm
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
            this.pbProgress = new System.Windows.Forms.ProgressBar();
            this.btnPlay = new System.Windows.Forms.Button();
            this.lblStatus = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.ddlServer = new System.Windows.Forms.ComboBox();
            this.btnRetry = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.btnPatch = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // pbProgress
            // 
            this.pbProgress.Location = new System.Drawing.Point(12, 362);
            this.pbProgress.Name = "pbProgress";
            this.pbProgress.Size = new System.Drawing.Size(623, 36);
            this.pbProgress.Step = 1;
            this.pbProgress.TabIndex = 0;
            // 
            // btnPlay
            // 
            this.btnPlay.Location = new System.Drawing.Point(15, 194);
            this.btnPlay.Name = "btnPlay";
            this.btnPlay.Size = new System.Drawing.Size(279, 46);
            this.btnPlay.TabIndex = 1;
            this.btnPlay.Text = "Play!";
            this.btnPlay.UseVisualStyleBackColor = true;
            this.btnPlay.Click += new System.EventHandler(this.btnPlay_Click);
            // 
            // lblStatus
            // 
            this.lblStatus.AutoSize = true;
            this.lblStatus.Location = new System.Drawing.Point(12, 333);
            this.lblStatus.Name = "lblStatus";
            this.lblStatus.Size = new System.Drawing.Size(35, 13);
            this.lblStatus.TabIndex = 2;
            this.lblStatus.Text = "label1";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 41);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(85, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "Server Selection";
            // 
            // ddlServer
            // 
            this.ddlServer.FormattingEnabled = true;
            this.ddlServer.Location = new System.Drawing.Point(15, 57);
            this.ddlServer.Name = "ddlServer";
            this.ddlServer.Size = new System.Drawing.Size(276, 21);
            this.ddlServer.TabIndex = 4;
            this.ddlServer.SelectionChangeCommitted += new System.EventHandler(this.ddlServer_SelectionChangeCommitted);
            // 
            // btnRetry
            // 
            this.btnRetry.Location = new System.Drawing.Point(595, 225);
            this.btnRetry.Name = "btnRetry";
            this.btnRetry.Size = new System.Drawing.Size(75, 65);
            this.btnRetry.TabIndex = 5;
            this.btnRetry.Text = "Retry";
            this.btnRetry.UseVisualStyleBackColor = true;
            this.btnRetry.Visible = false;
            this.btnRetry.Click += new System.EventHandler(this.btnRetry_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(10, 9);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(281, 29);
            this.label2.TabIndex = 6;
            this.label2.Text = "Step 1: Select a Server";
            // 
            // btnPatch
            // 
            this.btnPatch.Location = new System.Drawing.Point(15, 113);
            this.btnPatch.Name = "btnPatch";
            this.btnPatch.Size = new System.Drawing.Size(279, 46);
            this.btnPatch.TabIndex = 7;
            this.btnPatch.Text = "Patch";
            this.btnPatch.UseVisualStyleBackColor = true;
            this.btnPatch.Click += new System.EventHandler(this.btnPatch_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(10, 81);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(242, 29);
            this.label3.TabIndex = 8;
            this.label3.Text = "Step 2: Patch Client";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(10, 162);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(159, 29);
            this.label4.TabIndex = 9;
            this.label4.Text = "Step 3: Play!";
            // 
            // ClientPatchForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(729, 416);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.btnPatch);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.btnRetry);
            this.Controls.Add(this.ddlServer);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.lblStatus);
            this.Controls.Add(this.btnPlay);
            this.Controls.Add(this.pbProgress);
            this.Name = "ClientPatchForm";
            this.Text = "OpenMeridian Client Patcher";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ProgressBar pbProgress;
        private System.Windows.Forms.Button btnPlay;
        private System.Windows.Forms.Label lblStatus;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox ddlServer;
        private System.Windows.Forms.Button btnRetry;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnPatch;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
    }
}

