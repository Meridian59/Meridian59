#ifndef _WAVEMIX_H
#define _WAVEMIX_H

#define MAXFILENAME 15 
typedef struct
{
	PCMWAVEFORMAT	pcm;
	WAVEHDR			wh;
	char			szWaveFilename[MAXFILENAME+1];
}
MIXWAVE, *LPMIXWAVE;

/* flag values for play params */
#define WMIX_QUEUEWAVE 0x00
#define WMIX_CLEARQUEUE  0x01
#define WMIX_USELRUCHANNEL 0x02
#define WMIX_HIPRIORITY 0x04
#define WMIX_HIGHPRIORITY WMIX_HIPRIORITY
#define WMIX_WAIT 0x08


typedef struct
{
   WORD wSize;
   HANDLE hMixSession;
   int iChannel;
   LPMIXWAVE lpMixWave;
   HWND hWndNotify;
   DWORD dwFlags;
   WORD wLoops;  /* 0xFFFF means loop forever */
} MIXPLAYPARAMS, * PMIXPLAYPARAM, *LPMIXPLAYPARAMS;

typedef struct
{
   WORD wSize;
   BYTE bVersionMajor;
   BYTE bVersionMinor;
   char szDate[12]; /* Mmm dd yyyy */
   DWORD dwFormats; /* see waveOutGetDevCaps (wavemix requires synchronous device) */
} WAVEMIXINFO, *PWAVEMIXINFO, *LPWAVEMIXINFO;

#define WMIX_CONFIG_CHANNELS 0x1
#define WMIX_CONFIG_SAMPLINGRATE 0x2
typedef struct
{
   WORD wSize;
   DWORD dwFlags;
   WORD wChannels;  /* 1 = MONO, 2 = STEREO */
   WORD wSamplingRate; /* 11,22,44  (11=11025, 22=22050, 44=44100 Hz) */
} MIXCONFIG, *PMIXCONFIG, *LPMIXCONFIG;

// Constants for volume added 8/95 ARK
#define MAX_VOLUME 64
#define LOG_MAX_VOLUME 6


WORD 		WINAPI WaveMixGetInfo(LPWAVEMIXINFO lpWaveMixInfo);

/* return same errors as waveOutOpen, waveOutWrite, and waveOutClose */
HANDLE	WINAPI WaveMixInit(void); /* returns hMixSession - you should keep it and use for subsequent API calls */
HANDLE	WINAPI WaveMixConfigureInit(LPMIXCONFIG lpConfig);
UINT		WINAPI WaveMixActivate(HANDLE hMixSession, BOOL fActivate);

#define WMIX_FILE			0x0001
#define WMIX_RESOURCE	0x0002
#define WMIX_MEMORY		0x0004
LPMIXWAVE	WINAPI WaveMixOpenWave(HANDLE hMixSession, LPSTR szWaveFilename, HINSTANCE hInst, DWORD dwFlags, int volume);

#define WMIX_OPENSINGLE 0	/* open the single channel specified by iChannel */
#define WMIX_OPENALL 1 /* opens all the channels, iChannel ignored */
#define WMIX_OPENCOUNT 2 /* open iChannel Channels (eg. if iChannel = 4 will create channels 0-3) */
UINT		WINAPI WaveMixOpenChannel(HANDLE hMixSession, int iChannel, DWORD dwFlags);

UINT		WINAPI WaveMixPlay(LPMIXPLAYPARAMS lpMixPlayParams);

#define WMIX_ALL     	0x0001 /* stops sound on all the channels, iChannel ignored */
#define WMIX_NOREMIX 	0x0002 /* prevents the currently submited blocks from being remixed to exclude new channel */
UINT		WINAPI WaveMixFlushChannel(HANDLE hMixSession, int iChannel, DWORD dwFlags);
UINT		WINAPI WaveMixCloseChannel(HANDLE hMixSession, int iChannel, DWORD dwFlags);

UINT		WINAPI WaveMixFreeWave(HANDLE HMixSession, LPMIXWAVE lpMixWave);
UINT		WINAPI WaveMixCloseSession(HANDLE hMixSession);
void		WINAPI WaveMixPump(void);

/* Windowsx.h style message handlers (copied from mixtest 6/95 ARK) */

/* void Cls_OnMM_WOM_DONE(HWND hwnd, int iChannel, LPMIXWAVE lpMixWave); */
#define HANDLE_MM_WOM_DONE(hwnd, wParam, lParam, fn) \
((fn)((hwnd), (int)(wParam), (LPMIXWAVE)(lParam)), 0L)
#define FORWARD_MM_WOM_DONE(hwnd, bCommand, lpNCB, fn) \
    (void)(fn)((hwnd), WM_MM_WOM_DONE, (WPARAM)(iChannel),(LPARAM) lpMixWave)

#endif /* _WAVEMIX_H */
