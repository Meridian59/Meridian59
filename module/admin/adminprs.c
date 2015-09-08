// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * adminprs.c:  Parse server responses to admin commands.
 */

#include "client.h"
#include "admin.h"

extern HWND   hObjectList;          // Dialog controls
extern HWND   hAdminMoveDlg;        // Move object dialog

// Possible server responses we're receiving
enum { ADMIN_NONE, ADMIN_OBJECT, ADMIN_RETURN };

static int response = ADMIN_NONE;   // What kind of result from server are we parsing?

// String that indicates that next word starts or ends a response
static char response_start_string[] = ":<";
static char response_end_string[] = ":>";

// String that signals start of a new line
static char response_line_start[] = ":";

static char whitespace[] = " \t\r\n";

// Table of words that start new responses
typedef struct {
   char *word;     // Word that indicates type of response
   int  response;  // Response type to start when word found
} AdminResponse;

static AdminResponse response_table[] = {
{ "object",      ADMIN_OBJECT, },
{ "return",      ADMIN_RETURN, },
{ NULL,          0, }
};

// Table of words that indicate Blakod types
typedef struct {
   char *word;  // Word that indicates Blakod type
   int type;    // Blakod constant type tag
   int control; // ID of dialog control that indicates this type (0 if none)
} AdminType;

static AdminType type_table[] = {
{ "$",           TAG_NIL,       IDC_NIL,      },
{ "INT",         TAG_INT,       IDC_INT,      },
{ "OBJECT",      TAG_OBJECT,    IDC_OBJECT,   },
{ "LIST",        TAG_LIST,      IDC_LIST,     },
{ "RESOURCE",    TAG_RESOURCE,  IDC_RESOURCE, },
{ "TIMER",       TAG_TIMER,     IDC_TIMER,    },
{ "SESSION",     TAG_SESSION,   0,            },
{ "ROOMDATA",    TAG_ROOM_DATA, 0,            },
{ "STRING",      TAG_STRING,    IDC_STRING,   },
{ "CLASS",       TAG_CLASS,     IDC_CLASS,    },
{ "MESSAGE",     TAG_MESSAGE,   0,            },
{ "HASHTABLE",   TAG_TABLE,     IDC_TABLE,    },
{ NULL,          0,             0,            },
};

typedef union {
   int int_val;
   constant_type v;
} val_type;

// Possible messages that give return values
enum { RETURN_NONE, RETURN_GETROW, RETURN_GETCOL, RETURN_GETANGLE, 
RETURN_GETFINEROW, RETURN_GETFINECOL, };

// Table of messages whose return values we handle
typedef struct {
   char *word;      // String name of message
   int   message;   // RETURN_* id of message
   HWND *window;    // Window to inform when we get this return value (NULL if none)
   int   msg;       // Windows message to send to window when we get return value
} ReturnMessage;

static ReturnMessage return_table[] = {
{ "GetRow",     RETURN_GETROW,     &hAdminMoveDlg,      BK_GOTROW,     },
{ "GetCol",     RETURN_GETCOL,     &hAdminMoveDlg,      BK_GOTCOL,     },
{ "GetAngle",   RETURN_GETANGLE,   &hAdminMoveDlg,      BK_GOTANGLE,   },
{ "GetFineRow", RETURN_GETFINEROW, &hAdminMoveDlg,      BK_GOTFINEROW, },
{ "GetFineCol", RETURN_GETFINECOL, &hAdminMoveDlg,      BK_GOTFINECOL, },
{ NULL,         0,                 NULL,                0,    },
};

static char line[MAX_PROPERTYLEN];  // Buffer used to build up a line of a server response
static int  num_lines;         // # of properties we've gotten for object so far

/* local function prototypes */
static void AdminStartResponse(int type);
static void AdminEndResponse(int type);
static void AdminParseToken(char *token);
static void AdminReturnLine(char *str);
static void AdminAddProperty(char *str);
static char *AdminPropertyValue(char *str, int *type);
static char *AdminParseValue(char *str, int *type);
static char *AdminNthToken(char *str, int n);
/****************************************************************************/
/*
 * AdminNewLine:  Got new text from server; see if we should parse it into 
 *   object information.
 *   str is the new string (assumed to be NULL-terminated).
 */
void AdminNewLine(char *str)
{
   int i;
   char *token;

   for (token = strtok(str, whitespace); token != NULL; token = strtok(NULL, whitespace))
   {
      // See if line starts a new response type
      if (!stricmp(token, response_start_string))
      {
	 line[0] = 0;	 
	 response = ADMIN_NONE;
	 i = 0;
	 token = strtok(NULL, whitespace);
	 if (token == NULL)
	    return;
	 
	 while (response_table[i].word != NULL)
	 {
	    if (!stricmp(token, response_table[i].word))
	       response = response_table[i].response;
	    i++;
	 }
	 AdminStartResponse(response);
      }
      
      // See if line ends response type
      if (!stricmp(token, response_end_string))
      {
	 AdminEndResponse(response);
	 response = ADMIN_NONE;
	 continue;
      }

      AdminParseToken(token);
   }
}
/****************************************************************************/
/*
 * AdminParseToken: We've received a token that doesn't start or end a response.
 */
