/*
 * textures.h:  Header file for textures.cpp
 */

#ifndef __textures_h
#define __textures_h

struct TextureInfo
{
   SHORT Width;
   SHORT Height;
   char  Name[MAX_BITMAPNAME];
   char  filename[MAX_BITMAPNAME];
   int   number;   // Bitmap #
   int   shrink;   // Factor to shrink bitmap by when displaying it in client
};

void AddWTextureInfo (char *name);
void ReadWTextureInfo ();
void ForgetFTextureInfo ();

void AddWTextureInfo (char *startEntry, char *endEntry);
void ReadFTextureInfo ();
void ForgetWTextureInfo ();

void GetWallTextureSize (SHORT *xsize_r, SHORT *ysize_r, int texnum);
TextureInfo *FindTextureByName(char *name);
TextureInfo *FindTextureByNumber(int num);
TextureInfo *GetSidedefTextureInfo(SHORT sdnum);

int TextureToNumber(char *texture);

extern SHORT NumWTexture;	/* number of wall textures */
extern TextureInfo **WTexture;	/* array of wall texture names */

extern SHORT NumFTexture;	/* number of floor/ceiling textures */
extern TextureInfo **FTexture;	/* array of texture names */

#endif
