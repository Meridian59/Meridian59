/*
 * paldump.c:  Dump palette for given bitmap file.
 */

#include <windows.h>
#include <stdio.h>

#include "memmap.h"

#define NUM_COLORS 256

typedef struct {
   BYTE red, green, blue;
} Color;

Color base_palette[NUM_COLORS] = {
{  0,   0,   0}, // Color 0
{128,   0,   0}, // Color 1
{  0, 128,   0}, // Color 2
{128, 128,   0}, // Color 3
{  0,   0, 128}, // Color 4
{128,   0, 128}, // Color 5
{  0, 128, 128}, // Color 6
{192, 192, 192}, // Color 7
{192, 220, 192}, // Color 8
{166, 202, 240}, // Color 9
{120, 252, 120}, // Color 10
{232, 168, 252}, // Color 11
{252, 192,  68}, // Color 12
{252, 168, 168}, // Color 13
{248, 200,  84}, // Color 14
{192, 192, 192}, // Color 15
{252, 192,  24}, // Color 16
{252, 192,  24}, // Color 17
{  4,   4,   4}, // Color 18
{252, 184,  56}, // Color 19
{224, 180, 148}, // Color 20
{180, 188, 176}, // Color 21
{184, 184, 184}, // Color 22
{220, 172, 136}, // Color 23
{208, 176, 132}, // Color 24
{244, 172,  48}, // Color 25
{252, 176,   4}, // Color 26
{239, 170, 124}, // Color 27
{176, 176, 176}, // Color 28
{172, 180, 164}, // Color 29
{204, 168, 124}, // Color 30
{252, 136, 136}, // Color 31
{239, 156, 115}, // Color 32
{236, 160,  24}, // Color 33
{252, 156,   0}, // Color 34
{196, 160, 116}, // Color 35
{212, 128, 240}, // Color 36
{  0,   4,  80}, // Color 37
{160, 168, 156}, // Color 38
{164, 164, 164}, // Color 39
{ 68, 196, 244}, // Color 40
{212, 148, 104}, // Color 41
{212, 160,   4}, // Color 42
{144, 148, 252}, // Color 43
{188, 152, 108}, // Color 44
{220, 148,  28}, // Color 45
{152, 160, 148}, // Color 46
{  4,   8,  56}, // Color 47
{232, 144,   4}, // Color 48
{204, 140,  92}, // Color 49
{252, 128,   0}, // Color 50
{148, 148, 172}, // Color 51
{180, 144, 100}, // Color 52
{ 92, 192,  80}, // Color 53
{192, 132,  92}, // Color 54
{144, 148, 136}, // Color 55
{204, 136,  24}, // Color 56
{144, 144, 144}, // Color 57
{168, 136,  92}, // Color 58
{223, 116,  74}, // Color 59
{128, 128, 240}, // Color 60
{252,  92,  92}, // Color 61
{248, 208,  96}, // Color 62
{  4, 232,   4}, // Color 63
{132, 140, 128}, // Color 64
{136, 136, 136}, // Color 65
{180, 124,  80}, // Color 66
{212, 120,   4}, // Color 67
{180, 136,   4}, // Color 68
{ 84, 172,  72}, // Color 69
{ 52, 156, 236}, // Color 70
{160, 128,  84}, // Color 71
{252,  96,   0}, // Color 72
{188, 124,  20}, // Color 73
{128, 128, 148}, // Color 74
{  0,   4,  64}, // Color 75
{172, 116,  72}, // Color 76
{196,  72, 232}, // Color 77
{124, 128, 120}, // Color 78
{  0, 212,   0}, // Color 79
{180, 112,  32}, // Color 80
{108, 112, 224}, // Color 81
{ 76, 156,  64}, // Color 82
{148, 116,  76}, // Color 83
{116, 124, 112}, // Color 84
{120, 120, 120}, // Color 85
{160, 108,  64}, // Color 86
{206,  92,  62}, // Color 87
{116, 116, 140}, // Color 88
{104, 128,  96}, // Color 89
{168, 104,  24}, // Color 90
{252,  64,   0}, // Color 91
{112, 112, 112}, // Color 92
{ 96, 124,  88}, // Color 93
{136, 108,  60}, // Color 94
{148, 112,   0}, // Color 95
{108, 108, 128}, // Color 96
{148, 100,  60}, // Color 97
{108, 112, 104}, // Color 98
{ 40, 120, 232}, // Color 99
{159,  73,  39}, // Color 100
{ 68, 136,  56}, // Color 101
{252,  44,  44}, // Color 102
{ 92, 116,  84}, // Color 103
{184,  84,   4}, // Color 104
{  8, 172,   8}, // Color 105
{ 92,  92, 208}, // Color 106
{104, 104, 104}, // Color 107
{140,  92,  52}, // Color 108
{ 88, 112,  80}, // Color 109
{124,  96,  56}, // Color 110
{100,  96, 120}, // Color 111
{ 96, 100,  96}, // Color 112
{152,  84,  16}, // Color 113
{ 96,  96,  96}, // Color 114
{144,  76,  64}, // Color 115
{255, 189, 123}, // Color 116
{ 60, 120,  52}, // Color 117
{ 80, 104,  76}, // Color 118
{128,  84,  48}, // Color 119
{ 92,  92,  92}, // Color 120
{ 88,  88, 108}, // Color 121
{ 88,  92,  84}, // Color 122
{168,  64,   0}, // Color 123
{ 72,  76, 196}, // Color 124
{120,  88,   0}, // Color 125
{140,  76,   8}, // Color 126
{ 76,  96,  72}, // Color 127
{108,  84,  48}, // Color 128
{124,  76,  40}, // Color 129
{  0,   4,  40}, // Color 130
{ 84,  84,  84}, // Color 131
{ 68,  92,  68}, // Color 132
{ 52, 104,  44}, // Color 133
{244,  12,   4}, // Color 134
{128, 128, 128}, // Color 135
{ 80,  80,  80}, // Color 136
{104,  76,  36}, // Color 137
{ 80,  76, 100}, // Color 138
{108,  80,   0}, // Color 139
{200, 200, 200}, // Color 140
{116,  68,  36}, // Color 141
{ 76,  80,  76}, // Color 142
{159,  35,  29}, // Color 143
{ 76,  76,  76}, // Color 144
{ 40,   0,   0}, // Color 145
{124,  64,   4}, // Color 146
{ 64,  84,  60}, // Color 147
{148,  16, 188}, // Color 148
{ 44,  96,  36}, // Color 149
{116,  56,  48}, // Color 150
{ 96,  68,  28}, // Color 151
{ 56,  60, 180}, // Color 152
{ 96,  72,   0}, // Color 153
{236,   0,   0}, // Color 154
{ 68,  72,  68}, // Color 155
{ 68,  68,  88}, // Color 156
{104,  60,  32}, // Color 157
{ 60,  76,  56}, // Color 158
{ 68,  68,  68}, // Color 159
{ 44,  84,  36}, // Color 160
{220,   0,   0}, // Color 161
{112,  52,   0}, // Color 162
{ 64,  64,  64}, // Color 163
{ 52,  72,  52}, // Color 164
{ 84,  60,  24}, // Color 165
{ 40,  80,  32}, // Color 166
{252, 216,  44}, // Color 167
{ 96,  52,  28}, // Color 168
{ 80,  64,   4}, // Color 169
{ 60,  60,  60}, // Color 170
{200,   0,   0}, // Color 171
{ 60,  56,  76}, // Color 172
{ 48,  64,  48}, // Color 173
{ 88,  48,  24}, // Color 174
{ 96,  48,   0}, // Color 175
{ 36,  72,  28}, // Color 176
{ 56,  56,  56}, // Color 177
{184,   0,   0}, // Color 178
{ 84,  44,  20}, // Color 179
{ 76,  48,  16}, // Color 180
{ 28,  80, 224}, // Color 181
{ 36,  40, 164}, // Color 182
{ 72,  52,   0}, // Color 183
{ 52,  52,  52}, // Color 184
{ 88,  36,  32}, // Color 185
{ 44,  56,  44}, // Color 186
{ 32,  64,  24}, // Color 187
{164,   0,   0}, // Color 188
{ 88,  16, 120}, // Color 189
{ 84,  40,   0}, // Color 190
{ 48,  48,  48}, // Color 191
{ 48,  44,  68}, // Color 192
{152,   0,   0}, // Color 193
{ 36,  52,  36}, // Color 194
{ 72,  36,  20}, // Color 195
{ 76,  36,   4}, // Color 196
{ 44,  44,  44}, // Color 197
{ 28,  56,  20}, // Color 198
{ 60,  40,  12}, // Color 199
{ 64,  36,  16}, // Color 200
{136,   0,   0}, // Color 201
{ 40,  40,  40}, // Color 202
{ 68,  32,   4}, // Color 203
{ 40,  36,  56}, // Color 204
{ 32,  44,  32}, // Color 205
{ 32,   4,   4}, // Color 206
{ 36,  40,  36}, // Color 207
{ 60,  32,  12}, // Color 208
{ 24,  48,  16}, // Color 209
{ 20,  24, 152}, // Color 210
{116,   0,   0}, // Color 211
{ 60,  12,  88}, // Color 212
{ 56,  28,   4}, // Color 213
{ 28,  36,  28}, // Color 214
{ 48,  28,  12}, // Color 215
{ 32,  32,  32}, // Color 216
{104,   0,   0}, // Color 217
{ 20,  40,  12}, // Color 218
{ 28,  32,  28}, // Color 219
{ 60,  16,  16}, // Color 220
{ 48,  24,   4}, // Color 221
{ 28,  28,  28}, // Color 222
{ 28,  28,  28}, // Color 223
{ 92,   0,   0}, // Color 224
{ 28,  24,  44}, // Color 225
{ 44,  12,  64}, // Color 226
{ 16,  32,  12}, // Color 227
{ 20,  28,  20}, // Color 228
{ 80,   0,   0}, // Color 229
{ 36,  20,  12}, // Color 230
{  4,   4, 184}, // Color 231
{ 36,  20,   4}, // Color 232
{ 68,   0,   0}, // Color 233
{ 20,  20,  20}, // Color 234
{ 20,  16,  36}, // Color 235
{ 28,  16,   8}, // Color 236
{ 12,  24,   8}, // Color 237
{  0,   4, 136}, // Color 238
{ 56,   0,   0}, // Color 239
{212, 212, 212}, // Color 240
{ 16,  16,  16}, // Color 241
{ 20,  16,   4}, // Color 242
{ 48,   0,   0}, // Color 243
{  0,   4, 104}, // Color 244
{  0,   4,  96}, // Color 245
{255, 251, 240}, // Color 246
{160, 160, 164}, // Color 247
{128, 128, 128}, // Color 248
{255,   0,   0}, // Color 249
{  0, 255,   0}, // Color 250
{255, 255,   0}, // Color 251
{  0,   0, 255}, // Color 252
{255,   0, 255}, // Color 253
{  0, 255, 255}, // Color 254
{255, 255, 255}, // Color 255

};

