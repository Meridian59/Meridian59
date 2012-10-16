DirectShow Sample -- DMOEnum
----------------------------

This sample demonstrates the use of the DMOEnum, IEnumDMO, and IMediaObject 
interfaces to enumerate and read information about DirectX Media Objects 
installed on your system.  You may view all DMOs or only those that are not 
keyed by checking the "Include Keyed DMOs?" checkbox.

When you select a DMO category, DMOEnum will enumerate all installed DMOs
that relate to the selected category.  When you select a DMO in the list, 
its server filename will be displayed.  The media types and subtypes supported
by the DMO will be displayed in listboxes, and the DMO's preferred input/output
media type, subtype, and media format will be displayed on the right.  The 
input/output check boxes will also display stream information flags available
through IMediaObject::GetInputStreamInfo() (and GetOutputStreamInfo()).


