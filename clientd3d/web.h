// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * web.h:  Header file for web.c
 */

#ifndef _WEB_H
#define _WEB_H

#include <string>

enum SignUpWebResponse
{

    // Outcome of a web api signup (0 or 1 issues).
    SUCCESS = 0,
    FAILED = 1 << 0,

    // Specific signup issue flags.
    ERROR_INVALIDUSERNAME = 1 << 1,
    ERROR_INVAILDEMAIL = 1 << 2,
    ERROR_EMAILINUSE = 1 << 3,
    ERROR_PASSWORDSINVALID = 1 << 4,
    ERROR_INVALIDSERVER = 1 << 5,
    ERROR_OFFLINE = 1 << 6,
    ERROR_FAILED = 1 << 7
};

enum AccountStatusWebResponse
{
    // Out of a web api to check if an bad login account is due to be unverified
    OK = 0,
    VERIFY = 1 << 0,
    FAILURE = 1 << 2
};

void WebLaunchBrowser(char *url);

// Send a HTTPS request to a given domain and resource.
bool SendHttpsRequest(HWND hDlg, const std::string& domain, const std::string& resource, const std::string& requestBody, std::wstring& response);

#endif /* #ifndef _WEB_H */
