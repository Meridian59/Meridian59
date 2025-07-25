// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* signup.c:  Signup for a Meridian 59 account via the WebAPI.
*
*  A Player can signup for a game account from the Meridian 59 client signup dialog
*  interacting with the meridian59.com web api.
*/
#include "client.h"
#include <algorithm>
#include <vector>
#include <sstream>

using namespace std;

Signup* Signup::s_inst = NULL;

static INT_PTR CALLBACK SignUpDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void Signup::UpdateInputs(HWND hDlg, int enabled)
{
    EnableWindow(GetDlgItem(hDlg, IDC_SIGNUP_OK), enabled);
    EnableWindow(GetDlgItem(hDlg, IDC_SIGNUP_TROUBLESHOOT), enabled);
    EnableWindow(GetDlgItem(hDlg, IDC_NEW_USERNAME), enabled);
    EnableWindow(GetDlgItem(hDlg, IDC_NEW_PW1), enabled);
    EnableWindow(GetDlgItem(hDlg, IDC_NEW_PW2), enabled);
    EnableWindow(GetDlgItem(hDlg, IDC_NEW_EMAIL), enabled);
    EnableWindow(GetDlgItem(hDlg, IDC_SERVER_101), enabled);
    EnableWindow(GetDlgItem(hDlg, IDC_SERVER_102), enabled);
}

/**
* Helper function to style static dialog text in red to indicate an error case.
*/
INT_PTR ApplyErrorStyleToStaticText(UINT wParam, LONG lParam)
{
    HDC hdcStatic = (HDC)wParam;
    HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    DWORD CtrlID = GetDlgCtrlID((HWND)lParam);
    SetTextColor(hdcStatic, RGB(255, 0, 0));
    SetBkMode(hdcStatic, TRANSPARENT);
    return (INT_PTR)hBrush;
}

/*
 * EditSubclassProc to support select all on the signup edit fields.
 */
LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass,
                                  DWORD_PTR dwRefData)
{
   if (msg == WM_KEYDOWN)
   {
      if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'A')
      {
         SendMessage(hwnd, EM_SETSEL, 0, -1);
         return 0;  // handled
      }
   }

   return DefSubclassProc(hwnd, msg, wParam, lParam);
}

