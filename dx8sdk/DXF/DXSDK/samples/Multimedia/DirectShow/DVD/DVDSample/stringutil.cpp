//------------------------------------------------------------------------------
// File: StringUtil.cpp
//
// Desc: This file contains several enum to string conversion functions
//       which are used throughout the rest of the program.
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

#include <dshow.h>
#include <TCHAR.H>
#include "StringUtil.h"




//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------

// this will create a case statement from the name of the enum type
#define ENUM_CASE( x )	case x: return TEXT(#x)




//------------------------------------------------------------------------------
// Name: AsStr()
// Desc: This overloaded function converts enumerationd into human-readable 
//       text.  In most cases, it utilizes the ENUM_CASE macro to create
//       generic strings out of the enumerated types.  If you want more user-friendly
//       strings, use the form found in the DVD_TextStringType function.
//------------------------------------------------------------------------------

const TCHAR* AsStr( DVD_TextStringType IDCD )
// Turn an Enumerated TextStringType into a human-readable string
{
	switch( IDCD ) {
	case DVD_Struct_Volume:	        return TEXT("Struct_Volume");
	case DVD_Struct_Title:	        return TEXT("Struct_Title");
	case DVD_Struct_ParentalID:	    return TEXT("Struct_ParentalID");
	case DVD_Struct_PartOfTitle:	return TEXT("Struct_Chapter");
	case DVD_Struct_Cell:       	return TEXT("Struct_Cell");
	case DVD_Stream_Audio:      	return TEXT("Stream_Audio");
	case DVD_Stream_Subpicture:	    return TEXT("Stream_Sub-picture");
	case DVD_Stream_Angle:	        return TEXT("Stream_Angle");
	case DVD_Channel_Audio:	        return TEXT("Audio Channel");
	case DVD_General_Name:      	return TEXT("General_name");
	case DVD_General_Comments:  	return TEXT("General_comments");
    case DVD_Title_Series:          return TEXT("Title_Series");
    case DVD_Title_Movie:           return TEXT("Title_Movie");
    case DVD_Title_Video:           return TEXT("Title_Video");
    case DVD_Title_Album:           return TEXT("Title_Album");
    case DVD_Title_Song:            return TEXT("Title_Song");
    case DVD_Title_Other:           return TEXT("Title_Other");
    case DVD_Title_Sub_Series:      return TEXT("Title_Sub_Series");
    case DVD_Title_Sub_Movie:       return TEXT("Title_Sub_Movie");
    case DVD_Title_Sub_Video:       return TEXT("Title_Sub_Video");
    case DVD_Title_Sub_Album:       return TEXT("Title_Sub_Album");
    case DVD_Title_Sub_Song:        return TEXT("Title_Sub_Song");
    case DVD_Title_Sub_Other:       return TEXT("Title_Sub_Other");
    case DVD_Title_Orig_Series:     return TEXT("Title_Orig_Series");
    case DVD_Title_Orig_Movie:      return TEXT("Title_Orig_Movie");
    case DVD_Title_Orig_Video:      return TEXT("Title_Orig_Video");
    case DVD_Title_Orig_Album:      return TEXT("Title_Orig_Album");
    case DVD_Title_Orig_Song:       return TEXT("Title_Orig_Song");
    case DVD_Title_Orig_Other:      return TEXT("Title_Orig_Other");
	default:	                    return TEXT("Unknown");
	}
}

const TCHAR* AsStr( DVD_AUDIO_LANG_EXT ext )
{
	switch( ext )
	{
	ENUM_CASE( DVD_AUD_EXT_NotSpecified );
	ENUM_CASE( DVD_AUD_EXT_Captions );
	ENUM_CASE( DVD_AUD_EXT_VisuallyImpaired );
	ENUM_CASE( DVD_AUD_EXT_DirectorComments1 );
	ENUM_CASE( DVD_AUD_EXT_DirectorComments2 );
	default:
		return TEXT("UNKNOWN");
	}
}

const TCHAR* AsStr( DVD_SUBPICTURE_LANG_EXT ext )
{
	switch( ext )
	{
	ENUM_CASE( DVD_SP_EXT_NotSpecified );
	ENUM_CASE( DVD_SP_EXT_Caption_Normal );
	ENUM_CASE( DVD_SP_EXT_Caption_Big );
	ENUM_CASE( DVD_SP_EXT_Caption_Children );
	ENUM_CASE( DVD_SP_EXT_CC_Normal );
	ENUM_CASE( DVD_SP_EXT_CC_Big );
	ENUM_CASE( DVD_SP_EXT_CC_Children );
	ENUM_CASE( DVD_SP_EXT_Forced );
	ENUM_CASE( DVD_SP_EXT_DirectorComments_Normal );
	ENUM_CASE( DVD_SP_EXT_DirectorComments_Big );
	ENUM_CASE( DVD_SP_EXT_DirectorComments_Children );
	default:
		return TEXT("UNKNOWN");
	}
}

