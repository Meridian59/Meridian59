//----------------------------------------------------------------------------
// ObjectWindows
// OWL NExt
//
// Created by Juraj Hercek <juraj@syncad.com>
//
//----------------------------------------------------------------------------


#if !defined(__OWL_WINELIB_H__)
#define __OWL_WINELIB_H__

//JJH most of the stuff is in winelib-port project

//<winsock problem section>
//This is here because we need to include winelib's winsock2.h before linux 
//stdlib.h, otherwise we get ntohl() and related defined in linux and not 
//in winelib's manner, problem occured during compilation of 
//source/owlcore/winsock.cpp and in subsequent files related to sockets.

//Check owl/include/private/defs.h:468
#define _OWLCLASS
#define _OWLCLASS_RTL

//JJH We need to include this before unix stdlib.h to avoid ntohl() problems.
#include <winsock2.h>
//</winsock problem section>

#endif //__OWL_WINELIB_H__


