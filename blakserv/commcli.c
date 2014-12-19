// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * commcli.c
 *

 This module has a few functions that concern communication from the
 server and blakod to the clients.  This is done with a buffer list
 and functions to add various types of data to the list.

 */

#include "blakserv.h"

#define NUMBER_OBJECT 5		/* writes 4 bytes, but diff "tag" */
#define STRING_RESOURCE 6	/* writes actual string, even though it's a resource */

static buffer_node *blist;

void InitCommCli()
{
   blist = NULL;
}

void AddBlakodToPacket(val_type obj_size,val_type obj_data)
{
   int num_bytes;
   char byte1;
   short byte2;
   int byte4;
   string_node *snod;
   resource_node *r;
   val_type temp_val;

   if (obj_size.v.tag != TAG_INT)
   {
      bprintf("AddBlakodToPacket looking for int, # of bytes, got %i,%i\n",
	      obj_size.v.tag,obj_size.v.data);
      return;
   }

   num_bytes = obj_size.v.data;

   if (obj_data.v.tag == TAG_NIL)
      bprintf("AddBlakodToPacket looking for value, got NIL\n");

/*   dprintf("Send %i bytes from %i,%i\n",obj_size.data,obj_data.v.tag,obj_data.v.data); */
   switch (obj_data.v.tag)
   {
   case TAG_STRING :
      snod = GetStringByID(obj_data.v.data);
      if (snod == NULL)
      {
			bprintf("AddBlakodToPacket can't find string id %i\n",obj_data.v.data);
			break;
      }
      AddStringToPacket(snod->len_data,snod->data);
      break;
      
   case TAG_TEMP_STRING :
      snod = GetTempString();
      AddStringToPacket(snod->len_data,snod->data);
      break;

   default :
      switch (num_bytes)
      {
      case 1 :
			byte1 = (char) obj_data.v.data;
			AddByteToPacket(byte1);
			break;
      case 2 : 
			byte2 = (short) obj_data.v.data;
			AddShortToPacket(byte2);
			break;
      case 4 : 
			byte4 = (int) obj_data.v.data;
			AddIntToPacket(byte4);
			break;
      case NUMBER_OBJECT :
			temp_val.int_val = obj_data.int_val;
			temp_val.v.tag = CLIENT_TAG_NUMBER;
			byte4 = temp_val.int_val;
			AddIntToPacket(byte4);
			break;
      case STRING_RESOURCE :
			if (obj_data.v.tag != TAG_RESOURCE)
			{
				bprintf("AddBlakodToPacket can't send %i,%i as a resource/string\n",
						  obj_data.v.tag,obj_data.v.data);
				return;
			}
			r = GetResourceByID(obj_data.v.data);
			if (r == NULL)
			{
				bprintf("AddBlakodToPacket can't find resource %i as a resource/string\n",
						  obj_data.v.data);
				return;
			}
			AddStringToPacket(strlen(r->resource_val),r->resource_val);
			break;
      default :
			bprintf("AddBlakodToPacket can't send %i bytes\n",num_bytes);
			break;
      }
   }
}

/* these few functions are for synched mode */
void AddByteToPacket(unsigned char byte1)
{
   blist = AddToBufferList(blist,&byte1,1);
}

void AddShortToPacket(short byte2)
{
   blist = AddToBufferList(blist,&byte2,2);
}

void AddIntToPacket(int byte4)
{
   blist = AddToBufferList(blist,&byte4,4);
}

void AddStringToPacket(int int_len,const char *ptr)
{
   unsigned short len;

   len = int_len;

   blist = AddToBufferList(blist,&len,2);
   blist = AddToBufferList(blist,(void *) ptr,int_len);
}

void SecurePacketBufferList(int session_id, buffer_node *bl)
{
   session_node *s = GetSessionByID(session_id);
   const char* pRedbook;

   if (!session_id || !s || !s->account || !s->account->account_id ||
       s->conn.type == CONN_CONSOLE)
   {
      //dprintf("SecurePacketBufferList cannot find session %i", session_id);
      return;
   }
   if (s->version_major < 4)
   {
      return;
   }
   if (bl == NULL || bl->buf == NULL)
   {
//      dprintf("SecurePacketBufferList can't use invalid buffer list");
      return;
   }

//   dprintf("Securing msg %u with %u", (unsigned char)bl->buf[0], (unsigned char)(s->secure_token & 0xFF));

   bl->buf[0] ^= (unsigned char)(s->secure_token & 0xFF);
   pRedbook = GetSecurityRedbook();
   if (s->sliding_token && pRedbook)
   {
      if (s->sliding_token < pRedbook ||
	  s->sliding_token > pRedbook+strlen(pRedbook))
      {
	 lprintf("SecurePacketBufferList lost redbook on session %i account %i (%s), may break session\n",
	    session_id, s->account->account_id, s->account->name);
	 s->sliding_token = pRedbook;
      }

      s->secure_token += ((*s->sliding_token) & 0x7F);
      s->sliding_token++;
      if (*s->sliding_token == '\0')
         s->sliding_token = pRedbook;
   }
}

void SendPacket(int session_id)
{
/*
   int i;
   dprintf("sending packet len %i\n",len_buf);
   for (i=0;i<len_buf;i++)
      dprintf("%i ",(unsigned char)buf[i]);
   dprintf("\n");
*/

//   dprintf("SendPacket msg %u", (unsigned char)blist->buf[0]);
   SecurePacketBufferList(session_id,blist);
   SendClientBufferList(session_id,blist);
   blist = NULL;
}

void SendCopyPacket(int session_id)
{
   buffer_node *bl = CopyBufferList(blist);
//   dprintf("SendCopyPacket msg %u", (unsigned char)bl->buf[0]);
   SecurePacketBufferList(session_id,bl);
   SendClientBufferList(session_id,bl);
}

void ClearPacket()
{
   DeleteBufferList(blist);
   blist = NULL;
}
void ClientHangupToBlakod(session_node *session)
{
   val_type command,parm_list;
   parm_node parms[1];

   command.v.tag = TAG_INT;
   command.v.data = BP_REQ_QUIT;

   parm_list.int_val = NIL;
   parm_list.v.data = Cons(command,parm_list);
   parm_list.v.tag = TAG_LIST;

   parms[0].type = CONSTANT;
   parms[0].value = parm_list.int_val;
   parms[0].name_id = CLIENT_PARM;

   SendTopLevelBlakodMessage(session->game->object_id,RECEIVE_CLIENT_MSG,1,parms);
}


void SendBlakodBeginSystemEvent(int type)
{
   val_type int_val;
   parm_node p[1];

   int_val.v.tag = TAG_INT;
   int_val.v.data = type;

   p[0].type = CONSTANT;
   p[0].value = int_val.int_val;
   p[0].name_id = TYPE_PARM;

   SendTopLevelBlakodMessage(GetSystemObjectID(),GARBAGE_MSG,1,p);
}

void SendBlakodEndSystemEvent(int type)
{
   val_type int_val;
   parm_node p[1];

   int_val.v.tag = TAG_INT;
   int_val.v.data = type;

   p[0].type = CONSTANT;
   p[0].value = int_val.int_val;
   p[0].name_id = TYPE_PARM;

   SendTopLevelBlakodMessage(GetSystemObjectID(),GARBAGE_DONE_MSG,1,p);

}


