// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* ccode.c
*

  This module has all of the C functions that are callable from Blakod.
  The parameters describe what class/message called the function, its
  parameters, and its local variables.  The return value of these
  functions is really a val_type, and is returned to the Blakod.
  
*/

#include "blakserv.h"

#define iswhite(c) ((c)==' ' || (c)=='\t' || (c)=='\n' || (c)=='\r')

// global buffers for zero-terminated string manipulation
static char buf0[LEN_MAX_CLIENT_MSG+1];
static char buf1[LEN_MAX_CLIENT_MSG+1];

/* just like strstr, except any case-insensitive match will be returned */
const char* stristr(const char* pSource, const char* pSearch)
{
   if (!pSource || !pSearch || !*pSearch)
      return NULL;
	
   size_t nSearch = strlen(pSearch);
   // Don't search past the end of pSource
   const char *pEnd = pSource + strlen(pSource) - nSearch;
   while (pSource <= pEnd)
   {
      if (0 == strnicmp(pSource, pSearch, nSearch))
         return pSource;

      pSource++;
   }
	
   return NULL;
}


blak_int C_Invalid(int object_id,local_var_type *local_vars,
			  int num_normal_parms,parm_node normal_parm_array[],
			  int num_name_parms,parm_node name_parm_array[])
{
	bprintf("C_Invalid called--bad C function number");
	return NIL;
}


blak_int C_AddPacket(int object_id,local_var_type *local_vars,
				int num_normal_parms,parm_node normal_parm_array[],
				int num_name_parms,parm_node name_parm_array[])
{
	int i;
	val_type send_len,send_data;
	
	i = 0;
	while (i < num_normal_parms)
	{
		send_len = RetrieveValue(object_id,local_vars,normal_parm_array[i].type,
			normal_parm_array[i].value);
		i++;
		if (i >= num_normal_parms)
		{
			bprintf("C_AddPacket has # of bytes, needs object\n");
			break;
		}
		
		send_data = RetrieveValue(object_id,local_vars,normal_parm_array[i].type,
			normal_parm_array[i].value);
		i++;
		AddBlakodToPacket(send_len,send_data);
	}
	
	return NIL;
}

blak_int C_SendPacket(int object_id,local_var_type *local_vars,
				 int num_normal_parms,parm_node normal_parm_array[],
				 int num_name_parms,parm_node name_parm_array[])
{
	val_type temp;
	
	temp = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (temp.v.tag != TAG_SESSION)
	{
		bprintf("C_SendPacket object %i can't send to non-session %s\n",
            object_id,fmt(temp));
		return NIL;
	}
	
	SendPacket(temp.v.data);
	
	return NIL;
}

blak_int C_SendCopyPacket(int object_id,local_var_type *local_vars,
					 int num_normal_parms,parm_node normal_parm_array[],
					 int num_name_parms,parm_node name_parm_array[])
{
	val_type temp;
	
	temp = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (temp.v.tag != TAG_SESSION)
	{
		bprintf("C_SendPacket object %i can't send to non-session %s\n",
            object_id,fmt(temp));
		return NIL;
	}
	
	SendCopyPacket(temp.v.data);
	
	return NIL;
}

blak_int C_ClearPacket(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	ClearPacket();
	
	return NIL;
}

blak_int C_Debug(int object_id,local_var_type *local_vars,
			int num_normal_parms,parm_node normal_parm_array[],
			int num_name_parms,parm_node name_parm_array[])
{
	int i;
	val_type each_val;
	class_node *c;
  std::string buf;
	kod_statistics *kstat;
	
	/* need the current interpreting class in case there are debug strings,
	which are stored in the class. */

	kstat = GetKodStats();
	
	c = GetClassByID(kstat->interpreting_class);
	if (c == NULL)
	{
		bprintf("C_Debug can't find class %i, can't print out debug strs\n",
			kstat->interpreting_class);
		return NIL;
	}
	
  buf += "[";
  buf += BlakodDebugInfo();
  buf += "] ";
	
	for (i=0;i<num_normal_parms;i++)
	{
		each_val = RetrieveValue(object_id,local_vars,normal_parm_array[i].type,
                             normal_parm_array[i].value);
		
		switch (each_val.v.tag)
		{
		case TAG_DEBUGSTR :
			buf += GetClassDebugStr(c,each_val.v.data);
			break;
			
		case TAG_RESOURCE :
			{
				resource_node *r;
				r = GetResourceByID(each_val.v.data);
				if (r == NULL)
				{
					buf += "<unknown RESOURCE " + std::to_string(each_val.v.data) + ">";
				}
				else
				{
					buf += r->resource_val;
				}
			}
			break;
			
		case TAG_INT :
			buf += std::to_string(each_val.v.data);
			break;
			
		case TAG_CLASS :
			{
				class_node *c;
				c = GetClassByID(each_val.v.data);
				if (c == NULL)
				{
					buf += "<unknown CLASS " + std::to_string(each_val.v.data) + ">";
				}
				else
				{
					buf += "&";
					buf += c->class_name;
				}
			}
			break;
			
		case TAG_STRING :
			{
				string_node *snod = GetStringByID(each_val.v.data);
				
				if (snod == NULL)
				{
					bprintf("C_Debug can't find string %" PRId64 "\n",each_val.v.data);
					return NIL;
				}
        buf.append(snod->data, snod->len_data);
			}
			break;
			
		case TAG_TEMP_STRING :
			{
				string_node *snod = GetTempString();
        buf.append(snod->data, snod->len_data);
			}
			break;
			
		case TAG_OBJECT :
			{
				object_node *o;
				class_node *c;
				user_node *u;
				
				/* for objects, print account if it's a user */
				
				o = GetObjectByID(each_val.v.data);
				if (o == NULL)
				{
					buf += "<OBJECT " + std::to_string(each_val.v.data) + " invalid>";
					break;
				}
				c = GetClassByID(o->class_id);
				if (c == NULL)
				{
					buf += "<OBJECT " + std::to_string(each_val.v.data) + " unknown class>";
					break;
				}
				
				if (c->class_id == USER_CLASS || c->class_id == DM_CLASS ||
            c->class_id == ADMIN_CLASS)
				{
					u = GetUserByObjectID(o->object_id);
					if (u == NULL)
					{
						buf += "<OBJECT " + std::to_string(each_val.v.data) + " broken user>";
						break;
					}
					buf += "ACCOUNT " + std::to_string(u->account_id) + " OBJECT " + std::to_string(each_val.v.data);
					break;
				}
			}
			//FALLTHRU
		default :
			buf += GetTagName(each_val);
      buf += " ";
      buf += GetDataName(each_val);
			break;
      }
      
      if (i != num_normal_parms-1)
        buf += ",";
   }
   dprintf("%s\n",buf.c_str());
   return NIL;
}

blak_int C_GetInactiveTime(int object_id,local_var_type *local_vars,
					  int num_normal_parms,parm_node normal_parm_array[],
					  int num_name_parms,parm_node name_parm_array[])
{
	val_type session_val,ret_val;
	session_node *s;
	
	session_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (session_val.v.tag != TAG_SESSION)
	{
		bprintf("C_GetInactiveTime can't use non-session %s\n", fmt(session_val));
		return NIL;
	}
	
	s = GetSessionByID(session_val.v.data);
	if (s == NULL)
	{
		bprintf("C_GetInactiveTime can't find session %" PRId64 "\n",session_val.v.data);
		return NIL;
	}
	if (s->state != STATE_GAME)
	{
		bprintf("C_GetInactiveTime can't use session %" PRId64 " in state %i\n",
            session_val.v.data,s->state);
		return NIL;
	}
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = GetTime() - s->game->game_last_message_time;
	
	return ret_val.int_val;   
}

