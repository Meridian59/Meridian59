/* WaveMix.C - A program to mix multiple input wave files in real time
**
** Designed and developed by Microsoft, Spring 1993
**
** History:
**   12/18/93: put if (gfShow) before all the message boxes, incremented minor version to 1.1
**   12/30/93: added stereo support, incremented minor version to 1.5
**     4/2/94: fixed bug with freeing wave channels, added return iChannel when playing LRU
**    6/12/94: add code to require DLL to be in system directory and ini file in Windows dir.
**    6/12/94: add code to cause WaveMixPlay to fail if not "Active"
**    8/13/95: Removed tons of superfluous garbage.  Removed requirement that DLL is
**             in system directory and INI file in Windows directory.  ARK
*/ 

#include <windows.h>
#include <mmsystem.h>
#include <windowsx.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include "wavemix.h"

#define MAJORVERSION 0x02
#define MINORVERSION 0x00

#define DEBUGOUTPUT 0

#if DEBUG
#include <assert.h>
#define Assert(exp) assert(exp)
#else
#define assert(exp)
#define Assert(exp)
#endif

#define INLINE __inline

#if DEBUGOUTPUT
#define ODS(n,s) MyODS(n,s)
#else
#define ODS(n,s) do {} while(0)
#endif

#define PEG(min,x,max) {if(x<min) x=min; else if (x>max) x=max;}

#define SILENCE (0x80)
#define MAXCHANNELS 8
#define MINWAVEBLOCKS 2
#define MAXWAVEBLOCKS 6
#define BYTESPERSAMPLE 1
#define BITSPERSAMPLE 8
#define BITSPERBYTE 8
#define MONO 1
#define MINWAVEBLOCKLEN 512
#define MAXWAVEBLOCKLEN 4096
#define SAMPLESPERSEC (11025)
#define SAMPLESPERBLOCK (g->dwWaveBlockLen/BYTESPERSAMPLE)
#define MAXQUEUEDWAVES 100
#define MAGICNO 0x5432

typedef unsigned char *LPSAMPLE;
typedef short int     *LPSAMPLE16;
typedef short int     *HPINT;

typedef struct _CHANNELNODE
{
   struct _CHANNELNODE * next;
   MIXPLAYPARAMS PlayParams;
   LPMIXWAVE lpMixWave;
   DWORD dwNumSamples;
   DWORD   dwStartPos; // this is the MyWaveOutGetPosition() or dwEndPos+1 of previous wave depending on whether high priority or getting queued
   DWORD   dwEndPos;   // this is the end pos: dwStartPos+dwNumSamples*(1+numloops)
   LPSAMPLE lpPos;         // this is pointer to current position within data, with current mixer play is equal to wh.lpData
   LPSAMPLE lpEnd;         // this is pointer to last sample of actual wave data
} CHANNELNODE, * PCHANNELNODE, *LPCHANNELNODE;

typedef struct 
{
   WORD wMagic1;
   HWND hWndApp; //=NULL;
   BOOL fShow;
   HWAVEOUT hWaveOut;
   BOOL fActive; //=FALSE;
   UINT wDeviceID;  //=(UINT)WAVE_MAPPER;
   char szDevicePName[MAXPNAMELEN];
   PCHANNELNODE aChannel[MAXCHANNELS];
   int iChannels; //=0;
   DWORD MRUChannel[MAXCHANNELS];
   DWORD dwMRU;
   PCMWAVEFORMAT pcm;
   DWORD dwWaveBlockLen;
   int iNumWaveBlocks;
   
   DWORD dwCurrentSample;
   DWORD dwBaseTime;
   
   BOOL fGoodGetPos; // =TRUE;
   DWORD dwWaveOutPos;
   BOOL (*pfnRemix)(DWORD dwRemixSamplePos, PCHANNELNODE pCD);  // so can use two different remix() depending on wave out device
   DWORD (*pfnSampleAdjust)(DWORD dwSamplePos, DWORD dwAdjustAmt);  // so can use two different remix() depending on wave out device
   PCHANNELNODE pWaitList; /* points to last node in wait list, last node points to first node */
   //BUGBUG: do I want to hand WaveMixFlush??? No for now
   WORD wMagic2;
} GLOBALS, *PGLOBALS, *LPGLOBALS;

typedef struct _XWAVEHDR
{
   WAVEHDR wh;  /* NOTE: wh must be first field in structure since pointer will get passed as WAVEHDR to waveOut functions */
   BOOL fAvailable;
   DWORD dwWavePos;
   PGLOBALS g;
   struct _XWAVEHDR *QNext;
} XWAVEHDR, *PXWAVEHDR, *LPXWAVEHDR;

typedef struct _PLAYQUEUE
{
   LPXWAVEHDR first;
   LPXWAVEHDR last;
} PLAYQUEUE, *PPLAYQUEUE, *LPPLAYQUEUE;


const PCMWAVEFORMAT gpFormat = 
{
{WAVE_FORMAT_PCM, MONO, SAMPLESPERSEC, SAMPLESPERSEC * BYTESPERSAMPLE * MONO, BYTESPERSAMPLE},
BITSPERSAMPLE
};

/* function prototypes for forward reference */
BOOL MixerPlay(LPXWAVEHDR lpXWH, BOOL fWriteBlocks);
DWORD MyWaveOutGetPosition(HWAVEOUT hWaveOut, BOOL fGoodGetPos);
INLINE void MyWaveOutReset(HWAVEOUT hWaveOut);

/* global variables! */
BOOL gfShow=FALSE;
int giDebug=0;
const char gszAppName[] = "WavMix32";
const char gszIniFile[_MAX_PATH+MAXFILENAME] = "";  // set in WaveMixInstalledCorrectly();
const char gszDefault[] = "default";
const char gszMemError[] = "Unable to allocate memory for waveform data.  Try making more memory available by closing other applications.";
HINSTANCE ghDLLInst;
LPXWAVEHDR   gaWaveBlock[MAXWAVEBLOCKS];
PLAYQUEUE gPlayQueue = {NULL,NULL};
CHANNELNODE  gaChannelNodes[MAXQUEUEDWAVES];
PCHANNELNODE gpFreeChannelNodes;

/* have this function pointer global since only want to set at lib main */
void (*gpfnMixit)(LPSAMPLE lpDest, LPSAMPLE rgWaveSrc[],int iNumWaves, WORD wLen);

char gsz[256];  //general purpose buffer for preparing strings

PGLOBALS g,gActiveSession;

/* code */

#if DEBUGOUTPUT
/* level  1 = most important
**	  2 = more detailed, less important
**        3 = highest detail, least important
*/
void MyODS(int level, LPSTR sz)
{
#if 0
   if (giDebug && level <= giDebug) 
      OutputDebugString(sz);
#else
   if (giDebug && level <= giDebug) 
   {
#define LINES 5
      
      static int tmHeight=15;
      HDC hDC=GetDC(NULL);
      
      if (!tmHeight)
      {
	 TEXTMETRIC tm;
	 GetTextMetrics(hDC,&tm);
	 tmHeight=tm.tmHeight;
      }
      
      BitBlt(hDC,0,0,200,(LINES-1)*tmHeight,hDC,0,tmHeight,SRCCOPY); /* scroll window */
      PatBlt(hDC,0,(LINES-1)*tmHeight,200,15,WHITENESS); /* erase last line */
      TextOut(hDC,0,(LINES-1)*tmHeight,sz,lstrlen(sz));
      ReleaseDC(NULL,hDC);
   }
#endif
}
#endif

INLINE PGLOBALS SessionToGlobalDataPtr(HANDLE hMixSession)
{
   PGLOBALS pG = (PGLOBALS)hMixSession;
   
   if (!pG || pG->wMagic1 != MAGICNO || pG->wMagic2 != MAGICNO)
   {
//      MessageBeep((UINT)-1);
      wsprintf(gsz,"Invalid session handle 0x%04X",hMixSession);
      ODS(1,gsz);
      return NULL;
   }
   else
      return pG;
}

DWORD MyWaveOutGetPosition(HWAVEOUT hWaveOut, BOOL fGoodGetPos)
// NOTE: this function used to use the code to call waveOutGetPosition or
//       interplotate from the last known wave done message and time to get
//       a more accurate wave out get position.
// overall I found that code to be buggy and error prone and not as effective
// as just keeping a linear time that never resets.  (although I do reset the
// g->dwBaseTime when there is no wave data playing to ensure that I will never
// overflow my time buffer.
{
   MMTIME mmt;
   DWORD t1;
   
   Assert(g);
   
   if (fGoodGetPos)
   {
      mmt.wType = TIME_BYTES;
      waveOutGetPosition(hWaveOut,&mmt,sizeof(MMTIME));
      return g->pfnSampleAdjust(mmt.u.sample,g->dwWaveOutPos);
   }
   else
   {
      // this code is potentially not that accurate.  If a sound board does not play
      // at exactly the sampling rate.  I think for the most part thought that the
      // drift will be negligible, especially for short sounds.  If this is not the
      // case then this function will have to be made more accurate.
      // animation)
      ODS(3,"{mywaveoutgetpos !fGoodGetPos}");
      Assert(g->dwBaseTime <= timeGetTime());
      t1=timeGetTime()-g->dwBaseTime;
      t1=t1*g->pcm.wf.nAvgBytesPerSec/1000;  // may need to and with 0xfffffffc  to align channels
      if (g->pcm.wf.nBlockAlign>1 && t1 & 1)  // return values must be block aligned or will screw up stereo
	 t1--;
      return t1;
   }
}

DWORD AddFactor(DWORD dwSample, DWORD dwAdjustAmt)
{
   ODS(2,"Add");
   return dwSample+dwAdjustAmt;
}

DWORD SubFactor(DWORD dwSample, DWORD dwAdjustAmt)
{
   ODS(2,"Sub");
   Assert(dwSample >= dwAdjustAmt);
   return dwSample-dwAdjustAmt;
}

void SetWaveOutPosition(DWORD dwSamplePos)
{
   MMTIME mmt;
   
   Assert(g);
   
#if DEBUGOUTPUT
   ODS(2,"SetWaveOutPositon");
   wsprintf(gsz,"=%lu",dwSamplePos);
   ODS(3,gsz);
#endif
   
   /*  for fGoodGetPos */
   mmt.wType = TIME_BYTES;
   if (g->hWaveOut)
   {
      waveOutGetPosition(g->hWaveOut,&mmt,sizeof(MMTIME));
      mmt.u.sample = mmt.u.sample;
   }
   else
      mmt.u.sample=0;
   
   if (mmt.u.sample >= dwSamplePos)
   {
      g->dwWaveOutPos = mmt.u.sample-dwSamplePos;
      g->pfnSampleAdjust = SubFactor;
   }
   else
   {
      g->dwWaveOutPos=dwSamplePos-mmt.u.sample;
      g->pfnSampleAdjust = AddFactor;
   }
   
   /* for fGoodGetPos=FALSE */
   
   // this method is subject to drift if the sound card does not play at 
   // exactly 11025 Hz.
   g->dwBaseTime=timeGetTime()-(dwSamplePos*1000)/g->pcm.wf.nAvgBytesPerSec;
   // bugbug: if stereo gets screwed up may need to force dwBaseTime to be block aligned
   
   g->dwCurrentSample = dwSamplePos;
}

