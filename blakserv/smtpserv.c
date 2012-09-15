// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* smtpserv.c
*

  This module contains functions that implement a barebones SMTP server.
  
*/

#include "blakserv.h"

/* uncomment next line for use in smtptest.exe */
/* #define SMTP_TEST */

void HandleSMTPHello(smtp_node *smtp);
void HandleSMTPQuit(smtp_node *smtp);
void HandleSMTPNoop(smtp_node *smtp);
void HandleSMTPMail(smtp_node *smtp);
void HandleSMTPRecipient(smtp_node *smtp);
void HandleSMTPData(smtp_node *smtp);
void HandleSMTPReset(smtp_node *smtp);

typedef struct
{
	char *name;
	void (*smtp_func)(smtp_node *smtp);
} smtp_table_type;

smtp_table_type smtp_table[] =
{
	{ "HELO", HandleSMTPHello },
	{ "QUIT", HandleSMTPQuit },
	{ "NOOP", HandleSMTPNoop },
	{ "MAIL", HandleSMTPMail },
	{ "RCPT", HandleSMTPRecipient },
	{ "DATA", HandleSMTPData },
	{ "RSET", HandleSMTPReset },
};

#define LEN_SMTP_TABLE (sizeof(smtp_table)/sizeof(smtp_table_type))



void __cdecl ThreadHandleSMTPConnection(void *param);
void ReadSMTPLine(smtp_node *smtp);
void ProcessSMTPLine(smtp_node *smtp);

void AddSMTPDataLine(smtp_node *smtp,char *s);
void ProcessMessageDataDone(smtp_node *smtp);
int MakeStringFromSMTPMail(smtp_node *smtp);
void CreateAccountFromSMTPMail(smtp_node *smtp);
void CreateAccountFromBuffer(char *buf);
void DeleteAccountFromBuffer(char *buf);
void CreateNewAccountAndCharacter(char *name,char *password);
void SendStringMailToObject(int string_id,int object_id);

int GetInternetMailObject(char *s);
Bool ExistInternetMailName(char *s);

void ClearSMTPBuffers(smtp_node *smtp);

void __cdecl smtpprintf(smtp_node *smtp,char *fmt,...);
void SendSMTPMessage(smtp_node *smtp,char *msg);
void SendSMTPBuffer(smtp_node *smtp,char *msg);

char *GetLocalMachineName(void);

char *SMTPTimeStr(int time);


void HandleSMTPConnection(smtp_node *smtp)
{
	HANDLE hThread;
	
	if (ConfigBool(DEBUG_SMTP))
		dprintf("SMTP Connection: %s\n",smtp->source_name);
	
	hThread = (HANDLE) _beginthread(ThreadHandleSMTPConnection,0,
                                   (void *)smtp);
	SetThreadPriority(hThread,THREAD_PRIORITY_BELOW_NORMAL);
}

/* this function is run in its own thread */
void __cdecl ThreadHandleSMTPConnection(void *param)
{
   smtp_node *smtp = (smtp_node *) param;
	/* gotta free the smtp node when done--allocated by AsyncSMTPSocketConnect in async.c */
	
	smtpprintf(smtp,"220 %s Ready to go",GetLocalMachineName());
	
	while (smtp->state != SMTP_QUIT && smtp->state != SMTP_ERROR)
	{
		ReadSMTPLine(smtp);
	}
	
	if (ConfigBool(DEBUG_SMTP))
		dprintf("SMTP Disconnection: %s\n",smtp->source_name);
	
	ClearSMTPBuffers(smtp);
	
	closesocket(smtp->sock);
	
	FreeMemory(MALLOC_ID_SMTP,smtp,sizeof(smtp_node));
}