blak_int C_DumpStack(int object_id,local_var_type *local_vars,
					  int num_normal_parms,parm_node normal_parm_array[],
					  int num_name_parms,parm_node name_parm_array[])
{
  std::string buf = "Stack:\n" + BlakodStackInfo() + "\n";
	dprintf("%s",buf.c_str());

	return NIL;
}

blak_int C_SendMessage(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type object_val,message_val;
	
	/* get the message to send first; that way other errors are more descriptive */
	message_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (message_val.v.tag != TAG_MESSAGE)
	{
		bprintf("C_SendMessage OBJECT %i can't send non-message %s\n",
            object_id,fmt(message_val));
		return NIL;
	}
	
	/* get the object (or class or int 0) to which we are sending the message */
	/* not to be confused with object_id, which is the 'self' object sending the message */
	object_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	/* special:  sending to int 0 goes to system */
	if (object_val.v.tag == TAG_INT && object_val.v.data == 0)
	{
		/* allowed to send to INT 0 rather than OBJECT 0 (obsolete but backwards compatible) */
		object_val.v.data = GetSystemObjectID();
	}
	else if (object_val.v.tag == TAG_CLASS)
	{
		/* allowed to send to TAG_CLASS (below) */
	}
	else if (object_val.v.tag != TAG_OBJECT)
	{
		/* assumes object_id (the current 'self') is a valid object */
		bprintf("C_SendMessage OBJECT %i CLASS %s can't send MESSAGE %s (%" PRId64 ") to non-object %s\n",
            object_id,
            GetClassByID(GetObjectByID(object_id)->class_id)->class_name,
            GetNameByID(message_val.v.data), message_val.v.data,
            fmt(object_val));
		return NIL;
	}
	
	if (object_val.v.tag == TAG_CLASS)
		return SendBlakodClassMessage(object_val.v.data,message_val.v.data,num_name_parms,name_parm_array);
	else
		return SendBlakodMessage(object_val.v.data,message_val.v.data,num_name_parms,name_parm_array);
}

blak_int C_PostMessage(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type object_val,message_val;
	
	/* get message to send first; later errors can be more descriptive */
	message_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (message_val.v.tag != TAG_MESSAGE)
	{
		bprintf("C_PostMessage OBJECT %i can't send non-messsage %s\n",
            object_id,fmt(message_val));
		return NIL;
	}
	
	object_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (object_val.v.tag != TAG_OBJECT)
	{
		/* assumes object_id (the current 'self') is a valid object */
		bprintf("C_PostMessage OBJECT %i CLASS %s can't send MESSAGE %s (%" PRId64 ") to non-object %s\n",
            object_id,
            GetClassByID(GetObjectByID(object_id)->class_id)->class_name,
            GetNameByID(message_val.v.data), message_val.v.data,
            fmt(object_val));
		return NIL;
	}
	
	PostBlakodMessage(object_val.v.data,message_val.v.data,
		num_name_parms,name_parm_array);
	return NIL;
}

blak_int C_CreateObject(int object_id,local_var_type *local_vars,
				   int num_normal_parms,parm_node normal_parm_array[],
				   int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val,class_val;
	
	class_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	if (class_val.v.tag != TAG_CLASS)
	{
		bprintf("C_CreateObject can't create non-class %s\n", fmt(class_val));
		return NIL;
	}
	
	ret_val.v.tag = TAG_OBJECT;
	ret_val.v.data = CreateObject(class_val.v.data,num_name_parms,name_parm_array);
	return ret_val.int_val;
}

blak_int C_IsClass(int object_id,local_var_type *local_vars,
			  int num_normal_parms,parm_node normal_parm_array[],
			  int num_name_parms,parm_node name_parm_array[])
{
	val_type object_val,class_val,ret_val;
	object_node *o;
	class_node *c;
	
	object_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (object_val.v.tag != TAG_OBJECT)
	{
		bprintf("C_IsClass can't deal with non-object %s\n", fmt(object_val));
		return NIL;
	}
	
	class_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
			     normal_parm_array[1].value);
	if (class_val.v.tag != TAG_CLASS)
	{
		bprintf("C_IsClass can't look for non-class %s\n", fmt(class_val));
		return NIL;
	}
	
	o = GetObjectByID(object_val.v.data);
	if (o == NULL)
	{
		bprintf("C_IsClass can't find object %" PRId64 "\n",object_val.v.data);
		return NIL;
	}
	
	c = GetClassByID(o->class_id);
	if (c == NULL)
	{
		bprintf("C_IsClass can't find class %i, DIE totally\n",o->class_id);
		FlushDefaultChannels();
		return NIL;
	}
	
	do
	{
		if (c->class_id == class_val.v.data)
		{
			ret_val.v.tag = TAG_INT;
			ret_val.v.data = true;
			return ret_val.int_val;
		}
		c = c->super_ptr;
	} while (c != NULL);
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;
	return ret_val.int_val;
}

blak_int C_GetClass(int object_id,local_var_type *local_vars,
			   int num_normal_parms,parm_node normal_parm_array[],
			   int num_name_parms,parm_node name_parm_array[])
{
	val_type object_val,ret_val;
	object_node *o;
	
	object_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (object_val.v.tag != TAG_OBJECT)
	{
		bprintf("C_GetClass can't deal with non-object %s\n", fmt(object_val));
		return NIL;
	}
	
	o = GetObjectByID(object_val.v.data);
	if (o == NULL)
	{
		bprintf("C_GetClass can't find object %" PRId64 "\n",object_val.v.data);
		return NIL;
	}
	
	ret_val.v.tag = TAG_CLASS;
	ret_val.v.data = o->class_id;
	return ret_val.int_val;
}

// Look up the string given by val.  If found, return true and set *str and *len
// to the string value and length respectively.  function_name is the C function
// name used in reporting errors.
// If the string isn't found (including if val corresponds to NIL), false is returned.
bool LookupString(val_type val, const char *function_name, const char **str, int *len)
{
	string_node *snod;
	resource_node *r;

	switch(val.v.tag)
	{
	case TAG_STRING :
		snod = GetStringByID(val.v.data);
		if (snod == NULL)
		{
			bprintf( "%s can't use invalid string %s\n",
               function_name, fmt(val));
			return false;
		}
		*str = snod->data;
		break;
		
	case TAG_TEMP_STRING :
		snod = GetTempString();
		*str = snod->data;
		break;
		
	case TAG_RESOURCE :
		r = GetResourceByID(val.v.data);
		if( r == NULL )
		{
			bprintf( "%s can't use invalid resource %" PRId64 " as string\n",
                  function_name, val.v.data);
			return false;
		}
		*str = r->resource_val;
		break;
		
	case TAG_DEBUGSTR :
   {
      kod_statistics *kstat;
      class_node *c;
		
      kstat = GetKodStats();
		
      c = GetClassByID(kstat->interpreting_class);
      if (c == NULL)
      {
         bprintf("%s can't find class %i, can't get debug str\n",
                 function_name, kstat->interpreting_class);
         return false;
      }
      *str = GetClassDebugStr(c, val.v.data);
      break;
   }

   case TAG_NIL:
		bprintf( "%s can't use nil as string\n", function_name );
      return false;
   
	default :
		bprintf( "%s can't use with non-string thing %s\n",
             function_name, fmt(val));
		return false;
	}

   if (*str == NULL)
      return false;
   *len = (int) strlen(*str);
   
   return true;
}


blak_int C_StringEqual(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type s1_val,s2_val,ret_val;
	const char *s1,*s2;
	int len1,len2;
	
	s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
   if (!LookupString(s1_val, "C_StringEqual", &s1, &len1))
      return NIL;
	
	s2_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
   if (!LookupString(s2_val, "C_StringEqual", &s2, &len2))
      return NIL;

	/*
	{
	int i;
	dprintf("-----------\n");
	for (i=0;i<len1;i++)
	dprintf("%c",s1[i]);
	dprintf("-----------\n");
	for (i=0;i<len2;i++)
	dprintf("%c",s2[i]);
	dprintf("-----------\n");
	}
	*/
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = FuzzyBufferEqual(s1,len1,s2,len2);
	/*
	dprintf("return %i\n-----\n",ret_val.v.data);
	*/
	
	return ret_val.int_val;
}

