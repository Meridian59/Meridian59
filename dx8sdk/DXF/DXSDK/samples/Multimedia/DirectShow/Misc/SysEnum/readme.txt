DirectShow Sample -- SysEnum
----------------------------

Demonstrates the use of the system device enumerator for enumerating devices
and DirectShow filters installed on your system.  

By default, the application lists several system device classes by CLSID.  
If you select "Show All Filter Categories", it will emulate the behavior of 
GraphEdit to enumerate all system device classes in the system.

When you select a device class, SysEnum will enumerate all installed filters
that relate to the selected class.  When you select a filter, its filename
and disk location will be displayed.


For more information, see "Enumerating Devices and Filters" in the DirectShow
Tutorials section of the DirectX 8 SDK.

