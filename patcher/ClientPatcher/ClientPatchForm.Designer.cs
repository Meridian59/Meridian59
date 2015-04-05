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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ClientPatchForm));
            this.pbProgress = new System.Windows.Forms.ProgressBar();
            this.btnPlay = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.ddlServer = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.btnPatch = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.gbOptions = new System.Windows.Forms.GroupBox();
            this.groupProfileSettings = new System.Windows.Forms.GroupBox();
            this.btnBrowse = new System.Windows.Forms.Button();
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
            this.txtLog = new System.Windows.Forms.TextBox();
            this.pbFileProgress = new System.Windows.Forms.ProgressBar();
            this.btnOptions = new System.Windows.Forms.Button();
            this.bgScanWorker = new System.ComponentModel.BackgroundWorker();
            this.bgDownloadWorker = new System.ComponentModel.BackgroundWorker();
            this.btnCacheGen = new System.Windows.Forms.Button();
            this.gbOptions.SuspendLayout();
            this.groupProfileSettings.SuspendLayout();
            this.SuspendLayout();
            // 
            // pbProgress
            // 
            this.pbProgress.Location = new System.Drawing.Point(7, 373);
            this.pbProgress.Name = "pbProgress";
            this.pbProgress.Size = new System.Drawing.Size(706, 32);
            this.pbProgress.Step = 1;
            this.pbProgress.TabIndex = 0;
            // 
            // btnPlay
            // 
            this.btnPlay.Location = new System.Drawing.Point(727, 339);
            this.btnPlay.Name = "btnPlay";
            this.btnPlay.Size = new System.Drawing.Size(95, 70);
            this.btnPlay.TabIndex = 1;
            this.btnPlay.Text = "Play!";
            this.btnPlay.UseVisualStyleBackColor = true;
            this.btnPlay.Click += new System.EventHandler(this.btnPlay_Click);
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
            this.ddlServer.Size = new System.Drawing.Size(277, 21);
            this.ddlServer.TabIndex = 4;
            this.ddlServer.SelectionChangeCommitted += new System.EventHandler(this.ddlServer_SelectionChangeCommitted);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(10, 9);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(192, 29);
            this.label2.TabIndex = 6;
            this.label2.Text = "Select a Profile";
            // 
            // btnPatch
            // 
            this.btnPatch.Location = new System.Drawing.Point(15, 134);
            this.btnPatch.Name = "btnPatch";
            this.btnPatch.Size = new System.Drawing.Size(279, 46);
            this.btnPatch.TabIndex = 7;
            this.btnPatch.Text = "Update/Install";
            this.btnPatch.UseVisualStyleBackColor = true;
            this.btnPatch.Click += new System.EventHandler(this.btnPatch_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(10, 102);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(174, 29);
            this.label3.TabIndex = 8;
            this.label3.Text = "Update/Install";
            // 
            // gbOptions
            // 
            this.gbOptions.Controls.Add(this.btnCacheGen);
            this.gbOptions.Controls.Add(this.groupProfileSettings);
            this.gbOptions.Controls.Add(this.btnStartModify);
            this.gbOptions.Controls.Add(this.btnRemove);
            this.gbOptions.Controls.Add(this.btnAdd);
            this.gbOptions.Location = new System.Drawing.Point(300, 12);
            this.gbOptions.Name = "gbOptions";
            this.gbOptions.Size = new System.Drawing.Size(522, 321);
            this.gbOptions.TabIndex = 10;
            this.gbOptions.TabStop = false;
            this.gbOptions.Text = "Options";
            this.gbOptions.Visible = false;
            // 
            // groupProfileSettings
            // 
            this.groupProfileSettings.Controls.Add(this.btnBrowse);
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
            this.groupProfileSettings.Size = new System.Drawing.Size(509, 200);
            this.groupProfileSettings.TabIndex = 3;
            this.groupProfileSettings.TabStop = false;
            this.groupProfileSettings.Text = "Profile Settings";
            // 
            // btnBrowse
            // 
            this.btnBrowse.Location = new System.Drawing.Point(406, 110);
            this.btnBrowse.Name = "btnBrowse";
            this.btnBrowse.Size = new System.Drawing.Size(97, 20);
            this.btnBrowse.TabIndex = 22;
            this.btnBrowse.Text = "Browse for Folder";
            this.btnBrowse.UseVisualStyleBackColor = true;
            this.btnBrowse.Click += new System.EventHandler(this.btnBrowse_Click);
            // 
            // btnSave
            // 
            this.btnSave.Location = new System.Drawing.Point(120, 175);
            this.btnSave.Name = "btnSave";
            this.btnSave.Size = new System.Drawing.Size(91, 19);
            this.btnSave.TabIndex = 21;
            this.btnSave.Text = "Save Profile";
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
            this.txtPatchBaseURL.Size = new System.Drawing.Size(400, 20);
            this.txtPatchBaseURL.TabIndex = 15;
            // 
            // txtClientFolder
            // 
            this.txtClientFolder.Location = new System.Drawing.Point(6, 110);
            this.txtClientFolder.Name = "txtClientFolder";
            this.txtClientFolder.Size = new System.Drawing.Size(400, 20);
            this.txtClientFolder.TabIndex = 14;
            // 
            // txtPatchInfoURL
            // 
            this.txtPatchInfoURL.Location = new System.Drawing.Point(6, 71);
            this.txtPatchInfoURL.Name = "txtPatchInfoURL";
            this.txtPatchInfoURL.Size = new System.Drawing.Size(400, 20);
            this.txtPatchInfoURL.TabIndex = 13;
            // 
            // txtServerName
            // 
            this.txtServerName.Location = new System.Drawing.Point(6, 32);
            this.txtServerName.Name = "txtServerName";
            this.txtServerName.Size = new System.Drawing.Size(400, 20);
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
            // txtLog
            // 
            this.txtLog.Location = new System.Drawing.Point(298, 13);
            this.txtLog.Multiline = true;
            this.txtLog.Name = "txtLog";
            this.txtLog.ReadOnly = true;
            this.txtLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtLog.Size = new System.Drawing.Size(524, 320);
            this.txtLog.TabIndex = 11;
            // 
            // pbFileProgress
            // 
            this.pbFileProgress.Location = new System.Drawing.Point(7, 339);
            this.pbFileProgress.Name = "pbFileProgress";
            this.pbFileProgress.Size = new System.Drawing.Size(706, 32);
            this.pbFileProgress.TabIndex = 12;
            this.pbFileProgress.Visible = false;
            // 
            // btnOptions
            // 
            this.btnOptions.Location = new System.Drawing.Point(7, 309);
            this.btnOptions.Name = "btnOptions";
            this.btnOptions.Size = new System.Drawing.Size(51, 23);
            this.btnOptions.TabIndex = 13;
            this.btnOptions.Text = "Options";
            this.btnOptions.UseVisualStyleBackColor = true;
            this.btnOptions.Click += new System.EventHandler(this.btnOptions_Click);
            // 
            // bgScanWorker
            // 
            this.bgScanWorker.DoWork += new System.ComponentModel.DoWorkEventHandler(this.bgScanWorker_DoWork);
            this.bgScanWorker.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.bgScanWorker_RunWorkerCompleted);
            // 
            // bgDownloadWorker
            // 
            this.bgDownloadWorker.DoWork += new System.ComponentModel.DoWorkEventHandler(this.bgDownloadWorker_DoWork);
            this.bgDownloadWorker.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.bgDownloadWorker_RunWorkerCompleted);
            // 
            // btnCacheGen
            // 
            this.btnCacheGen.Location = new System.Drawing.Point(127, 19);
            this.btnCacheGen.Name = "btnCacheGen";
            this.btnCacheGen.Size = new System.Drawing.Size(105, 23);
            this.btnCacheGen.TabIndex = 15;
            this.btnCacheGen.Text = "Verify All Files";
            this.btnCacheGen.UseVisualStyleBackColor = true;
            this.btnCacheGen.Click += new System.EventHandler(this.btnCacheGen_Click);
            // 
            // ClientPatchForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(834, 417);
            this.Controls.Add(this.btnOptions);
            this.Controls.Add(this.pbFileProgress);
            this.Controls.Add(this.gbOptions);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.btnPatch);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.ddlServer);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.btnPlay);
            this.Controls.Add(this.pbProgress);
            this.Controls.Add(this.txtLog);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "ClientPatchForm";
            this.Text = "OpenMeridian Client Patcher";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.gbOptions.ResumeLayout(false);
            this.groupProfileSettings.ResumeLayout(false);
            this.groupProfileSettings.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ProgressBar pbProgress;
        private System.Windows.Forms.Button btnPlay;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox ddlServer;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnPatch;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.GroupBox gbOptions;
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
        private System.Windows.Forms.Button btnBrowse;
        private System.Windows.Forms.TextBox txtLog;
        private System.Windows.Forms.ProgressBar pbFileProgress;
        private System.Windows.Forms.Button btnOptions;
        private System.ComponentModel.BackgroundWorker bgScanWorker;
        private System.ComponentModel.BackgroundWorker bgDownloadWorker;
        private System.Windows.Forms.Button btnCacheGen;
    }
}