void FuzzyCollapseString(char* pTarget, const char* pSource, int len)
{
	if (!pTarget || !pSource || len <= 0)
	{
		*pTarget = '\0';
		return;
	}
	
	// skip over leading and trailing whitespace
	while (len && iswhite(*pSource)) { pSource++; len--; }
	while (len && iswhite(pSource[len-1])) { len--; }
	
	// copy the core string in uppercase
	while (len)
	{
		*pTarget++ = toupper(*pSource++);
		len--;
	}
	
	*pTarget = '\0';
}

bool FuzzyBufferEqual(const char *s1,int len1,const char *s2,int len2)
{
	if (!s1 || !s2 || len1 <= 0 || len2 <= 0)
		return false;
	
	// skip over leading whitespace
	while (len1 && iswhite(*s1)) { s1++; len1--; }
	while (len2 && iswhite(*s2)) { s2++; len2--; }
	
	// cut off trailing whitespace
	while (len1 && iswhite(s1[len1-1])) { len1--; }
	while (len2 && iswhite(s2[len2-1])) { len2--; }
	
	// empty strings can't match anything
	if (!len1 || !len2)
		return false;
	
	// walk the strings until we find a mismatch or an end
	while (len1 && len2 && toupper(*s1) == toupper(*s2))
	{
		s1++;
		s2++;
		len1--;
		len2--;
	}
	
	// we matched only if we finished both strings at the same time
	return (len1 == 0 && len2 == 0);
}