void ReadSMTPLine(smtp_node *smtp)
{
	int i,read_bytes;
	
	for(;;)
	{
		for (i=1;i<smtp->len_buf;i++)
		{
			if (smtp->buf[i-1] == CR && smtp->buf[i] == LF)
			{
				memcpy(smtp->cmd,smtp->buf,i+1); /* copy over cr or lf */
				smtp->cmd[i+1] = 0; /* null terminate string */
				
				/* take out command from buffer */
				memmove(smtp->buf,smtp->buf + i+1,smtp->len_buf-i-1); 
				smtp->len_buf -= i+1;
				
				ProcessSMTPLine(smtp);
				return;
			}
		}
		
		read_bytes = recv(smtp->sock,smtp->buf+smtp->len_buf,SMTP_MAX_LINE-smtp->len_buf,0);
		
		if (read_bytes <= 0)
		{
			if (read_bytes == SOCKET_ERROR)
				eprintf("ReadSMTPLine got recv error %i\n",WSAGetLastError());
			smtp->state = SMTP_ERROR;
			return;
		}
		
		smtp->len_buf += read_bytes;
		
		/* check for full buffer w/o CRLF */
		if (smtp->len_buf == SMTP_MAX_LINE)
		{
			memcpy(smtp->cmd,smtp->buf,smtp->len_buf);
			smtp->cmd[smtp->len_buf+1] = 0; /* null terminate string */
			
			/* take out command from buffer */
			smtp->len_buf = 0;
			
			ProcessSMTPLine(smtp);
			return;
		}
	}
	eprintf("ReadSMTPLine can't get here\n");
	smtp->state = SMTP_ERROR;
	return;
}

void ProcessSMTPLine(smtp_node *smtp)
{
	int i;
	char *cmd;
	
	if (smtp->state == SMTP_DATA)
	{
		if (!stricmp(smtp->cmd,".\r\n"))
		{
			ProcessMessageDataDone(smtp);
			smtpprintf(smtp,"250 Ok, got the message");
			/* dprintf("Sent ok, got message"); */
			smtp->state = SMTP_READY;
		}
		else
		{
			if (smtp->cmd[0] == '.')
				AddSMTPDataLine(smtp,smtp->cmd+1);
			else
				AddSMTPDataLine(smtp,smtp->cmd);
		}
		return;
	}
	
	if (ConfigBool(DEBUG_SMTP))
		dprintf("SMTP Command: %s\n",smtp->cmd);
	
	cmd = strtok(smtp->cmd," \t\r\n");
	if (cmd == NULL)
	{
		return;
	}
	
	for (i=0;i<LEN_SMTP_TABLE;i++)
	{
		if (!stricmp(smtp_table[i].name,cmd))
		{
			smtp_table[i].smtp_func(smtp);
			return;
		}
	}
	smtpprintf(smtp,"502 Command not handled");
}

void HandleSMTPHello(smtp_node *smtp)
{
	smtpprintf(smtp,"250 Hello yourself");
}

void HandleSMTPQuit(smtp_node *smtp)
{
	smtpprintf(smtp,"250 Ok, bye");
	smtp->state = SMTP_QUIT;
}

void HandleSMTPNoop(smtp_node *smtp)
{
	smtpprintf(smtp,"250 Ok");
}

void HandleSMTPMail(smtp_node *smtp)
{
	char *s;
	
	/* Syntax for this command is MAIL FROM:<xxx> */
	
	s = strtok(NULL," \t\r\n");
	if (s == NULL)
	{
		smtpprintf(smtp,"501 Syntax error, no parameter");
		return;
	}
	
	if (strnicmp(s,"FROM:<",6) != 0)
	{
		smtpprintf(smtp,"501 Syntax error, parameter doesn't start from:<");
		return;
	}
	
	if (s[strlen(s)-1] != '>')
	{
		smtpprintf(smtp,"501 Syntax error, parameter doesn't end >");
		return;
	}
	
	s[strlen(s)-1] = 0; /* kill off the > */
	s += 6; /* skip over the to:< */
	
	
	ClearSMTPBuffers(smtp);
	smtp->reverse_path = (char *) AllocateMemory(MALLOC_ID_SMTP,strlen(s)+1);
	strcpy(smtp->reverse_path,s);
	
	smtpprintf(smtp,"250 Ok");
}

