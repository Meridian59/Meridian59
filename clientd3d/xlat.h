// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * xlat.h:  Header for xlat.c
 */

#ifndef _XLAT_H
#define _XLAT_H

/***************************************************************************/

typedef struct {
    BYTE entry[256];
} xlat;

typedef struct {
    BYTE entry[256*256];
} bixlat;

/***************************************************************************/

// fastXLAT for cases when xlat ptr is known to be valid, nonnull
// safeXLAT for cases when xlat ptr may be null

#define fastXLAT(index,ptable) (ptable->entry[(index)])
#define fastBIXLAT(index1,index2,ptable)  (ptable->entry[((index1)<<8)|(index2)])

#define safeXLAT(index,ptable) (ptable? ptable->entry[index] : index)
#define safeBIXLAT(index1,index2,ptable)  (ptable? ptable->entry[((index1)<<8)|(index2)] : index1)

void InitStandardXlats(HPALETTE hPalette);

M59EXPORT xlat* FindStandardXlat(UINT uXlatID);
M59EXPORT bixlat* FindStandardBiXlat(UINT uBiXlatID);

M59EXPORT void CalcBlendXlat(HPALETTE hPalette, xlat* pXlat, COLORREF crMix, int partsFirst, int partsSecond);
M59EXPORT void CalcFilterXlat(HPALETTE hPalette, xlat* pXlat, COLORREF crMix);
M59EXPORT void CalcTransparentBiXlat(HPALETTE hPalette, bixlat* pBiXlat, COLORREF* pcrMix, int partsFirst, int partsSecond);

M59EXPORT void XlatDib(BYTE* pabyBits, int width, int height, xlat* pXlat);

/***************************************************************************/

/* Standard XLATs */

/*

**************** 0x00 - 0x0F
****************
****************
****--********--  (*'s are used XLAT_* constants.)
-*********---***
-***-********---
**------------**
****************
*****--XXXXXXXXX 0x80 - 0x8F
XXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXX  (X's are XLAT_GUILDCOLOR_BASE-XLAT_GUILDCOLOR_END.)
XXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXX 0xF0 - 0xFF

Whenever you define a new color, mark off the appropriate spot above with *.

*/


#define XLAT_IDENTITY     0x0000

// For faces
#define XLAT_DBLUETOSKIN1      0x0001
#define XLAT_DBLUETOSKIN2      0x0002
#define XLAT_DBLUETOSKIN3      0x0003
#define XLAT_DBLUETOSKIN4      0x0004
#define XLAT_DBLUETOSICKGREEN  0x0005
#define XLAT_DBLUETOSICKYELLOW 0x0006
#define XLAT_DBLUETOGRAY       0x0007
#define XLAT_DBLUETOLBLUE      0x0008
#define XLAT_DBLUETOASHEN      0x0009 // also makes gray eyes reddish

// For multicolored objects
#define XLAT_GRAYTOORANGE  0x000A
#define XLAT_GRAYTODGREEN  0x000B
#define XLAT_GRAYTOBGREEN  0x000C
#define XLAT_GRAYTOSKY     0x000D
#define XLAT_GRAYTODBLUE   0x000E
#define XLAT_GRAYTOPURPLE  0x000F
#define XLAT_GRAYTOGOLD    0x0010
#define XLAT_GRAYTOBBLUE   0x0011
#define XLAT_GRAYTORED     0x0012
#define XLAT_GRAYTOLORANGE 0x0013
#define XLAT_GRAYTOLGREEN  0x0014
#define XLAT_GRAYTOLBGREEN 0x0015
#define XLAT_GRAYTOLSKY    0x0016
#define XLAT_GRAYTOLBLUE   0x0017
#define XLAT_GRAYTOLPURPLE 0x0018
#define XLAT_GRAYTOLGOLD   0x0019

// New red, blue and purple to black translations.
#define XLAT_REDTOBLACK    0x005A
#define XLAT_BLUETOBLACK   0x005B
#define XLAT_PURPLETOBLACK 0x005C

#define XLAT_REDTODGREEN1  0x007A
#define XLAT_REDTODGREEN2  0x007B
#define XLAT_REDTODGREEN3  0x007C
#define XLAT_REDTOBLACK1   0x007D
#define XLAT_REDTOBLACK2   0x007E
#define XLAT_REDTOBLACK3   0x007F
#define XLAT_REDTODKBLACK1 0x0080
#define XLAT_REDTODKBLACK2 0x0081
#define XLAT_REDTODKBLACK3 0x0082
#define XLAT_REDBLK_BLWHT  0x0083
#define XLAT_BLBLK_REDWHT  0x0084

