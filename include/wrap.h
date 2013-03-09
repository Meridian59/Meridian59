// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Library to wrap encryption/decryption, compression/decompression

#ifndef _WRAP_H_
#define _WRAP_H_

#include <windows.h>

#define WRAPAPI WINAPI

// Call this before any other function.
void WRAPAPI WrapInit();

void WRAPAPI WrapShutdown();

// Encrypt buffer in place.  Return response to security challenge.
int WRAPAPI WrapEncrypt(char *ptr, int len, const char *password, int security);

// Decrypt buffer in place
bool WRAPAPI WrapDecrypt(char *ptr, int len, const char *password, int challenge, int response);

// Return compressed length, or < 0 if incompressible.
int WRAPAPI WrapCompress(const char *input, char *output, int len);

// input_len is the compressed length, output_len must match the uncompressed length.
bool WRAPAPI WrapDecompress(const char *input, int input_len, char *output, int output_len);

// Return true iff the given archive file exists and has the right format.
bool WINAPI WrapIsArchive(const char *archive_name);

// Extract the given archive to the given directory, using the given temp dir if it's non-NULL.
void WINAPI WrapExtractArchive(const char *archive_name, const char *dest_dir, const char *temp_dir);


enum ExtractionStatus
{
   EXTRACT_OK, EXTRACT_DONE, EXTRACT_CANT_RENAME, EXTRACT_BAD_PERMISSIONS,
   EXTRACT_OUT_OF_MEMORY, EXTRACT_BAD_CRC, EXTRACT_UNKNOWN,
   EXTRACT_DISK_FULL,
};

// Callback invoked for each file as it's extracted via
// WrapExtractArchive.  The callback should return true to continue
// extraction, false to abort.
typedef bool (ExtractionCallbackFunc)(const char *filename, ExtractionStatus status);

// Setting callback to NULL removes any previously installed callback.
void WINAPI WrapSetExtractionCallback(ExtractionCallbackFunc callback);

#endif  // _WRAP_H_
