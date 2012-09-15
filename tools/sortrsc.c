// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include <stdio.h>
#include <stdlib.h>



void main(int argc,char*argv[])
{
	FILE*fp;
	unsigned long magic;
	unsigned long num_res;
	unsigned long temp;
	unsigned long id,t1,t2,t3,t4;
	int i;

	if( argc< 2 ){
		fprintf(stderr,"Need a .rsc name dummy\n");
		exit(1);
	}

	fp = fopen(argv[1],"rb");
	if( fp == NULL ) {
		fprintf(stderr,"Couldn`t open file %s\n",argv[1]);
		exit(1);
	}
	fread(&magic,4,1,fp);
	fread(&temp,4,1,fp);
	fread(&num_res,4,1,fp);

	printf("Magic         % 10ld   0x%lx\n",magic,magic);
	printf("num resources % 8ld     0x%lx\n",num_res,num_res);
	printf("Version       % 8ld     0x%lx\n",temp,temp);

	fread(&id,4,1,fp);
	printf("ID            % 8ld     0x%lx\n",id,id);
	fread(&t1,4,1,fp);
	printf("t1            % 8ld     0x%lx\n",t1,t1);

	for(i=0;i<num_res;i++) {
		fread(&t2,4,1,fp);
		printf("ID            % 8ld     - ",t2,t2);
		{
			char ch;
			do{
				ch = fgetc(fp);
				if(ch!=0) printf("%c",ch);

			}while(ch!=0);
			printf("\n");
		}
	}
	printf("%s file ptr @ %ld\n",argv[1],ftell( fp ) );

	if( fp ) fclose( fp );

}