//	Blakod parameters; string0, string1, string2
//	Substitute first occurrence of string1 in string0 with string2
//	Returns 1 if substituted, 0 if not found, NIL if error
blak_int C_StringSubstitute(int object_id,local_var_type *local_vars,
					   int num_normal_parms,parm_node normal_parm_array[],
					   int num_name_parms,parm_node name_parm_array[])
{
	val_type s0_val, s1_val, s2_val, r_val;
	string_node *snod0, *snod1;
	char buf0[LEN_MAX_CLIENT_MSG+1], buf1[LEN_MAX_CLIENT_MSG+1];
	char *s0, *copyspot;
   const char *s1, *s2, *subspot;
	int len1, len2, new_len;
	resource_node *r;
	
	s0 = buf0;
	s1 = buf1;
	s2 = subspot = copyspot = NULL;
	
	s0_val = RetrieveValue( object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	snod0 = NULL;
	if (s0_val.v.tag == TAG_STRING)
		snod0 = GetStringByID(s0_val.v.data);
	else if (s0_val.v.tag == TAG_TEMP_STRING)
		snod0 = GetTempString();
	
	if (snod0 == NULL)
	{
		bprintf( "C_StringSub can't modify first argument non-string %s\n", fmt(s0_val));
		return NIL;
	}
	
	s1_val = RetrieveValue( object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value );
	
	switch(s1_val.v.tag)
	{
	case TAG_STRING :
		snod1 = GetStringByID( s1_val.v.data);
		if( snod1 == NULL )
		{
			bprintf( "C_StringSub can't sub for invalid string %s\n", fmt(s1_val));
			return NIL;
		}
		
		// make a zero-terminated scratch copy of string1
		len1 = snod1->len_data;
		memcpy( buf1, snod1->data, len1 );
		buf1[len1] = 0x0;
		break;
		
	case TAG_TEMP_STRING :
		snod1 = GetTempString();
		
		// make a zero-terminated scratch copy of string1
		len1 = snod1->len_data;
		memcpy( buf1, snod1->data, len1 );
		buf1[len1] = 0x0;
		break;
		
	case TAG_RESOURCE :
		r = GetResourceByID( s1_val.v.data );
		if( r == NULL )
		{
			bprintf( "C_StringSub can't sub for invalid resource %" PRId64 "\n", s1_val.v.data);
			return NIL;
		}
		s1 = r->resource_val;
		len1 = (int) strlen( r->resource_val );
		break;
		
	case TAG_DEBUGSTR :
		{
			kod_statistics *kstat;
			class_node *c;
			
			kstat = GetKodStats();
			
			c = GetClassByID(kstat->interpreting_class);
			if (c == NULL)
			{
				bprintf("C_StringSub can't find class %i, can't get debug str\n",
					kstat->interpreting_class);
				return NIL;
			}
			s1 = GetClassDebugStr(c,s1_val.v.data);
			len1 = 0;
			if (s1 != NULL)
				len1 = (int) strlen(s1);
			break;
		}
		
	case TAG_NIL :
		bprintf( "C_StringSub can't sub for nil\n" );
		return NIL;
		
	default :
		bprintf( "C_StringSub can't sub for non-string thing %s\n", fmt(s1_val));
		return NIL;
	}
	
	if( ( len1 < 1 ) || ( len1 > LEN_MAX_CLIENT_MSG ) )
	{
		bprintf( "C_StringSub can't sub for null string %s\n", fmt(s1_val));
		return NIL;
	}
	
	s2_val = RetrieveValue( object_id, local_vars, normal_parm_array[2].type,
		normal_parm_array[2].value );

   if (!LookupString(s2_val, "C_StringSub", &s2, &len2))
      return NIL;
	
	new_len = snod0->len_data - len1 + len2;
	if( new_len > LEN_MAX_CLIENT_MSG )
	{
		bprintf("C_StringSub can't sub, string too long.");
		return NIL;
	}
	
	// now make a zero-terminated scratch copy of string0
	memcpy( s0, snod0->data, snod0->len_data );
	s0[snod0->len_data] = 0x0;
	
	// so we can use stristr to do the work
	subspot = stristr( s0, s1 );
	
    r_val.v.tag = TAG_INT;
    r_val.v.data = 0;
	
	if( subspot != NULL )	// only substitute if string1 is found in string0
	{
		if (snod0 != GetTempString())
		{
			// free the old string0 and allocate a new (possibly longer) string0
			FreeMemory(MALLOC_ID_STRING,snod0->data,snod0->len_data);
			snod0->data = (char *) AllocateMemory( MALLOC_ID_STRING, new_len+1);
		}
		
		// copy the piece before string1
		copyspot = snod0->data;
		memcpy( copyspot, s0, subspot - s0 );
		
		// copy string2
		copyspot += ( subspot - s0 );
		memcpy( copyspot, s2, len2 );
		
		// copy the piece after string1
		copyspot += len2;
		memcpy( copyspot, subspot + len1, new_len - (subspot - s0) - len2 );
		snod0->len_data = new_len;
		snod0->data[snod0->len_data] = '\0';
		
		r_val.v.data = 1;
	}
	
	return r_val.int_val;
}

blak_int C_StringContain(int object_id,local_var_type *local_vars,
					int num_normal_parms,parm_node normal_parm_array[],
					int num_name_parms,parm_node name_parm_array[])
{
	val_type s1_val,s2_val,ret_val;
	const char *s1,*s2;
	int len1,len2;
	
	s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
   if (!LookupString(s1_val, "C_StringContain", &s1, &len1))
      return NIL;
	
	s2_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
   if (!LookupString(s2_val, "C_StringContain", &s2, &len2))
      return NIL;

	/*
	{
	int i;
	dprintf("-----------\n");
	for (i=0;i<len1;i++)
	dprintf("%c",s1[i]);
	dprintf("-----------\n");
	for (i=0;i<len2;i++)
	dprintf("%c",s2[i]);
	dprintf("-----------\n");
	}
	*/
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = FuzzyBufferContain(s1,len1,s2,len2);
	/*
	dprintf("return %i\n-----\n",ret_val.v.data);
	*/
	
	return ret_val.int_val;
}

/*
"   orc teeth" == "orc teeth"
" orc  teeth" == "orc teeth"
"orcteeth" == "orc teeth"
*/

// return true if s1 contains s2,
//	first converting to uppercase and squashing tabs and spaces to a single space
bool FuzzyBufferContain(const char *s1,int len_s1,const char *s2,int len_s2)
{
	if (!s1 || !s2 || len_s1 <= 0 || len_s2 <= 0)
		return false;
	
	FuzzyCollapseString(buf0, s1, len_s1);
	FuzzyCollapseString(buf1, s2, len_s2);
	
	return (NULL != strstr(buf0, buf1));
}

blak_int C_SetResource(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type drsc_val,str_val;
	resource_node *r;
	string_node *snod;
	int new_len;
	char *new_str;
	
	drsc_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (drsc_val.v.tag != TAG_RESOURCE)
	{
		bprintf("C_SetResource can't set non-resource %s\n", fmt(drsc_val));
		return NIL;
	}
	
	if (drsc_val.v.data < MIN_DYNAMIC_RSC)
	{
		bprintf("C_SetResource can't set non-dynamic resource %s\n", fmt(drsc_val));
		return NIL;
	}
	
	str_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	switch (str_val.v.tag)
	{
	case TAG_TEMP_STRING :
		snod = GetTempString();
		new_len = snod->len_data;
		new_str = snod->data;
		break;
		
	case TAG_RESOURCE :
		{
			r = GetResourceByID(str_val.v.data);
			if (r == NULL)
			{
				bprintf("C_SetResource can't set from bad resource %" PRId64 "\n",
					str_val.v.data);
				return NIL;
			}
			new_len = (int) strlen(r->resource_val);
			new_str = r->resource_val;
			break;
		}
	default :
		bprintf("C_SetResource can't set from non temp string %s\n", fmt(str_val));
		return NIL;
	}
	
	r = GetResourceByID(drsc_val.v.data);
	if (r == NULL)
	{
		eprintf("C_SetResource got dyna rsc number that doesn't exist\n");
		return NIL;
	}
	
	ChangeDynamicResource(r,new_str,new_len);
	
	return NIL;
}

blak_int C_ParseString(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type parse_str_val,separator_str_val,callback_val,string_val;
	parm_node p[1];
	string_node *snod;
	const char *separators;
	kod_statistics *kstat;
	class_node *c;
	char *each_str;
	
	kstat = GetKodStats();
	
	if (kstat->interpreting_class == INVALID_CLASS)
	{
		eprintf("C_ParseString can't find current class\n");
		return NIL;
	}
	
	c = GetClassByID(kstat->interpreting_class);
	if (c == NULL)
	{
		eprintf("C_ParseString can't find class %i\n",kstat->interpreting_class);
		return NIL;
	}
	
	parse_str_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (parse_str_val.v.tag != TAG_TEMP_STRING)
	{
		bprintf("C_ParseString can't parse non-temp string %s\n", fmt(parse_str_val));
		return NIL;
	}
	
	snod = GetTempString();
	/* null terminate it to do strtok */
	snod->data[std::min(LEN_TEMP_STRING-1,snod->len_data)] = 0;
	
	separator_str_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (separator_str_val.v.tag != TAG_DEBUGSTR)
	{
		bprintf("C_ParseString can't use separator non-debugstr %s\n",
            fmt(separator_str_val));
		return NIL;
	}
	separators = GetClassDebugStr(c,separator_str_val.v.data);
	
	callback_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	if (callback_val.v.tag != TAG_MESSAGE)
	{
		bprintf("C_ParseString can't callback non-message %s\n", fmt(callback_val));
		return NIL;
	}
	
	/* setup our parameter to callback */
	string_val.v.tag = TAG_TEMP_STRING;
	string_val.v.data = 0;
	
	p[0].type = CONSTANT;
	p[0].value = string_val.int_val;
	p[0].name_id = STRING_PARM;
	
	each_str = strtok(snod->data,separators);
	while (each_str != NULL)
	{
	/* move the parsed string to beginning of the temp string for kod's use.
	also, fake the length on it for kod's sake.  Doesn't matter to
		us because we null terminated the real string*/
		
		strcpy(snod->data,each_str);
		snod->len_data = (int) strlen(snod->data);
		
		SendBlakodMessage(object_id,callback_val.v.data,1,p);
		
		each_str = strtok(NULL,separators);
	}
	return NIL;
}

blak_int C_SetString(int object_id,local_var_type *local_vars,
				int num_normal_parms,parm_node normal_parm_array[],
				int num_name_parms,parm_node name_parm_array[])
{
	val_type s1_val,s2_val;
	string_node *snod,*snod2;
	resource_node *r;
	
	s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (s1_val.v.tag != TAG_STRING)
	{
		bprintf("C_SetString can't set non-string %s\n", fmt(s1_val));
		return NIL;
	}
	
	snod = GetStringByID(s1_val.v.data);
	if (snod == NULL)
	{
		bprintf("C_SetString can't set invalid string %s\n", fmt(s1_val));
		return NIL;
	}
	
	s2_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	switch (s2_val.v.tag)
	{
	case TAG_STRING :
		snod2 = GetStringByID( s2_val.v.data);
		if( snod2 == NULL )
		{
			bprintf( "C_SetString can't find string %s\n", fmt(s2_val));
			return NIL;
		}
		//bprintf("SetString string%i<--string%i\n",s1_val.v.data,s2_val.v.data);
		SetString(snod,snod2->data,snod2->len_data);
		break;
		
	case TAG_TEMP_STRING :
		snod2 = GetTempString();
		//bprintf("SetString string%i<--tempstring\n",s1_val.v.data);
		SetString(snod,snod2->data,snod2->len_data);
		break;
		
	case TAG_RESOURCE :
		r = GetResourceByID(s2_val.v.data);
		if (r == NULL)
		{
			bprintf("C_SetString can't set from invalid resource %" PRId64 "\n",s2_val.v.data);
			return NIL;
		}
		//bprintf("SetString string%i<--resource%i\n",s1_val.v.data,s2_val.v.data);
		SetString(snod,r->resource_val,(int) strlen(r->resource_val));
		break;
		
	default :
		bprintf("C_SetString can't set from non-string thing %s\n", fmt(s2_val));
		return NIL;
	}
	
	return s1_val.int_val;
}

blak_int C_ClearTempString(int object_id,local_var_type *local_vars,
					  int num_normal_parms,parm_node normal_parm_array[],
					  int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val;
	
	ClearTempString();
	
	ret_val.v.tag = TAG_TEMP_STRING;
	ret_val.v.data = 0;		/* doesn't matter for TAG_TEMP_STRING */
	return ret_val.int_val;
}

blak_int C_GetTempString(int object_id,local_var_type *local_vars,
					int num_normal_parms,parm_node normal_parm_array[],
					int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val;
	
	ret_val.v.tag = TAG_TEMP_STRING;
	ret_val.v.data = 0;		/* doesn't matter for TAG_TEMP_STRING */
	return ret_val.int_val;
}

blak_int C_AppendTempString(int object_id,local_var_type *local_vars,
					   int num_normal_parms,parm_node normal_parm_array[],
					   int num_name_parms,parm_node name_parm_array[])
{
	val_type s_val;
	string_node *snod;
	resource_node *r;
	
	s_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	switch (s_val.v.tag)
	{
	case TAG_INT :
		AppendNumToTempString(s_val.v.data);
		break;
		
	case TAG_STRING :
		snod = GetStringByID( s_val.v.data);
		if(snod == NULL )
		{
			bprintf( "C_AppendTempString can't find string %s\n", fmt(s_val));
			return NIL;
		}
		AppendTempString(snod->data,snod->len_data);
		break;
		
	case TAG_TEMP_STRING :
		bprintf("C_AppendTempString attempting to append temp string to itself!\n");
		return NIL;
		
	case TAG_RESOURCE :
		r = GetResourceByID(s_val.v.data);
		if (r == NULL)
		{
			bprintf("C_AppendTempString can't set from invalid resource %" PRId64 "\n",s_val.v.data);
			return NIL;
		}
		AppendTempString(r->resource_val,(int) strlen(r->resource_val));
		break;
		
	case TAG_DEBUGSTR :
		{
			kod_statistics *kstat = GetKodStats();
			class_node *c = GetClassByID(kstat->interpreting_class);
			const char *pStrConst;
			int strLen = 0;
			
			if (c == NULL)
			{
				bprintf("C_AppendTempString can't find class %i, can't get debug str\n",kstat->interpreting_class);
				return NIL;
			}
			pStrConst = GetClassDebugStr(c,s_val.v.data);
			strLen = 0;
			if (pStrConst != NULL)
			{
				strLen = (int) strlen(pStrConst);
				AppendTempString(pStrConst,strLen);
			}
			else
			{
				bprintf("C_AppendTempString: GetClassDebugStr returned NULL");
				return NIL;
			}
		}
		break;
		
	case TAG_NIL :
		bprintf("C_AppendTempString can't set from NIL\n");
		break;
		
	default :
		bprintf("C_AppendTempString can't set from non-string thing %s\n", fmt(s_val));
		return NIL;
	}
	return NIL;
}

blak_int C_CreateString(int object_id,local_var_type *local_vars,
				   int num_normal_parms,parm_node normal_parm_array[],
				   int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val;
	
	ret_val.v.tag = TAG_STRING;
	ret_val.v.data = CreateString("");
	
	return ret_val.int_val;
}

blak_int C_StringLength(int object_id,local_var_type *local_vars,
				   int num_normal_parms,parm_node normal_parm_array[],
				   int num_name_parms,parm_node name_parm_array[])
{
	val_type s1_val,ret_val;
	const char *s1;
	int len;
	
	s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
   if (!LookupString(s1_val, "C_StringLength", &s1, &len))
      return NIL;
   
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = len;
	
	return ret_val.int_val;
}

blak_int C_StringConsistsOf(int object_id,local_var_type *local_vars,
                       int num_normal_parms,parm_node normal_parm_array[],
                       int num_name_parms,parm_node name_parm_array[])
{
	val_type s1_val,s2_val,ret_val;
	const char *s1,*s2;
	int len1,len2;
	
	s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
   if (!LookupString(s1_val, "C_StringConsistsOf", &s1, &len1))
      return NIL;
	
	s2_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
   if (!LookupString(s2_val, "C_StringConsistsOf", &s2, &len2))
      return NIL;

   // See if all characters in s1 are from s2.
   bool all_found = true;
   for (int i = 0; i < len1; ++i)
   {
      if (strchr(s2, s1[i]) == NULL)
      {
         all_found = false;
         break;
      }
   }
   
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = (int) all_found;
	
	return ret_val.int_val;
}

blak_int C_CreateTimer(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type object_val,message_val,time_val,ret_val;
	object_node *o;
	
	o = GetObjectByID(object_id);
	if (o == NULL)
	{
		eprintf("C_CreateTimer can't find object %i\n",object_id);
		return NIL;
	}
	
	object_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (object_val.v.tag != TAG_OBJECT)
	{
		bprintf("C_CreateTimer can't create a timer for non-object %s\n",
            fmt(object_val));
		return NIL;
	}
	
	message_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (message_val.v.tag != TAG_MESSAGE)
	{
		bprintf("C_CreateTimer can't create timer w/ non-message id %s\n",
            fmt(message_val));
		return NIL;
	}
	
	time_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	
	if (time_val.v.tag != TAG_INT || time_val.v.data < 0)
	{
		bprintf("C_CreateTimer can't create timer in negative int %s milliseconds\n",
            fmt(time_val));
		return NIL;
	}
	
	if (GetMessageByID(o->class_id,message_val.v.data,NULL) == NULL)
	{
		bprintf("C_CreateTimer can't create timer w/ message %" PRId64 " not for class %i\n",
			message_val.v.data,o->class_id);
		return NIL;
	}
	
	ret_val.v.tag = TAG_TIMER;
	ret_val.v.data = CreateTimer(o->object_id,message_val.v.data,time_val.v.data);
	/*   dprintf("create timer %i\n",ret_val.v.data); */
	
	return ret_val.int_val;
}

blak_int C_DeleteTimer(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type timer_val,ret_val;
	
	timer_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	if (timer_val.v.tag != TAG_TIMER)
	{
		bprintf("C_DeleteTimer can't delete non-timer %s\n", fmt(timer_val));
		return NIL;
	}
	ret_val.v.tag = TAG_INT; /* really a boolean */
	ret_val.v.data = DeleteTimer(timer_val.v.data);
	
	return ret_val.int_val;
}

blak_int C_GetTimeRemaining(int object_id,local_var_type *local_vars,
					   int num_normal_parms,parm_node normal_parm_array[],
					   int num_name_parms,parm_node name_parm_array[])
{
	val_type timer_val,ret_val;
	timer_node *t;
	
	timer_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	if (timer_val.v.tag != TAG_TIMER)
	{
		bprintf("C_GetTimeRemaining can't use non-timer %s\n", fmt(timer_val));
		return NIL;
	}
	
	t = GetTimerByID(timer_val.v.data);
	if (t == NULL)
	{
		bprintf("C_GetTimeRemaining can't find timer %s\n", fmt(timer_val));
		return NIL;
	}
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = (int)(t->time - GetMilliCount());
	if (ret_val.v.data < 0)
		ret_val.v.data = 0;
	
	return ret_val.int_val;
}

blak_int C_LoadRoom(int object_id,local_var_type *local_vars,
			   int num_normal_parms,parm_node normal_parm_array[],
			   int num_name_parms,parm_node name_parm_array[])
{
	val_type room_val;
	
	room_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (room_val.v.tag != TAG_RESOURCE)
	{
		bprintf("C_CreateRoomData can't use non-resource %s\n", fmt(room_val));
		return NIL;
	}
	
	return LoadRoomData(room_val.v.data);
}

blak_int C_RoomData(int object_id,local_var_type *local_vars,
			   int num_normal_parms,parm_node normal_parm_array[],
			   int num_name_parms,parm_node name_parm_array[])
{
	val_type room_val,ret_val,rows,cols,security;
	roomdata_node *room;
	
	room_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_RoomSize can't operate on non-room %si\n", fmt(room_val));
		return NIL;
	}
	
	room = GetRoomDataByID(room_val.v.data);
	if (room == NULL)
	{
		bprintf("C_RoomSize can't find room id %" PRId64 "\n",room_val.v.data);
		return NIL;
	}
	
	rows.v.tag = TAG_INT;
	rows.v.data = room->file_info.rows;
	cols.v.tag = TAG_INT;
	cols.v.data = room->file_info.cols;
	security.v.tag = TAG_INT;
	security.v.data = room->file_info.security;
	
	ret_val.int_val = NIL;
	
	ret_val.v.data = Cons(security,ret_val);
	ret_val.v.tag = TAG_LIST;
	
	ret_val.v.data = Cons(cols,ret_val);
	ret_val.v.tag = TAG_LIST;
	
	ret_val.v.data = Cons(rows,ret_val);
	ret_val.v.tag = TAG_LIST;
	
	return ret_val.int_val;
}