INLINE void MyWaveOutReset(HWAVEOUT hWaveOut)
{
   // if don't do this, this will screw up the current pos time in channel blocks.
   DWORD dwTmp;
   
   Assert(g);
   Assert(hWaveOut);
   dwTmp = MyWaveOutGetPosition(hWaveOut,g->fGoodGetPos);
   waveOutReset(hWaveOut);
   SetWaveOutPosition(dwTmp);
}


/* cmixit: function that actually mixes wave data.  It must be 286 compatible 
**
*/
void cmixit(LPSAMPLE lpDest, LPSAMPLE rgWaveSrc[], int iNumWaves, WORD wLen)
{
#if DEBUG
{
   int i;
   
   if (!iNumWaves)
   {
      ODS(1,"{Error!!! Mixit called with iNumWaves==0}");
   }
   if (!rgWaveSrc)
   {
      ODS(1,"{Error!!! Mixit called with a NULL array}");
   }
   if (!wLen)
   {
      ODS(1,"{Error!!! Mixit called with a wLen==0}");
   }
   if (!lpDest)
   {
      ODS(1,"{Error!!! Mixit called with a NULL lpDest}");
   }
   for (i=0;i<iNumWaves;i++)
      if (!rgWaveSrc[i])
      {
	 ODS(1,"{Error!!! Mixit called with a NULL pointer in wave array}");
      }
}
#endif

   /* special case ctr == 1: there is only one channel with data
   ** so just copy it directly to the wave buffer
   */
   if (iNumWaves==1)
      memcpy(lpDest,rgWaveSrc[0],wLen);
   else /* iNumWaves >= 2 */
   {
      // BUGBUG: this code should be optimized but should remain in C
      int i,iSum;
      WORD ctr;
      ctr = 0;
      while (wLen)
      {
	 iSum=128;
	 for (i=0;i<iNumWaves;i++)
	    iSum = iSum + *(rgWaveSrc[i]+ctr) -128;
	 PEG((int)0,iSum,(int)255);
	 *lpDest++=iSum;
	 ctr++;
	 wLen--;
      }
   }
}

void InitChannelNodes(void)
{
   int i;
   
   for (i=0;i<MAXQUEUEDWAVES-2;i++)
      gaChannelNodes[i].next = &gaChannelNodes[i+1];
   gaChannelNodes[MAXQUEUEDWAVES-1].next = NULL;
   gpFreeChannelNodes=&gaChannelNodes[0];
}

PCHANNELNODE GetChannelNode(void)
{
   PCHANNELNODE pNode;
   
   Assert(g);
   if (!gpFreeChannelNodes)
   {
      ODS(1,"GetChannelNode failed!");
      return NULL;
   }
   pNode = gpFreeChannelNodes;
   gpFreeChannelNodes = gpFreeChannelNodes->next;
   pNode->next=NULL;
   return pNode;
}

INLINE void FreeChannelNode(PCHANNELNODE pNode)
{
   ODS(2,"{FreeCNode");
   if (!pNode)
   {
      ODS(1," FreeCN: NULL parameter!}");
      return;
   }
   pNode->next=gpFreeChannelNodes;
   gpFreeChannelNodes=pNode;
   ODS(2,"}");
}

void FreeWaveBlocks(HWAVEOUT hWaveOut)
{
   int i;
   
   for (i=0;i<MAXWAVEBLOCKS;i++)
   {
      if (!gaWaveBlock[i])
	 continue;
      waveOutUnprepareHeader(hWaveOut,(LPWAVEHDR)gaWaveBlock[i],sizeof(WAVEHDR));
      GlobalFreePtr(gaWaveBlock[i]);
      gaWaveBlock[i]=NULL;
   }
}

BOOL AllocWaveBlocks(HWAVEOUT hWaveOut)
{
   int i;
   
   /* create the waveform data blocks that we will actually be using to send to the waveform device */
   for (i=0; i<MAXWAVEBLOCKS; i++)
   {
      if (!(gaWaveBlock[i] = (LPXWAVEHDR)GlobalAllocPtr(GMEM_FIXED | GMEM_SHARE,sizeof(XWAVEHDR)+MAXWAVEBLOCKLEN)))
      {
	 if (gfShow)
	    MessageBox(NULL, gszMemError,  gszAppName, MB_OK | MB_ICONEXCLAMATION);
	 while (--i >= 0)
	 {
	    GlobalFreePtr(gaWaveBlock[i]);
	    gaWaveBlock[i]=NULL;
	 }
	 
	 return FALSE;
      }
      gaWaveBlock[i]->wh.lpData= ((LPSTR)gaWaveBlock[i]) + sizeof(XWAVEHDR);
      gaWaveBlock[i]->wh.dwBufferLength = g->dwWaveBlockLen;
      gaWaveBlock[i]->wh.dwFlags=0;
      gaWaveBlock[i]->wh.dwLoops=0;
      gaWaveBlock[i]->fAvailable=TRUE;
      gaWaveBlock[i]->dwWavePos=0;
   }
   
   for (i=0; i<MAXWAVEBLOCKS; i++)
   {
      if (waveOutPrepareHeader(hWaveOut, (LPWAVEHDR)gaWaveBlock[i], sizeof(WAVEHDR)))
      {
	 if (gfShow)
	    MessageBox(NULL, "Unable to prepare wave header.", gszAppName, MB_OK | MB_ICONEXCLAMATION);
	 FreeWaveBlocks(hWaveOut);       
	 return FALSE;
      }
      gaWaveBlock[i]->wh.dwFlags |= WHDR_DONE;
   }
   
   return TRUE;
}

INLINE LPXWAVEHDR GetWaveBlock(void)
{
   int i;
   
   /* this is kinda inefficient, may want to reimplement with pointer list */
   /* but since we only have ~4 wave blocks what the hell */
   for (i=0;i<g->iNumWaveBlocks; i++)
      if (gaWaveBlock[i]->fAvailable)
	 break;
   if (i>=g->iNumWaveBlocks) /* no blocks available now, wait till the wave driver is done with one */
   {
      ODS(2,"{gwb:out of wav blks}");
      return NULL;
   }
   
   gaWaveBlock[i]->fAvailable=FALSE;
   
   /* need to reset these two fields since Remix() can modify them */
   gaWaveBlock[i]->wh.dwBufferLength = g->dwWaveBlockLen;
   gaWaveBlock[i]->wh.lpData= ((LPSTR)gaWaveBlock[i]) + sizeof(XWAVEHDR);
   gaWaveBlock[i]->g = gActiveSession;
   
   return gaWaveBlock[i];
}

INLINE void ReleaseWaveBlock(LPXWAVEHDR lpXWH)
{
   lpXWH->fAvailable=TRUE;
}

/* return pointer to the removed node */
LPXWAVEHDR RemoveFromPlayingQueue(LPXWAVEHDR lpXWH)
{
   /* ideally the wave blocks that we pass to the wave driver should be returned 
   ** in the same order so we could just deal with this like a queue and always just
   ** remove the first element.  In practice, however, this isn't the case and so it
   ** is necessary to deal with it as a regular linked list.
   */
   LPXWAVEHDR lp,lpPrev;
   Assert(g);
   
   ODS(2,"{RemoveFPQ");
   
   /* if queue is empty do nothing */
   if (gPlayQueue.first==NULL)
      return NULL;
   
   /* check if at head of list */
   if (lpXWH == gPlayQueue.first)
   {
      ODS(2,"h");
      gPlayQueue.first = lpXWH->QNext;
      
      /* check if the queue is now empty */
      if (gPlayQueue.first==NULL)
	 gPlayQueue.last = NULL;
   }
   else /* traverse looking for node to remove */
   {
      ODS(2,".");
      for (lpPrev=gPlayQueue.first,lp=gPlayQueue.first->QNext;lp && lp != lpXWH; lpPrev=lp, lp = lp->QNext)
	 ;
      if (!lp) /* error we couldn't find the specified element */
      {
	 ODS(1,"RemoveFromPlayingQueue: lpXWH is not in Queue!\n\r");
	 return NULL;
      }
      /* remove node from list */
      lpPrev->QNext=lp->QNext;
      if (lp==gPlayQueue.last)
	 gPlayQueue.last=gPlayQueue.first;
   }
   
   ODS(2,"}");
   lpXWH->QNext=NULL;
   return lpXWH;
}

INLINE void DestroyPlayQueue(void)
{
   Assert(g);
   while (gPlayQueue.first)
   {
      ReleaseWaveBlock(gPlayQueue.first);
      RemoveFromPlayingQueue(gPlayQueue.first);
   }
}

void ReleaseWaveDevice(PGLOBALS pG)
{
   Assert(pG);
   if (!pG->fActive || !pG->hWaveOut)
      return;
   
   MyWaveOutReset(pG->hWaveOut);
   DestroyPlayQueue();
   FreeWaveBlocks(pG->hWaveOut);
   waveOutClose(pG->hWaveOut);
   pG->hWaveOut=NULL;
   DestroyWindow(pG->hWndApp);
   pG->hWndApp=NULL;
}

UINT GetWaveDevice(void)
{
   UINT uErr;
   
   Assert(g);
   if (!g->hWaveOut)
   {
      /* create an invisible window so that we can handle messages from wavedriver */
      if (!(g->hWndApp=CreateWindow(gszAppName,"",WS_DISABLED,0,0,0,0,NULL,NULL,ghDLLInst,NULL)))
      {
	 if (gfShow)
	    MessageBox(NULL, "Failed to create callback window.",  gszAppName, MB_OK | MB_ICONEXCLAMATION);
	 return MMSYSERR_ERROR;
      }
      
      /* Open a waveform output device.
	 */
      if (uErr = waveOutOpen( (LPHWAVEOUT)&g->hWaveOut, g->wDeviceID, (LPCWAVEFORMATEX)&g->pcm,
			     (DWORD_PTR)g->hWndApp, (DWORD_PTR)NULL, CALLBACK_WINDOW))
      {
	 //          BUGBUG: for some reason a MessageBox here screws up 
	 //          MessageBox(GetFocus(), "Failed to open waveform output device.",  NULL, MB_OK | MB_ICONEXCLAMATION);
	 DestroyWindow(g->hWndApp);
	 g->hWndApp=NULL;
	 return uErr;
      }
      
      if (!AllocWaveBlocks(g->hWaveOut))
      {
	 /* error msg already displayed in AllocWaveBlocks */
	 waveOutClose(g->hWaveOut);
	 g->hWaveOut=NULL;
	 DestroyWindow(g->hWndApp);
	 return MMSYSERR_ERROR;
      }
   }
   return MMSYSERR_NOERROR;
}

