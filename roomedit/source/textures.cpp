/*
 * textures.cpp:  Load and get info on texture files
 */

#include "common.h"
#pragma hdrstop

#include <windowsx.h>

#include "levels.h"
#include "dibutil.h"

SHORT NumWTexture = 0;			/* number of wall textures */
TextureInfo **WTexture = NULL;	/* array of wall texture info */

SHORT NumFTexture = 0;			/* number of floor/ceiling textures */
TextureInfo **FTexture = NULL;	/* array of texture info */

/* texture functions */
/***************************************************************************/
/*
 *  function used by qsort to sort the texture names
 */
int _USERENTRY SortTextures (const void *a, const void *b)
{
   return strcmp ( (*(TextureInfo **)a)->Name, (*(TextureInfo **)b)->Name );
}
/***************************************************************************/
/*
 * AddTextures: Read in the wall texture names with given filespec in given directory.
 *   Return texture information, or NULL if none loaded.  Set num to number of textures
 *   loaded.
 */
TextureInfo ** AddTextures (char *dir, char *filespec, SHORT *num)
{
   int i, num_textures, texnum;
   char path[MAX_PATH], filename[MAX_PATH];
   HWND box;
   TextureInfo **Textures;
   Bitmaps b;
   
   box = CreateWindow("listbox", "", LBS_SORT,
		      0, 0, 0, 0, 
		      NULL, NULL, NULL, NULL);

   if (box == NULL)
      Notify("Unable to load textures--listbox creation failed!");

   sprintf(path, "%s\\%s", dir, filespec);

   ListBox_Dir(box, 0x0000, path);

   num_textures = ListBox_GetCount(box);

   if (num_textures == 0)
   {
      Notify("Found no texture files; looked in %s.", path);
      DestroyWindow(box);
      return 0;
   }

   Textures = (TextureInfo **) GetMemory (num_textures * sizeof(TextureInfo *));
   for (i=0; i < num_textures; i++)
   {
      int width, height;
      BYTE shrink;

      // Init. texture info
      Textures[i] = (TextureInfo *) GetMemory (sizeof(TextureInfo));

      ListBox_GetText(box, i, filename);

      // Get texture size
      sprintf(path, "%s\\%s", dir, filename);
      if (DibOpenFileSimple(path, &b, &shrink, &width, &height));
      {
	 Textures[i]->Width  = width;
	 Textures[i]->Height = height;
	 Textures[i]->shrink = shrink;
	 strcpy(Textures[i]->Name, BitmapsName(b));
      }
      
      // Get number out of filename
      sscanf(filename, "%*[^0123456789]%d", &texnum);
      Textures[i]->number = texnum;

      strcpy(Textures[i]->filename, filename);
   }

   DestroyWindow(box);
   *num = (SHORT) num_textures;
   return Textures;
}
/***************************************************************************/
/*
 * TextureToNumber:  Return the number associated with the given texture name.
 */
int TextureToNumber(char *name)
{
   if (!strcmp(name, "-") || name[0] == 0)
      return 0;

   TextureInfo *info = FindTextureByName(name);
   if (info == NULL)
   {
      Notify("TextureToNumber got bad texture name %s", name);
      return 0;
   }
   return info->number;
}
/***************************************************************************/
/*
 *  read in the wall texture names
 */
void ReadWTextureInfo()
{
   SELECT_WAIT_CURSOR();
   SAVE_WORK_MSG();
   WorkMessage ("Reading Wall texture info ...");
   
   // Forget previous list of texture
   ForgetWTextureInfo();
   assert (WTexture == NULL);
   assert (NumWTexture == 0);
   
   // Load all grid files
   WTexture = AddTextures(BitmapDir, BitmapSpec, &NumWTexture);
   
   // sort the names
   qsort (WTexture, NumWTexture, sizeof(TextureInfo *), SortTextures);
   
   RESTORE_WORK_MSG();
   UNSELECT_WAIT_CURSOR();
}
/***************************************************************************/
/*
 * forget the wall texture names
 */