blak_int C_CanMoveInRoom(int object_id,local_var_type *local_vars,
					int num_normal_parms,parm_node normal_parm_array[],
					int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val,room_val,row_source,col_source,row_dest,col_dest;
	roomdata_node *r;
	
	/* determine whether there's a wall between the row,col to row,col
    * in the given room.
    */
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;
	
	room_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	row_source = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	col_source = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	row_dest = RetrieveValue(object_id,local_vars,normal_parm_array[3].type,
		normal_parm_array[3].value);
	col_dest = RetrieveValue(object_id,local_vars,normal_parm_array[4].type,
		normal_parm_array[4].value);
	
	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_CanMoveInRoom can't use non room %s\n", fmt(room_val));
		return ret_val.int_val;
	}
	
	if (row_source.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoom row source can't use non int %s\n", fmt(row_source));
		return ret_val.int_val;
	}
	
	if (col_source.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoom col source can't use non int %s\n", fmt(col_source));
		return ret_val.int_val;
	}
	
	if (row_dest.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoom col dest can't use non int %s\n", fmt(row_dest));
		return ret_val.int_val;
	}
	
	if (col_dest.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoom col dest can't use non int %s\n", fmt(col_dest));
		return ret_val.int_val;
	}
	
	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_CanMoveInRoom can't find room %" PRId64 "\n",room_val.v.data);
		return ret_val.int_val;
	}
	
	/* remember that kod uses 1-based arrays, and of course we don't */
	ret_val.v.data = CanMoveInRoom(r,(int) (row_source.v.data-1),
                                 (int) (col_source.v.data-1),
                                 (int) (row_dest.v.data-1),
                                 (int) (col_dest.v.data-1));
	
	return ret_val.int_val;
}