void HandleSMTPRecipient(smtp_node *smtp)
{
	char *s,*dest_machine;
	string_list *sl;
	
	/* Syntax for this command is RCPT TO:<xxx@yyy.zzz> */
	
	s = strtok(NULL," \t\r\n");
	if (s == NULL)
	{
		smtpprintf(smtp,"501 Syntax error, no parameter");
		return;
	}
	
	if (strnicmp(s,"TO:<",4) != 0)
	{
		smtpprintf(smtp,"501 Syntax error, parameter doesn't start to:<");
		return;
	}
	
	if (s[strlen(s)-1] != '>')
	{
		smtpprintf(smtp,"501 Syntax error, parameter doesn't end >");
		return;
	}
	
	s[strlen(s)-1] = 0; /* kill off the > */
	s += 4; /* skip over the to:< */
	
	/* check if there's a colon, which means it should be forwarded */
	if (strchr(s,':') != 0)
	{
		smtpprintf(smtp,"550 Can't forward (To:<xxx> had a :)");
		return;
	}
	
	/* make sure there's an @, and get the name & machine name */
	if (strchr(s,'@') == NULL)
	{
		smtpprintf(smtp,"550 Can't find destination (To:<xxx> had no @)");
		return;
	}
	
	s = strtok(s,"@");
	dest_machine = strtok(NULL,"@");
	
	if (s == NULL || dest_machine == NULL)
	{
		smtpprintf(smtp,"550 Can't find destination person or machine");
		return;
	}
	
	if (stricmp(dest_machine,GetLocalMachineName()) != 0)
	{
		smtpprintf(smtp,"550 Can only send mail to this machine (%s)",GetLocalMachineName());
		return;    
	}
	
	/* dprintf("recipient %s\n",s); */
	
	/* check if the person in s is a local user */
	
	if (ExistInternetMailName(s) == False)
	{
		smtpprintf(smtp,"550 No user by the name %s here\n",s);
		return;
	}
	
	/* insert node on front of string list which is the destinations */
	sl = (string_list *) AllocateMemory(MALLOC_ID_SMTP,sizeof(string_list));
	sl->str = (char *) AllocateMemory(MALLOC_ID_SMTP,strlen(s)+1);
	strcpy(sl->str,s);
	sl->next = smtp->forward_path;
	smtp->forward_path = sl;
	
	smtpprintf(smtp,"250 Ok");
}

void HandleSMTPData(smtp_node *smtp)
{
	char s[SMTP_MAX_LINE];
	
	if (smtp->forward_path == NULL || smtp->reverse_path == NULL)
	{
		smtpprintf(smtp,"503 Bad sequence of commands");
		return;
	}
	
	sprintf(s,"Return-Path: <%s>\r\n",smtp->reverse_path);
	AddSMTPDataLine(smtp,s);
	sprintf(s,"Received: from %s by %s with SMTP ; %s\r\n",smtp->source_name,
		GetLocalMachineName(),SMTPTimeStr(GetTime()));
	
	AddSMTPDataLine(smtp,s);
	
	smtp->state = SMTP_DATA;
	smtpprintf(smtp,"354 Send mail now; end with <CRLF>.<CRLF>");
}

void HandleSMTPReset(smtp_node *smtp)
{
	ClearSMTPBuffers(smtp);
	smtpprintf(smtp,"250 Ok");
}

void AddSMTPDataLine(smtp_node *smtp,char *s)
{
	string_list *sl,*temp;
	
	sl = (string_list *) AllocateMemory(MALLOC_ID_SMTP,sizeof(string_list));
	sl->str = (char *) AllocateMemory(MALLOC_ID_SMTP,strlen(s)+1);
	strcpy(sl->str,s);
	sl->next = NULL;
	
	/* add new line at end, to keep message in order */
	
	sl->next = NULL;
	if (smtp->data == NULL)
		smtp->data = sl;
	else
	{
		temp = smtp->data;
		while (temp->next != NULL)
			temp = temp->next;
		temp->next = sl;
	}
}