void AdminParseToken(char *token)
{
   switch (response)
   {
   case ADMIN_OBJECT:
      // If token starts a new property, move to new listbox entry, else save up entry
      if (!stricmp(token, response_line_start))
      {
	 AdminAddProperty(line);
	 line[0] = 0;
      }
      else 
      {
	 if (line[0] != 0)
	    strcat(line, " ");
	 strcat(line, token);
      }

      break;

   case ADMIN_RETURN:
      if (hAdminMoveDlg == NULL)
	 break;

      // If token starts a new line
      if (!stricmp(token, response_line_start))
      {
	 AdminReturnLine(line);
	 line[0] = 0;
      }
      else 
      {
	 if (line[0] != 0)
	    strcat(line, " ");
	 strcat(line, token);
      }

      break;

   }
}
/****************************************************************************/
/*
 * AdminStartResponse:  We've just started a new response of the given type.
 */
void AdminStartResponse(int type)
{
   num_lines = 0;
   switch (type)
   {
   case ADMIN_OBJECT:
      WindowBeginUpdate(hObjectList);
      ListBox_ResetContent(hObjectList);
      break;
   }
}
/****************************************************************************/
/*
 * AdminEndResponse:  We've just received the end of a server response; we
 *   were handling the given response type.
 */
void AdminEndResponse(int type)
{
   switch (type)
   {
   case ADMIN_OBJECT:
      // Add last property of object to list box
      AdminAddProperty(line);
      WindowEndUpdate(hObjectList);
      break;

   case ADMIN_RETURN:
      AdminReturnLine(line);
      break;
   }
}
/****************************************************************************/
/*
 * AdminAddProperty:  Add the given property string to the object list box.
 */
void AdminAddProperty(char *str)
{
   int type, num = 0;
   char *ptr, *prop;

   if (str[0] == 0)
      return;

   // If this starts first property, then line contains "object x is class y"
   if (num_lines == 0)
   {
      // Get object number
      ptr = AdminNthToken(str, 2);
      if (ptr != NULL)
	 num = atoi(ptr);

      // Get object class
      ptr = AdminNthToken(str, 5);
      if (ptr != NULL)
	 AdminDisplayObject(num, ptr);
   }
   else
   {
      // Otherwise add property to list box
      ListBox_AddString(hObjectList, str);

      prop = AdminNthToken(str, 1);

      // If this property gives object's owner, display it
      if (!strnicmp(prop, "poOwner", 7))
      {
	 ptr = AdminPropertyValue(str, &type);
	 if (ptr != NULL)
	    AdminDisplayOwner(atoi(ptr));
      }      
   }
   num_lines++;
}
/****************************************************************************/
/*
 * AdminNthToken:  Return pointer to start of nth token in str, or NULL if none.
 *   First token is numbered 1.
 */
char *AdminNthToken(char *str, int n)
{
   int index;
   char *ptr = str;

   do {
      if (ptr == NULL)
	 return NULL;

      // Skip spaces
      index = strspn(ptr, whitespace);
      str = ptr + index;

      // Move to next token
      ptr = strchr(str, ' ');
      
      n--;
   } while (n > 0);
   return str;
}
/****************************************************************************/
/*
 * AdminPropertyValue:  Parse the value of the property from the given property
 *   description line from the server.  If successful, set type to the type of the
 *   property, and return a pointer to the value.  
 *   Otherwise, return NULL.
 */
char *AdminPropertyValue(char *str, int *type)
{
   char *ptr;

   // "property = type value" -- skip to type
   ptr = AdminNthToken(str, 3);
   if (ptr == NULL)
      return NULL;
   
   return AdminParseValue(ptr, type);
}
/****************************************************************************/
/*
 * AdminParseValue.  Parse the Blakod value in str.
 *   If successful, set type to the type of the value, and return a pointer to the value.  
 *   Otherwise, return NULL.
 */
char *AdminParseValue(char *str, int *type)
{
   int i;
   char *ptr;

   // Get type
   for (i = 0; type_table[i].word != NULL; i++)
      if (!strnicmp(str, type_table[i].word, strlen(type_table[i].word)))
      {
	 *type = type_table[i].type;
	 break;
      }
   
   // Get value
   ptr = strchr(str, ' ');
   if (ptr == NULL)
      return NULL;
   ptr++;

   return ptr;
}
/****************************************************************************/
/*
 * AdminShowProperty:  Show more info on property with given index in the object list box.
 */
