using System;
using System.ComponentModel;
using System.Globalization;
using System.Windows.Forms;
using System.Net;
using System.Diagnostics;

namespace ClientPatcher
{
    enum ChangeType
    {
        None = 0,
        AddProfile = 1,
        ModProfile = 2
    }

    public partial class ClientPatchForm : Form
    {

        SettingsManager _settings;
        ClientPatcher _patcher;
        ChangeType _changetype = ChangeType.None;


        public ClientPatchForm()
        {
            InitializeComponent();
        }
        private void Form1_Load(object sender, EventArgs e)
        {
            btnPlay.Enabled = false;

            _settings = new SettingsManager();
            _settings.LoadSettings();

            _patcher = new ClientPatcher(_settings.GetDefault());
            _patcher.FileScanned += Patcher_FileScanned;
            _patcher.StartedDownload += Patcher_StartedDownload;
            _patcher.ProgressedDownload += Patcher_ProgressedDownload;
            _patcher.EndedDownload += Patcher_EndedDownload;

            RefreshDdl();
        }

        private void btnPlay_Click(object sender, EventArgs e)
        {
            ProcessStartInfo meridian = new ProcessStartInfo();
            meridian.FileName = _patcher.CurrentProfile.ClientFolder + "\\meridian.exe";
            meridian.WorkingDirectory = _patcher.CurrentProfile.ClientFolder + "\\";
            Process.Start(meridian);
            Application.Exit();
        }
        private void ddlServer_SelectionChangeCommitted(object sender, EventArgs e)
        {
            //lblStatus.Text = ddlServer.SelectedItem.ToString();
            PatcherSettings selected = _settings.FindByName(ddlServer.SelectedItem.ToString());

            if (selected == null) return;
            _patcher.CurrentProfile = selected;
            txtLog.Text += String.Format("Server {0} selected. Client located at: {1}\r\n", selected.ServerName, selected.ClientFolder);
            btnPlay.Enabled = false;

            if (groupProfileSettings.Enabled != true) return;
            groupProfileSettings.Enabled = false;
            txtClientFolder.Text = "";
            txtPatchBaseURL.Text = "";
            txtPatchInfoURL.Text = "";
            txtServerName.Text = "";
            cbDefaultServer.Checked = false;
        }
        private void btnPatch_Click(object sender, EventArgs e)
        {
            PreScan();
        }
        private void btnAdd_Click(object sender, EventArgs e)
        {
            groupProfileSettings.Enabled = true;
            _changetype = ChangeType.AddProfile;
        }
        private void btnStartModify_Click(object sender, EventArgs e)
        {
            groupProfileSettings.Enabled = true;
            PatcherSettings ps = _settings.FindByName(ddlServer.SelectedItem.ToString());
            txtClientFolder.Text = ps.ClientFolder;
            txtPatchBaseURL.Text = ps.PatchBaseUrl;
            txtPatchInfoURL.Text = ps.PatchInfoUrl;
            txtServerName.Text = ps.ServerName;
            cbDefaultServer.Checked = ps.Default;
            _changetype = ChangeType.ModProfile;
        }
        private void btnSave_Click(object sender, EventArgs e)
        {
            switch (_changetype)
            {
                case ChangeType.AddProfile:
                    _settings.AddProfile(txtClientFolder.Text, txtPatchBaseURL.Text, txtPatchInfoURL.Text, txtServerName.Text, cbDefaultServer.Checked);
                    groupProfileSettings.Enabled = false;
                    break;
                case ChangeType.ModProfile:
                    ModProfile();
                    groupProfileSettings.Enabled = false;
                    break;
            }
        }
        private void btnRemove_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("Are you sure you want to delete this Profile?", "Delete Profile?", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
            {
                int selected = _settings.Servers.FindIndex(x => x.ServerName == ddlServer.SelectedItem.ToString());
                _settings.Servers.RemoveAt(selected);
                _settings.SaveSettings();
                _settings.LoadSettings();
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
            PbProgressPerformStep();
            TxtLogAppendText(String.Format("Scanning Files.... {0}\r\n", e.Filename));
        }
        private void Patcher_StartedDownload(object sender, StartDownloadEventArgs e)
        {
            PbProgressPerformStep();
            TxtLogAppendText(String.Format("Downloading File..... {0} ({1})\r\n", e.Filename, e.Filesize.ToString(CultureInfo.InvariantCulture)));
            pbFileProgress.Maximum = 100;
            PbFileProgressSetValueStep(0);
        }
        private void Patcher_ProgressedDownload(object sender, DownloadProgressChangedEventArgs e)
        {
            PbFileProgressSetValueStep(e.ProgressPercentage);
        }
        private void Patcher_EndedDownload(object sender, AsyncCompletedEventArgs e)
        {
            PbFileProgressSetValueStep(100);
        }

        private void ModProfile()
        {
            int selected = _settings.Servers.FindIndex(x => x.ServerName == ddlServer.SelectedItem.ToString());
            _settings.Servers[selected].ClientFolder = txtClientFolder.Text;
            _settings.Servers[selected].PatchBaseUrl = txtPatchBaseURL.Text;
            _settings.Servers[selected].PatchInfoUrl = txtPatchInfoURL.Text;
            _settings.Servers[selected].ServerName = txtServerName.Text;
            _settings.Servers[selected].Default = cbDefaultServer.Checked;
            _changetype = ChangeType.None;
            _settings.SaveSettings();
            _settings.LoadSettings();
        }
        private void RefreshDdl()
        {
            foreach (PatcherSettings profile in _settings.Servers)
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
            if (_patcher.DownloadJson() == 1)
            {
                pbProgress.Value = 0;
                pbProgress.Maximum = _patcher.PatchFiles.Count;
                bgScanWorker.RunWorkerAsync(_patcher);
            }
            else
                txtLog.AppendText("ERROR: Unable to download Patch Information!\r\n");
        }
        private void PostScan()
        {
            if (_patcher.LocalFiles.Count > 0)
            {
                pbProgress.Value = 0;
                pbProgress.Maximum = _patcher.LocalFiles.Count;
                pbFileProgress.Visible = true;
                bgDownloadWorker.RunWorkerAsync(_patcher);
            }
            else
                PostDownload();
        }
        private void PostDownload()
        {
            pbProgress.Value = pbProgress.Maximum;
            pbFileProgress.Visible = true;
            pbFileProgress.Value = pbFileProgress.Maximum;
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
        #region bgDownloadWorker
        private void bgDownloadWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            ClientPatcher myPatcher = (ClientPatcher)e.Argument;
            //myPatcher.DownloadFiles();
            myPatcher.DownloadFilesAsync();
        }
        private void bgDownloadWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            PostDownload();
        }
        #endregion
        #region ThreadSafe Control Updates
        //Used to update main progress bar, one step
        delegate void ProgressPerformStepCallback();
        private void PbProgressPerformStep()
        {
            if (pbProgress.InvokeRequired)
            {
                ProgressPerformStepCallback d = PbProgressPerformStep;
                Invoke(d);
            }
            else
            {
                pbProgress.PerformStep();
            }
        }
        //Used to update per-file progress bar, set to value
        delegate void FileProgressSetValueCallback(int value);
        private void PbFileProgressSetValueStep(int value)
        {
            if (pbFileProgress.InvokeRequired)
            {
                FileProgressSetValueCallback d = PbFileProgressSetValueStep;
                Invoke(d, new object[] { value });
            }
            else
            {
                pbFileProgress.Value = value;
            }
        }
        //Used to add stuff to the Log
        delegate void TxtLogAppendTextCalback(string text);
        private void TxtLogAppendText(string text)
        {
            if (txtLog.InvokeRequired)
            {
                TxtLogAppendTextCalback d = TxtLogAppendText;
                Invoke(d, new object[] { text });
            }
            else
            {
                txtLog.AppendText(text);
            }
        }
        #endregion

        


    }
}
