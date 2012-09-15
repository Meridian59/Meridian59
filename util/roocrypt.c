/*
 * roocrypt.c: Encrypt client section of roo file.
 */

#include <stdio.h>

#include <windows.h>
#include <wincon.h>

#include "memmap.h"
#include "wrap.h"

// number of bytes we're adding to the client section of the file
#define EXTRA_BYTES 12

#define HEADER_SIZE 20  // # of bytes before room data

// Encryption password
static char *password = "\x06F\x0CA\x054\x0B7\x0EC\x064\x0B7";

/***************************************************************************/
/*
 * MerEncryptFile:  Convert the uncompressed room file in_filename to the compressed room
 *   file out_filename.  Return True on success.
 */
Bool MerEncryptFile(char *in_filename, char *out_filename)
{
   file_node in_file, out_file;
   char *out_ptr, header[HEADER_SIZE];
   int server_offset, temp, orig_len, response, security;

   // Open input file memory mapped
   if (!MappedFileOpenRead(in_filename, &in_file))
   {
      printf("Can't open input file %s\n", in_filename);
      return False;
   }
   
   // Copy modified header over to new file
   if (MappedFileRead(&in_file, header, HEADER_SIZE) != HEADER_SIZE)
      printf("Error reading from file\n");

   // Remember room security value
   memcpy(&security, header + 8, 4);

   // Remember offset of server info
   memcpy(&server_offset, header + 16, 4);

   // Increase server offset to compensate for extra bytes
   temp = server_offset + EXTRA_BYTES;
   memcpy(header + 16, &temp, 4);

   // Open output file memory mapped
   // Will be as large as input file, plus 8 bytes for length information
   // 2/12/02 ARK--Now that we're chopping out the server section,
   // we need to subtract off that length
   if (!MappedFileOpenWrite(out_filename, &out_file,
                            server_offset + EXTRA_BYTES))
   {
      printf("Can't open output file %s\n", out_filename);
      return False;
   }
   
   // Write out modified header
   MappedFileWrite(&out_file, header, HEADER_SIZE);

   // Write out -1, then size of encrypted region, then space for response to challenge
   temp = -1;
   MappedFileWrite(&out_file, &temp, 4);
   orig_len = server_offset - HEADER_SIZE;
   MappedFileWrite(&out_file, &orig_len, 4);
   MappedFileWrite(&out_file, &temp, 4);

   out_ptr = out_file.ptr;

   // Copy client section to output file
   if (MappedFileWrite(&out_file, in_file.ptr, orig_len) != orig_len)
     printf("\nMappedFileWrite failed!\n");

   // Encrypt client section
   WrapInit();
   response = WrapEncrypt(out_ptr, orig_len, password, security);
   WrapShutdown();

   // Write out response to challenge
   out_ptr -= 4;
   memcpy(out_ptr, &response, 4);

   in_file.ptr += orig_len;

   // Write out remainder of file (server section)
   // Removed this 2/2/2002 ARK, so that publicly available rooms
   // can't be used by servers.
//   MappedFileWrite(&out_file, in_file.ptr, in_file.length - server_offset);

   MappedFileClose(&in_file);
   MappedFileClose(&out_file);
   return True;
}
/***************************************************************************/
void Usage(void)
{
   printf("Usage: roocrypt <input file> <output file>\n");
}
/***************************************************************************/
int main(int argc, char **argv)
{
   if (argc < 3)
      Usage();

   MerEncryptFile(argv[1], argv[2]);
}