void ProcessMessageDataDone(smtp_node *smtp)
{
	string_list *sl;
	val_type object_val;
	int string_id;
	
	CreateAccountFromSMTPMail(smtp);
	return;
	
	EnterServerLock();   
	
	/* take smtp->data and make it into a string in the game */
	
	string_id = MakeStringFromSMTPMail(smtp);
	
	/* now, send it to each person who it was sent to */
	
	sl = smtp->forward_path;
	while (sl != NULL)
	{
		object_val.int_val = GetInternetMailObject(sl->str);
		
		if (object_val.v.tag != TAG_OBJECT)
			eprintf("ProcessMessageDataDone could not find object for internet mail name %s\n",
			sl->str);
		else
		{
			dprintf("got object %i for name %s\n",object_val.v.data,sl->str);
			SendStringMailToObject(string_id,object_val.v.data);
		}
		sl = sl->next;
		
	}
	
	LeaveServerLock();
	
	ClearSMTPBuffers(smtp);
}

int MakeStringFromSMTPMail(smtp_node *smtp)
{
#ifdef SMTP_TEST
	return 0;
#else
	
	int len_buf,string_id;
	char *buf;
	string_list *sl;
	
	/* cheesy here--allocate buffer, make the string, then free buffer.
	would be nice to not have to allocate here */
	
	len_buf = 0;
	sl = smtp->data;
	while (sl != NULL)
	{
		len_buf += strlen(sl->str);
		sl = sl->next;
	}
	
	buf = (char *) AllocateMemory(MALLOC_ID_SMTP,len_buf+1);
	len_buf = 0;
	
	sl = smtp->data;
	while (sl != NULL)
	{
		strcpy(buf+len_buf,sl->str);
		len_buf += strlen(sl->str);
		sl = sl->next;
	}
	
	string_id = CreateStringWithLen(buf,len_buf+1);
	
	FreeMemory(MALLOC_ID_SMTP,buf,len_buf);
	
	return string_id;
	
#endif
}

void CreateAccountFromSMTPMail(smtp_node *smtp)
{
#ifdef SMTP_TEST
	return;
#else
	
	int len_buf;
	char *buf;
	string_list *sl;
	
	/* put mail into buffer, than analyze, then free buffer */
	
	len_buf = 0;
	sl = smtp->data;
	while (sl != NULL)
	{
		len_buf += strlen(sl->str);
		sl = sl->next;
	}
	
	buf = (char *) AllocateMemory(MALLOC_ID_SMTP,len_buf+1);
	len_buf = 0;
	
	sl = smtp->data;
	while (sl != NULL)
	{
		strcpy(buf+len_buf,sl->str);
		len_buf += strlen(sl->str);
		sl = sl->next;
	}
	
	if (strcmp(smtp->forward_path->str,ConfigStr(EMAIL_ACCOUNT_CREATE_NAME)) == 0)
	{
		CreateAccountFromBuffer(buf);
	}
	else
	{
		if (strcmp(smtp->forward_path->str,ConfigStr(EMAIL_ACCOUNT_DELETE_NAME)) == 0)
			DeleteAccountFromBuffer(buf);
	}
	
	FreeMemory(MALLOC_ID_SMTP,buf,len_buf+1);
#endif
}

void CreateAccountFromBuffer(char *buf)
{
	char *line;
	char *version,*name,*password;
	
	version = name = password = NULL;
	
	line = strtok(buf,"\r\n");
	while (line != NULL)
	{
	/*
	if (ConfigBool(DEBUG_SMTP))
	{
	dprintf(": %i %s\n",strlen(line),line); 
	Sleep(500);
	}
		*/
		
		if (strcmp(line,"VERSION:") == 0)
			version = strtok(NULL,"\r\n");
		
		if (strcmp(line,"NAME:") == 0)
			name = strtok(NULL,"\r\n");
		if (strcmp(line,"PASSWORD:") == 0)
			password = strtok(NULL,"\r\n");
		
		line = strtok(NULL,"\r\n");
	}
	
	if (version == NULL)
	{
		eprintf("CreateAccountFromBuffer got no version\n");
		return;
	}
	
	if (strcmp(version,"2") != 0)
	{
		eprintf("CreateAccountFromBuffer got version != 2 (%s)\n",version);
		return;
	}
	
	if (name == NULL || password == NULL)
	{
		eprintf("CreateAccountFromBuffer got email w/o name or password\n");
		return;
	}
	
	if (strlen(name) > MAX_LOGIN_NAME || strlen(password) > MAX_LOGIN_PASSWORD)
	{
		eprintf("CreateAccountFromBuffer got name or password too long\n");
		return;
	}
	
	if (strlen(name) < 3 || strlen(password) < 3)
	{
		eprintf("CreateAccountFromBuffer got charname or password too short\n");
		return;
	}
	
	EnterServerLock();
	CreateNewAccountAndCharacter(name,password);
	LeaveServerLock();
}

