using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace m59bind
{
    public partial class M59Bind : Form
    {

        //public bool capturing = false;
        string configFile = "./config.ini";
        string keyPrompt = "press a key";
        bool capturing = false;
        bool configChanged;

        public M59Bind()
        {
            InitializeComponent();
        }

        private void M59Bind_Load(object sender, EventArgs e)
        {
            // Movement Tab
            this.buttonForward.Text = m59BindProgram.GetIni(configFile, "keys", "forward", "w");
            this.buttonBackward.Text = m59BindProgram.GetIni(configFile, "keys", "backward", "s");
            this.buttonTurnLeft.Text = m59BindProgram.GetIni(configFile, "keys", "left", "left");
            this.buttonTurnRight.Text = m59BindProgram.GetIni(configFile, "keys", "right", "right");
            this.buttonSlideLeft.Text = m59BindProgram.GetIni(configFile, "keys", "slideleft", "a");
            this.buttonSlideRight.Text = m59BindProgram.GetIni(configFile, "keys", "slideright", "d");
            this.buttonRunWalk.Text = m59BindProgram.GetIni(configFile, "keys", "run/walk", "shift");
            this.buttonLookUp.Text = m59BindProgram.GetIni(configFile, "keys", "lookup", "pageup");
            this.buttonLookDown.Text = m59BindProgram.GetIni(configFile, "keys", "lookdown", "pagedown");
            this.buttonLookStraight.Text = m59BindProgram.GetIni(configFile, "keys", "lookstraight", "home");
            this.buttonFlip.Text = m59BindProgram.GetIni(configFile, "keys", "flip", "end");
            this.buttonMouselookToggle.Text = m59BindProgram.GetIni(configFile, "keys", "mouselooktoggle", "c+any");

            // Communication Tab
            this.buttonSay.Text = m59BindProgram.GetIni(configFile, "keys", "say", "f");
            this.buttonChat.Text = m59BindProgram.GetIni(configFile, "keys", "chat", "enter");
            this.buttonTell.Text = m59BindProgram.GetIni(configFile, "keys", "tell", "t");
            this.buttonYell.Text = m59BindProgram.GetIni(configFile, "keys", "yell", "y");
            this.buttonBroadcast.Text = m59BindProgram.GetIni(configFile, "keys", "broadcast", "b");
            this.buttonWho.Text = m59BindProgram.GetIni(configFile, "keys", "who", "w+ctrl");
            this.buttonEmote.Text = m59BindProgram.GetIni(configFile, "keys", "emote", ";");

            // Interaction Tab
            this.buttonOpen.Text = m59BindProgram.GetIni(configFile, "keys", "open", "space");
            this.buttonPickUp.Text = m59BindProgram.GetIni(configFile, "keys", "pickup", "g");
            this.buttonLook.Text = m59BindProgram.GetIni(configFile, "keys", "look", "l");
            this.buttonExamine.Text = m59BindProgram.GetIni(configFile, "keys", "examine", "mouse1+any");
            this.buttonOffer.Text = m59BindProgram.GetIni(configFile, "keys", "offer", "o+ctrl");
            this.buttonBuy.Text = m59BindProgram.GetIni(configFile, "keys", "buy", "b+shift");
            this.buttonDeposit.Text = m59BindProgram.GetIni(configFile, "keys", "deposit", "i+shift");
            this.buttonWithdraw.Text = m59BindProgram.GetIni(configFile, "keys", "withdraw", "o+shift");
            this.buttonAttack.Text = m59BindProgram.GetIni(configFile, "keys", "attack", "e+any");

            // Targeting Tab
            this.buttonTargetNext.Text = m59BindProgram.GetIni(configFile, "keys", "targetnext", "]");
            this.buttonTargetPrevious.Text = m59BindProgram.GetIni(configFile, "keys", "targetprevious", "[");
            this.buttonTargetClear.Text = m59BindProgram.GetIni(configFile, "keys", "targetclear", "esc");
            this.buttonTargetSelf.Text = m59BindProgram.GetIni(configFile, "keys", "targetself", "q");
            this.buttonTabForward.Text = m59BindProgram.GetIni(configFile, "keys", "tabforward", "tab");
            this.buttonTabBackward.Text = m59BindProgram.GetIni(configFile, "keys", "tabbackward", "tab+shift");
            this.buttonSelectTarget.Text = m59BindProgram.GetIni(configFile, "keys", "mousetarget", "mouse0");

            // Map Tab
            this.buttonMap.Text = m59BindProgram.GetIni(configFile, "keys", "map", "m+shift");
            this.buttonMapZoomIn.Text = m59BindProgram.GetIni(configFile, "keys", "mapzoomin", "add");
            this.buttonMapZoomOut.Text = m59BindProgram.GetIni(configFile, "keys", "mapzoomout", "subtract");

            // Mouse Tab
            if (m59BindProgram.GetIni(configFile, "config", "invertmouse", "false") == "true")
                this.checkBoxInvertMouse.Checked = true;
            else
                this.checkBoxInvertMouse.Checked = false;

            this.trackBarMouseXScale.Value = Convert.ToInt32(m59BindProgram.GetIni(configFile, "config", "mouselookxscale", "15"));
            labelMouselookXScaleValue.Text = trackBarMouseXScale.Value.ToString();

            this.trackBarMouseYScale.Value = Convert.ToInt32(m59BindProgram.GetIni(configFile, "config", "mouselookyscale", "9"));
            labelMouselookYScaleValue.Text = trackBarMouseYScale.Value.ToString();

            // Options Group
            if (m59BindProgram.GetIni(configFile, "config", "classickeybindings", "false") == "true")
                this.checkBoxClassicKeyBind.Checked = true;
            else
                this.checkBoxClassicKeyBind.Checked = false;

            if (m59BindProgram.GetIni(configFile, "config", "quickchat", "false") == "true")
                this.checkBoxQuickChat.Checked = true;
            else
                this.checkBoxQuickChat.Checked = false;

            if (m59BindProgram.GetIni(configFile, "config", "alwaysrun", "true") == "true")
                this.checkBoxAlwaysRun.Checked = true;
            else
                this.checkBoxAlwaysRun.Checked = false;

            if (m59BindProgram.GetIni(configFile, "config", "attackontarget", "false") == "true")
                this.checkBoxAttackOnTarget.Checked = true;
            else
                this.checkBoxAttackOnTarget.Checked = false;

            if (m59BindProgram.GetIni(configFile, "config", "dynamiclighting", "true") == "true")
                this.checkBoxDynamicLighting.Checked = true;
            else
                this.checkBoxDynamicLighting.Checked = false;

            if (m59BindProgram.GetIni(configFile, "config", "softwarerendering", "false") == "true")
                this.checkBoxSoftwareRenderer.Checked = true;
            else
                this.checkBoxSoftwareRenderer.Checked = false;

            configChanged = false;
        }

        private void writeToConfigFile()
        {
            // movement tab
            m59BindProgram.WriteIni(configFile, "keys", "forward", buttonForward.Text);
            m59BindProgram.WriteIni(configFile, "keys", "backward", buttonBackward.Text);
            m59BindProgram.WriteIni(configFile, "keys", "left", buttonTurnLeft.Text);
            m59BindProgram.WriteIni(configFile, "keys", "right", buttonTurnRight.Text);
            m59BindProgram.WriteIni(configFile, "keys", "slideleft", buttonSlideLeft.Text);
            m59BindProgram.WriteIni(configFile, "keys", "slideright", buttonSlideRight.Text);
            m59BindProgram.WriteIni(configFile, "keys", "run/walk", buttonRunWalk.Text);
            m59BindProgram.WriteIni(configFile, "keys", "lookup", buttonLookUp.Text);
            m59BindProgram.WriteIni(configFile, "keys", "lookdown", buttonLookDown.Text);
            m59BindProgram.WriteIni(configFile, "keys", "lookstraight", buttonLookStraight.Text);
            m59BindProgram.WriteIni(configFile, "keys", "flip", buttonFlip.Text);
            m59BindProgram.WriteIni(configFile, "keys", "mouselooktoggle", buttonMouselookToggle.Text);

            // Communication Tab
            m59BindProgram.WriteIni(configFile, "keys", "say", buttonSay.Text);
            m59BindProgram.WriteIni(configFile, "keys", "chat", buttonChat.Text);
            m59BindProgram.WriteIni(configFile, "keys", "tell", buttonTell.Text);
            m59BindProgram.WriteIni(configFile, "keys", "yell", buttonYell.Text);
            m59BindProgram.WriteIni(configFile, "keys", "broadcast", buttonBroadcast.Text);
            m59BindProgram.WriteIni(configFile, "keys", "who", buttonWho.Text);
            m59BindProgram.WriteIni(configFile, "keys", "emote", buttonEmote.Text);

            // Interaction Tab
            m59BindProgram.WriteIni(configFile, "keys", "open", buttonOpen.Text);
            m59BindProgram.WriteIni(configFile, "keys", "pickup", buttonPickUp.Text);
            m59BindProgram.WriteIni(configFile, "keys", "look", buttonLook.Text);
            m59BindProgram.WriteIni(configFile, "keys", "examine", buttonExamine.Text);
            m59BindProgram.WriteIni(configFile, "keys", "offer", buttonOffer.Text);
            m59BindProgram.WriteIni(configFile, "keys", "buy", buttonBuy.Text);
            m59BindProgram.WriteIni(configFile, "keys", "deposit", buttonDeposit.Text);
            m59BindProgram.WriteIni(configFile, "keys", "withdraw", buttonWithdraw.Text);
            m59BindProgram.WriteIni(configFile, "keys", "attack", buttonAttack.Text);

            // Targeting Tab
            m59BindProgram.WriteIni(configFile, "keys", "targetnext", buttonTargetNext.Text);
            m59BindProgram.WriteIni(configFile, "keys", "targetprevious", buttonTargetPrevious.Text);
            m59BindProgram.WriteIni(configFile, "keys", "targetclear", buttonTargetClear.Text);
            m59BindProgram.WriteIni(configFile, "keys", "targetself", buttonTargetSelf.Text);
            m59BindProgram.WriteIni(configFile, "keys", "tabforward", buttonTabForward.Text);
            m59BindProgram.WriteIni(configFile, "keys", "tabbackward", buttonTabBackward.Text);
            m59BindProgram.WriteIni(configFile, "keys", "mousetarget", buttonSelectTarget.Text);

            // Map Tab
            m59BindProgram.WriteIni(configFile, "keys", "map", buttonMap.Text);
            m59BindProgram.WriteIni(configFile, "keys", "mapzoomin", buttonMapZoomIn.Text);
            m59BindProgram.WriteIni(configFile, "keys", "mapzoomout", buttonMapZoomOut.Text);

            // Mouse Tab
            if (checkBoxInvertMouse.Checked)
            {
                m59BindProgram.WriteIni(configFile, "config", "invertmouse", "true");
            }
            else
            {
                m59BindProgram.WriteIni(configFile, "config", "invertmouse", "false");
            }

            m59BindProgram.WriteIni(configFile, "config", "mouselookxscale", trackBarMouseXScale.Value.ToString());
            m59BindProgram.WriteIni(configFile, "config", "mouselookyscale", trackBarMouseYScale.Value.ToString());

            // Options Group
            if (checkBoxClassicKeyBind.Checked)
            {
                m59BindProgram.WriteIni(configFile, "config", "classickeybindings", "true");
            }
            else
            {
                m59BindProgram.WriteIni(configFile, "config", "classickeybindings", "false");
            }
            
            if (checkBoxQuickChat.Checked)
            {
                m59BindProgram.WriteIni(configFile, "config", "quickchat", "true");
            }
            else
            {
                m59BindProgram.WriteIni(configFile, "config", "quickchat", "false");
            }

            if (checkBoxAlwaysRun.Checked)
            {
                m59BindProgram.WriteIni(configFile, "config", "alwaysrun", "true");
            }
            else
            {
                m59BindProgram.WriteIni(configFile, "config", "alwaysrun", "false");
            }

            if (checkBoxAttackOnTarget.Checked)
            {
                m59BindProgram.WriteIni(configFile, "config", "attackontarget", "true");
            }
            else
            {
                m59BindProgram.WriteIni(configFile, "config", "attackontarget", "false");
            }

            if (checkBoxDynamicLighting.Checked)
            {
                m59BindProgram.WriteIni(configFile, "config", "dynamiclighting", "true");
            }
            else
            {
                m59BindProgram.WriteIni(configFile, "config", "dynamiclighting", "false");
            }

            if (checkBoxSoftwareRenderer.Checked)
            {
                m59BindProgram.WriteIni(configFile, "config", "softwarerendering", "true");
            }
            else
            {
                m59BindProgram.WriteIni(configFile, "config", "softwarerendering", "false");
            }
        }

        private string handleKeyDown(KeyEventArgs e)
        {
            string returnString;

            switch (e.KeyCode)
            {
                case Keys.Shift:
                case Keys.ShiftKey:
                case Keys.LShiftKey:
                case Keys.RShiftKey:
                case Keys.Control:
                case Keys.ControlKey:
                case Keys.LControlKey:
                case Keys.RControlKey:
                case Keys.Alt:
                case Keys.Menu:
                case Keys.LMenu:
                case Keys.RMenu:
                    e.Handled = true;
                    return keyPrompt;

                case Keys.Down:
                    returnString = "down";
                    break;
                case Keys.Up:
                    returnString = "up";
                    break;
                case Keys.Left:
                    returnString = "left";
                    break;
                case Keys.Right:
                    returnString = "right";
                    break;
                case Keys.Tab:
                    returnString = "tab";
                    break;
                case Keys.PageUp:
                    returnString = "pageup";
                    break;
                case Keys.PageDown:
                    returnString = "pagedown";
                    break;
                case Keys.Home:
                    returnString = "home";
                    break;
                case Keys.End:
                    returnString = "end";
                    break;
                case Keys.Insert:
                    returnString = "insert";
                    break;
                case Keys.Delete:
                    returnString = "delete";
                    break;
                case Keys.Enter:
                    returnString = "enter";
                    break;
                case Keys.Space:
                    returnString = "space";
                    break;
                case Keys.F1:
                case Keys.F2:
                case Keys.F3:
                case Keys.F4:
                case Keys.F5:
                case Keys.F6:
                case Keys.F7:
                case Keys.F8:
                case Keys.F9:
                case Keys.F10:
                case Keys.F11:
                case Keys.F12:
                    returnString = "";
                    returnString += e.KeyData.ToString().Substring(0, 2);
                    break;
                default:
                    returnString = "";
                    returnString += e.KeyData.ToString().ToLower().Substring(0, 1);
                    break;
            }
            if (e.Shift == true)
            {
                returnString += "+shift";
            }
            else if (e.Control)
            {
                returnString += "+ctrl";
            }
            else if (e.Alt)
            {
                returnString += "+alt";
            }
            e.Handled = true;

            configChanged = true;

            return returnString;
        }

        private void previewKeySetIsInputKey(PreviewKeyDownEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Down:
                case Keys.Up:
                case Keys.Left:
                case Keys.Right:
                case Keys.Tab:
                case Keys.Shift:
                case Keys.ShiftKey:
                case Keys.LShiftKey:
                case Keys.RShiftKey:
                case Keys.Control:
                case Keys.ControlKey:
                case Keys.LControlKey:
                case Keys.RControlKey:
                case Keys.Alt:
                case Keys.Menu:
                case Keys.LMenu:
                case Keys.RMenu:
                case Keys.PageUp:
                case Keys.PageDown:
                case Keys.Home:
                case Keys.End:
                case Keys.Insert:
                case Keys.Delete:
                case Keys.Enter:
                case Keys.Space:
                    e.IsInputKey = true;
                    break;
            }
        }

        private string handleMouseDown(MouseEventArgs e)
        {
            string returnString = "";

            if (capturing)
            {
                if (e.Button == MouseButtons.Left)
                    returnString = "mouse0";
                else if (e.Button == MouseButtons.Right)
                    returnString = "mouse1";
                else if (e.Button == MouseButtons.Middle)
                    returnString = "mouse2";

                capturing = false;
            }
            else
            {
                returnString = keyPrompt;
                capturing = true;
            }

            return returnString;
        }

        public void handleKeyModFormClose(object originalCaller, string modifier)
        {

        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            if (configChanged)
            {
                MessageBox.Show("You must restart Meridian before these changes will take effect.", "Reload Game");
                writeToConfigFile();
            }

            Application.Exit();
        }

        private void buttonForward_MouseDown(object sender, MouseEventArgs e)
        {
            buttonForward.Text = handleMouseDown(e);
        }

        private void buttonForward_KeyDown(object sender, KeyEventArgs e)
        {
            buttonForward.Text = handleKeyDown(e);
            capturing = false;
        }

        private void buttonForward_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonBackward_MouseDown(object sender, MouseEventArgs e)
        {
            buttonBackward.Text = handleMouseDown(e);
        }

        private void buttonBackward_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonBackward_KeyDown(object sender, KeyEventArgs e)
        {
            buttonBackward.Text = handleKeyDown(e);
        }

        private void buttonTurnLeft_MouseDown(object sender, MouseEventArgs e)
        {
            buttonTurnLeft.Text = handleMouseDown(e);
        }

        private void buttonTurnLeft_KeyDown(object sender, KeyEventArgs e)
        {
            buttonTurnLeft.Text = handleKeyDown(e);
        }

        private void buttonTurnLeft_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonTurnRight_MouseDown(object sender, MouseEventArgs e)
        {
            buttonTurnRight.Text = handleMouseDown(e);
        }

        private void buttonTurnRight_KeyDown(object sender, KeyEventArgs e)
        {
            buttonTurnRight.Text = handleKeyDown(e);
        }

        private void buttonTurnRight_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonSlideLeft_MouseDown(object sender, MouseEventArgs e)
        {
            buttonSlideLeft.Text = handleMouseDown(e);
        }

        private void buttonSlideLeft_KeyDown(object sender, KeyEventArgs e)
        {
            buttonSlideLeft.Text = handleKeyDown(e);
        }

        private void buttonSlideLeft_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonSlideRight_MouseDown(object sender, MouseEventArgs e)
        {
            buttonSlideRight.Text = handleMouseDown(e);
        }

        private void buttonSlideRight_KeyDown(object sender, KeyEventArgs e)
        {
            buttonSlideRight.Text = handleKeyDown(e);
        }

        private void buttonSlideRight_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonRunWalk_MouseDown(object sender, MouseEventArgs e)
        {
            buttonRunWalk.Text = handleMouseDown(e);
        }

        private void buttonRunWalk_KeyDown(object sender, KeyEventArgs e)
        {
            buttonRunWalk.Text = handleKeyDown(e);
        }

        private void buttonRunWalk_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonLookUp_MouseDown(object sender, MouseEventArgs e)
        {
            buttonLookUp.Text = handleMouseDown(e);
        }

        private void buttonLookUp_KeyDown(object sender, KeyEventArgs e)
        {
            buttonBackward.Text = handleKeyDown(e);
        }

        private void buttonLookUp_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonLookDown_MouseDown(object sender, MouseEventArgs e)
        {
            buttonLookDown.Text = handleMouseDown(e);
        }

        private void buttonLookDown_KeyDown(object sender, KeyEventArgs e)
        {
            buttonLookDown.Text = handleKeyDown(e);
        }

        private void buttonLookDown_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonLookStraight_MouseDown(object sender, MouseEventArgs e)
        {
            buttonLookStraight.Text = handleMouseDown(e);
        }

        private void buttonLookStraight_KeyDown(object sender, KeyEventArgs e)
        {
            buttonLookStraight.Text = handleKeyDown(e);
        }

        private void buttonLookStraight_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonFlip_MouseDown(object sender, MouseEventArgs e)
        {
            buttonFlip.Text = handleMouseDown(e);
        }

        private void buttonFlip_KeyDown(object sender, KeyEventArgs e)
        {
            buttonFlip.Text = handleKeyDown(e);
        }

        private void buttonFlip_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonMouselookToggle_MouseDown(object sender, MouseEventArgs e)
        {
            buttonMouselookToggle.Text = handleMouseDown(e);
        }

        private void buttonMouselookToggle_KeyDown(object sender, KeyEventArgs e)
        {
            buttonMouselookToggle.Text = handleKeyDown(e);
        }

        private void buttonMouselookToggle_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void checkBoxInvertMouse_CheckedChanged(object sender, EventArgs e)
        {
            configChanged = true;
        }

        private void trackBarMouseXScale_Scroll(object sender, EventArgs e)
        {
            labelMouselookXScaleValue.Text = trackBarMouseXScale.Value.ToString();
            configChanged = true;
        }

        private void trackBarMouseYScale_Scroll(object sender, EventArgs e)
        {
            labelMouselookYScaleValue.Text = trackBarMouseYScale.Value.ToString();
            configChanged = true;
        }

        private void checkBoxClassicKeyBind_CheckedChanged(object sender, EventArgs e)
        {
            configChanged = true;
        }

        private void checkBoxQuickChat_CheckedChanged(object sender, EventArgs e)
        {
            configChanged = true;
        }

        private void checkBoxAlwaysRun_CheckedChanged(object sender, EventArgs e)
        {
            configChanged = true;
        }

        private void checkBoxAttackOnTarget_CheckedChanged(object sender, EventArgs e)
        {
            configChanged = true;
        }

        private void checkBoxDynamicLighting_CheckedChanged(object sender, EventArgs e)
        {
            configChanged = true;
        }

        private void checkBoxSoftwareRenderer_CheckedChanged(object sender, EventArgs e)
        {
            configChanged = true;
        }

        private void buttonForwardMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonForward);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonBackwardMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonBackward);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonTurnLeftMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonTurnLeft);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonTurnRightMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonTurnRight);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonSlideLeftMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonSlideLeft);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonSlideRightMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonSlideRight);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonRunWalkMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonRunWalk);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonLookUpMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonLookUp);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonLookDownMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonLookDown);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonLookStraightMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonLookStraight);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonFlipMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonFlip);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonMouselookToggleMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonMouselookToggle);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonRestoreDefaults_Click(object sender, EventArgs e)
        {
            // Movement Tab
            this.buttonForward.Text = "w";
            this.buttonBackward.Text = "s";
            this.buttonTurnLeft.Text = "left";
            this.buttonTurnRight.Text = "right";
            this.buttonSlideLeft.Text = "a";
            this.buttonSlideRight.Text = "d";
            this.buttonRunWalk.Text = "shift";
            this.buttonLookUp.Text = "pageup";
            this.buttonLookDown.Text = "pagedown";
            this.buttonLookStraight.Text = "home";
            this.buttonFlip.Text = "end";
            this.buttonMouselookToggle.Text = "c+any";

            // Communication Tab
            this.buttonSay.Text = "f";
            this.buttonChat.Text = "enter";
            this.buttonTell.Text = "t";
            this.buttonYell.Text = "y";
            this.buttonBroadcast.Text = "b";
            this.buttonWho.Text = "w+ctrl";
            this.buttonEmote.Text = ";";

            // Interaction Tab
            this.buttonOpen.Text = "space";
            this.buttonPickUp.Text = "g";
            this.buttonLook.Text = "l";
            this.buttonExamine.Text = "mouse1+any";
            this.buttonOffer.Text = "o+ctrl";
            this.buttonBuy.Text = "b+shift";
            this.buttonDeposit.Text = "i+shift";
            this.buttonWithdraw.Text = "o+shift";
            this.buttonAttack.Text = "e+any";

            // Targeting Tab
            this.buttonTargetNext.Text = "]";
            this.buttonTargetPrevious.Text = "[";
            this.buttonTargetClear.Text = "esc";
            this.buttonTargetSelf.Text = "q";
            this.buttonTabForward.Text = "tab";
            this.buttonTabBackward.Text = "tab+shift";
            this.buttonSelectTarget.Text = "mouse0";

            // Map Tab
            this.buttonMap.Text = "m+shift";
            this.buttonMapZoomIn.Text = "add";
            this.buttonMapZoomOut.Text = "subtract";

            // Mouse Tab
            m59BindProgram.GetIni(configFile, "config", "invertmouse", "false");

            this.trackBarMouseXScale.Value = 15;
            this.trackBarMouseYScale.Value = 9;

            // Options Group
            m59BindProgram.GetIni(configFile, "config", "classickeybindings", "false");
            m59BindProgram.GetIni(configFile, "config", "quickchat", "false");
            m59BindProgram.GetIni(configFile, "config", "alwaysrun", "true");
            m59BindProgram.GetIni(configFile, "config", "attackontarget", "false");
            m59BindProgram.GetIni(configFile, "config", "dynamiclighting", "true");
            m59BindProgram.GetIni(configFile, "config", "softwarerendering", "false");

            configChanged = false;
        }

        private void buttonSay_KeyDown(object sender, KeyEventArgs e)
        {
            buttonSay.Text = handleKeyDown(e);
        }

        private void buttonSay_MouseDown(object sender, MouseEventArgs e)
        {
            buttonSay.Text = handleMouseDown(e);
        }

        private void buttonSay_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonChat_KeyDown(object sender, KeyEventArgs e)
        {
            buttonChat.Text = handleKeyDown(e);
        }

        private void buttonChat_MouseDown(object sender, MouseEventArgs e)
        {
            buttonChat.Text = handleMouseDown(e);
        }

        private void buttonChat_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonTell_KeyDown(object sender, KeyEventArgs e)
        {
            buttonTell.Text = handleKeyDown(e);
        }

        private void buttonTell_MouseDown(object sender, MouseEventArgs e)
        {
            buttonTell.Text = handleMouseDown(e);
        }

        private void buttonTell_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonYell_KeyDown(object sender, KeyEventArgs e)
        {
            buttonYell.Text = handleKeyDown(e);
        }

        private void buttonYell_MouseDown(object sender, MouseEventArgs e)
        {
            buttonYell.Text = handleMouseDown(e);
        }

        private void buttonYell_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonBroadcast_KeyDown(object sender, KeyEventArgs e)
        {
            buttonBroadcast.Text = handleKeyDown(e);
        }

        private void buttonBroadcast_MouseDown(object sender, MouseEventArgs e)
        {
            buttonBroadcast.Text = handleMouseDown(e);
        }

        private void buttonBroadcast_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonWho_KeyDown(object sender, KeyEventArgs e)
        {
            buttonWho.Text = handleKeyDown(e);
        }

        private void buttonWho_MouseDown(object sender, MouseEventArgs e)
        {
            buttonWho.Text = handleMouseDown(e);
        }

        private void buttonWho_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonEmote_KeyDown(object sender, KeyEventArgs e)
        {
            buttonEmote.Text = handleKeyDown(e);
        }

        private void buttonEmote_MouseDown(object sender, MouseEventArgs e)
        {
            buttonEmote.Text = handleMouseDown(e);
        }

        private void buttonEmote_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonSayMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonSay);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonChatMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonChat);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonTellMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonTell);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonYellMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonYell);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonBroadcastMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonBroadcast);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonWhoMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonWho);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        private void buttonEmoteMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonEmote);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
        * Open
        */
        private void buttonOpen_KeyDown(object sender, KeyEventArgs e)
        {
            buttonOpen.Text = handleKeyDown(e);
        }

        private void buttonOpen_MouseDown(object sender, MouseEventArgs e)
        {
            buttonOpen.Text = handleMouseDown(e);
        }

        private void buttonOpen_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonOpenMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonOpen);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Pick Up
         */
        private void buttonPickUp_KeyDown(object sender, KeyEventArgs e)
        {
            buttonPickUp.Text = handleKeyDown(e);
        }

        private void buttonPickUp_MouseDown(object sender, MouseEventArgs e)
        {
            buttonPickUp.Text = handleMouseDown(e);
        }

        private void buttonPickUp_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonPickUpMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonPickUp);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Look
         */
        private void buttonLook_KeyDown(object sender, KeyEventArgs e)
        {
            buttonLook.Text = handleKeyDown(e);
        }

        private void buttonLook_MouseDown(object sender, MouseEventArgs e)
        {
            buttonLook.Text = handleMouseDown(e);
        }

        private void buttonLook_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonLookMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonLook);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Examine
         */
        private void buttonExamine_KeyDown(object sender, KeyEventArgs e)
        {
            buttonExamine.Text = handleKeyDown(e);
        }

        private void buttonExamine_MouseDown(object sender, MouseEventArgs e)
        {
            buttonExamine.Text = handleMouseDown(e);
        }

        private void buttonExamine_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonExamineMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonExamine);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Offer
         */
        private void buttonOffer_KeyDown(object sender, KeyEventArgs e)
        {
            buttonOffer.Text = handleKeyDown(e);
        }

        private void buttonOffer_MouseDown(object sender, MouseEventArgs e)
        {
            buttonOffer.Text = handleMouseDown(e);
        }

        private void buttonOffer_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonOfferMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonOffer);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Buy
         */
        private void buttonBuy_KeyDown(object sender, KeyEventArgs e)
        {
            buttonBuy.Text = handleKeyDown(e);
        }

        private void buttonBuy_MouseDown(object sender, MouseEventArgs e)
        {
            buttonBuy.Text = handleMouseDown(e);
        }

        private void buttonBuy_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonBuyMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonBuy);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Deposit
         */
        private void buttonDeposit_KeyDown(object sender, KeyEventArgs e)
        {
            buttonDeposit.Text = handleKeyDown(e);
        }

        private void buttonDeposit_MouseDown(object sender, MouseEventArgs e)
        {
            buttonDeposit.Text = handleMouseDown(e);
        }

        private void buttonDeposit_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonDepositMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonDeposit);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Withdraw
         */
        private void buttonWithdraw_KeyDown(object sender, KeyEventArgs e)
        {
            buttonWithdraw.Text = handleKeyDown(e);
        }

        private void buttonWithdraw_MouseDown(object sender, MouseEventArgs e)
        {
            buttonWithdraw.Text = handleMouseDown(e);
        }

        private void buttonWithdraw_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonWithdrawMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonWithdraw);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Attack
         */
        private void buttonAttack_KeyDown(object sender, KeyEventArgs e)
        {
            buttonAttack.Text = handleKeyDown(e);
        }

        private void buttonAttack_MouseDown(object sender, MouseEventArgs e)
        {
            buttonAttack.Text = handleMouseDown(e);
        }

        private void buttonAttack_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonAttackMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonAttack);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Target Next
         */
        private void buttonTargetNext_KeyDown(object sender, KeyEventArgs e)
        {
            buttonTargetNext.Text = handleKeyDown(e);
        }

        private void buttonTargetNext_MouseDown(object sender, MouseEventArgs e)
        {
            buttonTargetNext.Text = handleMouseDown(e);
        }

        private void buttonTargetNext_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonTargetNextMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonTargetNext);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Target Previous
         */
        private void buttonTargetPrevious_KeyDown(object sender, KeyEventArgs e)
        {
            buttonTargetPrevious.Text = handleKeyDown(e);
        }

        private void buttonTargetPrevious_MouseDown(object sender, MouseEventArgs e)
        {
            buttonTargetPrevious.Text = handleMouseDown(e);
        }

        private void buttonTargetPrevious_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonTargetPreviousMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonTargetPrevious);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Target Clear
         */
        private void buttonTargetClear_KeyDown(object sender, KeyEventArgs e)
        {
            buttonTargetClear.Text = handleKeyDown(e);
        }

        private void buttonTargetClear_MouseDown(object sender, MouseEventArgs e)
        {
            buttonTargetClear.Text = handleMouseDown(e);
        }

        private void buttonTargetClear_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonTargetClearMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonTargetClear);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Target Self
         */
        private void buttonTargetSelf_KeyDown(object sender, KeyEventArgs e)
        {
            buttonTargetSelf.Text = handleKeyDown(e);
        }

        private void buttonTargetSelf_MouseDown(object sender, MouseEventArgs e)
        {
            buttonTargetSelf.Text = handleMouseDown(e);
        }

        private void buttonTargetSelf_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonTargetSelfMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonTargetSelf);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Tab Forward
         */
        private void buttonTabForward_KeyDown(object sender, KeyEventArgs e)
        {
            buttonTabForward.Text = handleKeyDown(e);
        }

        private void buttonTabForward_MouseDown(object sender, MouseEventArgs e)
        {
            buttonTabForward.Text = handleMouseDown(e);
        }

        private void buttonTabForward_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonTabForwardMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonTabForward);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Tab Backward
         */
        private void buttonTabBackward_KeyDown(object sender, KeyEventArgs e)
        {
            buttonTabBackward.Text = handleKeyDown(e);
        }

        private void buttonTabBackward_MouseDown(object sender, MouseEventArgs e)
        {
            buttonTabBackward.Text = handleMouseDown(e);
        }

        private void buttonTabBackward_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonTabBackwardMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonTabBackward);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Select Target
         */
        private void buttonSelectTarget_KeyDown(object sender, KeyEventArgs e)
        {
            buttonSelectTarget.Text = handleKeyDown(e);
        }

        private void buttonSelectTarget_MouseDown(object sender, MouseEventArgs e)
        {
            buttonSelectTarget.Text = handleMouseDown(e);
        }

        private void buttonSelectTarget_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonSelectTargetMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonSelectTarget);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Map
         */
        private void buttonMap_KeyDown(object sender, KeyEventArgs e)
        {
            buttonMap.Text = handleKeyDown(e);
        }

        private void buttonMap_MouseDown(object sender, MouseEventArgs e)
        {
            buttonMap.Text = handleMouseDown(e);
        }

        private void buttonMap_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonMapMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonMap);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Map Zoom In
         */
        private void buttonMapZoomIn_KeyDown(object sender, KeyEventArgs e)
        {
            buttonMapZoomIn.Text = handleKeyDown(e);
        }

        private void buttonMapZoomIn_MouseDown(object sender, MouseEventArgs e)
        {
            buttonMapZoomIn.Text = handleMouseDown(e);
        }

        private void buttonMapZoomIn_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonMapZoomInMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonMapZoomIn);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }

        /*
         * Map Zoom Out
         */
        private void buttonMapZoomOut_KeyDown(object sender, KeyEventArgs e)
        {
            buttonMapZoomOut.Text = handleKeyDown(e);
        }

        private void buttonMapZoomOut_MouseDown(object sender, MouseEventArgs e)
        {
            buttonMapZoomOut.Text = handleMouseDown(e);
        }

        private void buttonMapZoomOut_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
        }

        private void buttonMapZoomOutMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonMapZoomOut);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }
    }
}