void AdminShowProperty(int index)
{
   char *value, line[MAX_PROPERTYLEN];
   int type;

   ListBox_GetText(hObjectList, index, line);

   value = AdminPropertyValue(line, &type);
   if (value == NULL)
      return;
   
   switch (type)
   {
   case TAG_OBJECT:
      sprintf(command, "show object %s", value);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      break;

   case TAG_RESOURCE:
      sprintf(command, "show resource %s", value);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      break;

   case TAG_LIST:
      sprintf(command, "show list %s", value);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      break;

   case TAG_STRING:
      sprintf(command, "show string %s", value);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
      break;

   case TAG_TABLE:
      sprintf(command, "show table %s", value);
      SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM)command);
      break;
   }
}
/****************************************************************************/
/*
 * AdminReturnLine:  Got given line from server while parsing return value.
 *   Requires that move dialog is up.
 */
void AdminReturnLine(char *str)
{
   static int return_message;    // Identifies message that produced this return value
   int type, i;
   char *ptr;
   HWND *wptr;

   if (str[0] == 0)
      return;

   switch (num_lines)
   {
   case 0:
      return_message = RETURN_NONE;
      // First line contains "return from OBJECT x MESSAGE y (msg#)"
      ptr = AdminNthToken(str, 4);
      debug(("AdminReturnLines token 4 is '%s'\n", ptr));

      // Make sure that response is for current object
      if (ptr == NULL || atoi(ptr) != current_obj)
      {
	 response = ADMIN_NONE;
	 break;
      }

      // Get message type
      ptr = AdminNthToken(str, 6);
      if (ptr == NULL)
	 break;
      debug(("AdminReturnLines token 6 is '%s'\n", ptr));

      for (i=0; return_table[i].word != NULL; i++)
	 if (0 == strnicmp(ptr, return_table[i].word, strlen(return_table[i].word)))
	    return_message = return_table[i].message;
      break;

   case 1:
      // Second line contains return value
      ptr = AdminParseValue(line, &type);
      if (ptr == NULL)
	 break;

      // Send message to window, if appropriate
      for (i=0; return_table[i].word != NULL; i++)
      {
	 wptr = return_table[i].window;
	 if (return_message == return_table[i].message && wptr != NULL)
	    if (*wptr != NULL)
	       SendMessage(*wptr, return_table[i].msg, (WPARAM) type, (LPARAM) ptr);
      }
      break;

   default:
      debug(("AdminReturnLines got more lines than expected\n"));
      break;
   }

   num_lines++;
}

/****************************************************************************/
/*
 * AdminValueDialogProc:  Dialog procedure for editing a property's value.
 *   lParam of INITDIALOG message is index of property in object list box.
 */
BOOL CALLBACK AdminValueDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   static int index;
   int type, i;
   char *value, line[MAX_PROPERTYLEN], *str, property[MAX_PROPERTYLEN];
   static HWND hEdit;

   switch (message)
   {
   case WM_INITDIALOG:
      index = lParam;
      hEdit = GetDlgItem(hDlg, IDC_EDITVALUE);
      Edit_LimitText(hEdit, MAX_PROPERTYLEN - 1);

      ListBox_GetText(hObjectList, index, line);

      value = AdminPropertyValue(line, &type);
      if (value != NULL)
      {
	 // Initialize dialog with current value
	 for (i = 0; type_table[i].word != NULL; i++)
	    if (type == type_table[i].type)
	       CheckDlgButton(hDlg, type_table[i].control, 1);

	 Edit_SetText(hEdit, value);
	 Edit_SetSel(hEdit, 0, -1);

	 strcpy(property, AdminNthToken(line, 1));
	 str = strchr(property, ' ');
	 if (str != NULL)
	    *str = 0;
	 SetDlgItemText(hDlg, IDC_PROPERTY, property);
      }

      sprintf(line, "%d", current_obj);
      SetDlgItemText(hDlg, IDC_OBJECTNUM, line);

      CenterWindow(hDlg, GetParent(hDlg));
      SetWindowFont(hEdit, GetFont(FONT_INPUT), TRUE);
      SetFocus(hEdit);
      return FALSE;

   case WM_COMMAND:
      switch (GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
	 // Get type and value
	 str = NULL;
	 for (i = 0; type_table[i].word != NULL; i++)
	    if (IsDlgButtonChecked(hDlg, type_table[i].control))
	    {
	       str = type_table[i].word;
	       break;
	    }
	 if (str != NULL)
	 {
	    Edit_GetText(hEdit, line, MAX_PROPERTYLEN);
	    GetDlgItemText(hDlg, IDC_PROPERTY, property, MAX_PROPERTYLEN);
	    
	    // Special case for NIL:  value must be zero
	    if (type_table[i].type == TAG_NIL)
	       strcpy(line, "0");
	    
	    sprintf(command, "set object %d %s %s %s", current_obj, property, str, line);
	    
	    SendMessage(hAdminDlg, BK_SENDCMD, 0, (LPARAM) command);
	    
	    // Change value in object list box
	    sprintf(command, "%s = %s %s", property, str, line);
	    WindowBeginUpdate(hObjectList);
	    ListBox_DeleteString(hObjectList, index);
	    ListBox_InsertString(hObjectList, index, command);
	    WindowEndUpdate(hObjectList);
	 }
	    
	 EndDialog(hDlg, IDOK);
	 return TRUE;

      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
      break;
   }

   return FALSE;
}