void DeleteAccountFromBuffer(char *buf)
{
	char *line;
	char *version,*name;
	account_node *a;
	
	version = name = NULL;
	
	line = strtok(buf,"\r\n");
	while (line != NULL)
	{
		if (ConfigBool(DEBUG_SMTP))
		{
			dprintf(": %i %s\n",strlen(line),line); 
			Sleep(500);
		}
		
		
		if (strcmp(line,"VERSION:") == 0)
			version = strtok(NULL,"\r\n");
		
		if (strcmp(line,"NAME:") == 0)
			name = strtok(NULL,"\r\n");
		
		line = strtok(NULL,"\r\n");
	}
	
	if (version == NULL)
	{
		eprintf("DeleteAccountFromBuffer got no version\n");
		return;
	}
	
	if (strcmp(version,"1") != 0)
	{
		eprintf("DeleteAccountFromBuffer got version != 1 (%s)\n",version);
		return;
	}
	
	if (name == NULL)
	{
		eprintf("DeleteAccountFromBuffer got email w/o name\n");
		return;
	}
	
	EnterServerLock();
	a = GetAccountByName(name);
	if (a == NULL)
		eprintf("DeleteAccountFromBuffer got email w/name that's not a valid account %s\n",name);
	else
		PostThreadMessage(main_thread_id,WM_BLAK_MAIN_DELETE_ACCOUNT,0,a->account_id);
	LeaveServerLock();
}

void CreateNewAccountAndCharacter(char *name,char *password)
{
	user_node *u;
	int account_id;
	
	if (ConfigBool(DEBUG_SMTP))
	{
		dprintf("name = %s\n",name);
		dprintf("password = %s\n",password);
	}
	
	if (CreateAccount(name,password,ACCOUNT_NORMAL,&account_id) == False)
	{
		lprintf("CreateNewAccountAndCharacter tried to create account %s which already exists\n",
			name);
		return;
	}
	u = CreateNewUser(account_id,USER_CLASS);
	if (u == NULL)
	{
		eprintf("CreateNewAccountAndCharacter can't find just created user for account %i!\n",
			account_id);
		return;
	}
	lprintf("Created account %i (%s), object %i\n",account_id,name,u->object_id);
}

void SendStringMailToObject(int string_id,int object_id)
{
#ifdef SMTP_TEST
	return;
#else
	
	val_type perm_string_const,ret_val;
	parm_node p[1];
	
	perm_string_const.v.tag = TAG_STRING;
	perm_string_const.v.data = string_id;
	
	p[0].type = CONSTANT;
	p[0].value = perm_string_const.int_val;
	p[0].name_id = PERM_STRING_PARM;
	
	ret_val.int_val = SendTopLevelBlakodMessage(object_id,RECEIVE_INTERNET_MAIL_MSG,1,p);
	
#endif
}

/* GetInternetMailObject
We return a valtype, the tag and data. */
int GetInternetMailObject(char *s)
{
#ifdef SMTP_TEST
	return NIL;
#else
	val_type temp_string_const,object_val;
	parm_node p[1];
	
	SetTempString(s,strlen(s));
	
	temp_string_const.v.tag = TAG_TEMP_STRING;
	temp_string_const.v.data = 0;
	
	p[0].type = CONSTANT;
	p[0].value = temp_string_const.int_val;
	p[0].name_id = NAME_PARM;
	
	object_val.int_val = SendTopLevelBlakodMessage(GetSystemObjectID(),
						  FIND_USER_BY_INTERNET_NAME_MSG,1,p);
	
	return object_val.int_val;
#endif   
	
}

