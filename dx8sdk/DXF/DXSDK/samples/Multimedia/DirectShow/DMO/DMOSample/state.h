//------------------------------------------------------------------------------
// File: State.h
//
// Desc: DirectShow sample code - definition of CStreamState class.
//       Provides state machine for picture start codes and timestamps.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//  We need to know:
//
//    - The time stamp to associate with a picture start code
//    - The next time stamp
//
//  State:
//
//       m_cbBytes - number of valid bytes of start code so far
//       m_arTS[4] - array of all the timestamps
//                   m_cbBytes+1 entries are valid



//  Not really invalid but unlikely enough for sample code.
static const REFERENCE_TIME INVALID_TIME = _I64_MAX;


class CStreamState
{
private:

    DWORD m_cbBytes;
    struct {
        bool           bValid;
        REFERENCE_TIME rt;
    } m_arTS[4];
    REFERENCE_TIME m_rt;
	BYTE m_bGOPData[4];
    DWORD m_dwTimeCode;
	DWORD m_dwNextTimeCode;

public:
    CStreamState()
    {
        Reset();
    }
    //  Returns true if a start code was identifed
    bool NextByte(BYTE bData);
    void TimeStamp(REFERENCE_TIME rt);
    REFERENCE_TIME PictureTime(DWORD *pdwTimeCode) const
    {
		*pdwTimeCode = m_dwTimeCode;
        return m_rt;
    }
    void Reset();
};