UINT WINAPI WaveMixActivate(HANDLE hMixSession, BOOL fActivate)
{
   UINT uErr;
   ODS(2,"<WaveMixActivate>");
   
   if (!(g=SessionToGlobalDataPtr(hMixSession)))
      return MMSYSERR_INVALHANDLE;
   
   if (fActivate)
   {
      ODS(2,"Activate");
#if 1
      // Bugfix for Utopia:  don't allow the wave device to be removed from an application
      // that has already allocated it.  Instead return an Error.  This will permit the
      // application to release it when it is ready to. (what happens if an APP hangs while
      // it has the device allocated? Should probably reboot at that point anyway)
      //
      if (gActiveSession)
      {
	 // the wavemixer has already beeen allocated to a device,
	 // stealing it asynchrounously is dangerous and can cause
	 // GP Faults.
	 if (g!=gActiveSession)
	    return MMSYSERR_ALLOCATED;
	 else
	    return MMSYSERR_NOERROR;  // this instance already has the device allocated
      }
#else
      WaveMixActivate((HANDLE)gActiveSession,FALSE);
#endif
      gActiveSession=g;
      
      // Maximize our chances of getting device by killing any current system sounds
      // e.g. SoundBits
      sndPlaySound(NULL,SND_SYNC);
      if (uErr=GetWaveDevice())
	 return uErr;
      g->fActive=TRUE;
      SetWaveOutPosition(g->dwCurrentSample);
      while (MixerPlay(GetWaveBlock(),TRUE))
	 ;
   }
   else
   {
      ODS(2,"Inactivate");
      if (g->fActive)
	 g->dwCurrentSample=MyWaveOutGetPosition(g->hWaveOut,g->fGoodGetPos);
      ReleaseWaveDevice(g);
      g->fActive=FALSE;
      if (g==gActiveSession)
	 gActiveSession=NULL;
   }
   return MMSYSERR_NOERROR;
}

/* return same errors as waveOutOpen, waveOutWrite, and waveOutClose */
UINT WINAPI WaveMixOpenChannel(HANDLE hMixSession, int iChannel, DWORD dwFlags)
{
   ODS(2,"<WaveMixOpenChannel>");
   
   if (!(g=SessionToGlobalDataPtr(hMixSession)))
      return MMSYSERR_INVALHANDLE;
   
   if (dwFlags>WMIX_OPENCOUNT)
      return MMSYSERR_INVALFLAG;
   
   if (dwFlags==WMIX_OPENSINGLE && iChannel >= MAXCHANNELS)
      return MMSYSERR_INVALPARAM;
   
   if (dwFlags==WMIX_OPENCOUNT && (iChannel > MAXCHANNELS || iChannel < 1))
      return MMSYSERR_INVALPARAM;
   
   switch (dwFlags)
   {
   case WMIX_OPENSINGLE:
      if (g->aChannel[iChannel]!=(PCHANNELNODE)-1)
	 return MMSYSERR_ALLOCATED;
      g->aChannel[iChannel]=NULL;
      g->iChannels++;
      break;
   case WMIX_OPENALL:
      iChannel = MAXCHANNELS;
      /* fall through */
   case WMIX_OPENCOUNT:
      while (--iChannel>=0)
      {
	 if (g->aChannel[iChannel]==(PCHANNELNODE)-1)
	 {
	    g->aChannel[iChannel]=NULL;
	    g->iChannels++;
	 }
      }
      break;
   default:
      ODS(2,"WaveMixOpenChannel: Invalid flag was not trapped!!!");
      return MMSYSERR_INVALFLAG;
   }
   
   return MMSYSERR_NOERROR;
}

/* return pointer to head of queue */
INLINE LPXWAVEHDR AddToPlayingQueue(LPXWAVEHDR lpXWH)
{
   /* since add to the end of queue this node should not point to any other */
   lpXWH->QNext = NULL;
   
   /* if queue is empty then both first and last pointers point to this node */
   if (gPlayQueue.first == NULL)
   {
      gPlayQueue.first = gPlayQueue.last = lpXWH;
   }
   else /* add to end of queue */
   {
      gPlayQueue.last->QNext = lpXWH;
      gPlayQueue.last = lpXWH;
   }
   return gPlayQueue.first;
}

// This is the main mixing function, it gets called often and so is a good
// candidate for assembly optimization.  Not sure how much benefit I could get from it.

/* return TRUE if a block was submitted for playing */
BOOL MixerPlay(LPXWAVEHDR lpXWH,BOOL fWriteBlocks)
{
   UINT i,j,uMaxChannel;
   LPSAMPLE lpDest;
   static PCHANNELNODE rgpCD[MAXCHANNELS];
   PCHANNELNODE pCD;
   DWORD dwBlkStartPos, dwBlkCopyPos, dwOffset, dwSoonest, dwNumSamples, dwBlkEndCopyPos;
   //LPSAMPLE rgpCDdata[MAXCHANNELS];   // made static since assembly version was assuming ds
   static LPSAMPLE rgpCDdata[MAXCHANNELS];
   WORD wBytesToCopy;
   
   ODS(2,"{MP}");
   if (!lpXWH)
   {
      ODS(2,"{lpXWH=NULL:return F}");
      return FALSE;
   }
   
   /* setup array to point to only the channels that have data on them */
   dwSoonest = 0xFFFFFFFF;
   for (j=0, uMaxChannel=0; j<MAXCHANNELS; j++)
   {
      if (g->aChannel[j]!=(PCHANNELNODE)-1)
      {
	 /* advance past waves already played: should at most be one or two traversels per channel */
	 for (pCD = g->aChannel[j]; pCD && pCD->dwEndPos <= g->dwCurrentSample; pCD = pCD->next)
	    ODS(3,"{MixerPlay:skipping}");

	 if (!pCD)
	    continue;
	 if (pCD->dwStartPos < dwSoonest)
	    dwSoonest = pCD->dwStartPos;
	 rgpCD[uMaxChannel]=pCD;
	 uMaxChannel++;
      }
   }
   
   /* BUGBUG: I tried playing silence when we were out of data to avoid
   **       activating the mute circuitry in Windows Sound System when
   **       sounds aren't playing, but I got a clicking sound so I took
   **       out the code to do that
   */
   if (uMaxChannel==0)
   {
      // BUGBUG: verify that this should happen here
      ODS(2,"{MP:uMaxChannel==0,ret F}");
      if (fWriteBlocks)
	 ReleaseWaveBlock(lpXWH);
      return FALSE;
   }
   
   lpDest=(LPSAMPLE)lpXWH->wh.lpData;
   wBytesToCopy = (WORD)g->dwWaveBlockLen;               // BUGBUG: wBytesToCopy should be wSamplesToCopy
   dwBlkStartPos = g->dwCurrentSample;
   dwBlkCopyPos= g->dwCurrentSample;
   
   while (wBytesToCopy > 0)
   {
      ODS(2,"+");
      
      if (dwBlkCopyPos < dwSoonest) /* then have to fill the block with silence */
      {
	 if (dwBlkCopyPos+wBytesToCopy<dwSoonest)
	    dwNumSamples=wBytesToCopy;
	 else
	    dwNumSamples=dwSoonest-dwBlkCopyPos;
	 memset(lpDest,SILENCE,(WORD)dwNumSamples);
	 lpDest+=dwNumSamples;
	 dwBlkCopyPos=dwBlkCopyPos+dwNumSamples;
	 wBytesToCopy=wBytesToCopy-(WORD)dwNumSamples;
	 continue;
      }
      
      /* now know that some data starts here - find the shortest amount of data and set pointers to the data */
      // BUGBUG: have to deal with:
      //            x - waves that end during this block,
      //            x - waves that have data for the entire block,
      //            x - waves that start playing sometime in the future
      //            x - loops, combined with all of the above
      dwBlkEndCopyPos = dwBlkCopyPos + wBytesToCopy;
      
      /* j will be the index of the array used to contain pointers to actual wave data
      ** as well as the count of how many waves will be mixed
      */
      for (i=0, j=0 ;i<uMaxChannel; i++)
      {
#if DEBUG
	 if (rgpCD[i]->dwEndPos <= dwBlkCopyPos)
	 {
	    ODS(2,"{dwEndPos <= dwBlkCopyPos!!!}");
	 }
#endif
	 if (rgpCD[i]->dwStartPos>dwBlkCopyPos) /* file to play in the future (eg. queued waves)*/
	 {
	    if (rgpCD[i]->dwStartPos < dwBlkEndCopyPos)
	       dwBlkEndCopyPos = rgpCD[i]->dwStartPos;
	    continue;  /* no more checks needed on this channel */
	 }
	 
	 if (rgpCD[i]->dwEndPos < dwBlkEndCopyPos) /* check for waves that end during this period */
	 {
	    dwBlkEndCopyPos = rgpCD[i]->dwEndPos;
	 }
	 
	 dwOffset = dwBlkCopyPos-rgpCD[i]->dwStartPos;  /* find offset of current sample in the wave data */
	 
	 if (rgpCD[i]->PlayParams.wLoops)
	 {
	    DWORD dwSamplesRemaining;
	    
	    dwOffset = dwOffset % rgpCD[i]->dwNumSamples;  // unsigned eax div on 386 takes 38/41 clocks depend on reg or mem, 
	    /* find the distance from the current offset until the end of actual wave data */
	    dwSamplesRemaining = rgpCD[i]->dwNumSamples - dwOffset;
	    if (dwBlkCopyPos + dwSamplesRemaining < dwBlkEndCopyPos)
	       dwBlkEndCopyPos= dwBlkCopyPos + dwSamplesRemaining;
	 }
	 
	 /* now have valid data to mix, so setup the wave data pointer */
	 rgpCDdata[j]=rgpCD[i]->lpPos+dwOffset;
#if DEBUG
	 if (!rgpCDdata[j])
	    ODS(1,"assertion failure line 784");
#endif
	 j++;
      }
      
      if (j) /* then have data we want to mix now */
      {
	 /* now mix the data! */
	 dwNumSamples = dwBlkEndCopyPos-dwBlkCopyPos;
	 gpfnMixit(lpDest,rgpCDdata,j,(WORD)dwNumSamples);
      }
      else
      {
	 ODS(2,"***No wave data has been setup!!!!");
	 continue;
      }
      
      /* update variables that keep track of how much data we have left to copy */
      lpDest+=dwNumSamples;
      wBytesToCopy = wBytesToCopy - (WORD)dwNumSamples;
      dwBlkCopyPos = dwBlkCopyPos + dwNumSamples;
      dwSoonest = 0xFFFFFFFF;
      
      /* now go through the channels and skip by any we are done with
      ** note: we are only going through the channels that had data when we entered MixerPlay
      */
      for (j=0; j < uMaxChannel;)
      {
	 while (rgpCD[j])
	 {
	    if (rgpCD[j]->dwEndPos <= dwBlkCopyPos)
	       rgpCD[j] = rgpCD[j]->next;
	    else
	       break;
	 }
	 if (!rgpCD[j]) /* remove this channel from the list */
	 {
	    uMaxChannel--;
	    rgpCD[j] = rgpCD[uMaxChannel];
#if DEBUG
	    if (uMaxChannel>=MAXCHANNELS)
	       ODS(1,"assertion failure line 817");
#endif
	    if (uMaxChannel)
	       continue;  // recheck index j again since it now contains a different channel
	    else
	       break; // no more valid data on any of the channels
	 }
	 if (rgpCD[j]->dwStartPos < dwSoonest)
	    dwSoonest = rgpCD[j]->dwStartPos;
	 
	 j++;
      }
#if DEBUG
      for (i=0, j=0 ;i<uMaxChannel; i++)
	 if (rgpCD[i]->dwEndPos <= dwBlkCopyPos)
	    ODS(1,"assertion failure line 832");
#endif
   }
   ODS(2,"mixing done");
   
   lpXWH->dwWavePos=g->dwCurrentSample;
   g->dwCurrentSample+=SAMPLESPERBLOCK;
   
   if (fWriteBlocks)
   {               
      AddToPlayingQueue(lpXWH);
      
      /* now play the block */
      if (waveOutWrite(g->hWaveOut,(LPWAVEHDR)lpXWH,sizeof(WAVEHDR)))
      {
	 if (gfShow)
	    MessageBox(NULL, "Failed to write block to device", gszAppName, MB_OK | MB_ICONEXCLAMATION);
	 ReleaseWaveBlock(lpXWH);
	 RemoveFromPlayingQueue(lpXWH);
      }
   }
   
   ODS(2,"{end MP: TRUE}");
   return TRUE;
}

