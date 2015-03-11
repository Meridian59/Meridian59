//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1995 by Borland International, All Rights Reserved
//
// Provides generic definitions preceeding the declaration the classes of
// the ClassLibrary, ObjectComponents and ObjectWindows
// NOTE: This header must *NOT* contain guards
//----------------------------------------------------------------------------

#if !defined(__GNUC__) // eliminate warning
#pragma nopackwarning
#endif

#if !(defined(lint) || defined(_lint)) && !defined(RC_INVOKED)
# if defined(_MSC_VER) && ( _MSC_VER >= 800 ) 
#   pragma pack(pop)
#   pragma warning(pop)
# elif(defined(__BORLANDC__)) //for Win32, Win64 and OSX
#   pragma pack()
# elif(defined(__HPUX__))     //do we have a better way to detect aCC compiler?
//
# elif(defined(__SOLARIS__))  //and for forte compiler?
//
# elif(defined(__ARM__))      //and for gcc compiler for ARM?
//
# elif(defined(__GNUC__))     //for Linux and OSX
#   pragma pack(pop)
# endif
#endif // ! (defined(lint) || defined(_lint))