blak_int C_CanMoveInRoomFine(int object_id,local_var_type *local_vars,
						   int num_normal_parms,parm_node normal_parm_array[],
						   int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val,room_val,row_source,col_source,row_dest,col_dest;
	roomdata_node *r;
	
	/* determine whether there's a wall between the row,col to row,col
    * in the given room.
    */
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;
	
	room_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	row_source = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	col_source = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	row_dest = RetrieveValue(object_id,local_vars,normal_parm_array[3].type,
		normal_parm_array[3].value);
	col_dest = RetrieveValue(object_id,local_vars,normal_parm_array[4].type,
		normal_parm_array[4].value);
	
	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_CanMoveInRoomFine can't use non room %s\n", fmt(room_val));
		return ret_val.int_val;
	}
	
	if (row_source.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomFine row source can't use non int %s\n", fmt(row_source));
		return ret_val.int_val;
	}
	
	if (col_source.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomFine col source can't use non int %s\n", fmt(col_source));
		return ret_val.int_val;
	}
	
	if (row_dest.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomFine col dest can't use non int %s\n", fmt(row_dest));
		return ret_val.int_val;
	}
	
	if (col_dest.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomFine col dest can't use non int %s\n", fmt(col_dest));
		return ret_val.int_val;
	}
	
	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_CanMoveInRoomFine can't find room %" PRId64 "\n",room_val.v.data);
		return ret_val.int_val;
	}
	
	/* remember that kod uses 1-based arrays, and of course we don't */
	ret_val.v.data = CanMoveInRoomFine(r,(int) (row_source.v.data-1),
                                     (int) (col_source.v.data-1),
                                     (int) (row_dest.v.data-1),
                                     (int) (col_dest.v.data-1));
	
	return ret_val.int_val;
}

blak_int C_Cons(int object_id,local_var_type *local_vars,
		   int num_normal_parms,parm_node normal_parm_array[],
		   int num_name_parms,parm_node name_parm_array[])
{
	val_type source_val,dest_val,ret_val;
	
	source_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	dest_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	ret_val.v.tag = TAG_LIST;
	ret_val.v.data = Cons(source_val,dest_val);
	return ret_val.int_val;
}

blak_int C_First(int object_id,local_var_type *local_vars,
			int num_normal_parms,parm_node normal_parm_array[],
			int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_First object %i can't take First of a non-list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}
	if (!IsListNodeByID(list_val.v.data))
	{
		bprintf("C_First object %i can't take First of an invalid list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}
	return First(list_val.v.data);
}

blak_int C_Rest(int object_id,local_var_type *local_vars,
		   int num_normal_parms,parm_node normal_parm_array[],
		   int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_Rest object %i can't take Rest of a non-list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}
	if (!IsListNodeByID(list_val.v.data))
	{
		bprintf("C_Rest object %i can't take Rest of an invalid list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}
	return Rest(list_val.v.data);
}

blak_int C_Length(int object_id,local_var_type *local_vars,
			 int num_normal_parms,parm_node normal_parm_array[],
			 int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val,ret_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (list_val.v.tag == TAG_NIL)
	{
		ret_val.v.tag = TAG_INT;
		ret_val.v.data = 0;
		return ret_val.int_val;
	}
	
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_Length object %i can't take Length of a non-list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = Length(list_val.v.data);
	return ret_val.int_val;
}

blak_int C_Nth(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[])
{
	val_type n_val,list_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_Nth object %i can't take Nth of a non-list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}
	if (!IsListNodeByID(list_val.v.data))
	{
		bprintf("C_Nth object %i can't take Nth of an invalid list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}
	
	n_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (n_val.v.tag != TAG_INT)
	{
		bprintf("C_Nth can't take Nth with n = non-int %s\n", fmt(n_val));
		return NIL;
	}
	
	return Nth(n_val.v.data,list_val.v.data);
}

blak_int C_List(int object_id,local_var_type *local_vars,
		   int num_normal_parms,parm_node normal_parm_array[],
		   int num_name_parms,parm_node name_parm_array[])
{
	val_type temp,ret_val;
	int i;
	
	if (num_normal_parms == 0)
		return NIL;
	
	ret_val.int_val = NIL;
	for (i=num_normal_parms-1;i>=0;i--)
	{
		extern list_node *list_nodes;
		temp = RetrieveValue(object_id,local_vars,normal_parm_array[i].type,
			normal_parm_array[i].value);
		ret_val.v.data = Cons(temp,ret_val);
		ret_val.v.tag = TAG_LIST; /* do this AFTER the cons call or DIE */
	}
	return ret_val.int_val;
}

blak_int C_IsList(int object_id,local_var_type *local_vars,
			 int num_normal_parms,parm_node normal_parm_array[],
			 int num_name_parms,parm_node name_parm_array[])
{
	val_type var_check,ret_val;
	
	var_check = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	
	ret_val.v.tag = TAG_INT;
	if (var_check.v.tag == TAG_LIST || var_check.v.tag == TAG_NIL)
		ret_val.v.data = true;
	else
		ret_val.v.data = false;
	
	return ret_val.int_val;
}

blak_int C_SetFirst(int object_id,local_var_type *local_vars,
			   int num_normal_parms,parm_node normal_parm_array[],
			   int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val,set_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_SetFirst object %i can't set elem of non-list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}
	
	set_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	
	return SetFirst(list_val.v.data,set_val);
}

blak_int C_SetNth(int object_id,local_var_type *local_vars,
			 int num_normal_parms,parm_node normal_parm_array[],
			 int num_name_parms,parm_node name_parm_array[])
{
	val_type n_val,list_val,set_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_SetFirst object %i can't set elem of non-list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}
	n_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (n_val.v.tag != TAG_INT)
	{
		bprintf("C_SetNth object %i can't take Nth with n = non-int %s\n",
            object_id,fmt(n_val));
		return NIL;
	}
	
	set_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	
	return SetNth(n_val.v.data,list_val.v.data,set_val);
}

blak_int C_DelListElem(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val,list_elem,ret_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_DelListElem object %i can't delete elem from non-list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}
	
	list_elem = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
			     normal_parm_array[1].value);
	
	ret_val.int_val = DelListElem(list_val,list_elem);
	
	return ret_val.int_val;
}

blak_int C_FindListElem(int object_id,local_var_type *local_vars,
				   int num_normal_parms,parm_node normal_parm_array[],
				   int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val,list_elem,ret_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (list_val.v.tag == TAG_NIL)
	{
		return NIL;
	}
	
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_FindListElem object %i can't find elem in non-list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}
	
	list_elem = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
			     normal_parm_array[1].value);
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = FindListElem(list_val,list_elem);
	if (NIL == ret_val.int_val)
	{
		return NIL;
	}
	
	return ret_val.int_val;
}

blak_int C_MoveListElem(int object_id,local_var_type *local_vars,
                        int num_normal_parms,parm_node normal_parm_array[],
                        int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
                                    normal_parm_array[0].value);
	
	if (list_val.v.tag == TAG_NIL)
	{
		return NIL;
	}
	
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_MoveListElem object %i can't move elem in non-list %s\n",
            object_id,fmt(list_val));
		return NIL;
	}

	val_type n_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
                                 normal_parm_array[1].value);
	if (n_val.v.tag != TAG_INT)
	{
		bprintf("C_MoveListElem object %i can't lookup non-int index %s\n",
            object_id, fmt(n_val));
    return NIL;
  }

  val_type m_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
                                 normal_parm_array[2].value);
	if (m_val.v.tag != TAG_INT)
	{
		bprintf("C_MoveListElem object %i can't lookup non-int index %s\n",
            object_id, fmt(m_val));
    return NIL;
  }

  MoveListElem(list_val, n_val, m_val);

  return NIL;
}

