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
        string configFile = "./config.ini";
        string keyPrompt = "press a key";
        bool capturing = false;
        bool configChanged;

        /// <summary>
        /// Constructor
        /// </summary>
        public M59Bind()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Loads keybind and config data from file
        /// </summary>
        /// <param name="sender">object that initiated this event</param>
        /// <param name="e">event details</param>
        private void M59Bind_Load(object sender, EventArgs e)
        {
            // Movement Tab
            buttonForward.Text = m59BindProgram.GetIni(configFile, "keys", "forward", "w");
            buttonBackward.Text = m59BindProgram.GetIni(configFile, "keys", "backward", "s");
            buttonTurnLeft.Text = m59BindProgram.GetIni(configFile, "keys", "left", "left");
            buttonTurnRight.Text = m59BindProgram.GetIni(configFile, "keys", "right", "right");
            buttonSlideLeft.Text = m59BindProgram.GetIni(configFile, "keys", "slideleft", "a");
            buttonSlideRight.Text = m59BindProgram.GetIni(configFile, "keys", "slideright", "d");
            buttonRunWalk.Text = m59BindProgram.GetIni(configFile, "keys", "run/walk", "shift");
            buttonLookUp.Text = m59BindProgram.GetIni(configFile, "keys", "lookup", "pageup");
            buttonLookDown.Text = m59BindProgram.GetIni(configFile, "keys", "lookdown", "pagedown");
            buttonLookStraight.Text = m59BindProgram.GetIni(configFile, "keys", "lookstraight", "home");
            buttonFlip.Text = m59BindProgram.GetIni(configFile, "keys", "flip", "end");
            buttonMouselookToggle.Text = m59BindProgram.GetIni(configFile, "keys", "mouselooktoggle", "c+any");

            // Communication Tab
            buttonSay.Text = m59BindProgram.GetIni(configFile, "keys", "say", "f");
            buttonChat.Text = m59BindProgram.GetIni(configFile, "keys", "chat", "enter");
            buttonTell.Text = m59BindProgram.GetIni(configFile, "keys", "tell", "t");
            buttonYell.Text = m59BindProgram.GetIni(configFile, "keys", "yell", "y");
            buttonBroadcast.Text = m59BindProgram.GetIni(configFile, "keys", "broadcast", "b");
            buttonWho.Text = m59BindProgram.GetIni(configFile, "keys", "who", "w+ctrl");
            buttonEmote.Text = m59BindProgram.GetIni(configFile, "keys", "emote", ";");

            // Interaction Tab
            buttonOpen.Text = m59BindProgram.GetIni(configFile, "keys", "open", "space");
            buttonPickUp.Text = m59BindProgram.GetIni(configFile, "keys", "pickup", "g");
            buttonLook.Text = m59BindProgram.GetIni(configFile, "keys", "look", "l");
            buttonExamine.Text = m59BindProgram.GetIni(configFile, "keys", "examine", "mouse1+any");
            buttonOffer.Text = m59BindProgram.GetIni(configFile, "keys", "offer", "o+ctrl");
            buttonBuy.Text = m59BindProgram.GetIni(configFile, "keys", "buy", "b+shift");
            buttonDeposit.Text = m59BindProgram.GetIni(configFile, "keys", "deposit", "i+shift");
            buttonWithdraw.Text = m59BindProgram.GetIni(configFile, "keys", "withdraw", "o+shift");
            buttonAttack.Text = m59BindProgram.GetIni(configFile, "keys", "attack", "e+any");

            // Targeting Tab
            buttonTargetNext.Text = m59BindProgram.GetIni(configFile, "keys", "targetnext", "]");
            buttonTargetPrevious.Text = m59BindProgram.GetIni(configFile, "keys", "targetprevious", "[");
            buttonTargetClear.Text = m59BindProgram.GetIni(configFile, "keys", "targetclear", "esc");
            buttonTargetSelf.Text = m59BindProgram.GetIni(configFile, "keys", "targetself", "q");
            buttonTabForward.Text = m59BindProgram.GetIni(configFile, "keys", "tabforward", "tab");
            buttonTabBackward.Text = m59BindProgram.GetIni(configFile, "keys", "tabbackward", "tab+shift");
            buttonSelectTarget.Text = m59BindProgram.GetIni(configFile, "keys", "mousetarget", "mouse0");

            // Map Tab
            buttonMap.Text = m59BindProgram.GetIni(configFile, "keys", "map", "m+shift");
            buttonMapZoomIn.Text = m59BindProgram.GetIni(configFile, "keys", "mapzoomin", "add");
            buttonMapZoomOut.Text = m59BindProgram.GetIni(configFile, "keys", "mapzoomout", "subtract");

            // Mouse Tab
            if (m59BindProgram.GetIni(configFile, "config", "invertmouse", "false") == "true")
                checkBoxInvertMouse.Checked = true;
            else
                checkBoxInvertMouse.Checked = false;

            trackBarMouseXScale.Value = Convert.ToInt32(m59BindProgram.GetIni(configFile, "config", "mouselookxscale", "15"));
            labelMouselookXScaleValue.Text = trackBarMouseXScale.Value.ToString();

            trackBarMouseYScale.Value = Convert.ToInt32(m59BindProgram.GetIni(configFile, "config", "mouselookyscale", "9"));
            labelMouselookYScaleValue.Text = trackBarMouseYScale.Value.ToString();

            // Options Group
            if (m59BindProgram.GetIni(configFile, "config", "classickeybindings", "false") == "true")
                checkBoxClassicKeyBind.Checked = true;
            else
                checkBoxClassicKeyBind.Checked = false;

            if (m59BindProgram.GetIni(configFile, "config", "quickchat", "false") == "true")
                checkBoxQuickChat.Checked = true;
            else
                checkBoxQuickChat.Checked = false;

            if (m59BindProgram.GetIni(configFile, "config", "alwaysrun", "true") == "true")
                checkBoxAlwaysRun.Checked = true;
            else
                checkBoxAlwaysRun.Checked = false;

            if (m59BindProgram.GetIni(configFile, "config", "attackontarget", "false") == "true")
                checkBoxAttackOnTarget.Checked = true;
            else
                checkBoxAttackOnTarget.Checked = false;

            if (m59BindProgram.GetIni(configFile, "config", "dynamiclighting", "true") == "true")
                checkBoxDynamicLighting.Checked = true;
            else
                checkBoxDynamicLighting.Checked = false;

            if (m59BindProgram.GetIni(configFile, "config", "softwarerenderer", "false") == "true")
                checkBoxSoftwareRenderer.Checked = true;
            else
                checkBoxSoftwareRenderer.Checked = false;

            configChanged = false;
        }

        ///
        /// <summary>
        /// Load default values into the configuration form
        /// </summary>
        /// <param name="sender">control that initiated the event</param>
        /// <param name="e">event details</param>
        ///
        private void buttonRestoreDefaults_Click(object sender, EventArgs e)
        {
            // Movement Tab
            buttonForward.Text = "w";
            buttonBackward.Text = "s";
            buttonTurnLeft.Text = "left";
            buttonTurnRight.Text = "right";
            buttonSlideLeft.Text = "a";
            buttonSlideRight.Text = "d";
            buttonRunWalk.Text = "shift";
            buttonLookUp.Text = "pageup";
            buttonLookDown.Text = "pagedown";
            buttonLookStraight.Text = "home";
            buttonFlip.Text = "end";
            buttonMouselookToggle.Text = "c+any";

            // Communication Tab
            buttonSay.Text = "f";
            buttonChat.Text = "enter";
            buttonTell.Text = "t";
            buttonYell.Text = "y";
            buttonBroadcast.Text = "b";
            buttonWho.Text = "w+ctrl";
            buttonEmote.Text = ";";

            // Interaction Tab
            buttonOpen.Text = "space";
            buttonPickUp.Text = "g";
            buttonLook.Text = "l";
            buttonExamine.Text = "mouse1+any";
            buttonOffer.Text = "o+ctrl";
            buttonBuy.Text = "b+shift";
            buttonDeposit.Text = "i+shift";
            buttonWithdraw.Text = "o+shift";
            buttonAttack.Text = "e+any";

            // Targeting Tab
            buttonTargetNext.Text = "]";
            buttonTargetPrevious.Text = "[";
            buttonTargetClear.Text = "esc";
            buttonTargetSelf.Text = "q";
            buttonTabForward.Text = "tab";
            buttonTabBackward.Text = "tab+shift";
            buttonSelectTarget.Text = "mouse0";

            // Map Tab
            buttonMap.Text = "m+shift";
            buttonMapZoomIn.Text = "add";
            buttonMapZoomOut.Text = "subtract";

            // Mouse Tab
            m59BindProgram.GetIni(configFile, "config", "invertmouse", "false");

            trackBarMouseXScale.Value = 15;
            trackBarMouseYScale.Value = 9;

            // Options Group
            m59BindProgram.GetIni(configFile, "config", "classickeybindings", "false");
            m59BindProgram.GetIni(configFile, "config", "quickchat", "false");
            m59BindProgram.GetIni(configFile, "config", "alwaysrun", "true");
            m59BindProgram.GetIni(configFile, "config", "attackontarget", "false");
            m59BindProgram.GetIni(configFile, "config", "dynamiclighting", "true");
            m59BindProgram.GetIni(configFile, "config", "softwarerenderer", "false");

            configChanged = true;
        }

        /// <summary>
        /// Writes keybind and config data to file
        /// </summary>
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
                m59BindProgram.WriteIni(configFile, "config", "softwarerenderer", "true");
            }
            else
            {
                m59BindProgram.WriteIni(configFile, "config", "softwarerenderer", "false");
            }
        }

        private bool checkForDuplicates()
        {
            for (int i = 0; i < tabControl1.TabPages.Count; ++i)
            {
                foreach (Control current in tabControl1.TabPages[i].Controls)
                {
                    if (current.GetType() == typeof(Button) &&
                        current.Text.CompareTo("+") != 0)
                    {
                        for (int j = 0; j < tabControl1.TabPages.Count; ++j)
                        {
                            foreach (Control compare in tabControl1.TabPages[j].Controls)
                            {
                                if (current != compare)
                                {
                                    if (current.Text.CompareTo(compare.Text) == 0)
                                    {
                                        MessageBox.Show("You have multiple actions assigned to the same key.  (" + current.Text + ")", "Duplicates");
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return false;
        }

        private bool checkForUnset()
        {
            for (int i = 0; i < tabControl1.TabPages.Count; ++i)
            {
                foreach (Control current in tabControl1.TabPages[i].Controls)
                {
                    if (current.GetType() == typeof(Button) &&
                        current.Text.CompareTo("press a key") == 0)
                    {
                        MessageBox.Show("One or more actions have not been set.", "Unset Action");
                        return true;
                    }
                }
            }
            return false;
        }

        /// <summary>
        /// Captures a key press and saves a keystring into a control
        /// </summary>
        /// <param name="e">key event details</param>
        /// <returns>a string representation of the key</returns>
        private string handleKeyDown(KeyEventArgs e)
        {
            string returnString;

            // Don't allow assigning of numbers.
            if ((e.KeyValue >= 48 && e.KeyValue <= 57)
                || (e.KeyValue >= 96 && e.KeyValue <= 105))
            {
                e.Handled = true;
                return keyPrompt;
            }

            switch (e.KeyCode)
            {
                case Keys.Add:
                    returnString = "add";
                    break;
                case Keys.Subtract:
                    returnString = "subtract";
                    break;
                  case Keys.Divide:
                    returnString = "divide";
                    break;
               case Keys.Multiply:
                    returnString = "multiply";
                    break;
               case Keys.OemBackslash:
                    returnString = "\\";
                    break;
              case Keys.OemCloseBrackets:
                    returnString = "]";
                    break;
               case Keys.OemOpenBrackets:
                    returnString = "[";
                    break;
               case Keys.OemPeriod:
                    returnString = ".";
                    break;
               case Keys.OemPipe:
                    returnString = "|";
                    break;
               case Keys.OemQuestion:
                    returnString = "?";
                    break;
               case Keys.OemQuotes:
                    returnString = "'";
                    break;
               case Keys.OemSemicolon:
                    returnString = ";";
                    break;
               case Keys.Oemcomma:
                    returnString = ",";
                    break;
               case Keys.Oemplus:
                    returnString = "+";
                    break;
               case Keys.Oemtilde:
                    returnString = "`";
                    break;
               case Keys.OemMinus:
               case Keys.Separator:
                    returnString = "-";
                    break;
                case Keys.Alt:
                case Keys.Menu:
                case Keys.LMenu:
                case Keys.RMenu:
                    returnString = "alt";
                    break;
                case Keys.Control:
                case Keys.ControlKey:
                case Keys.LControlKey:
                case Keys.RControlKey:
                    returnString = "ctrl";
                    break;
                case Keys.Escape:
                    returnString = "esc";
                    break;
                case Keys.ShiftKey:
                case Keys.LShiftKey:
                case Keys.RShiftKey:
                case Keys.Shift:
                    returnString = "shift";
                    break;
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
                case Keys.Back:
                    returnString = "backspace";
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
                    // These are used for aliases.
                    e.Handled = true;
                    return keyPrompt;
                default:
                    returnString = "";
                    returnString += e.KeyData.ToString().ToLower().Substring(0, 1);
                    break;
            }
            if (e.Shift && !(e.KeyCode == Keys.Shift || 
                             e.KeyCode == Keys.ShiftKey ||
                             e.KeyCode == Keys.LShiftKey ||
                             e.KeyCode == Keys.RShiftKey ))
            {
                returnString += "+shift";
            }
            else if (e.Control && !(e.KeyCode == Keys.Control ||
                                    e.KeyCode == Keys.ControlKey ||
                                    e.KeyCode == Keys.LControlKey ||
                                    e.KeyCode == Keys.RControlKey))
            {
                returnString += "+ctrl";
            }
            else if (e.Alt && !(e.KeyCode == Keys.Alt ||
                                e.KeyCode == Keys.Menu ||
                                e.KeyCode == Keys.LMenu ||
                                e.KeyCode == Keys.RMenu))
            {
                returnString += "+alt";
            }
            e.Handled = true;

            configChanged = true;

            return returnString;
        }

        // NOTE:  The following event handler is required to handle special
        // keys like tab and enter which would otherwise be used by a form
        // to navigate through the controls on the form.
        /// <summary>
        /// overrides default behavior for special keys in the form
        /// </summary>
        /// <param name="e">key event details</param>
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
                case Keys.Escape:
                    e.IsInputKey = true;
                    break;
            }
        }

        /// <summary>
        /// Captures a mouse click and enters a string into the control
        /// </summary>
        /// <param name="e">mouse event details</param>
        /// <returns>a string representation of the mouse click</returns>
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

        /// <summary>
        /// Closes the form without saving
        /// </summary>
        /// <param name="sender">objec tthat initiated this event</param>
        /// <param name="e">event details</param>
        private void buttonCancel_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        /// <summary>
        /// Closes the form, saves settings and notifies user to restart the game
        /// </summary>
        /// <param name="sender">object that initiated this event</param>
        /// <param name="e">event details</param>
        private void buttonOK_Click(object sender, EventArgs e)
        {
            if (!checkForDuplicates() && !checkForUnset())
            {
                if (configChanged)
                {
                    MessageBox.Show("You must restart Meridian before these changes will take effect.", "Reload Game");
                    writeToConfigFile();
                }

                Application.Exit();
            }
        }

        /// <summary>
        /// Opens the help dialog
        /// </summary>
        /// <param name="sender">object that initiated this event</param>
        /// <param name="e">event details</param>
        private void buttonHelp_Click(object sender, EventArgs e)
        {
            HelpForm helpForm = new HelpForm();
            helpForm.ShowDialog(this);
        }

        #region Forward Keybind
        // Forward Keybind
        private void buttonForward_KeyDown(object sender, KeyEventArgs e)
        {
            buttonForward.Text = handleKeyDown(e);
            capturing = false;
        }

        private void buttonForward_MouseDown(object sender, MouseEventArgs e)
        {
            buttonForward.Text = handleMouseDown(e);
        }

        private void buttonForward_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Backward Keybind
        // Backward Keybind
        private void buttonBackward_KeyDown(object sender, KeyEventArgs e)
        {
            buttonBackward.Text = handleKeyDown(e);
        }

        private void buttonBackward_MouseDown(object sender, MouseEventArgs e)
        {
            buttonBackward.Text = handleMouseDown(e);
        }

        private void buttonBackward_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Turn Left Keybind
        // Turn Left Keybind
        private void buttonTurnLeft_KeyDown(object sender, KeyEventArgs e)
        {
            buttonTurnLeft.Text = handleKeyDown(e);
        }

        private void buttonTurnLeft_MouseDown(object sender, MouseEventArgs e)
        {
            buttonTurnLeft.Text = handleMouseDown(e);
        }

        private void buttonTurnLeft_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Turn Right Keybind
        // Turn Right Keybind
        private void buttonTurnRight_KeyDown(object sender, KeyEventArgs e)
        {
            buttonTurnRight.Text = handleKeyDown(e);
        }

        private void buttonTurnRight_MouseDown(object sender, MouseEventArgs e)
        {
            buttonTurnRight.Text = handleMouseDown(e);
        }

        private void buttonTurnRight_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Slide Left Keybind
        // Slide Left Keybind
        private void buttonSlideLeft_KeyDown(object sender, KeyEventArgs e)
        {
            buttonSlideLeft.Text = handleKeyDown(e);
        }

        private void buttonSlideLeft_MouseDown(object sender, MouseEventArgs e)
        {
            buttonSlideLeft.Text = handleMouseDown(e);
        }

        private void buttonSlideLeft_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Slide Right Keybind
        // Slide Right Keybind
        private void buttonSlideRight_KeyDown(object sender, KeyEventArgs e)
        {
            buttonSlideRight.Text = handleKeyDown(e);
        }

        private void buttonSlideRight_MouseDown(object sender, MouseEventArgs e)
        {
            buttonSlideRight.Text = handleMouseDown(e);
        }

        private void buttonSlideRight_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Run/Walk Keybind
        // Run/Walk Keybind
        private void buttonRunWalk_KeyDown(object sender, KeyEventArgs e)
        {
            buttonRunWalk.Text = handleKeyDown(e);
        }

        private void buttonRunWalk_MouseDown(object sender, MouseEventArgs e)
        {
            buttonRunWalk.Text = handleMouseDown(e);
        }

        private void buttonRunWalk_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Look Up Keybind
        // Look Up Keybind
        private void buttonLookUp_KeyDown(object sender, KeyEventArgs e)
        {
            buttonLookUp.Text = handleKeyDown(e);
        }

        private void buttonLookUp_MouseDown(object sender, MouseEventArgs e)
        {
            buttonLookUp.Text = handleMouseDown(e);
        }

        private void buttonLookUp_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Look Down Keybind
        // Look Down Keybind
        private void buttonLookDown_KeyDown(object sender, KeyEventArgs e)
        {
            buttonLookDown.Text = handleKeyDown(e);
        }
        
        private void buttonLookDown_MouseDown(object sender, MouseEventArgs e)
        {
            buttonLookDown.Text = handleMouseDown(e);
        }

        private void buttonLookDown_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Look Straight Keybind
        // Look Straight Keybind
        private void buttonLookStraight_KeyDown(object sender, KeyEventArgs e)
        {
            buttonLookStraight.Text = handleKeyDown(e);
        }

        private void buttonLookStraight_MouseDown(object sender, MouseEventArgs e)
        {
            buttonLookStraight.Text = handleMouseDown(e);
        }

        private void buttonLookStraight_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Flip Keybind
        // Flip Keybind
        private void buttonFlip_KeyDown(object sender, KeyEventArgs e)
        {
            buttonFlip.Text = handleKeyDown(e);
        }

        private void buttonFlip_MouseDown(object sender, MouseEventArgs e)
        {
            buttonFlip.Text = handleMouseDown(e);
        }

        private void buttonFlip_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Mouselook Toggle Keybind
        // Mouselook Toggle Keybind
        private void buttonMouselookToggle_KeyDown(object sender, KeyEventArgs e)
        {
            buttonMouselookToggle.Text = handleKeyDown(e);
        }

        private void buttonMouselookToggle_MouseDown(object sender, MouseEventArgs e)
        {
            buttonMouselookToggle.Text = handleMouseDown(e);
        }

        private void buttonMouselookToggle_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            previewKeySetIsInputKey(e);
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
        #endregion

        #region Say Keybind
        // Say Keybind
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

        private void buttonSayMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonSay);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }
        #endregion

        #region Chat Keybind
        // Chat Keybind
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

        private void buttonChatMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonChat);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }
        #endregion

        #region Tell Keybind
        // Tell Keybind
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

        private void buttonTellMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonTell);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }
        #endregion

        #region Yell Keybind
        // Yell Keybind
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

        private void buttonYellMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonYell);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }
        #endregion

        #region Broadcast Keybind
        // Broadcast Keybind
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

        private void buttonBroadcastMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonBroadcast);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }
        #endregion

        #region Who Keybind
        // Who Keybind
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

        private void buttonWhoMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonWho);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }
        #endregion

        #region Emote Keybind
        // Emote Keybind
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

        private void buttonEmoteMod_Click(object sender, EventArgs e)
        {
            KeyModForm modForm = new KeyModForm(buttonEmote);
            Point position = Cursor.Position;
            position.Y -= 115;

            modForm.StartPosition = FormStartPosition.Manual;
            modForm.Location = position;
            modForm.ShowDialog(this);
        }
        #endregion

        #region Open Keybind
        // Open Keybind
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
        #endregion

        #region Pick Up Keybind
        // Pick Up Keybind
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
        #endregion

        #region Look Keybind
        // Look Keybind
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
        #endregion

        #region Examine Keybind
        // Examine Keybind
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
        #endregion

        #region Offer Keybind
        // Offer Keybind
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
        #endregion

        #region Buy Keybind
        // Buy Keybind
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
        #endregion

        #region Deposit Keybind
        // Deposit Keybind
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
        #endregion

        #region Withdraw Keybind
        // Withdraw Keybind
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
        #endregion

        #region Attack Keybind
        // Attack Keybind
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
        #endregion

        #region Target Next Keybind
        // Target Next Keybind
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
        #endregion

        #region Target Previos Keybind
        // Target Previous Keybind
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
        #endregion

        #region Target Clear Keybind
        // Target Clear Keybind
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
        #endregion

        #region Target Self Keybind
        // Target Self Keybind
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
        #endregion

        #region Tab Forward Keybind
        // Tab Forward Keybind
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
        #endregion

        #region Tab Backward Keybind
        // Tab Backward Keybind
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
        #endregion

        #region Select Target Keybind
        // Select Target Keybind
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
        #endregion

        #region Map Keybind
        // Map Keybind
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
        #endregion

        #region Map Zoom In Keybind
        // Map Zoom In Keybind
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
        #endregion

        #region Map Zoom Out Keybind
        // Map Zoom Out Keybind
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
        #endregion

        #region Mouse Tab Controls
        // Mouse Tab Controls
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
        #endregion

        #region Options Group Controls
        // Options Group Controls
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
        #endregion
    }
}
