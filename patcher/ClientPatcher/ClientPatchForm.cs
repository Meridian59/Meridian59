using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net;
using Newtonsoft.Json;
using PatchListGenerator;
using System.IO;
using System.Diagnostics;

namespace ClientPatcher
{
    enum ChangeType
    {
        NONE = 0,
        ADD_PROFILE = 1,
        MOD_PROFILE = 2
    }

    public partial class ClientPatchForm : Form
    {

        SettingsManager Settings;
        ClientPatcher Patcher;

        ChangeType changetype = ChangeType.NONE;


        public ClientPatchForm()
        {
            InitializeComponent();
        }
        private void Form1_Load(object sender, EventArgs e)
        {
            btnPlay.Enabled = false;

            Settings = new SettingsManager();
            Settings.LoadSettings();

            Patcher = new ClientPatcher(Settings.GetDefault());
            Patcher.FileScanned += new ScanFileEventHandler(Patcher_FileScanned);
            Patcher.StartedDownload += new StartDownloadEventHandler(Patcher_StartedDownload);
            Patcher.ProgressedDownload += new ProgressDownloadEventHandler(Patcher_ProgressedDownload);
            RefreshDDL();
        }

        private void btnPlay_Click(object sender, EventArgs e)
        {
            ProcessStartInfo meridian = new ProcessStartInfo();
            meridian.FileName = Patcher.CurrentProfile.ClientFolder + "\\meridian.exe";
            meridian.WorkingDirectory = Patcher.CurrentProfile.ClientFolder + "\\";
            Process.Start(meridian);
            Application.Exit();
        }
        private void ddlServer_SelectionChangeCommitted(object sender, EventArgs e)
        {
            //lblStatus.Text = ddlServer.SelectedItem.ToString();
            PatcherSettings selected = Settings.FindByName(ddlServer.SelectedItem.ToString());
            if (selected != null)
            {
                Patcher.CurrentProfile = selected;
                txtLog.Text += String.Format("Server {0} selected. Client located at: {1}\r\n", selected.ServerName, selected.ClientFolder);
                btnPlay.Enabled = false;
                if (groupProfileSettings.Enabled == true)
                {
                    groupProfileSettings.Enabled = false;
                    txtClientFolder.Text = "";
                    txtPatchBaseURL.Text = "";
                    txtPatchInfoURL.Text = "";
                    txtServerName.Text = "";
                    cbDefaultServer.Checked = false;
                }
            }
        }
        private void btnPatch_Click(object sender, EventArgs e)
        {
            PreScan();
        }
        private void btnAdd_Click(object sender, EventArgs e)
        {
            groupProfileSettings.Enabled = true;
            changetype = ChangeType.ADD_PROFILE;
        }
        private void btnStartModify_Click(object sender, EventArgs e)
        {
            groupProfileSettings.Enabled = true;
            PatcherSettings ps = Settings.FindByName(ddlServer.SelectedItem.ToString());
            txtClientFolder.Text = ps.ClientFolder;
            txtPatchBaseURL.Text = ps.PatchBaseURL;
            txtPatchInfoURL.Text = ps.PatchInfoURL;
            txtServerName.Text = ps.ServerName;
            cbDefaultServer.Checked = ps.Default;
            changetype = ChangeType.MOD_PROFILE;
        }
        private void btnSave_Click(object sender, EventArgs e)
        {
            switch (changetype)
            {
                case ChangeType.ADD_PROFILE:
                    Settings.AddProfile(txtClientFolder.Text, txtPatchBaseURL.Text, txtPatchInfoURL.Text, txtServerName.Text, cbDefaultServer.Checked);
                    groupProfileSettings.Enabled = false;
                    break;
                case ChangeType.MOD_PROFILE:
                    ModProfile();
                    groupProfileSettings.Enabled = false;
                    break;
                default:
                    break;
            }
        }
        private void btnRemove_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("Are you sure you want to delete this Profile?", "Delete Profile?", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
            {
                int selected = Settings.Servers.FindIndex(x => x.ServerName == ddlServer.SelectedItem.ToString());
                Settings.Servers.RemoveAt(selected);
                Settings.SaveSettings();
                Settings.LoadSettings();
            }
        }
        private void btnBrowse_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            fbd.ShowDialog(this);
            txtClientFolder.Text = fbd.SelectedPath;
        }
        private void btnOptions_Click(object sender, EventArgs e)
        {
            gbOptions.Visible = !gbOptions.Visible;
        }

