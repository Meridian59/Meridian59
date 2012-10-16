//------------------------------------------------------------------------------
// File: MediaTypes.h
//
// Desc: DirectShow sample code - hardware/project-specific support for
//       Jukebox application.
//
// Copyright (c) 1998-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
// Structures
//
typedef struct _media_info
{
    LPTSTR pszType;
    LPTSTR pszName;

} MEDIA_INFO, *PMEDIA_INFO;


//
// Some projects support different types of DirectShow media
//
#define DEFAULT_SEARCH_PATH   TEXT("\\")

#define NUM_MEDIA_TYPES   21

const MEDIA_INFO TypeInfo[NUM_MEDIA_TYPES] = {
      {TEXT("*.qt"),   TEXT("QuickTime video") },
      {TEXT("*.mov"),  TEXT("QuickTime video") },
      {TEXT("*.avi"),  TEXT("AVI video")    },
      {TEXT("*.mpg"),  TEXT("MPEG video")   },
      {TEXT("*.mpe*"), TEXT("MPEG video")   },  /* MPE, MPEG */
      {TEXT("*.m1v"),  TEXT("MPEG video")   },  /* MPEG-1 video */
      {TEXT("*.wav"),  TEXT("WAV audio")    },
      {TEXT("*.au"),   TEXT("AU audio")     },
      {TEXT("*.aif*"), TEXT("AIFF audio")   },  /* AIF, AIFF, AIFC */
      {TEXT("*.snd"),  TEXT("SND audio")    },
      {TEXT("*.mpa"),  TEXT("MPEG audio")   },  /* MPEG audio */
      {TEXT("*.mp1"),  TEXT("MPEG audio")   },  /* MPEG audio */
      {TEXT("*.mp2"),  TEXT("MPEG audio")   },  /* MPEG audio */
      {TEXT("*.mid"),  TEXT("MIDI")         },  /* MIDI       */
      {TEXT("*.midi"), TEXT("MIDI")         },  /* MIDI       */
      {TEXT("*.rmi"),  TEXT("MIDI")         },  /* MIDI       */
      {TEXT("*.asf"),  TEXT("ASF Video")       },  /* Advanced Streaming */
      {TEXT("*.wma"),  TEXT("Windows Audio")   },  /* Windows Media Audio */
      {TEXT("*.mp3"),  TEXT("MP3 audio")       },  /* MPEG-1 Layer III */
      {TEXT("*.wmv"),  TEXT("Windows Video")   },  /* Windows Media Video */
      {TEXT("*.dat"),  TEXT("Video CD")     },  /* Video CD format */
};