void ForgetWTextureInfo()
{
   SHORT n;
   
   /* forget all names */
   for (n = 0; n < NumWTexture; n++)
      FreeMemory (WTexture[n]);
   
   /* forget the array */
   NumWTexture = 0;
   if ( WTexture != NULL )
      FreeMemory (WTexture);
}
/***************************************************************************/
/*
 * ReadFTextureInfo:  Read in the floor/ceiling texture names.
 *   Since we don't distinguish between floor and wall textures, this does
 *   basically the same thing as ReadWTextureInfo.
 */
void ReadFTextureInfo()
{
   SELECT_WAIT_CURSOR();
   SAVE_WORK_MSG();
   WorkMessage ("Reading Wall texture info ...");
   
   // Forget previous list of texture
   ForgetFTextureInfo();
   assert (FTexture == NULL);
   assert (NumFTexture == 0);
   
   // Load all grid files
   FTexture = AddTextures(BitmapDir, BitmapSpec, &NumFTexture);
   
   // sort the names
   qsort (FTexture, NumFTexture, sizeof(TextureInfo *), SortTextures);
   
   RESTORE_WORK_MSG();
   UNSELECT_WAIT_CURSOR();
}



/*
   forget the floor/ceiling texture names
*/

void ForgetFTextureInfo()
{
   SHORT n;

   /* forget all names */
   for (n = 0; n < NumFTexture; n++)
	  FreeMemory (FTexture[n]);

   /* forget the array */
   NumFTexture = 0;
   if ( FTexture != NULL )		FreeMemory (FTexture);
}


/***************************************************************************/
/*
 * FindTextureByName:  Return TextureInfo structure for texture with given name.
 */
TextureInfo *FindTextureByName(char *name)
{
   if (WTexture == NULL || name == NULL)
     return NULL;
   
   for (SHORT ti = 0 ; ti < NumWTexture ; ti++ )
   {
      assert (WTexture[ti] != NULL);
      
      if (!strcmp(WTexture[ti]->Name, name))
	 return WTexture[ti];
   }
   Notify ("Couldn't find Wall texture info: \"%s\" (BUG)!", name);
   return NULL;
}
/***************************************************************************/
/*
 * FindTextureByNumber:  Return TextureInfo structure for texture with given number, or
 *   NULL if none.
 */
TextureInfo *FindTextureByNumber(int num)
{
   if (WTexture == NULL || num == 0)
     return NULL;

   for (SHORT ti = 0 ; ti < NumWTexture ; ti++ )
   {
      assert (WTexture[ti] != NULL);
      
      if (WTexture[ti]->number == num)
	 return WTexture[ti];
   }
   Notify ("Couldn't find Wall texture info: \"%d\" (BUG)!", num);
   return NULL;
}

/*
   Function to get the (stretched/shrunk) size of a wall texture
*/

void GetWallTextureSize (SHORT *xsize_r, SHORT *ysize_r, int texnum)
{
   assert (WTexture!=NULL);
   
   for (SHORT ti = 0 ; ti < NumWTexture ; ti++ )
   {
      assert (WTexture[ti] != NULL);
      
      if ( WTexture[ti]->number == texnum )
      {
	 *xsize_r = WTexture[ti]->Width / WTexture[ti]->shrink;
	 *ysize_r = WTexture[ti]->Height / WTexture[ti]->shrink;
	 return;
      }
   }
   
   Notify ("Couldn't find Wall texture info: \"%d\" (BUG)!", texnum);
}

/***************************************************************************/
/*
 * GetSidedefTextureInfo:  Return TextureInfo for a texture on the given sidedef,
 *   or NULL if there are no textures on the sidedef.
 */
TextureInfo *GetSidedefTextureInfo(SHORT sdnum)
{
   int texnum = 0;
   SideDef *sd;

   // Try to get normal texture, then below texture, then above texture
   sd = &SideDefs[sdnum];
   if (sd->type3 != 0)
      texnum = sd->type3;
   else if (sd->type2 != 0)
      texnum = sd->type2;
   else if (sd->type1 != 0)
      texnum = sd->type1;

   return FindTextureByNumber(texnum);
}
