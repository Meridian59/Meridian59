//------------------------------------------------------------------------------
// File: MediaTypes.h
//
// Desc: DirectShow sample code - hardware/project-specific support for
//       StillView application.
//
// Copyright (c) 1998 - 2001, Microsoft Corporation.  All rights reserved.
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

#define NUM_MEDIA_TYPES   5

const MEDIA_INFO TypeInfo[NUM_MEDIA_TYPES] = {
      {TEXT("*.bmp"),  TEXT("Bitmap")       },
      {TEXT("*.jpg"),  TEXT("JPEG Image")   },
      {TEXT("*.jpeg"), TEXT("JPEG Image")   },
      {TEXT("*.gif"),  TEXT("GIF Image")    },
      {TEXT("*.tga"),  TEXT("Targa File")   },
};

