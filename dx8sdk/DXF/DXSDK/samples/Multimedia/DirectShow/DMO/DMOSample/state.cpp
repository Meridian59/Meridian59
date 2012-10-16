//------------------------------------------------------------------------------
// File: State.cpp
//
// Desc: DirectShow sample code - implementation of CStreamState class.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#include <limits.h>     //  _I64_MAX
#include <crtdbg.h>
#include <dmo.h>
#include "state.h"

void CStreamState::TimeStamp(REFERENCE_TIME rt)
{
	DWORD dwIndex = m_cbBytes >= 4 ? 0 : m_cbBytes;
	m_arTS[dwIndex].bValid = true;
	m_arTS[dwIndex].rt = rt;
}

void CStreamState::Reset()
{
    m_cbBytes = 0;
	m_dwNextTimeCode = 0;
    for (int i = 0; i < 4; i++) {
        m_arTS[i].bValid = false;
    }
}

bool CStreamState::NextByte(BYTE bData)
{
    _ASSERTE(m_arTS[0].bValid);
    switch (m_cbBytes) {
    case 0:
        if (bData == 0) {
            m_cbBytes++;
        }
        return false;
    case 1:
        if (bData == 0) {
            m_cbBytes++;
        } else {
            m_cbBytes = 0;

            //  Pick up new timestamp if there was one
            if (m_arTS[1].bValid) {
                m_arTS[0].rt = m_arTS[1].rt;
                m_arTS[1].bValid = false;
            }
        }
        return false;
    case 2:
        if (bData == 1) {
            m_cbBytes++;
        } else {
            if (bData == 0) {
               if (m_arTS[1].bValid) {
                   m_arTS[0].rt = m_arTS[1].rt;
                   m_arTS[1].bValid = false;
               }
               if (m_arTS[2].bValid) {
                   m_arTS[1] = m_arTS[2];
                   m_arTS[2].bValid = false;
               }
            } else {
                //  Not 0 or 1, revert
                m_cbBytes = 0;
                //  and pick up latest timestamp
                if (m_arTS[1].bValid) {
                    m_arTS[0].rt = m_arTS[1].rt;
                    m_arTS[1].bValid = false;
                }
                if (m_arTS[2].bValid) {
                    m_arTS[0].rt = m_arTS[2].rt;
                    m_arTS[2].bValid = false;
                }
            }
        }
        return false;
    case 3:
	{
		//  It's a start code whatever it is
		//  return the timestamp and reset everything
		m_rt = m_arTS[0].rt;

        //  If it's a picture start code can't use this timestamp again.
		if (0 == bData) {
            m_arTS[0].rt = INVALID_TIME;
			m_cbBytes = 0;
		}

        //  Catch up and clean out timestamps
        for (int i = 1; i < 4; i++) {
            if (m_arTS[i].bValid) {
                m_arTS[0].rt = m_arTS[i].rt;
                m_arTS[i].bValid = false;
            }
        }

		// Picture start code?
		if (0 == bData) {
            m_cbBytes = 0;
			m_dwTimeCode = m_dwNextTimeCode;
			m_dwNextTimeCode++;
			return true;
		} else {
			//  Is it a GOP start code?
			if (bData == 0xb8) {
				m_cbBytes++;
			} else {
				m_cbBytes = 0;
			}
            return false;
		}
	}

	case 4:
	case 5:
	case 6:
	case 7:
        m_bGOPData[m_cbBytes - 4] = bData;
		m_cbBytes++;
		if (m_cbBytes == 8) {
			m_cbBytes = 0;
			m_dwNextTimeCode = ((DWORD)m_bGOPData[0] << 17) +
                               ((DWORD)m_bGOPData[1] << 9) +
                               ((DWORD)m_bGOPData[2] << 1) +
                               ((DWORD)m_bGOPData[3] >> 7);
		}
		return false;
    }

	// Should never reach here
	return false;
};
