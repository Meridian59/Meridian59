// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include <stdio.h>
#include <stdlib.h>


typedef struct rsc {
	unsigned long magic;
	unsigned long num_res;
	unsigned long temp;
	unsigned long id,t1,t2,t3,t4;
} RSC ;

RSC rsc;

FILE*  ReadRSCHdr(char *name, RSC *r );

void main(int argc,char*argv[])
{
	int i;

	FILE *fp;

	if( argc< 2 ){
		fprintf(stderr,"Need a .rsc name dummy\n");
		exit(1);
	}

	fp = ReadRSCHdr( argv[1] , &rsc);
	if(fp == NULL )
		exit( 1 );

	printf("Magic         % 10ld   0x%lx\n",rsc.magic,rsc.magic);
	printf("num resources % 8ld     0x%lx\n",rsc.num_res,rsc.num_res);
	printf("Version       % 8ld     0x%lx\n",rsc.temp,rsc.temp);
	printf("ID            % 8ld     0x%lx\n",rsc.id,rsc.id);
	printf("t1            % 8ld     0x%lx\n",rsc.t1,rsc.t1);

	printf("\n");

	for(i=0;i<rsc.num_res;i++) {
		ReadRSC(fp,&rsc);
		printf("\n");
	}

	printf("%s file ptr @ %ld\n",argv[1],ftell( fp ) );

	if( fp ) fclose ( fp );


	exit( 0 );
}


FILE*  ReadRSCHdr(char *name, RSC *r )
{
	FILE*fp;

	fp = fopen(name,"rb");
	if( fp == NULL ) {
		fprintf(stderr,"Couldn`t open file %s\n",name);
		return NULL;
	}

	fread(&r->magic,4,1,fp);
	fread(&r->temp,4,1,fp);
	fread(&r->num_res,4,1,fp);

	fread(&r->id,4,1,fp);
	fread(&r->t1,4,1,fp);

	return fp;

}

int ReadRSC( FILE*fp,RSC*r)
{
	int i;
	char ch;

	/* Read the resource ID */

	fread(&r->t2,4,1,fp);

	printf("\tID            % 8ld     - ",r->t2,r->t2);

	/* since this next part contains no useful information,
		i won`t keep it around */

	do {
		ch = fgetc(fp);
		if(ch!=0) printf("%c",ch);
	} while(ch!=0);


	/* resource ID */
	return r->t2;
}
