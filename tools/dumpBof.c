// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

#include <stdio.h>
#include <stdlib.h>
#include "..\blakserv\bof.h"

bof_file_header bh ;
bof_class_header ch;

long num_str;
unsigned long prop_def;
unsigned long size_mess,mess_def;

void main(int argc,char*argv[])
{
	FILE*fp;
	int i,j;
	unsigned long size_prop;

	if( argc < 2 ) {
		fprintf(stderr,"Need filename !\n");
		exit(1);
	}
	fp = fopen(argv[1],"rb");
	if( fp == NULL ) {
		fprintf(stderr,"Can`t open file %s !\n",argv[1]);
		exit(1);
	}

	fread(&bh,1,sizeof( bof_file_header ) ,fp);
	printf("@ %d\n",ftell(fp));

	printf("Magic             %c%c%c\n",bh.magic[0],bh.magic[1],bh.magic[2],bh.magic[3]);
	printf("Version           %d\n",bh.version);
	printf("Source Filename   %d\n",bh.source_filename);
	printf("dstring_offset    %d\n",bh.dstring_offset);
	printf("line_table_offset %d\n",bh.line_table_offset);
/* Class table
	4 - number of classes
		4 - class id
		4 - offset in file of class

*/
	printf("num_classes       %d\n",bh.num_classes);

	fread(&bh.class[0].id,8,1,fp);

	for(i=0;i<bh.num_classes;i++) {
		printf("ble id            %d\n",bh.class[0].id);
		printf("ble offset,class  %d\n",bh.class[0].offset);
	}

	fread(&ch,sizeof(bof_class_header),1,fp);
	printf("@ %d\n",ftell(fp));
	printf("SuperClass           %d\n",ch.superclass);
	printf("Offset Prop          %d\n",ch.offset_properties);
	printf("Offset Dispatch      %d\n",ch.offset_dispatch);
	printf("Num Classvars        %d\n",ch.num_classvars);
	printf("Num default classvar %d\n",ch.num_default_classvar_vals);

//	fseek(fp,-4,SEEK_CUR);
	for(i=0;i<ch.num_default_classvar_vals;i++) {
		long cv_num,cv_def;
		fread(&cv_num,4,1,fp);
		fread(&cv_def,4,1,fp);
		printf("ClassVar Num     %d\n",cv_num);
		printf("ClassVar Default %d\n",cv_def);
	}

	printf("Property section\n\n");

	fread(&size_prop,4,1,fp);
	printf("Number of props     %d\n",size_prop);

	fread(&prop_def,4,1,fp);
	printf("Number of prop defs %d\n",prop_def);

	for(i=0;i<prop_def;i++) {
		long cv_num,cv_def;
		fread(&cv_num,4,1,fp);
		fread(&cv_def,4,1,fp);
		printf("%d Prop Num         %d\n",i,cv_num);
		printf("Prop Default     %d\n",cv_def);
	}



//	fseek(fp,-8,SEEK_CUR);
	printf("Message section\n\n");

	fread(&size_mess,4,1,fp);
	printf("Number of messages  %d\n\n",size_mess);

//	fread(&mess_def,4,1,fp);
//	printf("Number of mess defs %d\n",mess_def);

	for(i=0;i<size_mess-1;i++) {
		long cv_num,cv_def,cv_ind;
		fread(&cv_num,4,1,fp);
		fread(&cv_def,4,1,fp);
		fread(&cv_ind,4,1,fp);
		printf("Message ID       %d\n",cv_num);
		printf("Offset           %d\n",cv_def);
		printf("Index of comment %d\n",cv_ind);
	}

	{
		unsigned char clv;
		unsigned char np;
		fread(&clv,1,1,fp);
		fread(&np,1,1,fp);
		printf("Number of locals %d\n",clv);
  		printf("Number of params %d\n",np);
  	}


	if( fp ) fclose( fp );


}
