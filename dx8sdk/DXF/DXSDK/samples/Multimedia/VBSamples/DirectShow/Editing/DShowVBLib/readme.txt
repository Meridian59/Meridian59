VisualBasic Helper Library
--------------------------

This type library is a helper file to allow the user of the IFileSinkFilter 
interface from Visual Basic.

The IFileSinkFilter interface is not in the quartz.dll (ActiveMovie) type library 
because one of the methods has an AM_MEDIA_TYPE argument which prevents it from 
being called from VB.  The DShowVBLib type library redefines the interface with
a different pointer type to allow VB to call IFileSinkFilter::SetFileName. 
A null should be passed in for the pointer argument.
