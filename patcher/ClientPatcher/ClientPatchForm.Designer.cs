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
            this.label2 = new System.Windows.Forms.Label();
            this.btnPatch = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupProfileSettings = new System.Windows.Forms.GroupBox();
            this.button1 = new System.Windows.Forms.Button();
            this.btnSave = new System.Windows.Forms.Button();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.cbDefaultServer = new System.Windows.Forms.CheckBox();
            this.txtPatchBaseURL = new System.Windows.Forms.TextBox();
            this.txtClientFolder = new System.Windows.Forms.TextBox();
            this.txtPatchInfoURL = new System.Windows.Forms.TextBox();
            this.txtServerName = new System.Windows.Forms.TextBox();
            this.btnStartModify = new System.Windows.Forms.Button();
            this.btnRemove = new System.Windows.Forms.Button();
            this.btnAdd = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.groupProfileSettings.SuspendLayout();
            this.SuspendLayout();
            // 
            // pbProgress
            // 
            this.pbProgress.Location = new System.Drawing.Point(15, 186);
            this.pbProgress.Name = "pbProgress";
            this.pbProgress.Size = new System.Drawing.Size(279, 36);
            this.pbProgress.Step = 1;
            this.pbProgress.TabIndex = 0;
            // 
            // btnPlay
            // 
            this.btnPlay.Location = new System.Drawing.Point(12, 277);
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
            this.lblStatus.Location = new System.Drawing.Point(12, 340);
            this.lblStatus.Name = "lblStatus";
            this.lblStatus.Size = new System.Drawing.Size(37, 13);
            this.lblStatus.TabIndex = 2;
            this.lblStatus.Text = "Status";
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
            this.btnPatch.Location = new System.Drawing.Point(15, 134);
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
            this.label3.Location = new System.Drawing.Point(10, 102);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(242, 29);
            this.label3.TabIndex = 8;
            this.label3.Text = "Step 2: Patch Client";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(10, 235);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(159, 29);
            this.label4.TabIndex = 9;
            this.label4.Text = "Step 3: Play!";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.groupProfileSettings);
            this.groupBox1.Controls.Add(this.btnStartModify);
            this.groupBox1.Controls.Add(this.btnRemove);
            this.groupBox1.Controls.Add(this.btnAdd);
            this.groupBox1.Location = new System.Drawing.Point(308, 9);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(194, 321);
            this.groupBox1.TabIndex = 10;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Options";
            // 
            // groupProfileSettings
            // 
            this.groupProfileSettings.Controls.Add(this.button1);
            this.groupProfileSettings.Controls.Add(this.btnSave);
            this.groupProfileSettings.Controls.Add(this.label8);
            this.groupProfileSettings.Controls.Add(this.label7);
            this.groupProfileSettings.Controls.Add(this.label6);
            this.groupProfileSettings.Controls.Add(this.label5);
            this.groupProfileSettings.Controls.Add(this.cbDefaultServer);
            this.groupProfileSettings.Controls.Add(this.txtPatchBaseURL);
            this.groupProfileSettings.Controls.Add(this.txtClientFolder);
            this.groupProfileSettings.Controls.Add(this.txtPatchInfoURL);
            this.groupProfileSettings.Controls.Add(this.txtServerName);
            this.groupProfileSettings.Enabled = false;
            this.groupProfileSettings.Location = new System.Drawing.Point(7, 109);
            this.groupProfileSettings.Name = "groupProfileSettings";
            this.groupProfileSettings.Size = new System.Drawing.Size(181, 200);
            this.groupProfileSettings.TabIndex = 3;
            this.groupProfileSettings.TabStop = false;
            this.groupProfileSettings.Text = "Profile Settings";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(120, 110);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(56, 20);
            this.button1.TabIndex = 22;
            this.button1.Text = "Browse";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // btnSave
            // 
            this.btnSave.Location = new System.Drawing.Point(120, 173);
            this.btnSave.Name = "btnSave";
            this.btnSave.Size = new System.Drawing.Size(56, 19);
            this.btnSave.TabIndex = 21;
            this.btnSave.Text = "Save";
            this.btnSave.UseVisualStyleBackColor = true;
            this.btnSave.Click += new System.EventHandler(this.btnSave_Click);
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(11, 133);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(81, 13);
            this.label8.TabIndex = 20;
            this.label8.Text = "PatchBaseURL";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(11, 94);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(62, 13);
            this.label7.TabIndex = 19;
            this.label7.Text = "ClientFolder";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(11, 55);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(75, 13);
            this.label6.TabIndex = 18;
            this.label6.Text = "PatchInfoURL";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(11, 16);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(67, 13);
            this.label5.TabIndex = 17;
            this.label5.Text = "Profile Name";
            // 
            // cbDefaultServer
            // 
            this.cbDefaultServer.AutoSize = true;
            this.cbDefaultServer.Location = new System.Drawing.Point(14, 175);
            this.cbDefaultServer.Name = "cbDefaultServer";
            this.cbDefaultServer.Size = new System.Drawing.Size(100, 17);
            this.cbDefaultServer.TabIndex = 16;
            this.cbDefaultServer.Text = "Default Server?";
            this.cbDefaultServer.UseVisualStyleBackColor = true;
            // 
            // txtPatchBaseURL
            // 
            this.txtPatchBaseURL.Location = new System.Drawing.Point(6, 149);
            this.txtPatchBaseURL.Name = "txtPatchBaseURL";
            this.txtPatchBaseURL.Size = new System.Drawing.Size(108, 20);
            this.txtPatchBaseURL.TabIndex = 15;
            // 
            // txtClientFolder
            // 
            this.txtClientFolder.Location = new System.Drawing.Point(6, 110);
            this.txtClientFolder.Name = "txtClientFolder";
            this.txtClientFolder.Size = new System.Drawing.Size(108, 20);
            this.txtClientFolder.TabIndex = 14;
            // 
            // txtPatchInfoURL
            // 
            this.txtPatchInfoURL.Location = new System.Drawing.Point(6, 71);
            this.txtPatchInfoURL.Name = "txtPatchInfoURL";
            this.txtPatchInfoURL.Size = new System.Drawing.Size(108, 20);
            this.txtPatchInfoURL.TabIndex = 13;
            // 
            // txtServerName
            // 
            this.txtServerName.Location = new System.Drawing.Point(6, 32);
            this.txtServerName.Name = "txtServerName";
            this.txtServerName.Size = new System.Drawing.Size(108, 20);
            this.txtServerName.TabIndex = 12;
            // 
            // btnStartModify
            // 
            this.btnStartModify.Location = new System.Drawing.Point(6, 79);
            this.btnStartModify.Name = "btnStartModify";
            this.btnStartModify.Size = new System.Drawing.Size(108, 23);
            this.btnStartModify.TabIndex = 2;
            this.btnStartModify.Text = "Modify Profile";
            this.btnStartModify.UseVisualStyleBackColor = true;
            this.btnStartModify.Click += new System.EventHandler(this.btnStartModify_Click);
            // 
            // btnRemove
            // 
            this.btnRemove.Location = new System.Drawing.Point(6, 48);
            this.btnRemove.Name = "btnRemove";
            this.btnRemove.Size = new System.Drawing.Size(108, 23);
            this.btnRemove.TabIndex = 1;
            this.btnRemove.Text = "Remove Profile";
            this.btnRemove.UseVisualStyleBackColor = true;
            this.btnRemove.Click += new System.EventHandler(this.btnRemove_Click);
            // 
            // btnAdd
            // 
            this.btnAdd.Location = new System.Drawing.Point(6, 19);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(108, 23);
            this.btnAdd.TabIndex = 0;
            this.btnAdd.Text = "Add New Profile";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // ClientPatchForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(506, 374);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.btnPatch);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.ddlServer);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.lblStatus);
            this.Controls.Add(this.btnPlay);
            this.Controls.Add(this.pbProgress);
            this.Name = "ClientPatchForm";
            this.Text = "OpenMeridian Client Patcher";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupProfileSettings.ResumeLayout(false);
            this.groupProfileSettings.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ProgressBar pbProgress;
        private System.Windows.Forms.Button btnPlay;
        private System.Windows.Forms.Label lblStatus;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox ddlServer;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnPatch;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnRemove;
        private System.Windows.Forms.Button btnAdd;
        private System.Windows.Forms.Button btnStartModify;
        private System.Windows.Forms.GroupBox groupProfileSettings;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.CheckBox cbDefaultServer;
        private System.Windows.Forms.TextBox txtPatchBaseURL;
        private System.Windows.Forms.TextBox txtClientFolder;
        private System.Windows.Forms.TextBox txtPatchInfoURL;
        private System.Windows.Forms.TextBox txtServerName;
        private System.Windows.Forms.Button btnSave;
        private System.Windows.Forms.Button button1;
    }
}