void FreePlayedBlocks(void)
{
   int i;
   DWORD dwPos;
   PCHANNELNODE pCD;
   
   ODS(2,"{F");
   
   // bugbug: need to use the waveOutGetPosition that is provided by the sound device
   //         otherwise it is possible that the position determined using timeGetTime()
   //         will be less than the length of the wave.  That can cause this function
   //         to think that the wave is not yet done and so it won't release it.  Since
   //         the multimedia device will not post anymore messages (it has already posted 
   //         one for each block) we will never inform the user that the wave has finished
   //         playing. (ARCADE bug #268, #300, #301)
   dwPos = MyWaveOutGetPosition(g->hWaveOut,TRUE);
   ODS(2,"_got_");
   
   for (i=0; i<MAXCHANNELS; i++)
   {
      if (!(pCD=g->aChannel[i]) || (pCD == (PCHANNELNODE)-1))
	 continue;
      ODS(2,".");
      
      while (pCD && dwPos >= pCD->dwEndPos) 
      {
	 ODS(2,"<!");
	 g->aChannel[i]=pCD->next;
	 
	 if (pCD->PlayParams.hWndNotify)
	    PostMessage(pCD->PlayParams.hWndNotify,MM_WOM_DONE,i,(LPARAM)pCD->lpMixWave);
	 FreeChannelNode(pCD);
	 pCD=g->aChannel[i];
	 ODS(2,">");
      }
   }
   ODS(2,"}");
}

void WINAPI WaveMixPump(void)
{
   LPXWAVEHDR lpXWH;
   
   ODS(1,"{pump}");
   
#if 1
   // to fix bug where WaveMixPump GPFaults because app called WaveMixPump before
   // it called WaveMixActivate, or called WaveMixPump after WaveMixInit failed.
   // Second part of bugfix in WaveMixActivate()
   // (Actual hang is in FreePlayedBlocks)
   g=gActiveSession;
   if (!g)
      return;
#endif
   
   /* first go through and remove blocks that have the done bit set from playing queue */
   lpXWH=gPlayQueue.first;
   while (lpXWH)
   {
      if (lpXWH->wh.dwFlags & WHDR_DONE)
      {
	 RemoveFromPlayingQueue(lpXWH);
	 ReleaseWaveBlock(lpXWH);
	 lpXWH=gPlayQueue.first;  // need to reset after altering list to avoid potential problems
      }
      else
	 lpXWH=lpXWH->QNext;
   }
   
   /* now free up channel blocks that have completed (ie. physically played) */
   FreePlayedBlocks();
   
   /* if no data on any of the channels, reset the position tracking variables */
   //BUGBUG: implement this code; This code implemented in WaveMixPlay()
   while (MixerPlay(GetWaveBlock(),TRUE)) /* then fill up the queue again */
      ;
   ODS(2,"{end pump}");
}

LRESULT WINAPI WndProc(HWND hWnd, unsigned msg, WPARAM wParam, LPARAM lParam)
{
   ODS(2,"{WndProc}");
   
   switch (msg)
   {
   case MM_WOM_DONE:
      if ( ((LPXWAVEHDR)lParam)->g != gActiveSession)
      {
	 ODS(1,"++++++ waveblock does not match current active global data pointer!!!!!!!");
	 g=gActiveSession;
      }
      /* mark the block as usable again and mix the next batch */
      WaveMixPump();
      return 0;
   default:
      return DefWindowProc(hWnd,msg,wParam,lParam);
   }
}

/*
this code will "Remix" the wave output without doing a waveOutReset.  This is necessary
for systems in which the waveOutReset cause hardware clicks or mute circutry or whatever.
The SLIMY version of this code remixes the sounds directly into the buffers that have 
already been submitted to waveOutWrite.  This is faster than just mixing when the next block
becomes available, but it does not work so well because many sound cards suck the submitted
data into a DMA buffer and so modifying the data in those buffers has no effect.  The NOSLIMY
version works well when the waveblocks are short (eg. ~512-600 bytes) and there are few of them
(eg <=3 blks) Doing this will cause blocks to be mixed and submitted often and so response time
is good.

SLIMY version removed 8/95 ARK
*/
BOOL NoResetRemix(DWORD dwRemixSamplePos, PCHANNELNODE pCD)
{
   DWORD dwOffset;
   
   ODS(1,"r");
   
   if (pCD)
   {
      dwOffset = g->dwCurrentSample - pCD->dwStartPos;
      pCD->dwStartPos = pCD->dwStartPos + dwOffset;
      pCD->dwEndPos = pCD->dwEndPos + dwOffset;
      if (pCD->dwStartPos > pCD->dwEndPos)	// if we wrapped the end beyond last possible position, fix it!
	 pCD->dwEndPos= 0xFFFFFFFF;
   }
   
   WaveMixPump();
   ODS(2,"{Remix done***}");
   return TRUE;
}

BOOL ResetRemix(DWORD dwRemixSamplePos, PCHANNELNODE pCD)
{
   LPXWAVEHDR lpXWH;
   DWORD dwTmp;
   
   ODS(1,"R");
   g->dwCurrentSample = dwRemixSamplePos;
   
   /* destroy the old queue */
   DestroyPlayQueue();
   
   /* this is sort of slimy, but necessary for speed reasons:
   ** remix the waves into blocks that have been submitted to the wave driver
   ** BUGBUG: will this cause problems in NT?
   */
   
   while (lpXWH=GetWaveBlock())
   {
      if (!MixerPlay(lpXWH,FALSE))
      {
	 ReleaseWaveBlock(lpXWH);
	 break;
      }
      AddToPlayingQueue(lpXWH);
   }
   
   /* Now after all the mixing, reset the wave driver, resubmit the blocks and
   ** restart the wave driver
   ** Note: we must save the currentSample because myWaveOutReset will reset it
   **       and since we have already remixed the data we don't want to go through
   **       it again.
   */
   dwTmp = g->dwCurrentSample;
   MyWaveOutReset(g->hWaveOut);
   g->dwCurrentSample=dwTmp;  // need to reset since MyWaveOutReset will update it
   waveOutPause(g->hWaveOut);
   
   /* now submit all the blocks in the play queue*/
   for (lpXWH = gPlayQueue.first; lpXWH; lpXWH = lpXWH->QNext)
   {
      if (waveOutWrite(g->hWaveOut,(LPWAVEHDR)lpXWH,sizeof(WAVEHDR)))
      {
	 if (gfShow)
	    MessageBox(NULL, "Failed to write block to device", gszAppName, MB_OK | MB_ICONEXCLAMATION);
	 ReleaseWaveBlock(lpXWH);
	 RemoveFromPlayingQueue(lpXWH);
      }
   }
   
   waveOutRestart(g->hWaveOut);
   
   return TRUE;
}

void ResetWavePosIfNoChannelData(void)
{
   PCHANNELNODE pCD;
   int i;
   
   // if the playqueue is empty we want to reset our global waveoutposition back to 0 so we
   // don't have to worry about the waveout position wrapping back to zero and messing up all
   // sounds still playing (this would take 13.5 hours of playing at 44.1khz to happen, but
   // we want to be as robust as possible.  However, we need to verify that all the channels
   // are empty too.  Otherwise resetting the waveoutpostion will cause these sounds to stay
   // on the channel and play again sometime in the future.
   
   if (gPlayQueue.first)
      return;
   
   for (i=0; i<MAXCHANNELS; i++)
      if ((pCD=g->aChannel[i]) && (pCD != (PCHANNELNODE)-1))
	 return;
   
   SetWaveOutPosition(0);
}