blak_int C_GetTime(int object_id,local_var_type *local_vars,
			  int num_normal_parms,parm_node normal_parm_array[],
			  int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val;
	
	ret_val.v.tag = TAG_INT;

	/* This offset is left over from when in-memory values were 32 bits,
	   and would overflow when the time wrapped around (every 8 years or so).     
	   This matches an offset in the client but is otherwise no longer strictly needed.
	   Removing it would be difficult, as some time values are stored in objects.
	*/

   ret_val.v.data = GetTime() - 1534000000L;    // Offset to sometime in mid-2018
	
	return ret_val.int_val;
}

blak_int C_Random(int object_id,local_var_type *local_vars,
			 int num_normal_parms,parm_node normal_parm_array[],
			 int num_name_parms,parm_node name_parm_array[])
{
	val_type low_bound,high_bound;
	val_type ret_val;
	int randomValue;
	
	low_bound = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	high_bound = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (low_bound.v.tag != TAG_INT || high_bound.v.tag != TAG_INT)
	{
		bprintf("C_Random got an invalid boundary %s or %s\n",
            fmt(low_bound), fmt(high_bound));
		return NIL;
	}
	if (low_bound.v.data > high_bound.v.data)
	{
		bprintf("C_Random got low > high boundary %" PRId64 " and %" PRId64 "\n",
            low_bound.v.data,high_bound.v.data);
		return NIL;
	}
	ret_val.v.tag = TAG_INT;
#if 0
	ret_val.v.data = low_bound.v.data + rand() % (high_bound.v.data -
		low_bound.v.data + 1);
#else
	// The rand() function returns number between 0 and 0x7fff (MAX_RAND)
	// we have to scale this to fit our range. -- call twice to fill all the bits
	// and mask to a 28 bit positive kod integer
	randomValue = MAX_KOD_INT & ((rand() << 15) + rand());
	ret_val.v.data = low_bound.v.data + randomValue % (high_bound.v.data - 
		low_bound.v.data + 1);
#endif
	return ret_val.int_val;
	
}

blak_int C_Abs(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[])
{
	val_type int_val,ret_val;
	
	int_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (int_val.v.tag != TAG_INT)
	{
		bprintf("C_Abs can't use %s\n",fmt(int_val));
		return NIL;
	}
	
	ret_val.v.tag = TAG_INT;
	if (int_val.v.data & (1 << 27))
		ret_val.v.data = -int_val.v.data;
	else
		ret_val.v.data = int_val.v.data;
	
	return ret_val.int_val;  
}

blak_int C_Sqrt(int object_id,local_var_type *local_vars,
		   int num_normal_parms,parm_node normal_parm_array[],
		   int num_name_parms,parm_node name_parm_array[])
{
	val_type int_val,ret_val;
	
	int_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (int_val.v.tag != TAG_INT)
	{
		bprintf("C_Sqrt can't use %s\n",fmt(int_val));
		return NIL;
	}
	if (int_val.v.data & (1 << 27))
	{
		bprintf("C_Sqrt result undefined for negative value\n");
		return NIL;
	}
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = (int)sqrt((double)int_val.v.data);
	
	return ret_val.int_val;  
}

blak_int C_Bound(int object_id,local_var_type *local_vars,
			int num_normal_parms,parm_node normal_parm_array[],
			int num_name_parms,parm_node name_parm_array[])
{
	val_type int_val,min_val,max_val;
	
	int_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (int_val.v.tag != TAG_INT)
	{
		bprintf("C_Bound can't use %s\n",fmt(int_val));
		return NIL;
	}
	
	min_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (min_val.v.tag != TAG_NIL)
	{
		if (min_val.v.tag != TAG_INT)
		{
			bprintf("C_Bound can't use min bound %s\n",fmt(min_val));
			return NIL;
		}
		if (int_val.v.data < min_val.v.data)
			int_val.v.data = min_val.v.data;
	}	 
	
	max_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	if (max_val.v.tag != TAG_NIL)
	{
		if (max_val.v.tag != TAG_INT)
		{
			bprintf("C_Bound can't use max bound %s\n",fmt(max_val));
			return NIL;
		}
		if (int_val.v.data > max_val.v.data)
			int_val.v.data = max_val.v.data;
	}	 
	
	return int_val.int_val;
}

blak_int C_CreateTable(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val;
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = CreateTable(2999);
	
	return ret_val.int_val;
	
}

blak_int C_AddTableEntry(int object_id,local_var_type *local_vars,
					int num_normal_parms,parm_node normal_parm_array[],
					int num_name_parms,parm_node name_parm_array[])
{
	val_type int_val,key_val,data_val;
	
	
	int_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (int_val.v.tag != TAG_INT)
	{
		bprintf("C_AddTableEntry can't use table id %s\n",fmt(int_val));
		return NIL;
	}
	
	key_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	
	data_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	
	InsertTable(int_val.v.data,key_val,data_val);
	return NIL;
}

blak_int C_GetTableEntry(int object_id,local_var_type *local_vars,
					int num_normal_parms,parm_node normal_parm_array[],
					int num_name_parms,parm_node name_parm_array[])
{
	val_type int_val,key_val,ret_val;
	
	
	int_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (int_val.v.tag != TAG_INT)
	{
		bprintf("C_GetTableEntry can't use table id %s\n",fmt(int_val));
		return NIL;
	}
	
	key_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	
	ret_val.int_val = GetTableEntry(int_val.v.data,key_val);
	return ret_val.int_val;
}

blak_int C_DeleteTableEntry(int object_id,local_var_type *local_vars,
					   int num_normal_parms,parm_node normal_parm_array[],
					   int num_name_parms,parm_node name_parm_array[])
{
	val_type int_val,key_val;
	
	
	int_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (int_val.v.tag != TAG_INT)
	{
		bprintf("C_DeleteTableEntry can't use table id %s\n",fmt(int_val));
		return NIL;
	}
	
	key_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	
	DeleteTableEntry(int_val.v.data,key_val);
	return NIL;
}

blak_int C_DeleteTable(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type int_val;
	
	
	int_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (int_val.v.tag != TAG_INT)
	{
		bprintf("C_DeleteTable can't use table id %s\n",fmt(int_val));
		return NIL;
	}
	
	DeleteTable(int_val.v.data);
	return NIL;
}


