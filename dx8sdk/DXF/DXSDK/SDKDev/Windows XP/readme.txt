This directory contains an executable and associated files that will allow you to install the DirectX 8.1 debug runtime components for Windows XP.  

If you install the runtime from the SDK and specify either "Retail" or "Debug" for the runtime install, the specified runtime will be installed.  However, if you specified "Retail" at the time of install and you would now like to install the DirectX 8.1 Windows XP debug runtime you can do so with the executable in this folder.  

	"dxpsetup.exe"		will install DirectX 8.1 Windows XP debug runtime components

To revert to the retail DirectX 8.1 runtime components you can use the Windows XP DirectX 8.1 uninstaller.  This is installed to the Windows system folder when the DirectX 8.1 debug runtime components are installed (with dxpsetup.exe).  One easy way to do this is to uninstall (or revert to the Retail runtime) is to:

	Select "Run..." from the Start menu and type "undxxpdebug"

Additionally, from a command line, you can type:

	"undxxpdebug.exe"	to revert to the DirectX 8.1 Windows XP retail runtime components

The Windows XP DirectX 8.1 uninstaller is only available after the DirectX 8.1 Windows XP Debug runtime has been installed.

Also, note that the DirectX 8.1 Retail runtime is the runtime that is shipped natively with Windows XP.  
