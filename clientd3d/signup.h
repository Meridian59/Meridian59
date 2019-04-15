// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* signup.h:  Header file for signup.c
*/
#ifndef _SIGNUP_H
#define _SIGNUP_H

#include "client.h"

class Signup
{
public:

    // Start the Signup dialog.
    bool GetSignUp();

    // Enable/Disable the input fields on the Signup dialog
    // @param enabled enable all the fields using TRUE and disable with FALSE.
    void UpdateInputs(HWND hDlg, int enabled);

    // singleton access to the Signup class.
    static Signup* GetInstance()
    {
        if (s_inst == nullptr)
            s_inst = new Signup();
        return(s_inst);
    }

    long GetWebApiResponse()
    {
        return m_webApiResponse;
    }

    void SetWebApiResponse(long newWebApiResponseValue)
    {
        m_webApiResponse = newWebApiResponseValue;
    }

private:
    static Signup* s_inst;
    Signup() {};
    ~Signup() { delete s_inst; }

    long m_webApiResponse = 0;
};

#endif /* #ifndef _SIGNUP_H */