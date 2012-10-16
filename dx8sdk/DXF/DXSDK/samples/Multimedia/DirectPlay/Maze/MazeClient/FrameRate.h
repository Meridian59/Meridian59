//----------------------------------------------------------------------------
// File: 
//
// Desc: 
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef	_FRAMERATE_H
#define	_FRAMERATE_H




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class	CFrameRate
{
public:
	CFrameRate() { Reset(); };

	void	Reset()
	{
		m_dwCount = m_dwLastFrameCount = 0;
		m_fLastAppTime = DXUtil_Timer( TIMER_GETAPPTIME );
		m_fRate = 0;
	};

	void	DoneFrame()
	{
		m_dwCount++;
		FLOAT fCurTime = DXUtil_Timer( TIMER_GETAPPTIME );
		if ( (fCurTime - m_fLastAppTime) >= 1.0f )
		{
			m_fRate = (float(m_dwCount - m_dwLastFrameCount) / (fCurTime - m_fLastAppTime));
			m_fLastAppTime = fCurTime;
			m_dwLastFrameCount = m_dwCount;
		}
	};

	float	GetRate() const { return m_fRate; };
	DWORD	GetCount() const { return m_dwCount; };

protected:
	DWORD	m_dwCount;
	float	m_fRate;
	FLOAT   m_fLastAppTime;
	DWORD	m_dwLastFrameCount;
};




#endif