UINT WINAPI WaveMixPlay(LPMIXPLAYPARAMS lpMixPlayParams)
{
   PCHANNELNODE pTmp,pCD;
   DWORD dwWavePos;
   int iChannel;
   
   BOOL fRemix=FALSE;
   BOOL fPause=FALSE;
   
   ODS(2,"<Play>");
   
   if (!lpMixPlayParams)
   {
      if (gfShow)
	 MessageBox(NULL, "NULL parameters pointer passed to WaveMixPlay!", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      return MMSYSERR_INVALHANDLE;
   }
   
   if (!(g=SessionToGlobalDataPtr(lpMixPlayParams->hMixSession)))
   {
      if (gfShow)
	 MessageBox(NULL, "Invalid session handle passed to WaveMixPlay", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      return MMSYSERR_INVALHANDLE;
   }
   
   if (!lpMixPlayParams->lpMixWave)
   {
      if (gfShow)
	 MessageBox(NULL, "NULL wave pointer passed to WaveMixPlay!", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      return MMSYSERR_INVALHANDLE;
   }
   
   if (!g->fActive)
   {
      if (gfShow)
	 MessageBox(NULL, "Wave device not allocated, call WaveMixActivate(hMixSession,TRUE)", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      return MMSYSERR_ALLOCATED;
   }
   
   if (g->iChannels==0)
   {
      if (gfShow)
	 MessageBox(NULL, "You must open a channel before you can play a wave!", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      return MMSYSERR_INVALHANDLE;
   }
   
   if (lpMixPlayParams->dwFlags & WMIX_USELRUCHANNEL)
   {
      int iLRU;
      ODS(2,"<LRU>");
      /* find the lease recently used channel */
      for (iLRU = 0, iChannel = 0; iChannel<MAXCHANNELS; iChannel++)
      {
	 if (g->aChannel[iChannel]==(PCHANNELNODE)-1)
	    continue;
	 
	 if (g->aChannel[iLRU] == NULL)  // we found an empty channel go ahead and use it.
	    break;
	 
	 if (iChannel != iLRU && g->MRUChannel[iChannel] < g->MRUChannel[iLRU])
	    iLRU=iChannel;
      }
      iChannel=iLRU;
      lpMixPlayParams->iChannel=iChannel;  // return channel back to calling app so can use other APIs
   }
   else
      iChannel = lpMixPlayParams->iChannel;
   g->dwMRU++;
   g->MRUChannel[iChannel]=g->dwMRU;
   
   if (g->aChannel[iChannel]==(PCHANNELNODE)-1)
      return MMSYSERR_INVALHANDLE;
   
   if (!(pCD = GetChannelNode()))
      return MMSYSERR_NOMEM;
   
   memcpy(&pCD->PlayParams,lpMixPlayParams,sizeof(MIXPLAYPARAMS));
   pCD->lpMixWave=pCD->PlayParams.lpMixWave;
   pCD->dwNumSamples = pCD->lpMixWave->wh.dwBufferLength/BYTESPERSAMPLE;
   pCD->lpPos = (LPSAMPLE)pCD->lpMixWave->wh.lpData;
   pCD->lpEnd = pCD->lpPos + pCD->dwNumSamples -1;
   pCD->PlayParams.iChannel=iChannel;
   
#if DEBUGOUTPUT
   wsprintf(gsz,"(%d:%s L%u %c)",iChannel,(LPSTR)pCD->lpMixWave->szWaveFilename,pCD->PlayParams.wLoops,pCD->PlayParams.dwFlags & WMIX_WAIT ? 'W':'!');
   ODS(1,gsz);
#endif
   
   /* add to end of play wait list */
   if (g->pWaitList)
   {
      pCD->next = g->pWaitList->next;
      g->pWaitList->next = pCD;
      g->pWaitList = pCD;
   }
   else
   {
      g->pWaitList=pCD;
      pCD->next=pCD;
   }
   
   if (pCD->PlayParams.dwFlags & WMIX_WAIT)
   {
      ODS(2,"wait list");
      return MMSYSERR_NOERROR;
   }
   
   /* if there is no sound current playing reset our wave position counters
   ** to avoid potential overflows.
   */
   ResetWavePosIfNoChannelData();
   
   dwWavePos = MyWaveOutGetPosition(g->hWaveOut,g->fGoodGetPos);
   
   while (g->pWaitList)
   {
      /* remove the first node from the list */
      pCD = g->pWaitList->next;
      if (pCD==g->pWaitList) /* then was the only one in list */
	 g->pWaitList=NULL;
      else
	 g->pWaitList->next = pCD->next;
      pCD->next=NULL;
      
      iChannel = pCD->PlayParams.iChannel;
      
      if (pCD->PlayParams.dwFlags & WMIX_CLEARQUEUE)
      {
	 PCHANNELNODE pCD1,pTmp;
	 
	 /* replace the stuff in the old channel with this new wave */
	 pCD1=g->aChannel[iChannel];
	 while (pCD1)
	 {
	    pTmp=pCD1->next;
	    FreeChannelNode(pCD1);
	    pCD1=pTmp;
	 }
	 g->aChannel[iChannel]=pCD;
	 
	 /* if there was already sound playing we must remix the output */
	 if (gPlayQueue.first)
	    fRemix=TRUE;
	 
	 if (pCD->PlayParams.dwFlags & WMIX_HIPRIORITY)
	    pCD->dwStartPos = dwWavePos;
	 else
	    pCD->dwStartPos = g->dwCurrentSample;
      }
      else /* queue the wave on this channel */
      {
	 if (g->aChannel[iChannel])
	 {
	    for (pTmp = g->aChannel[iChannel]; pTmp->next; pTmp=pTmp->next)
	       ;
	    pTmp->next=pCD;
#if 1
	    if (pTmp->dwEndPos==0xFFFFFFFF)
	       ODS(1,"!!!! adding to channel at a position where will never get played!!!!\n\r");
#endif  
	    if (pCD->PlayParams.dwFlags & WMIX_HIPRIORITY)
	       pCD->dwStartPos = pTmp->dwEndPos;  /* start pos = end pos of previous in queue: don't do +1 */ 
	    else                                                               /* or MixerPlay will need to handle a single sample copy */
	       pCD->dwStartPos = max(g->dwCurrentSample,pTmp->dwEndPos);
	 }
	 else /* queue wave onto an empty channel */
	 {
	    g->aChannel[iChannel]=pCD;
	    if (pCD->PlayParams.dwFlags & WMIX_HIPRIORITY)
	       pCD->dwStartPos = dwWavePos;
	    else
	       pCD->dwStartPos = g->dwCurrentSample;
	 }
	 
	 if (g->dwCurrentSample > pCD->dwStartPos)
	    fRemix=TRUE;
      }
      
      if (pCD->PlayParams.wLoops==0xFFFF)
	 pCD->dwEndPos=0xFFFFFFFF;
      else
	 pCD->dwEndPos=pCD->dwStartPos + (pCD->PlayParams.wLoops+1) * pCD->dwNumSamples -1;
      
#if DEBUG
      if (g->dwCurrentSample > pCD->dwStartPos)
      {
	 wsprintf(gsz,"{currentsample<dwStartPos by %lu}",g->dwCurrentSample-pCD->dwStartPos);
	 ODS(2,gsz);
      }
#endif
   } /* while (g->pWaitList) */
   
   if (fRemix && /* lpMixPlayParams->dwFlags & WMIX_HIPRIORITY && */ g->pfnRemix(pCD->dwStartPos,pCD))
      return MMSYSERR_NOERROR;
   else
   {
      /* if we are not already playing we should pause before submitting data or we
      ** may not be able to do it fast enough and will cause a hicup
      */
      if (!gPlayQueue.first)  
      {
	 fPause=TRUE;
	 waveOutPause(g->hWaveOut);
      }
      while(MixerPlay(GetWaveBlock(),TRUE))
	 ;
      if (fPause)
	 waveOutRestart(g->hWaveOut);
   }
   return MMSYSERR_NOERROR;
}

UINT WINAPI WaveMixFlushChannel(HANDLE hMixSession, int iChannel, DWORD dwFlags)
{
   PCHANNELNODE pCD,pTmp;
   int iLast;
   BOOL fRemix=FALSE; /* don't remix if that channel didn't have data on it. */
   
   ODS(1,"F");
   
   if (!(g=SessionToGlobalDataPtr(hMixSession)))
      return MMSYSERR_INVALHANDLE;
   
   if (dwFlags & WMIX_ALL)
   {
      iChannel=0;
      iLast=MAXCHANNELS;
   }
   else
   {
      if (iChannel < 0 || iChannel >= MAXCHANNELS)
	 return MMSYSERR_INVALPARAM;
      if (g->aChannel[iChannel]==(PCHANNELNODE)-1)
	 return MMSYSERR_INVALHANDLE;
      iLast = iChannel+1;
   }
   
   for (;iChannel<iLast;iChannel++) 
   {
      if ((pCD=g->aChannel[iChannel])==(PCHANNELNODE)-1)
	 continue;
      g->aChannel[iChannel]=NULL;
      
      while (pCD)
      {
	 pTmp=pCD->next;
	 FreeChannelNode(pCD);
	 pCD=pTmp;
	 fRemix=TRUE;
      }
   }
   
   if (fRemix && !(dwFlags & WMIX_NOREMIX) && g->fActive)
      g->pfnRemix(MyWaveOutGetPosition(g->hWaveOut,g->fGoodGetPos),NULL);
   
   return MMSYSERR_NOERROR;
}

UINT WINAPI WaveMixCloseChannel(HANDLE hMixSession, int iChannel, DWORD dwFlags)
{
   UINT uErr;
   int iLast;
   
   ODS(2,"<WaveMixCloseChannel>");
   
   if (!(g=SessionToGlobalDataPtr(hMixSession)))
      return MMSYSERR_INVALHANDLE;
   
   /* flush the channel and let WaveMixFlushChannel do all the 
   ** error checking for us
   */
   if (uErr=WaveMixFlushChannel(hMixSession, iChannel, dwFlags|WMIX_NOREMIX))
      return uErr;
   
   if (dwFlags & WMIX_ALL)
   {
      iChannel=0;
      iLast=MAXCHANNELS;
   }
   else
   {
      iLast = iChannel+1;
   }
   
   for (;iChannel<iLast;iChannel++) 
   {
      if (g->aChannel[iChannel]!=(PCHANNELNODE)-1)
      {
	 g->aChannel[iChannel]=(PCHANNELNODE)-1;
	 g->iChannels--;
      }
   }
   
   return MMSYSERR_NOERROR;
}

UINT WINAPI WaveMixFreeWave(HANDLE hMixSession, LPMIXWAVE lpMixWave)
{
   int i;
   PCHANNELNODE pCD,pPrev;
   PGLOBALS g = (PGLOBALS) hMixSession;
   
   ODS(2,"<WaveMixFreeWave>");
   
   if (!(g=SessionToGlobalDataPtr(hMixSession)))
      return MMSYSERR_INVALHANDLE;
   
   if (!lpMixWave)
      return MMSYSERR_INVALHANDLE;
   
   for (i=0; i<MAXCHANNELS; i++)
   {
      
      if (g->aChannel[i]==(PCHANNELNODE)-1)
	 continue;
      
      pPrev=NULL;
      pCD=g->aChannel[i];
      
      while (pCD)
      {
	 if (pCD->lpMixWave == lpMixWave)
	 {
	    if (!pPrev) /* then at head of list */
	    {
	       pCD=pCD->next;
	       FreeChannelNode(g->aChannel[i]);
	       g->aChannel[i]=pCD;
	    }
	    else
	    {
	       pPrev->next = pCD->next;
	       FreeChannelNode(pCD);
	       pCD=pPrev->next;
	    }
	 }
	 else
	 {
	    pPrev=pCD;
	    pCD=pCD->next;
	 }
      }
   }
   
   if (lpMixWave->wh.lpData)
      GlobalFreePtr(lpMixWave->wh.lpData);
   GlobalFreePtr(lpMixWave);
   
   return MMSYSERR_NOERROR;
}

// Added ScaleSample 8/95 ARK
/******************************************************************************************/
/*
 * ScaleSample: Modifies volume of sample in place.
 *   A volume of 0 corresponds to silence, and a value of MAX_VOLUME leaves the sound unchanged.
 */
void ScaleSample(char *lpInData, DWORD dwDataSize, int bits_per_sample, int volume)
{
   int bytes_per_sample, num_samples;

   if (volume == MAX_VOLUME)
      return;
   
   // XXX Put in a case for 0 volume here
   
   bytes_per_sample = bits_per_sample / BITSPERBYTE;
   num_samples = dwDataSize / bytes_per_sample;
   
   if (bits_per_sample == 8)
   {
      // 8-bit samples have silence = 128
      BYTE *ptr;
      for (ptr = (BYTE *) lpInData; num_samples--; ptr++)
         *ptr = (BYTE) ((((((int) *ptr) - 128) * volume) >> LOG_MAX_VOLUME ) + 128);
   }
   else 
   {
      // Seems that 16-bit samples have silence = 0
      WORD *ptr;
      for (ptr = (WORD *) lpInData; num_samples--; ptr++)
	 *ptr = (WORD) ((((int) *ptr) * volume) >> LOG_MAX_VOLUME );
   }
}


/* dwNumSamples should be double if the file is stereo */
HPSTR BitsPerSampleAlign(HPSTR lpInData, WORD nInBPS, WORD nOutBPS, DWORD *dwDataSize)
{
   HPSTR lpOutData,lpB;
   HPINT lpW;
   DWORD dwNumSamples;
   WORD nInBytesPerSample,nOutBytesPerSample;
   
   if (nInBPS == nOutBPS)
      return lpInData;
   
   /* if not 8 or 16 then ADPCM (compressed) format which we don't support */
   if ((nInBPS != 8 && nInBPS != 16) || (nOutBPS != 8 && nOutBPS != 16))
   {
      GlobalFreePtr(lpInData);
      return NULL;
   }
   
   nInBytesPerSample = nInBPS / BITSPERBYTE;
   nOutBytesPerSample = nOutBPS / BITSPERBYTE;
   dwNumSamples = *dwDataSize / nInBytesPerSample;
   *dwDataSize = dwNumSamples * nOutBytesPerSample;
   
   if (!(lpOutData = (HPSTR) GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, *dwDataSize)))
   {
      if (gfShow)
	 MessageBox(NULL, gszMemError, gszAppName, MB_OK | MB_ICONINFORMATION);
      GlobalFreePtr(lpInData);
      return NULL;
   }
   
   if (nInBytesPerSample > nOutBytesPerSample)  /* convert from 16 bit to eight bit */
   {
      for (lpB=lpOutData,lpW=(short *)lpInData;dwNumSamples;dwNumSamples--)
	 *lpB++ = (char)(int)(((int)*lpW++)/256 +128);
   }
   else /* convert from 8 bit to 16 bit */
   {
      for (lpB = lpInData,lpW = (HPINT)lpOutData;dwNumSamples;dwNumSamples--)
         *lpW++ = (((short)(*lpB++))-128)*256;
   }
   
   GlobalFreePtr(lpInData);
   return lpOutData;
}

HPSTR ChannelAlign(HPSTR lpInData, WORD nInChannels, WORD nOutChannels, WORD nBytesPerSample, DWORD * dwDataSize)
{
   LPSAMPLE lpOutData;
   LPSAMPLE lpB1, lpB2;
   HPINT lpW1;
   HPINT lpW2;
   DWORD dwNumSamples;
   
   if (nInChannels == nOutChannels)
      return lpInData;
   
   dwNumSamples = *dwDataSize / nBytesPerSample / nInChannels;
   *dwDataSize = dwNumSamples * nBytesPerSample * nOutChannels;
   
   if (!(lpOutData = (LPSAMPLE) GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, *dwDataSize)))
   {
      if (gfShow)
	 MessageBox(NULL, gszMemError, gszAppName, MB_OK | MB_ICONINFORMATION);
      GlobalFreePtr(lpInData);
      return NULL;
   }
   
   /* BUGBUG: this should really be more general to allow you to convert from stereo to 4 channel */
   if (nInChannels < nOutChannels) /* convert from mono to stereo */
   {
      if (nBytesPerSample==1)  /* eight bit */
      {
         for (lpB1 = (LPSAMPLE) lpInData,lpB2=lpOutData;dwNumSamples--;)
	 {
	    *lpB2++ = *lpB1;
	    *lpB2++	= *lpB1++;
	 }
      }
      else
      {
	 for (lpW1=(HPINT)lpInData,lpW2=(HPINT)lpOutData;dwNumSamples--;)
	 {
	    *lpW2++ = *lpW1;
	    *lpW2++	= *lpW1++;
	 }
      }
   }
   else /* convert from stereo to mono */
   {
      if (nBytesPerSample==1)  /* eight bit */
      {
	 for (lpB1=(LPSAMPLE)lpInData,lpB2=lpOutData; dwNumSamples--;  lpB1+=2)
	 {
	    /* mix the two channels */
	    // note: lpB1 and lpB2 must be pointers to unsigned chars or normalized
	    // numbers above 0 (eg. 140) are negative numbers and so the averaging
	    // gets confused
	    *lpB2++ = ((int)(*lpB1)+(int)(*(lpB1+1)))/2;
	 }
      }
      else /* 16 bit */
      {
	 for (lpW1=(short *)lpInData,lpW2=(short *)lpOutData;dwNumSamples--;lpW1+=2)
	 {
	    *lpW2++ = (int)(((long)*lpW1) + ((long)(*(lpW1+1))))/2;
	 }
      }
   }
   
   GlobalFreePtr(lpInData);
   return (HPSTR) lpOutData;
}

void AvgSample(HPSTR lpOutData, HPSTR lpInData,int nSkip,int nBytesPerSample,int nChannels)
{
   if (nBytesPerSample==1) // 8 bit samples
   {
      HPSTR lpTmp;
      int sum,i,j;
      
      for (i=0; i<nChannels;i++)
      {
	 lpTmp = lpInData++;
	 sum=0;
	 for (j=0;j<nSkip;j++)
	 {
	    sum += (BYTE)*lpTmp - 128;
	    lpTmp += nChannels;
	 }
	 sum /= nSkip;
	 *lpOutData++ = (char) (sum + 128);
      }
   }
   else // 16 bit samples
   {
      HPINT lpWOutData = (HPINT) lpOutData;
      HPINT lpWInData = (HPINT) lpInData;
      HPINT lpWTmp;
      long sum;
      int i,j;
      
      Assert(nBytesPerSample==2);
      
      for (i=0;i<nChannels;i++)
      {
	 lpWTmp = lpWInData++;
	 sum = 0;
	 for (j=0;j<nSkip;j++)
	 {
	    sum += *lpWTmp;
	    lpWTmp += nChannels;
	 }
	 sum /= nSkip;
	 *lpWOutData = (int)sum;
      }
   }
}

// this routine interpolates the samples along the different channels
void RepSample(HPSTR lpOutData, HPSTR lpInData, int nRep, int nBytesPerSample,int nChannels)
{
   Assert (nRep > 1);
   
   if (nBytesPerSample==1)  // 8 bit samples
   {
      // bugbug: this code is supposed to interpolate the samples
      //         that it is going to be 
      HPSTR lpTmp = lpOutData;
      char diff, val;
      int i, j;
      
      for (i=0;i<nChannels;i++)
      {
	 lpOutData = lpTmp++;
	 diff = ((BYTE)*(lpInData+nChannels) - (BYTE)*lpInData)/nRep; 
	 //diff = 0;  // hack to remove interpolation
	 val = *lpOutData = *lpInData;
	 lpOutData +=nChannels;
	 
	 for (j=1; j<nRep; j++)
	 {
	    val += diff;
	    *lpOutData = (char)val;
	    lpOutData+=nChannels;
	 }
	 
	 lpInData++;
      }
   }
   else // 16 bit samples
   {
      HPINT lpWIn=(HPINT)lpInData;
      HPINT lpWOut=(HPINT)lpOutData;
      HPINT lpWTmp=lpWOut;
      int diff, val, i, j;
      
      Assert(nBytesPerSample == 2);
      
      for (i=0;i<nChannels;i++)
      {
	 lpWOut = lpWTmp++;
	 diff = (*(lpWOut+nChannels) - *lpWIn)/nRep;
	 val = *lpWOut = *lpWIn;
	 lpWOut+=nChannels;
	 
	 for (j=1; j<nRep; j++)
	 {
	    val += diff;
	    *lpWOut = val;
	    lpWOut+=nChannels;
	 }
	 lpWIn ++;
      }
   }
}

HPSTR SamplesPerSecAlign(HPSTR lpInData, DWORD nInSamplesPerSec, DWORD nOutSamplesPerSec, WORD nBytesPerSample, WORD nChannels, DWORD * dwDataSize)
{
   int SampleSize;
   int nRep,nSkip,i,n;
   DWORD dwNumSamples,dwNewNumSamples,dw;
   LPSAMPLE lpOutData,lpOutSave,lpInSave;
   
   if (nInSamplesPerSec == nOutSamplesPerSec)
      return lpInData;
   
   SampleSize = nBytesPerSample * nChannels;
   dwNumSamples = *dwDataSize / SampleSize;
   
   if (nOutSamplesPerSec > nInSamplesPerSec)
   {
      // then need to add in extra samples
      nRep = (int)(nOutSamplesPerSec / nInSamplesPerSec);
      nSkip = 0;
      dwNewNumSamples = dwNumSamples * nRep;
      
   }
   else  // replace the sample with the average of nSkip samples
   {
      nRep=0;
      nSkip = (int)(nInSamplesPerSec / nOutSamplesPerSec);
      Assert(nSkip>0);
      dwNewNumSamples = dwNumSamples / nSkip;
   }
   
   *dwDataSize = dwNewNumSamples * SampleSize;
   
   if (!(lpOutData = (LPSAMPLE) GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, *dwDataSize)))
   {
      if (gfShow)
         MessageBox(NULL, gszMemError, gszAppName, MB_OK | MB_ICONINFORMATION);
      GlobalFreePtr(lpInData);
      return NULL;
   }
   
   lpInSave = (LPSAMPLE) lpInData;
   lpOutSave = lpOutData;
   
   if (nRep > 0 )
   {
      for (dw = dwNumSamples-1; dw--; )
      {
         RepSample((HPSTR) lpOutData,lpInData,nRep,nBytesPerSample,nChannels);  // this routine should interpolate the samples
	 lpOutData = lpOutData + nRep * SampleSize;
	 lpInData = lpInData + SampleSize;
      }
      /* up sample last sample without filtering */
      for (n=0;n<nRep;n++)
      {
         LPSAMPLE lpTmp = (LPSAMPLE) lpInData;
         for (i=0;i<SampleSize;i++)
            *lpOutData++ = *lpTmp++;
      }
   }
   else 
   {
      for (dw = dwNewNumSamples-1; dw--; )
      {
         AvgSample((HPSTR) lpOutData,lpInData,nSkip,nBytesPerSample,nChannels);
         lpOutData = lpOutData + SampleSize;
         lpInData = lpInData + nSkip * SampleSize;
      }
      // just copy the last sample
      for (i=0;i<SampleSize;i++)
         *lpOutData++ = *lpInData++;
   }
   
   GlobalFreePtr(lpInSave);
   return (HPSTR) lpOutSave;
}

/* Note: waveFormatConvert and supporting functions were written in C for portability.  They are not very fast 
** if they need to convert all the wave file attributes, but if you are only up/down sampling or something similar
** they perform ok.  Since games will typically open the wave files and then play the game then it is not such a
** big deal.  If it does become a big deal then we should looking into using assembly optimized code (e.g. waveconv.asm
** already exists, but it uses 386 code which is non portable to NT on non-8086 machines.
** BUGBUG:
** The conversion allocates a new buffer each time it modifies the data, converts the old buffer to the
** new buffer and then release the old buffer.  This is somewhat inefficient, and there is the potential for the GlobalAlloc
** to fail (especially for large buffers).  If you are feeling ambitious, feel free to rewrite them.
*/
HPSTR WaveFormatConvert(LPPCMWAVEFORMAT lpOutWF, LPPCMWAVEFORMAT lpInWF, HPSTR lpInData, DWORD *dwDataSize)
{
   /* if wave formats are the same just return the input buffer */
   if ( (lpInWF->wf.nChannels == lpOutWF->wf.nChannels) &&
       (lpInWF->wf.nSamplesPerSec == lpOutWF->wf.nSamplesPerSec) &&
       (lpInWF->wBitsPerSample == lpOutWF->wBitsPerSample) )
      return lpInData;
   
   /* block align the data, eg. convert from 16 bit samples to eight bit samples or vice versa */
   if (!(lpInData = BitsPerSampleAlign(lpInData,lpInWF->wBitsPerSample,lpOutWF->wBitsPerSample,dwDataSize)))
      return NULL;
   
   /* channel align the data, e.g. convert from stereo to mono or vice versa */
   if (!(lpInData = ChannelAlign(lpInData, lpInWF->wf.nChannels, 
				 lpOutWF->wf.nChannels, 
				 lpOutWF->wBitsPerSample/BITSPERBYTE,
				 dwDataSize)))
      return NULL;
   
   /* SamplesPerSec align the data, e.g. convert from 44.1kHz to 11.025 kHz or vice versa*/
   if (!(lpInData = SamplesPerSecAlign(lpInData,lpInWF->wf.nSamplesPerSec,
				       lpOutWF->wf.nSamplesPerSec, 
				       lpOutWF->wBitsPerSample/BITSPERBYTE, 
				       lpOutWF->wf.nChannels, dwDataSize)))
      return NULL;
   
   return lpInData;
}	

// Added volume 8/95 ARK
LPMIXWAVE WINAPI WaveMixOpenWave(HANDLE hMixSession, LPCSTR szWaveFilename, HINSTANCE hInst, DWORD dwFlags, int volume)
{
   MMIOINFO        mmioInfo;
   MMCKINFO	   mmckinfoParent;
   MMCKINFO        mmckinfoSubchunk;
   DWORD           dwFmtSize;
   DWORD           dwDataSize;
   HWAVEOUT        hWaveOutTmp;
   HGLOBAL         hMem   = NULL;
   HMMIO           hmmio  = NULL;
   LPMIXWAVE       lpMix  = NULL;
   HPSTR           lpData = NULL;
   int		   i;
   WORD		   wDeviceID;
   
   ODS(2,"<WaveMixOpenWave>");
   
   /* Make sure a waveform output device supports this format. */
   if (g=SessionToGlobalDataPtr(hMixSession))
      wDeviceID = g->wDeviceID;
   else
      wDeviceID = (WORD)WAVE_MAPPER;
   
   if (waveOutOpen(&hWaveOutTmp, wDeviceID, (LPCWAVEFORMATEX)&g->pcm, (DWORD_PTR)NULL, 0L, WAVE_FORMAT_QUERY))
   {
      if (gfShow)
	 MessageBox(NULL, "The waveform device can't play this format.", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      return NULL;
   }
   
   /* allocate a header for the wave, this will lets us keep the wave information along with the wave
   ** data, then the calling application has a simpler API to us
   */
   if (!(lpMix=(LPMIXWAVE)GlobalAllocPtr(GMEM_SHARE|GMEM_ZEROINIT,sizeof(MIXWAVE))))
   {
      if (gfShow)
	 MessageBox(NULL, gszMemError, gszAppName, MB_OK | MB_ICONINFORMATION);
      return NULL;
   }
   
   if (dwFlags & WMIX_RESOURCE)
   {
      HRSRC hRsrc = FindResource(hInst,szWaveFilename,"WAVE");
      if (!(hMem = LoadResource(hInst, hRsrc)))
      {
	 if (HIWORD(szWaveFilename)) //  then have a string
	    wsprintf(gsz,"Failed to open 'WAVE' resource '%s'.",szWaveFilename);
	 else
	    wsprintf(gsz,"Failed to open 'WAVE' resource %u.",LOWORD(szWaveFilename));
	 
	 if (gfShow)
	    MessageBox(NULL, gsz, gszAppName, MB_OK | MB_ICONEXCLAMATION);
	 goto ErrorCleanup;
      }
      
      memset((LPSTR)&mmioInfo,0,sizeof(MMIOINFO));
      if (!(mmioInfo.pchBuffer = (HPSTR) LockResource(hMem)))
      {
	 if (gfShow)
	    MessageBox(NULL, "Failed to lock 'WAVE' resource", gszAppName, MB_OK | MB_ICONEXCLAMATION);
	 FreeResource(hMem);
	 hMem = NULL;
	 goto ErrorCleanup;
      }
      mmioInfo.cchBuffer=SizeofResource(hInst,hRsrc);
      mmioInfo.fccIOProc=FOURCC_MEM;
      mmioInfo.adwInfo[0]=0;
      
      if (!(hmmio = mmioOpen(NULL, &mmioInfo, MMIO_READ)))
      {
	 if (gfShow)
	 {
	    wsprintf(gsz,"Failed to open resource, mmioOpen error=%u.\nMay need to make sure resource is marked read-write",mmioInfo.wErrorRet);
	    MessageBox(NULL, gsz, gszAppName, MB_OK | MB_ICONEXCLAMATION);
	 }
	 goto ErrorCleanup;
      }
   }
   else if (dwFlags & WMIX_MEMORY)
   {
      memcpy((LPSTR)&mmioInfo,szWaveFilename,sizeof(MMIOINFO));
      
      if (!(hmmio = mmioOpen(NULL, &mmioInfo, MMIO_READ)))
      {
	 if (gfShow)
	 {
	    wsprintf(gsz,"Failed to open memory file, mmioOpen error=%u.\nMay need to make sure memory is read-write",mmioInfo.wErrorRet);
	    MessageBox(NULL, gsz, gszAppName, MB_OK | MB_ICONEXCLAMATION);
	 }
	 goto ErrorCleanup;
      }
   }
   else /* Open the given file for reading using buffered I/O. */
   {
     if (!(hmmio = mmioOpen((LPSTR) szWaveFilename, NULL, MMIO_READ | MMIO_ALLOCBUF)))
      {
	 if (gfShow)
	 {
	    wsprintf(gsz,"Failed to open wave file %s.",szWaveFilename);
	    MessageBox(NULL, gsz, gszAppName, MB_OK | MB_ICONEXCLAMATION);
	 }
	 goto ErrorCleanup;
      }
   }
   
   /* Locate a 'RIFF' chunk with a 'WAVE' form type
    * to make sure it's a WAVE file.
    */
   mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
   if (mmioDescend(hmmio, (LPMMCKINFO) &mmckinfoParent, NULL, MMIO_FINDRIFF))
   {
      if (gfShow)
	 MessageBox(NULL, "This is not a WAVE file.", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      goto ErrorCleanup;
   }
   
   /* Now, find the format chunk (form type 'fmt '). It should be
    * a subchunk of the 'RIFF' parent chunk.
    */
   mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
   if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
   {
      if (gfShow)
	 MessageBox(NULL, "WAVE file is corrupted.", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      goto ErrorCleanup;
   }
   
   /* Get the size of the format chunk, allocate and lock memory for it. */
   dwFmtSize = mmckinfoSubchunk.cksize;
   if (dwFmtSize != sizeof(PCMWAVEFORMAT))
      ODS(2,"format size is not the size of PCMWAVEFORMAT!");
   
   /* Read the format chunk. */
   if (mmioRead(hmmio, (HPSTR) &lpMix->pcm, sizeof(PCMWAVEFORMAT)) != (LONG) sizeof(PCMWAVEFORMAT))
   {
      if (gfShow)
	 MessageBox(NULL, "Failed to read format chunk.", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      goto ErrorCleanup;
   }
   
   /* Make sure it's a PCM file. */
   
   if (lpMix->pcm.wf.wFormatTag != WAVE_FORMAT_PCM)
   {
      if (gfShow)
	 MessageBox(NULL, "The file is not a PCM file.", gszAppName, MB_OK | MB_ICONEXCLAMATION);
		goto ErrorCleanup;
   }
   
   /* Ascend out of the format subchunk. */
   mmioAscend(hmmio, &mmckinfoSubchunk, 0);
   
   /* Find the data subchunk. */
   mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
   if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
   {
      if (gfShow)
	 MessageBox(NULL, "WAVE file has no data chunk.", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      goto ErrorCleanup;
   }
   
    /* Get the size of the data subchunk. */
   dwDataSize = mmckinfoSubchunk.cksize;
   if (dwDataSize == 0L)
   {
      if (gfShow)
	 MessageBox(NULL, "The data chunk has no data.", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      goto ErrorCleanup;
   }
   
   /* Allocate and lock memory for the waveform data. */
   if (!(lpData = (HPSTR)GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE, dwDataSize)))
   {
      if (gfShow)
	 MessageBox(NULL, gszMemError, gszAppName, MB_OK | MB_ICONINFORMATION);
      goto ErrorCleanup;
   }
   
   /* Read the waveform data subchunk. */
   if (mmioRead(hmmio, (HPSTR) lpData, dwDataSize) != (LONG) dwDataSize)
   {
      if (gfShow)
	 MessageBox(NULL, "Failed to read data chunk.", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      goto ErrorCleanup;
   }
   
   lpData = WaveFormatConvert(&g->pcm,&lpMix->pcm, lpData, &dwDataSize);
   if (!lpData)
   {
      if (gfShow)
	 MessageBox(NULL, "Failed to convert wave format.", gszAppName, MB_OK | MB_ICONEXCLAMATION);
      goto ErrorCleanup;
   }
   
   // Added 8/95 ARK 
   ScaleSample(lpData, dwDataSize, g->pcm.wBitsPerSample, volume);
   
   /* We're done with the file, close it. */
   mmioClose(hmmio, 0);
   if (hMem)
   {
      UnlockResource(hMem);
      FreeResource(hMem);
   }
   
   /* Set up WAVEHDR structure and prepare it to be written to wave device. */
   lpMix->wh.lpData = lpData;
   lpMix->wh.dwBufferLength = dwDataSize;
   lpMix->wh.dwFlags = 0L;
   lpMix->wh.dwLoops = 0L;
   lpMix->wh.dwUser = 0L;
   
   if (HIWORD(szWaveFilename)==0) /* then name was a resource ID */
      wsprintf(lpMix->szWaveFilename,"res#%u",LOWORD(szWaveFilename));
   else
   {	/* copy the file name, if name too long just copy the last MAXFILENAME characters */
      i = lstrlen(szWaveFilename);
      i = (i<=MAXFILENAME) ? 0 : i-MAXFILENAME;
      lstrcpy(lpMix->szWaveFilename,szWaveFilename+i);
   }
   
   return lpMix;
   
 ErrorCleanup:
   if (hmmio) mmioClose(hmmio,0);
   if (lpMix) GlobalFreePtr(lpMix);
   if (lpData) GlobalFreePtr(lpData);
   if (hMem)
   {
      UnlockResource(hMem);
      FreeResource(hMem);
   }
   return NULL;
}

DWORD FigureOutDMABufferSize(int iDefBufferSize, LPWAVEFORMAT lpWaveFormat)
{
   DWORD dwDMALen;
   
   Assert(g);
   
   /* if len has been set in ini file for this device - use it */
   dwDMALen=(DWORD)GetPrivateProfileInt(g->szDevicePName,"WaveBlockLen",0,gszIniFile);
   if (dwDMALen != 0)
   {
      PEG(MINWAVEBLOCKLEN,dwDMALen,MAXWAVEBLOCKLEN);
      return dwDMALen;
   }
   /* if len has been set in ini file for the default device - use it */
   if ((dwDMALen=(DWORD)iDefBufferSize)!=0)
   {
      PEG(MINWAVEBLOCKLEN,dwDMALen,MAXWAVEBLOCKLEN);
      return dwDMALen;
   }
   
   /* if we are running on NT don't want to use our technique of flooding the wave output to
   ** try to figure out the DMA size since the way NT protects the hardware makes this inaccurate
   */
#define DEFAULT_NTWAVEBLOCKLEN	2048
   
   return DEFAULT_NTWAVEBLOCKLEN * (lpWaveFormat->nSamplesPerSec / 11025);

   // Removed old Win16 code here 8/95 ARK
}

HANDLE WINAPI WaveMixConfigureInit(LPMIXCONFIG lpConfig)
{
   WAVEOUTCAPS caps;
   int i,iDevices;
   UINT u;
   int iDefRemix;
   int iDefGoodWavePos;
   int iDefWaveBlocks;
   int iDefWaveBlockLen;
   int iDefSamplesPerSec,iSamplesPerSec;
   
   ODS(2,"<WaveMixConfigureInit>");
   
   // if a size is passed in which we don't understand, then don't use the passed in info
   // it may have been created with another version
   if (lpConfig && lpConfig->wSize != sizeof(MIXCONFIG))
      lpConfig = NULL;
   
   if ((iDevices=waveOutGetNumDevs())==0)
      return NULL;
   
//   gfShow = GetPrivateProfileInt("general","ShowDevices",0,gszIniFile)!=0;
   gfShow = 0;     // Don't put up any MessageBoxes ARK
   
   if (!(g = (PGLOBALS)LocalAlloc(LPTR,sizeof(GLOBALS))))
      return NULL;
   g->wMagic1=g->wMagic2=MAGICNO;
   
   for (i=0; i<MAXCHANNELS; i++)
      g->aChannel[i]=(PCHANNELNODE)-1;
   
   memcpy(&g->pcm,&gpFormat,sizeof(PCMWAVEFORMAT));  /* default wave format */
   
#define DEFAULT_REMIXALGORITHM  1    /* ResetRemix()                */
#define DEFAULT_GOODWAVPOS		0    /* use timeGetTime()           */
#define DEFAULT_WAVEBLOCKS 		3    /* number of ping-pong buffers */
#define DEFAULT_WAVEBLOCKLEN	0    /* 0 forces code to figure DMA size */
#define DEFAULT_SAMPLESPERSEC	11	 /* 11025 hz                    */
   
   iDefRemix        = GetPrivateProfileInt(gszDefault,"Remix",DEFAULT_REMIXALGORITHM,gszIniFile);
   iDefGoodWavePos  = GetPrivateProfileInt(gszDefault,"GoodWavePos",DEFAULT_GOODWAVPOS,gszIniFile);
   iDefWaveBlocks   = GetPrivateProfileInt(gszDefault,"WaveBlocks",DEFAULT_WAVEBLOCKS,gszIniFile);
   iDefWaveBlockLen = GetPrivateProfileInt(gszDefault,"WaveBlockLen",DEFAULT_WAVEBLOCKLEN,gszIniFile) & 0xFFFC; // force dword align 
   iDefSamplesPerSec= GetPrivateProfileInt(gszDefault,"SamplesPerSec",DEFAULT_SAMPLESPERSEC,gszIniFile);
   
   lstrcpy(g->szDevicePName,gszDefault);
   g->wDeviceID=(UINT)GetPrivateProfileInt("general","WaveOutDevice",0,gszIniFile);
   if (g->wDeviceID >= (UINT)iDevices) // make sure we don't use an invalid ID number
      g->wDeviceID=(UINT)WAVE_MAPPER;
   
   // BUGBUG: sometimes this function returns an error even if there is a device installed
   //         so just act as if I don't know what card it is and don't worry about it.
   if (u=waveOutGetDevCaps(g->wDeviceID,&caps,sizeof(WAVEOUTCAPS)))
      lstrcpy(caps.szPname,"Unknown Device");
   
   if (caps.dwSupport & WAVECAPS_SYNC)
   {
      if (gfShow)
      {
	 wsprintf(gsz,"%s is a syncronous (blocking) wave output device.  This will not permit audio to play while other applications are running.",(LPSTR)caps.szPname);
	 MessageBox(NULL, gsz, gszAppName, MB_OK | MB_ICONINFORMATION);
      }
      LocalFree((HLOCAL)g);
      g = NULL;
      return NULL;
   }
   
   if (GetPrivateProfileInt("not compatible",caps.szPname,0,gszIniFile))
   {
      if (gfShow)
      {
	 wsprintf(gsz,"%s is not compatible with the realtime wavemixer.",(LPSTR)caps.szPname);
	 MessageBox(NULL, gsz, gszAppName, MB_OK | MB_ICONINFORMATION);
      }
      LocalFree((HLOCAL)g);
      g = NULL;
      return NULL;
   }
   
   lstrcpy(g->szDevicePName,caps.szPname);
   
   g->pfnRemix = (GetPrivateProfileInt(g->szDevicePName,"Remix",iDefRemix,gszIniFile)
		  == 2) ? NoResetRemix : ResetRemix;
   g->fGoodGetPos=GetPrivateProfileInt(g->szDevicePName,"GoodWavePos",iDefGoodWavePos,gszIniFile);
   
   g->iNumWaveBlocks=GetPrivateProfileInt(g->szDevicePName,"WaveBlocks",iDefWaveBlocks,gszIniFile);
   PEG(MINWAVEBLOCKS,g->iNumWaveBlocks,MAXWAVEBLOCKS);
   
   if (lpConfig && (lpConfig->dwFlags & WMIX_CONFIG_CHANNELS))
   {
      if (lpConfig->wChannels > 1 && caps.wChannels > 1)
      {
	 g->pcm.wf.nChannels = 2;
	 g->pcm.wf.nBlockAlign = 2;  // number of bytes for one sample including both channels
      }
   }
   
   iSamplesPerSec= GetPrivateProfileInt(g->szDevicePName,"SamplesPerSec",iDefSamplesPerSec,gszIniFile);
   
   /* application wants to play at a specific sampling rate */
   if (lpConfig && (lpConfig->dwFlags & WMIX_CONFIG_SAMPLINGRATE))
   {
      switch (lpConfig->wSamplingRate)
      {
      case 11:
	 iSamplesPerSec=11;
	 break;
      case 22:
	 iSamplesPerSec=22;
	 break;
      case 44:
	 iSamplesPerSec=44;
	 break;
      default: /* leave as read from ini file */
	 break;
      }
   }
   
   /* adjust the wave format if the sampling rate is no longer set to the default */
   switch(iSamplesPerSec)
   {
   default: /* default is 11 */
   case 11:
      g->pcm.wf.nAvgBytesPerSec = 11025 * BYTESPERSAMPLE * g->pcm.wf.nChannels;
      break;
   case 22:
      g->pcm.wf.nSamplesPerSec = 22050;
      g->pcm.wf.nAvgBytesPerSec = 22050 * BYTESPERSAMPLE * g->pcm.wf.nChannels;
      break;
   case 44:
      g->pcm.wf.nSamplesPerSec = 44100;
      g->pcm.wf.nAvgBytesPerSec = 44100 * BYTESPERSAMPLE * g->pcm.wf.nChannels;
      break;
   }
   
   g->dwWaveBlockLen=FigureOutDMABufferSize(iDefWaveBlockLen,&g->pcm.wf);
   
#if DEBUG
   wsprintf(gsz,"global memory alloced for ping pong buffers = %lu bytes\n\r",(sizeof(XWAVEHDR)+MAXWAVEBLOCKLEN)*g->iNumWaveBlocks);
   ODS(1,gsz);
   
   if (gfShow)
   {
      wsprintf(gsz,"Using:\t%s.\n\tRemix = %s\n\tGoodGetPos=%s\n\t%d ping pong wave blocks\n\tWave block len = %lu bytes\n\tgpfnMixit=%s\n\tSamplesPerSec=%lu,\n\tChannels=%d",
	       (LPSTR)caps.szPname,
	       (g->pfnRemix == ResetRemix) ? (LPSTR)"Reset" : (LPSTR)"NoReset",
	       (g->fGoodGetPos) ? (LPSTR)"TRUE" : (LPSTR)"FALSE",
	       g->iNumWaveBlocks,
	       g->dwWaveBlockLen,
	       (gpfnMixit == cmixit) ? (LPSTR)"cmixit" : (LPSTR)"386 mixit",
	       g->pcm.wf.nSamplesPerSec,
	       g->pcm.wf.nChannels
	       );
      MessageBox(NULL, gsz, gszAppName, MB_OK | MB_ICONINFORMATION);
   }
#endif
   
   return (HANDLE) g;
}

HANDLE WINAPI WaveMixInit(void)
{
   MIXCONFIG config;
   
   memset((LPSTR)&config,0,sizeof(MIXCONFIG));
   config.wSize = sizeof(MIXCONFIG);
   
   return WaveMixConfigureInit(&config);
}

UINT WINAPI WaveMixCloseSession(HANDLE hMixSession)
{
   ODS(2,"<WaveMixCloseSession>");
   
   if (!(g=SessionToGlobalDataPtr(hMixSession)))
      return MMSYSERR_INVALHANDLE;
   
   WaveMixActivate(hMixSession,FALSE);
   WaveMixCloseChannel(hMixSession,0,WMIX_ALL);
   
   /* null out all the data so we can catch references to it after we have freed it */
   memset(g,0,sizeof(GLOBALS));
   g=NULL;
   
   if (!hMixSession ||	!LocalFree((HLOCAL)hMixSession))
      return MMSYSERR_INVALHANDLE;
   
   return MMSYSERR_NOERROR;
}

WORD WINAPI WaveMixGetInfo(LPWAVEMIXINFO lpWaveMixInfo)
{
   const static WAVEMIXINFO Info = {sizeof(WAVEMIXINFO),MAJORVERSION,MINORVERSION,__DATE__,
				    WAVE_FORMAT_1M08|\
  			            WAVE_FORMAT_1S08|\
				    WAVE_FORMAT_1M16|\
			            WAVE_FORMAT_1S16|\
				    WAVE_FORMAT_2M08|\
				    WAVE_FORMAT_2S08|\
			            WAVE_FORMAT_2M16|\
				    WAVE_FORMAT_2S16|\
				    WAVE_FORMAT_4M08|\
				    WAVE_FORMAT_4S08|\
  				    WAVE_FORMAT_4M16|\
			            WAVE_FORMAT_4S16 };
   
   if (lpWaveMixInfo->wSize != sizeof(WAVEMIXINFO))  /* the application requires a different version of wavemix */
      return sizeof(WAVEMIXINFO);
   
   memcpy(lpWaveMixInfo,(LPVOID)&Info,sizeof(WAVEMIXINFO));
   return 0;
}

int WINAPI LibMain(HINSTANCE hModule,WORD wDataSeg,WORD cbHeapSize,LPSTR lpszCmdLine)
{
   WNDCLASS    wc;
   
//   giDebug=GetPrivateProfileInt("general","debug",0,gszIniFile);
   giDebug = 0;  // Don't spew debugging junk ARK
   
   gpfnMixit=cmixit;
   ghDLLInst = hModule;
   
   wc.hCursor        = NULL;
   wc.hIcon          = NULL;
   wc.lpszMenuName   = NULL;
   wc.lpszClassName  = gszAppName;
   wc.hbrBackground  = NULL;
   wc.hInstance      = ghDLLInst;
   wc.style          = 0;
   wc.lpfnWndProc    = WndProc;
   wc.cbWndExtra     = 0;
   wc.cbClsExtra     = 0;
   
   if (!RegisterClass(&wc))
      return FALSE;
   
   InitChannelNodes();  // BUGBUG: need to do this for each session
   
#if DEBUG
   wsprintf(gsz,"sizeof(CHANNELNODE)=%d X %d nodes = %d bytes in data segment\n\r",sizeof(CHANNELNODE), MAXQUEUEDWAVES, sizeof(CHANNELNODE)* MAXQUEUEDWAVES);
   ODS(1,gsz);
   wsprintf(gsz,"sizeof(GLOBALS)=%d local bytes\n\r",sizeof(GLOBALS));
   ODS(1,gsz);
#endif	
   
   return 1;
}

BOOL WINAPI DllMain(HINSTANCE hModule, ULONG Reason, LPVOID pv)
{
   switch (Reason)
   {
   case DLL_PROCESS_ATTACH:
      return LibMain(hModule, 0, 0, NULL);
   }
   
   return TRUE;
}
