// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * web.c:  Handle interface to Web browser.
 *
 */

#include "client.h"
#include <sstream>

using namespace std;

/************************************************************************/
/*
 * WebLaunchBrowser:  Attempt to run browser on given URL.
 */
void WebLaunchBrowser(const char *url)
{
    if (url == nullptr || strlen(url) == 0) {
        debug(("URL string empty\n"));
        return;
    }

    std::string urlStr(url);

    if (ContainsSpaces(urlStr)) {
        debug(("URL contains spaces\n"));
        return;
    }

    if (urlStr.find("https://") != 0 && urlStr.find("https://") != 0) {
        urlStr = "https://" + urlStr;
    }

    HINSTANCE result = ShellExecute(NULL, "open", urlStr.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)result <= 32) {
        debug(("Failed to open URL\n"));
        return;
    }
}

bool ContainsSpaces(const std::string& str)
{
    return str.find(' ') != std::string::npos;
}

// convert c string to wstring
wstring CharPToWstring(const char* characterPtr)
{
    return wstring(characterPtr, characterPtr + strlen(characterPtr));
}

bool SendHttpsRequest(HWND hDlg, const string& domain, const string& resource, const string& requestBody, wstring& response)
{
    HINTERNET internetSession = ::InternetOpen("Meridian59 Signup Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

    HINTERNET httpSession = ::InternetConnect(internetSession, domain.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0, 0, INTERNET_SERVICE_HTTP, 0, NULL);

    HINTERNET httpRequest = ::HttpOpenRequest(
        httpSession,
        "POST",
        resource.c_str(),
        0, 0, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);

    stringstream httpHeaderStream;
    httpHeaderStream << "Content-Type: application/x-www-form-urlencoded\n";
    httpHeaderStream << "Client-Version: ";
    httpHeaderStream << VERSION_NUMBER(MAJOR_REV, MINOR_REV);

    bool result = false;

    if (::HttpSendRequest(httpRequest, httpHeaderStream.str().c_str(),
                          (DWORD) httpHeaderStream.str().length(),
                          (LPVOID)(requestBody.c_str()),
                          (DWORD) strlen(requestBody.c_str())))
    {
        result = true;
        char sBuffer[1025];
        DWORD dwRead = 0;
        while (::InternetReadFile(httpRequest, sBuffer, sizeof(sBuffer) - 1, &dwRead) && dwRead) {
            sBuffer[dwRead] = 0;
            response = response + CharPToWstring(sBuffer);
            dwRead = 0;
        }
    }

    ::InternetCloseHandle(httpRequest);
    ::InternetCloseHandle(httpSession);
    ::InternetCloseHandle(internetSession);

    return result;
}