/*
* SignUpDialogProc: Manage the signup dialog window callbacks.
*/
INT_PTR CALLBACK SignUpDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hUsername, hPasswd1, hPasswd2, hEmail, hServer101, hServer102;

    Signup* su = Signup::GetInstance();

    if (!su)
        return false;

    switch (message)
    {
    case WM_INITDIALOG:

        CenterWindow(hDlg, GetParent(hDlg));

        hUsername = GetDlgItem(hDlg, IDC_NEW_USERNAME);
        hPasswd1 = GetDlgItem(hDlg, IDC_NEW_PW1);
        hPasswd2 = GetDlgItem(hDlg, IDC_NEW_PW2);
        hEmail = GetDlgItem(hDlg, IDC_NEW_EMAIL);
        hServer101 = GetDlgItem(hDlg, IDC_NEW_EMAIL);
        hServer102 = GetDlgItem(hDlg, IDC_NEW_EMAIL);

        SetWindowFont(hUsername, GetFont(FONT_INPUT), FALSE);
        SetWindowFont(hPasswd1, GetFont(FONT_INPUT), FALSE);
        SetWindowFont(hPasswd2, GetFont(FONT_INPUT), FALSE);
        SetWindowFont(hEmail, GetFont(FONT_INPUT), FALSE);

        Edit_LimitText(hUsername, MAXUSERNAME);
        Edit_LimitText(hPasswd1, MAXPASSWORD);
        Edit_LimitText(hPasswd2, MAXPASSWORD);
        Edit_LimitText(hEmail, MAXEMAIL);

        SetWindowSubclass(hUsername, EditSubclassProc, 0, 0);
        SetWindowSubclass(hPasswd1, EditSubclassProc, 0, 0);
        SetWindowSubclass(hPasswd2, EditSubclassProc, 0, 0);
        SetWindowSubclass(hEmail, EditSubclassProc, 0, 0);

        su->SetWebApiResponse(0);

        if (GetFocus() == hEmail)
        {
            return 1;
        }
        else
        {
            SetFocus(hEmail);
            CheckDlgButton(hDlg, IDC_SERVER_101, TRUE);
            return 0;
        }
        break;
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDOK:
            return TRUE;
        case IDC_SIGNUP_TROUBLESHOOT:
        {
            char url[256];
            LoadString(hInst, IDS_TROUBLESHOOT, url, sizeof(url));
            if (*url)
            {
                WebLaunchBrowser(url);
                EndDialog(hDlg, IDCANCEL);
                PostMessage(hMain, WM_SYSCOMMAND, SC_CLOSE, 0);
            }
            return TRUE;
        }
        break;
        case IDC_SIGNUP_OK:
        {
            // disable buttons to prevent multiple requests.
            su->UpdateInputs(hDlg, FALSE);

            // make web api request.
            char domain[256];
            LoadString(hInst, IDS_WEBAPIDOMAIN, domain, sizeof(domain));
            char resource[256];
            LoadString(hInst, IDS_SIGNUPAPI, resource, sizeof(resource));
            wstring response;

            // New account detail holders used to pass to the web api.
            char username[MAXUSERNAME + 1];
            char email[MAXEMAIL + 1];
            char password1[MAXPASSWORD + 1];
            char password2[MAXPASSWORD + 1];
            int  server = 101;

            // Get the new account details.
            Edit_GetText(GetDlgItem(hDlg, IDC_NEW_USERNAME), username, MAXUSERNAME + 1);
            Edit_GetText(GetDlgItem(hDlg, IDC_NEW_PW1), password1, MAXPASSWORD + 1);
            Edit_GetText(GetDlgItem(hDlg, IDC_NEW_PW2), password2, MAXPASSWORD + 1);
            Edit_GetText(GetDlgItem(hDlg, IDC_NEW_EMAIL), email, MAXEMAIL + 1);

            // Determine the users choice of server.
            if (!SendDlgItemMessage(hDlg, IDC_SERVER_101, BM_GETCHECK, 0, 0))
            {
                server = 102;
            }

            // Build http post body with user input values.
            stringstream ss;
            ss << "submit=1";
            ss << "&username=" << username;
            ss << "&email=" << email;
            ss << "&pw1=" << password1;
            ss << "&pw2=" << password2;
            ss << "&server=" << server;

            char errorStr[256];

            // Request can take a long time; show waiting cursor
            SetMainCursor(LoadCursor(NULL, IDC_WAIT));
            bool success = SendHttpsRequest(hDlg, domain, resource, ss.str(), response);
            SetMainCursor(LoadCursor(NULL, IDC_ARROW));
            
            if (success)
            {
                string dStr(response.begin(), response.end());
                char* text = const_cast<char *>(dStr.c_str());

                char* pEnd = nullptr;
                long webResponse = strtol(text, &pEnd, 10);

                if (pEnd)
                {
                    // successfully parsed web api response.
                    su->SetWebApiResponse(webResponse);
                }
                else
                {
                    // web request returned invalid numerical response (server side error)
                    su->SetWebApiResponse(1);
                    su->UpdateInputs(hDlg, TRUE);
                }

                if ((SignUpWebResponse::SUCCESS == su->GetWebApiResponse()))
                {
                    char username[MAXUSERNAME + 1];
                    char email[MAXEMAIL + 1];
                    Edit_GetText(GetDlgItem(hDlg, IDC_NEW_USERNAME), username, MAXUSERNAME + 1);
                    Edit_GetText(GetDlgItem(hDlg, IDC_NEW_EMAIL), email, MAXEMAIL + 1);

                    // Display account successfully created message.
                    ClientError(hInst, hDlg, IDS_SIGNUPMSG, username, email);

                    // Close the create account window.
                    EndDialog(hDlg, IDCANCEL);
                }
                else
                {
                    // Re-enable inputs.
                    su->UpdateInputs(hDlg, TRUE);

                    // Refresh static text colors.
                    InvalidateRect(hDlg, NULL, TRUE);

                    // Determine the errors and build list description for players.
                    stringstream ss;
                    for (int errorCaseIt = SignUpWebResponse::ERROR_INVALIDUSERNAME; errorCaseIt != SignUpWebResponse::ERROR_FAILED; ++errorCaseIt)
                    {
                        SignUpWebResponse errorCase = static_cast<SignUpWebResponse>(errorCaseIt);
                        if ((errorCase & su->GetWebApiResponse()))
                        {
                            int errorMessage = 0;

                            switch (errorCase)
                            {
                            case SignUpWebResponse::ERROR_INVALIDUSERNAME:
                                errorMessage = IDS_USERNAMEINVALID;
                                break;
                            case SignUpWebResponse::ERROR_INVAILDEMAIL:
                                errorMessage = IDS_EMAILINVALID;
                                break;
                            case SignUpWebResponse::ERROR_EMAILINUSE:
                                errorMessage = IDS_EMAILINUSE;
                                break;
                            case SignUpWebResponse::ERROR_PASSWORDSINVALID:
                                errorMessage = IDS_PASSWORDINVALID;
                                break;
                            case SignUpWebResponse::ERROR_INVALIDSERVER:
                                errorMessage = IDS_SERVERUNKNOWN;
                                break;
                            case SignUpWebResponse::ERROR_OFFLINE:
                                errorMessage = IDS_SIGNUPOFFLINE;
                                break;
                            case SignUpWebResponse::ERROR_FAILED:
                                errorMessage = IDS_SIGNUPUNKNOWNERROR;
                                break;
                            default:
                                // unknown error
                                break;
                            }

                            LoadString(hInst, errorMessage, errorStr, sizeof(errorStr));
                            if (errorMessage && errorStr)
                            {
                                ss << " - ";
                                ss << errorStr;
                            }
                        }
                    }
                    ClientError(hInst, hDlg, IDS_SIGNUPFAILED, ss.str().c_str());
                }
            }
            else
            {
              // Failed to send http request, show client error popup message.
              std::stringstream os;
              os << GetLastError();
              ClientError(hInst, hDlg, IDS_CANTSIGNUP, os.str().c_str());

              su->UpdateInputs(hDlg, TRUE);
            }
            return TRUE;
        }
        break;
        case IDC_HANGUP:
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        return FALSE;
        break;
    case WM_CTLCOLORSTATIC:
    {
        // Color relevant fields red to indicate errors.
        switch (GetDlgCtrlID((HWND)lParam))
        {
        case ID_SU_EMAIL:
        {
            if ((SignUpWebResponse::ERROR_INVAILDEMAIL & su->GetWebApiResponse()) || (SignUpWebResponse::ERROR_EMAILINUSE & su->GetWebApiResponse()))
            {
                return ApplyErrorStyleToStaticText(wParam, lParam);
            }
            break;
        }
        case ID_SU_USERNAME:
        {
            if ((SignUpWebResponse::ERROR_INVALIDUSERNAME & su->GetWebApiResponse()))
            {
                return ApplyErrorStyleToStaticText(wParam, lParam);
            }
            break;
        }
        case ID_SU_PW1:
        case ID_SU_PW2:
        {
            if ((SignUpWebResponse::ERROR_PASSWORDSINVALID & su->GetWebApiResponse()))
            {
                return ApplyErrorStyleToStaticText(wParam, lParam);
            }
            break;
        }
        case ID_SU_SERVER:
        {
            if ((SignUpWebResponse::ERROR_INVALIDSERVER & su->GetWebApiResponse()))
            {
                return ApplyErrorStyleToStaticText(wParam, lParam);
            }
            break;
        }
        }
        return FALSE;
    }
    break;
    case BK_DIALOGDONE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }

    return FALSE;
};

bool Signup::GetSignUp()
{
    return DialogBox(hInst, MAKEINTRESOURCE(IDD_SIGNUP), hMain, SignUpDialogProc) == IDCANCEL;
};
