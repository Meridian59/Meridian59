// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * some structs for convenience in dealing with bof files.
 *
 */

#ifndef _bof_h
#define _bof_h

/* util struct */
typedef struct
{
   int id;
   int offset;
} bof_list_elem;

/* util struct for message table */
typedef struct
{
   int id;
   int offset;
   int dstr_id;
} bof_dispatch_list_elem;

/* struct at beginning of file */
typedef struct
{
   unsigned char magic[4];
   int version;
   int source_filename;
   int dstring_offset;
   int line_table_offset; /* for source line number info */
   int num_classes;
   bof_list_elem classes;
} bof_file_header;

/* struct at beginning of each class */
typedef struct
{
   int superclass;
   int offset_properties;
   int offset_dispatch;
   int num_classvars;
   int num_default_classvar_vals;
   bof_list_elem classvar_values;

} bof_class_header;

/* struct of property stuff */
typedef struct
{
   int num_properties;
   int num_default_prop_vals;
   bof_list_elem prop_values;
} bof_class_props;
   
/* struct at beginning of message section for each class */
typedef struct
{
   int num_messages;
   bof_dispatch_list_elem messages;
} bof_dispatch;

/* debug string table, aka "dstr"s. */
typedef struct
{
   int num_strings;
   int string_offsets;
   /* here in file is where the strings are */
} bof_dstring;

/* line number table data */
typedef struct
{
   int file_offset;
   int line_number;
} bof_line_entry;

/* line number table */
typedef struct
{
   int num_line_entries;
   bof_line_entry entries;
} bof_line_table;


#endif
