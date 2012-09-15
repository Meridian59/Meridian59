// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include <stdio.h>
#include <stdlib.h>

#define WINDOWS_MEAN_AND_LEAN 1
#include <windows.h>
#include "wrap.h"

int DecryptBuffer( char *in,long size);

#define RSC_VERSION 2

char verbose = 0;
FILE*fp;

void DoHelp(void)
{
	fprintf(stderr,"\nUsage: unpak <source>\n\n");
	fprintf(stderr,"This program dencrypts meridian59 .rsc files\n\n");
	fprintf(stderr,"Where source is source encrypted .rsc file\n");
	fprintf(stderr,"program will not trash input file on error\n");
}

long  FileSize( FILE*infp)
{
	long end,oldpos;

	if( infp == NULL ) return -1;

	oldpos = ftell( fp );
	fseek(fp,0,SEEK_END );
	end = ftell( fp );
	fseek(fp,oldpos,SEEK_SET);

	return end;

}

static char *buffer = NULL;
// Encryption password
static char *password = "\x0F1\x071\x0C6\x0BB\x019\x06E\x02E\x071\0x6F";

int main(int argc,char*argv[])
{
	long size;

	fprintf(stderr,"UnPak .rsc v0.0 by Charlie (C)3DO 1998\n\n");

	if( argc < 2 ) {
		DoHelp();
		exit(1);
	}

	fp = fopen(argv[1],"rb");
	if(fp == NULL) {
		fprintf(stderr,"Cannot open file %s for reading\n",argv[1]);
		exit(2);
	}

	size = FileSize( fp );
	if(size<1) {
		fprintf(stderr,"File is empty!\n");
		fclose( fp );
		exit(2);
	}

	buffer = (char *)malloc( size ) ;
	if( buffer == NULL ) {
	 	fprintf(stderr,"Out of memory, could not allocate %ld bytes\n",size );
		fclose( fp );
		exit(3);
	}

	fprintf(stderr,"Reading %ld bytes from %s\n",size,argv[1]);
	if( fread(buffer,size,1,fp) != 1 ) {
		fprintf(stderr,"Short read of file %s, expected %ld bytes\n",argv[1],size );
		fclose( fp );
		free( buffer ) ;
		exit(2);
	};

	fclose( fp );

	fprintf(stderr,"DeEncrypting file\n");
	if( DecryptBuffer(buffer,size) != 0 ) {
		fprintf(stderr,"Decryption failed!\n");
		free(buffer);
		exit( 4 );
	}

	fp = fopen("temp.unp","wb");
	if( fp == NULL ) {
		fprintf(stderr,"Couldn`t open output file temp.unp\n");
		free( buffer );
		exit( 2 );
	}

	fprintf(stderr,"Writing output to temporary file\n");
	if( fwrite(buffer,size,1,fp) != 1 ) {
		fprintf(stderr,"Error writing temporary output file temp.unp\n");
		fclose( fp );
		free( buffer );
		exit( 2 );
	};

	fclose( fp );

	fprintf(stderr,"removing original file\n");
	unlink( argv[1] );

	fprintf(stderr,"replacing with decrypted version\n");
	rename("temp.unp",argv[1]);

	free( buffer );
}

int DecryptBuffer( char *in,long size)
{
	unsigned long response;
	int i;
	unsigned long *hdr=(unsigned long *)in;

	if(hdr[1]!=RSC_VERSION) {
		fprintf(stderr,"file is not type 2 can`t do\n");
		return 1;
	}

	if(verbose) {
		for(i=0;i<10;i++)
			fprintf(stderr,"%d 0x%lx\n",i,hdr[i]);
	}

	response = hdr[4];

	if(verbose) fprintf(stderr,"response is %lx\n",response);


	if(verbose)	fprintf(stderr,"in 0x%lx\n",in);

	in += sizeof(unsigned long)*5;
	size -= sizeof(unsigned long)*5;

	if(verbose) fprintf(stderr,"   0x%lx\n",in);

   WrapInit();

   if (!WrapDecrypt(in, size, password, RSC_VERSION, response)) {
		fprintf(stderr,"Failed to deencrypt!\n");
		return 1;
	}

	if(verbose)fprintf(stderr,"RC is %d\n",rc);
   WrapShutdown();

	if(verbose) {
		for(i=0;i<size;i++) {
			if(isprint(in[i]))
				fprintf(stderr,"%c",in[i]);
			else
				fprintf(stderr,".");
		}
	}

	/* all ok */

	/* ok its now a type 3 resource file */
	hdr[1] = 3;

	return 0;
}