/*****************************************************************************************/
void ComparePalette(char *filename)
{
   file_node f;
   int i;
   BITMAPFILEHEADER fh;
   BITMAPINFOHEADER bi;
   RGBQUAD colors[NUM_COLORS];

   if (!MappedFileOpenRead(filename, &f))
   {
      printf("%12s can't open\n", filename);
      return;
   }

   MappedFileRead(&f, &fh, sizeof(BITMAPFILEHEADER));
   MappedFileRead(&f, &bi, sizeof(BITMAPINFOHEADER));

   for (i=0; i < NUM_COLORS; i++)
   {
      MappedFileRead(&f, &colors[i].rgbBlue, 1);
      MappedFileRead(&f, &colors[i].rgbGreen, 1);
      MappedFileRead(&f, &colors[i].rgbRed, 1);
      MappedFileRead(&f, &colors[i].rgbReserved, 1);

      if (colors[i].rgbRed != base_palette[i].red ||
	  colors[i].rgbGreen != base_palette[i].green ||
	  colors[i].rgbBlue != base_palette[i].blue)
	{
	  printf("file %s differs in color %d\n", filename, i);
	  break;
	}
   }

   MappedFileClose(&f);
}
/*****************************************************************************************/
int main (int argc, char **argv)
{
  int i;

  for (i=1; i < argc; i++)
    ComparePalette(argv[i]);
}