Bool ExistInternetMailName(char *s)
{
#ifdef SMTP_TEST
	return True;
#else
	
	val_type ret_val;
	
	return (strcmp(s,ConfigStr(EMAIL_ACCOUNT_CREATE_NAME)) == 0) ||
		(strcmp(s,ConfigStr(EMAIL_ACCOUNT_DELETE_NAME)) == 0);
	
	/* only account creation right now */
	
	EnterServerLock();
	ret_val.int_val = GetInternetMailObject(s);
	LeaveServerLock();
	
	return (ret_val.v.tag == TAG_OBJECT);
	
#endif   
}

void ClearSMTPBuffers(smtp_node *smtp)
{
	string_list *sl,*temp;
	
	if (smtp->reverse_path != NULL)
	{
		FreeMemory(MALLOC_ID_SMTP,smtp->reverse_path,strlen(smtp->reverse_path)+1);
		smtp->reverse_path = NULL;
	}
	
	sl = smtp->forward_path;
	while (sl != NULL)
	{
		temp = sl->next;
		FreeMemory(MALLOC_ID_SMTP,sl->str,strlen(sl->str)+1);
		FreeMemory(MALLOC_ID_SMTP,sl,sizeof(string_list));
		sl = temp;
	}
	smtp->forward_path = NULL;
	
	sl = smtp->data;
	while (sl != NULL)
	{
		temp = sl->next;
		FreeMemory(MALLOC_ID_SMTP,sl->str,strlen(sl->str)+1);
		FreeMemory(MALLOC_ID_SMTP,sl,sizeof(string_list));
		sl = temp;
	}
	smtp->data = NULL;
}

void __cdecl smtpprintf(smtp_node *smtp,char *fmt,...)
{
	char s[SMTP_MAX_LINE];
	va_list marker;
	
	va_start(marker,fmt);
	vsprintf(s,fmt,marker);
	va_end(marker);
	
	SendSMTPMessage(smtp,s);
}


void SendSMTPMessage(smtp_node *smtp,char *msg)
{
	if (ConfigBool(DEBUG_SMTP))
		dprintf("SMTP reply: %s\n",msg);
	
	SendSMTPBuffer(smtp,msg);
	SendSMTPBuffer(smtp,"\r\n");
}

void SendSMTPBuffer(smtp_node *smtp,char *msg)
{
	int written;
	
	written = send(smtp->sock,msg,strlen(msg),0);
	if (written == SOCKET_ERROR || written < (int)strlen(msg))
	{
		if (written == SOCKET_ERROR)
			eprintf("SendSMTPBuffer got error %i\n",WSAGetLastError());
		smtp->state = SMTP_ERROR;
	}
}

/* GetLocalMachineName
Return value is valid only temporarily.  NOT THREAD SAFE */
char *GetLocalMachineName(void)
{
	static char name[200],full_name[200];
	struct hostent *site_data;
	
	strcpy(full_name,LockConfigStr(EMAIL_LOCAL_MACHINE_NAME));
	UnlockConfigStr();
	return full_name;
	
	if (gethostname(name,sizeof(name)) == SOCKET_ERROR)
		strcpy(name,"localhost");
	
	site_data = NULL;
	site_data = gethostbyname(name);
	
	if (site_data != NULL)
		strcpy(full_name,site_data->h_name);
	else
		strcpy(full_name,"localhost");
	
	return full_name;
}


char *SMTPTimeStr(int time)
{
	struct tm *tm_time;
	static char s[80];
	char *time_format;
	
	if (time == 0)
		return "Invalid Time";
	
	tm_time = localtime((time_t *)&time);
	
	if (tm_time == NULL)
		return "Invalid Time";
	
	time_format = "%d %b %y %H:%M:%S %Z";
	
	if (strftime(s,sizeof(s),time_format,tm_time) == 0)
		return "Time string too long";
	
	return s;
}