blak_int C_RecycleUser(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type object_val;
	object_node *o;
	user_node *old_user;
	user_node *new_user;
	
	object_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (object_val.v.tag != TAG_OBJECT)
	{
		bprintf("C_RecycleUser can't recycle non-object %s\n",fmt(object_val));
		return NIL;
	}
	
	o = GetObjectByID(object_val.v.data);
	if (o == NULL)
	{
		bprintf("C_RecycleUser can't find object %" PRId64 "\n",object_val.v.data);
		return NIL;
	}
	
	// Find the old user from the object that KOD gives us.
	old_user = GetUserByObjectID(o->object_id);
	if (old_user == NULL)
	{
		bprintf("C_RecycleUser can't find user which is object %" PRId64 "\n",object_val.v.data);
		return NIL;
	}
	
	// Create another user which matches the old one.
	new_user = CreateNewUser(old_user->account_id,o->class_id);
	//bprintf("C_RecycleUser made new user, got object %i\n",new_user->object_id);
	
	// Delete the old user/object.
	// KOD:  post(old_user_object,@Delete);
	//
	PostBlakodMessage(old_user->object_id,DELETE_MSG,0,NULL);
	DeleteUserByObjectID(old_user->object_id);
	//bprintf("C_RecycleUser deleted old user and object %i\n",old_user->object_id);
	
	object_val.v.tag = TAG_OBJECT;
	object_val.v.data = new_user->object_id;
	
	return object_val.int_val;
}

blak_int C_IsObject(int object_id,local_var_type *local_vars,
			   int num_normal_parms,parm_node normal_parm_array[],
			   int num_name_parms,parm_node name_parm_array[])
{
	val_type var_check,ret_val;
	
	var_check = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	
	ret_val.v.tag = TAG_INT;
	if (var_check.v.tag == TAG_OBJECT && GetObjectByID(var_check.v.data))
		ret_val.v.data = true;
	else
		ret_val.v.data = false;
	
	return ret_val.int_val;
}

blak_int C_MinigameNumberToString(int object_id,local_var_type *local_vars,
				int num_normal_parms,parm_node normal_parm_array[],
				int num_name_parms,parm_node name_parm_array[])
{
	val_type s1_val,s2_val;
	string_node *snod;
   int number;
   char newString[4];
   int index, iTemp;
	
	s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (s1_val.v.tag != TAG_STRING)
	{
		bprintf("C_MinigameNumberToString can't set non-string %s\n", fmt(s1_val));
		return NIL;
	}
	
	snod = GetStringByID(s1_val.v.data);
	if (snod == NULL)
	{
		bprintf("C_MinigameNumberToString can't set invalid string %s\n", fmt(s1_val));
		return NIL;
	}
	
	s2_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);

	if (s2_val.v.tag != TAG_INT)
	{
		bprintf("C_MinigameNumberToString can't set from non-int %s\n", fmt(s2_val));
		return NIL;
	}

   number = s1_val.v.data;

   index = 3;

   // This reverses the number on purpose, since the first four bits aren't a full number.
   while (index >= 0)
   {
      // Take off last byte off number.
      iTemp = number & 0xFF;
      number = number >> 8;

      newString[index] = (char)iTemp;

      index--;
   }

   SetString(snod, newString, 4);

	return NIL;
}

blak_int C_MinigameStringToNumber(int object_id,local_var_type *local_vars,
				int num_normal_parms,parm_node normal_parm_array[],
				int num_name_parms,parm_node name_parm_array[])
{
 	val_type s1_val, ret_val;
	string_node *snod;
   int number;
   int index;

	s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (s1_val.v.tag != TAG_STRING)
	{
		bprintf("C_MinigameNumberToString can't set non-string %s\n", fmt(s1_val));
		return NIL;
	}
	
	snod = GetStringByID(s1_val.v.data);
	if (snod == NULL)
	{
		bprintf("C_MinigameNumberToString can't set invalid string %s\n", fmt(s1_val));
		return NIL;
	}

   number = 0;

   index = 0;

   // Chew the string, spit out an int in reverse order.
   while (index < 4)
   {
      number = number << 8;
      number = number & snod->data[index];

      index = index + 1;
   }

	ret_val.v.tag = TAG_INT;
	ret_val.v.data = number;
	
	return ret_val.int_val;
}

blak_int C_SendWebhook(int object_id, local_var_type *local_vars,
    int num_normal_parms, parm_node normal_parm_array[],
    int num_name_parms, parm_node name_parm_array[])
{
    val_type msg_val, event_val, key_val, value_val;
    const char *content, *event_name, *key_str, *value_str;
    int content_len, event_len, key_len, value_len;
    std::string json;
    
    // Early exit if webhooks not enabled - avoid all string/JSON work
    if (!IsWebhookEnabled()) {
        return NIL;
    }

    // Handle single string parameter
    if (num_normal_parms == 1) {
        msg_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type, normal_parm_array[0].value);
        
        if (!LookupString(msg_val, "C_SendWebhook", &content, &content_len)) {
            bprintf("C_SendWebhook: LookupString failed\n");
            return NIL;
        }

        SendWebhookMessage(content, content_len);
        return NIL;
    }
    
    // Handle JSON style: event name + key-value pairs
    if (num_normal_parms < 3) {
        bprintf("C_SendWebhook: Invalid number of parameters\n");
        return NIL;
    }
    
    // First parameter is the event name
    event_val = RetrieveValue(object_id, local_vars, 
        normal_parm_array[0].type, normal_parm_array[0].value);
    
    if (!LookupString(event_val, "C_SendWebhook", &event_name, &event_len)) {
        bprintf("C_SendWebhook: Event name lookup failed\n");
        return NIL;
    }
    
    // Start building JSON: {"event": "EventName", "params": {
    json += "{\"event\":\"";
    json.append(event_name, event_len);
    json += "\",\"params\":{";
    
    // Process remaining parameters as key-value pairs
    for (int i = 1; i < num_normal_parms; i += 2) {
        if (i + 1 >= num_normal_parms) {
            break; // Need pairs of arguments
        }
        
        // Get the key (parameter name)
        key_val = RetrieveValue(object_id, local_vars,
            normal_parm_array[i].type, normal_parm_array[i].value);
        
        if (!LookupString(key_val, "C_SendWebhook", &key_str, &key_len)) {
            continue; // Skip if key is not a string
        }
        
        // Get the value
        value_val = RetrieveValue(object_id, local_vars,
            normal_parm_array[i + 1].type, normal_parm_array[i + 1].value);
        
        // Add comma if not first parameter
        if (i > 1) {
            json += ",";
        }
        
        // Add key
        json += "\"";
        json.append(key_str, key_len);
        json += "\":";
        
        // Handle different value types
        if (value_val.v.tag == TAG_STRING || value_val.v.tag == TAG_TEMP_STRING || 
            value_val.v.tag == TAG_RESOURCE) {
            if (LookupString(value_val, "C_SendWebhook", &value_str, &value_len)) {
                // Escape quotes in the string value
                json += "\"";
                for (int j = 0; j < value_len; j++) {
                    if (value_str[j] == '\"' || value_str[j] == '\\') {
                        json += '\\';
                    }
                    json += value_str[j];
                }
                json += "\"";
            } else {
                json += "null";
            }
        } else if (value_val.v.tag == TAG_INT) {
            json += std::to_string(value_val.v.data);
        } else if (value_val.v.tag == TAG_NIL) {
            json += "null";
        } else {
            // For objects or other types, try to get their name or use ID
            json += std::to_string(value_val.v.data);
        }
    }
    
    // Close JSON: }}
    json += "}}";
    
    // Send the webhook message
    SendWebhookMessage(json.c_str(), (int)json.length());
    return NIL;
}