const TCHAR* AsStr( DVD_AUDIO_APPMODE ext )
{
	switch( ext )
	{
	ENUM_CASE( DVD_AudioMode_None );
	ENUM_CASE( DVD_AudioMode_Karaoke );
	ENUM_CASE( DVD_AudioMode_Surround );
	ENUM_CASE( DVD_AudioMode_Other );
	default: 
		return TEXT("UNKNOWN");
	}
}

const TCHAR* AsStr( DVD_AUDIO_FORMAT  ext )
{
	switch( ext )
	{
	ENUM_CASE( DVD_AudioFormat_AC3 );
	ENUM_CASE( DVD_AudioFormat_MPEG1 );
	ENUM_CASE( DVD_AudioFormat_MPEG1_DRC );
	ENUM_CASE( DVD_AudioFormat_MPEG2 );
	ENUM_CASE( DVD_AudioFormat_MPEG2_DRC );
	ENUM_CASE( DVD_AudioFormat_LPCM );
	ENUM_CASE( DVD_AudioFormat_DTS );
	ENUM_CASE( DVD_AudioFormat_SDDS );
	ENUM_CASE( DVD_AudioFormat_Other );
	default:
		return TEXT("UNKNOWN");
	}
}

const TCHAR* AsStr( DVD_KARAOKE_ASSIGNMENT ext )
{
	switch( ext )
	{
		ENUM_CASE( DVD_Assignment_reserved0 );
		ENUM_CASE( DVD_Assignment_reserved1 );
		ENUM_CASE( DVD_Assignment_LR  );
		ENUM_CASE( DVD_Assignment_LRM );
		ENUM_CASE( DVD_Assignment_LR1 );
		ENUM_CASE( DVD_Assignment_LRM1);
		ENUM_CASE( DVD_Assignment_LR12);
		ENUM_CASE( DVD_Assignment_LRM12);
	default:
		return TEXT("UNKNOWN");
	}
}

const TCHAR* AsStr( DVD_SUBPICTURE_TYPE type )
{
	switch( type )
	{
		ENUM_CASE( DVD_SPType_NotSpecified );
		ENUM_CASE( DVD_SPType_Language );
		ENUM_CASE( DVD_SPType_Other );
	default:
		return TEXT("UNKNOWN");
	}
}

const TCHAR* AsStr( DVD_SUBPICTURE_CODING type )
{
	switch( type )
	{
		ENUM_CASE( DVD_SPCoding_RunLength );
		ENUM_CASE( DVD_SPCoding_Extended );
		ENUM_CASE( DVD_SPCoding_Other );
	default:
		return TEXT("UNKNOWN");
	}
}

const TCHAR* AsStr( DVD_VIDEO_COMPRESSION type )
{
	switch( type )
	{
		ENUM_CASE( DVD_VideoCompression_Other );
		ENUM_CASE( DVD_VideoCompression_MPEG1 );
		ENUM_CASE( DVD_VideoCompression_MPEG2 );
	default:
		return TEXT("UNKNOWN");
	}
}

const TCHAR* AsStr( bool b )
{
	if( b )
		return TEXT("true");
	else
		return TEXT("false");
}

const TCHAR* AsStr( BOOL b )
{
	if( b )
		return TEXT("true");
	else
		return TEXT("false");
}


//------------------------------------------------------------------------------
// Name: KaraokeAsStr()
// Desc: This method returns the text name of the contents of a karaoke channel.
//       a function header should look like.  This function is set apart from 
//       the other AsStr functions because the DVD_KARAOKE_CONTENTS type is passed
//       as a word, not an enum.
//
//       A single channel can contain several of these content types.  Here just 
//       the first is returned. 
//------------------------------------------------------------------------------
const TCHAR* KaraokeAsStr (WORD type)
{
    if (type & DVD_Karaoke_GuideVocal1)
        return TEXT("GuideVocal1");
    if (type & DVD_Karaoke_GuideVocal2)
        return TEXT("GuideVocal2");
    if (type & DVD_Karaoke_GuideMelody1)
        return TEXT("GuideMelody1");
    if (type & DVD_Karaoke_GuideMelody2)
        return TEXT("GuideMelody2");
    if (type & DVD_Karaoke_GuideMelodyA)
        return TEXT("GuideMelodyA");
    if (type & DVD_Karaoke_GuideMelodyB)
        return TEXT("GuideMelodyB");
    if (type & DVD_Karaoke_SoundEffectA)
        return TEXT("SoundEffectA");
    if (type & DVD_Karaoke_SoundEffectB)
        return TEXT("SoundEffectB");
    else return TEXT("Unknown");
}




