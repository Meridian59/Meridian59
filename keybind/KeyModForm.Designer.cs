namespace m59bind
{
    partial class KeyModForm
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
            this.buttonAlt = new System.Windows.Forms.Button();
            this.buttonCtrl = new System.Windows.Forms.Button();
            this.buttonShift = new System.Windows.Forms.Button();
            this.buttonAny = new System.Windows.Forms.Button();
            this.buttonNone = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // buttonAlt
            // 
            this.buttonAlt.Location = new System.Drawing.Point(12, 50);
            this.buttonAlt.Name = "buttonAlt";
            this.buttonAlt.Size = new System.Drawing.Size(75, 23);
            this.buttonAlt.TabIndex = 0;
            this.buttonAlt.Text = "Alt";
            this.buttonAlt.UseVisualStyleBackColor = true;
            this.buttonAlt.Click += new System.EventHandler(this.buttonAlt_Click);
            // 
            // buttonCtrl
            // 
            this.buttonCtrl.Location = new System.Drawing.Point(12, 79);
            this.buttonCtrl.Name = "buttonCtrl";
            this.buttonCtrl.Size = new System.Drawing.Size(75, 23);
            this.buttonCtrl.TabIndex = 1;
            this.buttonCtrl.Text = "Ctrl";
            this.buttonCtrl.UseVisualStyleBackColor = true;
            this.buttonCtrl.Click += new System.EventHandler(this.buttonCtrl_Click);
            // 
            // buttonShift
            // 
            this.buttonShift.Location = new System.Drawing.Point(12, 108);
            this.buttonShift.Name = "buttonShift";
            this.buttonShift.Size = new System.Drawing.Size(75, 23);
            this.buttonShift.TabIndex = 2;
            this.buttonShift.Text = "Shift";
            this.buttonShift.UseVisualStyleBackColor = true;
            this.buttonShift.Click += new System.EventHandler(this.buttonShift_Click);
            // 
            // buttonAny
            // 
            this.buttonAny.Location = new System.Drawing.Point(12, 137);
            this.buttonAny.Name = "buttonAny";
            this.buttonAny.Size = new System.Drawing.Size(75, 23);
            this.buttonAny.TabIndex = 3;
            this.buttonAny.Text = "Any";
            this.buttonAny.UseVisualStyleBackColor = true;
            this.buttonAny.Click += new System.EventHandler(this.buttonAny_Click);
            // 
            // buttonNone
            // 
            this.buttonNone.Location = new System.Drawing.Point(12, 166);
            this.buttonNone.Name = "buttonNone";
            this.buttonNone.Size = new System.Drawing.Size(75, 23);
            this.buttonNone.TabIndex = 4;
            this.buttonNone.Text = "None";
            this.buttonNone.UseVisualStyleBackColor = true;
            this.buttonNone.Click += new System.EventHandler(this.buttonNone_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.Location = new System.Drawing.Point(12, 195);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 5;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(23, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(52, 13);
            this.label1.TabIndex = 6;
            this.label1.Text = "Choose a";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(18, 22);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(63, 13);
            this.label2.TabIndex = 7;
            this.label2.Text = "modifier key";
            // 
            // KeyModForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(99, 228);
            this.ControlBox = false;
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonNone);
            this.Controls.Add(this.buttonAny);
            this.Controls.Add(this.buttonShift);
            this.Controls.Add(this.buttonCtrl);
            this.Controls.Add(this.buttonAlt);
            this.Cursor = System.Windows.Forms.Cursors.Default;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "KeyModForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonAlt;
        private System.Windows.Forms.Button buttonCtrl;
        private System.Windows.Forms.Button buttonShift;
        private System.Windows.Forms.Button buttonAny;
        private System.Windows.Forms.Button buttonNone;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
    }
}