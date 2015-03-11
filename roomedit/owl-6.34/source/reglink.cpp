//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/registry.h>
#include <string.h>

namespace owl {

//
/// Constructs a reglink pointing to a reglist, and adds it to the end of the list.
//
TRegLink::TRegLink(TRegList& regList, TRegLink*& head)
:
  Next(0),
  RegList(&regList)
{
  AddLink(&head, this);
}

//
/// Adds a new link to the end of the link list.
//
void TRegLink::AddLink(TRegLink** head, TRegLink* newLink)
{
#if !defined(BI_VIRTUAL_BUG)
  TRegLink** link = head;
  while (*link)                 // put new link at end of list
    link = &(*link)->Next;
  *link = newLink;
#else
  if(*head == 0)
    *head = newLink;
  else{
    TRegLink* link = *head;
    while (link->GetNext() != 0)                 // put new link at end of list
      link = link->GetNext();// = &(*link)->Next;
    link->SetNext(newLink);
  }
#endif

}

//
/// Removes a link from the link list. Returns true if the link is found and removed.
//
bool TRegLink::RemoveLink(TRegLink** head, TRegLink* remLink)
{
#if !defined(BI_VIRTUAL_BUG)
  for (TRegLink** link = head; *link; link = &(*link)->Next){
    if (*link == remLink) {
      *link = (*link)->Next;     // remove from list
      return true;
    }
  }
#else
  if(*head == remLink){
    (*head) = (*head)->GetNext();
    return true;
  }
    
  for( TRegLink* link = *head; link->GetNext(); link = link->GetNext()){
    if(link->GetNext() == remLink){
      link->SetNext(link->GetNext());// remove from list
      return true;
    }
  }
  
#endif
  return false;
}

} // OWL namespace
/* ========================================================================== */