#define XLAT_GRAYTOLBBLUE  0x0020
#define XLAT_GRAYTOLRED    0x0021
#define XLAT_GRAYTOKORANGE 0x0022
#define XLAT_GRAYTOKGREEN  0x0023
#define XLAT_GRAYTOKBGREEN 0x0024
#define XLAT_GRAYTOKSKY    0x0025
#define XLAT_GRAYTOKBLUE   0x0026
#define XLAT_GRAYTOKPURPLE 0x0027
#define XLAT_GRAYTOKGOLD   0x0028
#define XLAT_GRAYTOKBBLUE  0x0029
#define XLAT_GRAYTOKRED    0x002A
#define XLAT_GRAYTOKGRAY   0x002B

#define XLAT_GRAYTOSKIN1   0x001A
#define XLAT_GRAYTOSKIN2   0x001B
#define XLAT_GRAYTOSKIN3   0x001C
#define XLAT_GRAYTOSKIN4   0x001D
#define XLAT_GRAYTOSKIN5   0x001E

#define XLAT_GRAYTOBLACK    0x002C
#define XLAT_GRAYTOOLDHAIR1 0x002D
#define XLAT_GRAYTOOLDHAIR2 0x002E
#define XLAT_GRAYTOBLOND    0x002F
#define XLAT_GRAYTOOLDHAIR3 XLAT_GRAYTOBLOND
#define XLAT_GRAYTOPLATBLOND 0x0030

// For sandstorm
#define XLAT_BLEND25YELLOW 0x0039

// For statues
#define XLAT_FILTERWHITE90  0x0031
#define XLAT_FILTERWHITE80  0x0032
#define XLAT_FILTERWHITE70  0x0033

#define XLAT_FILTERBRIGHT1  0x0036
#define XLAT_FILTERBRIGHT2  0x0037
#define XLAT_FILTERBRIGHT3  0x0038

#define XLAT_PURPLETOLBLUE  0x003A
#define XLAT_PURPLETOBRED   0x003B
#define XLAT_PURPLETOGREEN  0x003C
#define XLAT_PURPLETOYELLOW 0x003D

// For red pain
#define XLAT_BLEND10RED   0x0041
#define XLAT_BLEND20RED   0x0042
#define XLAT_BLEND30RED   0x0043
#define XLAT_BLEND40RED   0x0044
#define XLAT_BLEND50RED   0x0045
#define XLAT_BLEND60RED   0x0046
#define XLAT_BLEND70RED   0x0047
#define XLAT_BLEND80RED   0x0048
#define XLAT_BLEND90RED   0x0049
#define XLAT_BLEND100RED  0x004A

#define XLAT_BLEND25RED   0x0051
#define XLAT_BLEND25BLUE  0x0052
#define XLAT_BLEND25GREEN 0x0053
//
#define XLAT_BLEND50BLUE  0x0055
#define XLAT_BLEND50GREEN 0x0056
#define XLAT_BLEND75RED   0x0057
#define XLAT_BLEND75BLUE  0x0058
#define XLAT_BLEND75GREEN 0x0059

#define XLAT_FILTERRED    0x004D
#define XLAT_FILTERBLUE   0x004E
#define XLAT_FILTERGREEN  0x004F

#define XLAT_RAMPUP1      0x0060
#define XLAT_RAMPUP2      0x0061
// room to grow rampup3/rampdown3 here
#define XLAT_RAMPDOWN2    0x006E
#define XLAT_RAMPDOWN1    0x006F

// For white flash
#define XLAT_BLEND10WHITE   0x0070
#define XLAT_BLEND20WHITE   0x0071
#define XLAT_BLEND30WHITE   0x0072
#define XLAT_BLEND40WHITE   0x0073
#define XLAT_BLEND50WHITE   0x0074
#define XLAT_BLEND60WHITE   0x0075
#define XLAT_BLEND70WHITE   0x0076
#define XLAT_BLEND80WHITE   0x0077
#define XLAT_BLEND90WHITE   0x0078
#define XLAT_BLEND100WHITE  0x0079

// Guild Color Xlats take the red ramp and dark blue ramp
// through all possible permutation of 11 ramps, leaving the rest alone.
// 11*11 possible Xlats.  Useful for shields, clothes, or whatever.
//
#define XLAT_GUILDCOLOR_BASE 0x0087 // between 0x0087 and 0x00FF inclusive
#define XLAT_GUILDCOLOR_END  0x00FF // between 0x0087 and 0x00FF inclusive

/* Standard BIXLATs */

#define BIXLAT_BLEND25      0x8025
#define BIXLAT_BLEND50      0x8050
#define BIXLAT_BLEND75      0x8075

/***************************************************************************/

#endif /* #ifndef _XLAT_H */