        private void Patcher_FileScanned(object sender, ScanEventArgs e)
        {
            pbProgressPerformStep();
            txtLogAppendText(String.Format("Scanning Files.... {0}\r\n", e.Filename));
        }
        private void Patcher_StartedDownload(object sender, StartDownloadEventArgs e)
        {
            pbProgressPerformStep();
            txtLogAppendText(String.Format("Downloading File..... {0} ({1})\r\n", e.Filename, e.Filesize.ToString()));
        }
        private void Patcher_ProgressedDownload(object sender, DownloadProgressChangedEventArgs e)
        {
            pbFileProgress.Maximum = 100;
            pbFileProgress.Step = 1;
            pbFileProgress.Value = e.ProgressPercentage;
            pbFileProgress.Update();
        }

        private void ModProfile()
        {
            int selected = Settings.Servers.FindIndex(x => x.ServerName == ddlServer.SelectedItem.ToString());
            Settings.Servers[selected].ClientFolder = txtClientFolder.Text;
            Settings.Servers[selected].PatchBaseURL = txtPatchBaseURL.Text;
            Settings.Servers[selected].PatchInfoURL = txtPatchInfoURL.Text;
            Settings.Servers[selected].ServerName = txtServerName.Text;
            Settings.Servers[selected].Default = cbDefaultServer.Checked;
            changetype = ChangeType.NONE;
            Settings.SaveSettings();
            Settings.LoadSettings();
        }
        private void RefreshDDL()
        {
            foreach (PatcherSettings profile in Settings.Servers)
            {
                ddlServer.Items.Add(profile.ServerName);
                if (profile.Default)
                    ddlServer.SelectedItem = profile.ServerName;
            }
        }
        private void PreScan()
        {
            btnPatch.Enabled = false;
            ddlServer.Enabled = false;
            txtLog.AppendText("Downloading Patch Information....\r\n");
            if (Patcher.DownloadJson() == 1)
            {
                pbProgress.Value = 0;
                pbProgress.Maximum = Patcher.PatchFiles.Count;
                //Patcher.ScanClient();
                bgScanWorker.RunWorkerAsync(Patcher);
            }
            else
                txtLog.AppendText("ERROR: Unable to download Patch Information!\r\n");
        }
        private void PostScan()
        {
            if (Patcher.LocalFiles.Count > 0)
            {
                pbProgress.Value = 0;
                pbProgress.Maximum = Patcher.LocalFiles.Count;
                bgDownloadWorker.RunWorkerAsync(Patcher);
                //Patcher.DownloadFiles();
                //Patcher.DownloadFilesAsync();
            }
            else
                PostDownload();
        }
        private void PostDownload()
        {
            pbProgress.Value = pbProgress.Maximum;
            txtLog.AppendText("Patching Complete!\r\n");
            btnPlay.Enabled = true;
        }
        

        #region bgScanWorker
        private void bgScanWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            ClientPatcher myPatcher = (ClientPatcher)e.Argument;
            myPatcher.ScanClient();
        }
        private void bgScanWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            PostScan();
        }
        #endregion

        #region ThreadSafe Control Updates
        //Used to update progress bar
        delegate void ProgressPerformStepCallback();
        private void pbProgressPerformStep()
        {
            if (this.pbProgress.InvokeRequired)
            {
                ProgressPerformStepCallback d = new ProgressPerformStepCallback(pbProgressPerformStep);
                this.Invoke(d);
            }
            else
            {
                this.pbProgress.PerformStep();
            }
        }
        //Used to add stuff to the Log
        delegate void txtLogAppendTextCalback(string text);
        private void txtLogAppendText(string text)
        {
            if (this.txtLog.InvokeRequired)
            {
                txtLogAppendTextCalback d = new txtLogAppendTextCalback(txtLogAppendText);
                this.Invoke(d, new object[] { text });
            }
            else
            {
                this.txtLog.AppendText(text);
            }
        }
        #endregion

        #region bgDownloadWorker
        private void bgDownloadWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            ClientPatcher myPatcher = (ClientPatcher)e.Argument;
            myPatcher.DownloadFiles();
        }
        private void bgDownloadWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            PostDownload();
        }
        #endregion


    }
}
