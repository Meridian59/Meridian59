Microsoft(R) DirectX (R) 8.1 SDK - Final Release

This is the Final Release of Microsoft's DirectX 8.1 Software Development
Kit (SDK).  It contains all DirectX software required to create
DirectX 8.1 compliant applications.  Following the Final Release,
parts of this SDK will also be made available for download at the
Microsoft DirectX developer webpage <http://msdn.microsoft.com/directx>.

Major components include:  DirectX 8.1 system components (runtimes); 
DirectX 8.1 Application Programming Interface (API) Documentation 
for each of the DirectX core areas (written for C++ or Visual Basic 
developers);  DirectX 8.1 headers & libraries;  Sample applications 
and source code;  miscellaneous tools and utilities.

After installing, those new to DirectX should start with the 
DirectX 8.1 documentation.  More seasoned developers may also 
want to view the "WHAT'S NEW" section.  Professional DirectX 
developers should refer to the "KNOWN ISSUES" section prior to 
raising concerns


__________________________________________________________________



Contents:

- INSTALLATION
- WHAT'S NEW
- CD LAYOUT
- DIRECTX 8.1 SDK RELEASE NOTES AND KNOWN ISSUES 
- COMPILER SUPPORT


__________________________________________________________________



INSTALLATION:

- ALWAYS UNINSTALL PREVIOUS RELEASES of the DirectX SDK prior to 
  installing DirectX 8.1 SDK Release (see UNINSTALL).  Installing 
  on top of a previous installation will cause you problems. 

- In addition, you should uninstall any DirectX SDK installations 
  that may be part of a Microsoft Platform SDK installation prior 
  to installing the DirectX 8.1 SDK.

- DirectX 8.1 SDK does not support pre-release versions of Windows XP, 
  Windows 95, Windows NT, or NEC PC98 systems. Installations on
  these systems have been blocked.

- On Windows 2000 or Windows XP, You must be logged on as an 
  administrator before installing the DirectX 8.1 SDK, installing 
  the runtime, using any of SDK tools, or running any of the SDK samples.

- Running the DirectX 8.1 SDK install on Windows XP and later checked 
  builds may result in assertions from inside the install package. 
  We have found no cases where continuing on past the assert dialog 
  boxes results in improper installation of the DirectX 8.1 SDK, so 
  it is safe to continue past these asserts.

- An Internal error can occur after the NTFS permissions for the 
  Administrators group or the System account have been removed from 
  the %SystemRoot%\Installer folder.  To resolve this issue, reset 
  the NTFS permissions for the %SystemRoot%\Installer folder so 
  that it includes both the Administrators group and the System 
  account. Search the Microsoft knowledge base article for more 
  information.

- Please be alerted to the fact that using the "subst" command can 
  result in path names that are too long for the installer to operate 
  correctly. Please do not use this command for determining target drives 
  of the DirectX SDK installer prior to invoking the installer. This is 
  also liable to cause the SDK uninstaller to fail.

- The DirectX 8.1 runtime installer should not be run in Compatibility Mode.  

- The DirectX 8.1 runtime requires a Pentium compatible CPU for operation. 
  Setup will not install on processors that do not report Pentium 
  compatibility.  Some older Cyrix processors, while compatible with 
  the Pentium instruction set, report as x486 processors, and thus do not 
  support installation of the DirectX 8.1 runtime.

- When installing the DirectX 8.1 developer runtime from the DirectX 8.1 SDK 
  you will notice that the size requirements are calculated incorrectly. 
  Please allow an extra 80 MB free above the calculated disk space when installing 
  the developer runtimes.  The extra free space is required for uncompression 
  and installation of the developer runtimes.  Only 40 MB is actually 
  needed once the developer runtime installation is complete.  The DirectX 
  Setup API will check for and display error messages when these requirements 
  are not met.


UNINSTALL:

To uninstall, use "Add/Remove Programs" from the Control Panel to 
uninstall the InstallShield installations.



__________________________________________________________________



WHAT'S NEW:
	
DirectX 8.1 shipped as an integral part of Windows XP and is available 
for all versions of Windows 98, Windows ME and Windows 2000.  Developers 
choosing to support Windows 95 should continue to use DirectX 8.0a. 

DirectX 8.1 includes all of the previous DirectX 8.0 functionality as 
well as new DirectX graphics features and new DirectShow features specific to 
Windows XP.

DirectX 8.1 graphics supports the following new features:

	- new pixel shader models 1.2, 1.3, and 1.4
	- new high precision texture formats
	- new nPatch quadratic interpolation order
	- new dynamic texture support
	- new D3DX methods 
	- updated DirectX graphics documentation

There have been considerable improvements made to the DirectShow content
for the DirectX 8.1 SDK.  These improvements include updating existing samples, 
adding many new samples (including Windows XP samples), providing modified 
source code for the sampleGrabber, and improving documentation.  See the 
DirectShow section below for more specific information.

The SDK contains several new tools, they include:
	- screensaver framework
	- AppWizard support
	- DX error lookup tool,
	- MView mesh viewer
	- DP8sim network simulator

This DirectX 8.1 version of DirectMusic Producer sports a number of new features to streamline the authoring process for composers and sound designers.  Some of these features include low enough software synthesizer latency to input musical performances from a keyboard in real time (WDM driver required), easier wave capture, and improved DLS/wave editing features like one-click stereo wave insert, insert silence and a resample wave option.   Segment Designer features include multiple wave insert as segments, the ability to edit segment length based on clocktime, and ability to start and stop segments while editing DMO effects.   Other improvements include many bug fixes, an improved look to segment strips and customizable wallpaper.   Documentation has been thoroughly revamped and a new tutorial with a sample game is also available. 


__________________________________________________________________



CD LAYOUT:


The following is a brief description of the directories found with a complete
DirectX 8.1 SDK download.  Depending on options specified during
installation, some of these directories can be installed on your hard drive.

\Bin
    \DXUtils
    High level DirectX applications & tools.
    All can be accessed from the Start menu if "Utilities" are installed. 

    Note: Japanese systems will require Internet Explorer 5.0 (IE5) for
    the DirectX Control Panel to display properly.  IE5 can be found at
    http://www.microsoft.com/windows/ie/default.htm.
    
    \AppWizard
      - This is the DirectX 8.1 application Wizard that can be plugged
        into Microsoft Visual C++ 6.0.  The AppWizard creates a minimal C++
        template application that optionally integrates Direct3D, 
        DirectInput, DirectMusic, DirectSound, and DirectPlay to work 
        together in conjunction.  It creates basic-- yet easy to build 
        upon-- functionality, demonstrating the use of each of these 
        components right from the start.
  
        To install:
           1. Go to \Bin\DXutils\AppWizard\ directory.
           2. copy DxAppWiz.awx to C:\Program Files\Microsoft 
              Visual Studio\Common\MSDev98\Template
        To use:
           1. run VC6
           2. Click "New", under the "New Projects" tab, click the "DirectX
              AppWizard" wizard.
           3. Type a project name and click OK.
           4. Choose wizard options, and click finish when done.
           5. Build and run the new project. 
     
\Doc
    Contains reference documentation for the DirectX 8.1 APIs.  
    This Documentation must be viewed with Windows HTMLHelp.  

    Note:  The HTMLHelp viewer requires a minimum installation of 
    Internet Explorer 5.0 (IE5).  IE5 can be found at
    http://www.microsoft.com/windows/ie/default.htm.

\Essentls
    \DMusProd (DirectMusic Producer)
       - DirectMusic Producer is the authoring tool for DirectMusic.  It 
         allows composers and sound designers to use the interactive and 
         variable resources of DirectMusic along with the consistent sound 
         performance of DLS.  DirectX Audio Scripting can also be authored 
         and tested within Producer.  The DirectMusic Producer setup program 
         and all files are located here.
 
\Extras
   \Direct3D
      - Skinning exporter tools
   \DirectShow
      - Contains the DVDBoilerplate.  See readme.txt within the directory 
        for more information.
      - Contains additional media that can be used with DirectShow and the
        DirectShow Editing Services (DES) interfaces.
    \Documentation
      - Contains DirectX 7 HTMLHelp Documentation for English and Japanese.
      - Also contains DirectX 7 Documentation in Microsoft Word format
        (English only).
      - Contains DirectX 8 HTMLHelp Documentation for Japanese.
   \Symbols
      - Contains directories of DirectX 8.1 symbol files for 
        Win9x, Win2000 and WinXP (retail and debug for each).
\Include
    contains DirectX 8.1 include files for DirectX core components.

\Lib
    contains DirectX 8.1 library files for DirectX core components.

\License
    Text versions of the DirectX SDK and End User License Agreements and 
    the Redistributable License Agreement.

\Redist
    Redistributable versions of the DirectX 8.1 Runtime.  

\Samples
    Contains all sample code and sample binaries.   Most samples can be
    accessed from the Start menu when installed via the downloaded 
    InstallShield setup.

\SDKDev
    This directory contains the runtime installs that are installed with 
    the SDK.  They are English only and contain both debug and retail 
    DirectX 8.1 system components that can be "switched" between retail 
    and debug without re-installing.  For the DirectX 8.1 components, 
    retail or debug can be switched via the DirectX Control Panel.

    The \retail folder contains the setup that will install non-DirectX 8.1
    retail components while the \debug folder will install all debug 
    components.  Both install DX8.1 retail and debug components; These are 
    the only components that can be "switched".  The \window XP folder 
    contains the Windows XP version of these files.

\Suppport
    This folder contains support tools required for the SDK installation.
    It can be deleted following installation.

\System32
    This folder contains support tools required for the SDK installation.
    It can be deleted following installation.


__________________________________________________________________



=========================================
DIRECTX 8.1 SDK NOTES
=========================================  
New features:

Direct3D SDK graphics support includes
	-Added new Direct3D samples, 
		(cull, lighting, volume fog, self-shadowing bumpmapping ) 

	-enhanced usage of D3DX in the samples,

	-continued improvement of the D3Dx documentations,

	-the SDK now contains a graphics screensaver framework
	
	-MView mesh utility, useful for previewing meshes, normals, etc.

- The SDK screensaver framework is modeled after the graphics sample framework. The
  screeensaver sample framework provides multi-monitor support; something the standard
  graphics sample framework does not.  There is one simple  screensaver sample 
  provided, "Moire", that uses the screensaver framework.  The DirectPlay Maze Client
  also uses this screensaver framework, but is not intended as a screensaver framework
  sample.  You can find the screensaver framework with the common sample classes 
  (d3dsaver.cpp & d3dsaver.h), the Moire screensaver sample with the Direct3D samples
  and the Maze Client with the DirectPlay samples.

- DirectX AppWizard for Visual C++ v6.0.  A DirectX AppWizard is included in the 
  DirectX 8.1 release.  See "What's New" or "CD Layout"-"Extras"-"AppWizard"
  for more information

- DirectX error lookup tool providing error lookup for Directx 8.x interfaces only. There is
  also an error lookup function you may use in your application.

Issues:


- Be sure to set the paths to the DirectX 8.1 lib and headers within Visual
  Studio.  They must be listed first.  Otherwise you may experience compiler
  errors.

- You must regserve the VB OCX before running the VB samples if you install the  
  DirectX 8.1 SDK before Visual Studio.  The easiest way is to open visual Studio, 
  which registers the ocx for you.

- The DirectDraw "Debug Level" slider in DirectX Control Panel Applet does 
  not function.   Instead use the "Debug Output Level" slider on the Direct3D 
  tab, this will effect both DDraw and D3D8.   Also note that DirectDraw labels
  it's debug spew as coming from "Direct3D8".

- Netmeeting and WindowsWP remoting disables graphic hardware acceleration.


=========================================
Documentation
=========================================
Issues:

- The links to the DirectX 8.1 documentation files from the start menu do not
  work on Windows 98 SE.  On all supported versions of Windows proper icons
  are not displayed for the start mention documentation links.  

- For proper viewing of the DirectX Documentation, install the docs from
  the DirectX 8.1 SDK CD.  Specify a full installation, or if you chose a 
  Custom install, ensure that you've selected Documentation.  

- The known issues with the DirectX 8.1 Documentation are written up in a
  separate file.  You will find these issues listed in the 
  DirectX8_Doc_Readme.txt file found in the <DXSDKCDRoot>\dxf\dxsdk\doc\DirectX8 
  folder.  This file is also installed into the <DXSDKInstallFolder>\doc\DirectX8
  when Documentation is installed.

- See <DXDKCDRoot>\dxf\dxsdk\extras\documentation for other related 
  documentation from previous versions of DirectX.  The Japanese version of
  DirectX 8.0 help files have been added.


=========================================
DirectX for Visual Basic Samples:
=========================================
Issues:

- DBMON.exe is a console application which allows the Visual Basic DirectX 
  developer to take advantage of extended output that debug installs of 
  DirectX provide. This application works only under Windows 2000. 
  To use, execute the application. All debug output will be captured and sent 
  to the console window. If you are not seeing debug output, ensure that 
  you have installed the debug version of DirectX, that you are logged on to an 
  account with admin priviledges, and that you are running Windows 2000.

- If you are experiencing problems searching for and connecting to sessions 
  using DirectPlay for VisualBasic, you may need to update your system.  
  To update, install DCOM98 from:
      http://download.microsoft.com/msdownload/dcom/98/x86/en/dcom98.exe 


=========================================
DirectX Audio (DirectSound & DirectMusic)
========================================= 
Issues:

- ParamEQ and Distortion effects have parameters that are sampling rate 
  dependent.
  Setting these params to a value that is out of range for that sampling 
  rate will result in:
     Distortion:  Parameters are 'clipped' at the valid range for the 
                  sampling rate
     ParamEQ:  If the frequency is greater than can be supported by the 
               sampling rate, the effect is disabled-  In the DirectMusic 
               3DAudio sample, there may be some breakup if Doppler is set > 4
- DirectSound buffers that are created with DSBCAPS_CTRLFX must be at 
  least 100ms in length.


=========================================
Direct3D
=========================================
New features:

- new pixel shader models 1.2, 1.3, and 1.4
- new high precision texture formats
- new nPatch quadratic interpolation order
- new dynamic texture support
- new D3DX methods 

New for DirectX 8.1 are pixel shader versions 1.2, 1.3, and 1.4. Information 
on the new instructions is integrated into the pixel shader reference.

Dynamic textures are a new Microsoft® DirectX® 8.1 feature. To find out if 
the driver supports dynamic textures, check the D3DCAPS2_DYNAMICTEXTURES 
flag of the D3DCAPS8 structure. See SDK Documentation section 
"Using Dynamic Textures".

Higher precision texture formats.

New npatch quadratic interpolation order.

In D3DX there have been a number of updates.  See the D3DX section below.

New exporter support, in the extras folder. Includes 3DSmax4 and Maya4 support.

General Issues include:

- Alt-enter in the samples results in texture corruption for some drivers. 
  Workaround: Get newer drivers.
- The value for MaxPrimitiveCount defines the max number of vertices
  as well as the max number of primitives that can be sent to a 
  DrawIndexedPrimitive or DrawPrimitive call.
- A bug in the Windows2000 kernel causes an issue upon mode-switches for 
  AGP surfaces locked with the NOSYSLOCK flag. The contents of any such 
  surface may  become inaccessible for the duration of the NOSYSLOCK lock 
  if a mode-switch happens after the Lock and before the Unlock. This happens 
  for both DX7 and DX8 applications. The most likely scenario is a DX8 
  application running on a driver that supports hardware vertex buffers, 
  but using software vertex processing (fixed or shader). Under these 
  scenarios, Direct3D might keep a NOSYSLOCK lock on an AGP vertex buffer. 
  If the user alt-tabs out of the application or does anything else to cause 
  a mode-change, the thread writing into the vertex buffer may fault. There 
  is no known workaround. 
- When using the function ValidateDevice on Win9x the return value will be 
  DD_OK if it is successful.

Point Sprite issues:  

- When using the Direct3D software T&L pipeline (device created with 
  D3DCREATE_SOFTWARE_VERTEXPROCESSING flag set or with 
  D3DCREATE_MIXED_VERTEXPROCESSING flag and software vertex processing 
  toggled on), then point sprite support is guaranteed.  When using 
  hardware vertex processing, some older drivers may not support point 
  sprites, indicated by d3dcaps->MaxPointSize = 0.  Upgrading to a DX8 
  driver for these parts will enable them.

    To ensure consistent behavior for point sprite support, set
	D3DRS_CULLMODE = D3DCULL_NONE 
	D3DRS_FILLMODE = D3DFILL_SOLID
    when rendering point sprites.

Vertex Shader issues:  

- Two vertex shader instructions may produce implementation-specific 
  results for out-of-range inputs, such as the log of a negative number.
  The log, logp and y- component of the lit instruction macro should not
  be passed negative values for this reason.

Driver/Device Specific Issues:

- The Windows Millennium driver for the ATI Rage 128 fails (draws nothing) 
  when using vertex shaders which output more than two texture coordinates 
  per texture coordinate set when the D3DTTFF_PROJECTED bit is not set.


Direct3D Sample issues:


- There are miscellaneous issues with some display hardware & drivers.
  Please let us know of any problems you encounter and specify whether you
  believe it is sample or driver related.


=========================================
D3DX
=========================================
New features:

-D3DX math function CPU-specific optimizations, 
-new methods on PMesh, 
-loading volumes and cube textures from resources, 
-new functions, including 
	-D3DXMatrixMultiplyTranspose(), 
	-D3DXQuaternionSquadSetup(), 
	-D3DXSplitMesh(),
	-D3DXIntersectTri(),
	-D3DXComputeNormalMap(). 

Issues:

- This product includes software developed by the University of California,
  Berkeley and its contributors D3DX Mesh APIs.

- SetLOD and/or UpdateTextures (when updating between dissimilar miplevels) 
  will produce an AV with mipvolumes in Refrast and potentially some drivers.

- If you create vertex shaders with constants in the declarator, please verify 
  that every vertex shader that has the shader constants in the declaration is 
  created with the correct usage flag. Always use:
	D3DUSAGE_SOFTWAREPROCESSING for software vertex processing, 
	0 for hardware vertex processing.  
   	
  Failure to do so may result in your vertex shaders not validating on the 
  DirectX 8.1 runtime. This behavior will only be different from DirectX 8.0 
  on devices which are created with D3DCREATE_SOFTWARE_VERTEXPROCESSING flag.  

=========================================
DirectInput
=========================================
New features:

-Action Mapper sample - a single player version of multimapper.

Issues:

- There are some know issues that can cause mouse behavior to be very sluggish 
  when debugging using DirectInput on Windows 2000 (Windows XP does not 
  exhibit this behavior at all).  The DirectX control panel has options to 
  select input device emulation for mouse and keyboard which makes the devices 
  perform properly inside the debugger.   
  These settings are global so effect all DirectInput clients.  They only 
  improve performance when using the debug binaries, and only for applications 
  that employ foreground exclusive device access.  Some features, such as 
  masking the Windows keys, are not available when devices are emulated so 
  emulation is only recommended for debugging.  

- On Windows 98, 98 Second Edition and Millennium Edition, the Gravis GrIP gamepad may not report data   after a reboot.  
  To fix this you need to run the Game Controller tool from the Control Panel and click Refresh.

  If the Game Controllers tool is missing from Control Panel, you need to restore a backup copy of the   Joy.cpl file. To do this:

  1.	Close the Control Panel, if it is open.
  2.	Click Start, point to Find, and then click Files Or Folders.
  3.	In the Named box, type joy.bak.
  4.	In the Look In box, click My Computer, and then click Find Now.
  5.	In the list of found files, right-click Joy.bak, and then click Rename.
  6.	Type joy.cpl, and then press ENTER.
  7.	Close the Find: Files Named Joy.bak window.
  8.	Open the Control Panel

- The following devices are supported with device images in this release.    
      + ACT LABS RS
      + ACT LABS Force RS
      + Gravis Xterminator Dual Control      
      + Gravis Gamepad Pro
      + Gravis Eliminator Gamepad Pro
      + Gravis Xterminator Digital GamePad
      + Guillemot Dual Analog Gamepad
      + Guillemot Digital Gamepad
      + InterAct/3dfx SV-262A HammerHead Fx
      + InterAct Axispad
      + InterAct SV-251 Raider Digital
      + Logitech WingMan Formula (Yellow)
      + Logitech WingMan Extreme Digital 3D
      + Logitech WingMan Gamepad Black (USB)
      + Logitech Wingman Rally Force USB
      + Logitech Wingman Rumblepad
      + Microsoft Sidewinder Gamepad Pro
      + Microsoft SideWinder Force Feedback Wheel
      + Microsoft SideWinder Force Feedback Pro
      + Microsoft Sidewinder FF 2
      + Microsoft Sidewinder Dual Strike
      + Microsoft Sidewinder Precision 2
      + Microsoft Sidewinder Plug and Play
      + Microsoft SideWinder Game Pad
      + Microsoft SideWinder Precision Racing Wheel
      + Microsoft SideWinder Precision Pro
      + Microsoft SideWinder Freestyle Pro
      + Microsoft SideWinder 3D Pro


=========================================
DirectPlay
=========================================
New features:

- You can test your application under a variety of network conditions 
  with the new DP8Sim tool.
- SDK documentation now contains DirectPlay tutorials

Issues:

- To use DirectPlay Voice in full duplex mode, we recommend that you update
  to the latest drivers for your audio device. Some older drivers do not
  provide reliable full duplex performance.
- If you successfully connect to a location with any asynchronous 
  enumerations still active, the enumerations will be canceled automatically
  for you.  The DPN_MSGID_ASYNC_OP_COMPLETE messages for the enumerations 
  will list DPNERR_CONNECTING as the result code.  As with all asynchronous 
  operations, if you attempt to call CancelAsyncOperation on the handle 
  after you have received its DPN_MSGID_ASYNC_OP_COMPLETE message, you 
  will get back the DPNERR_INVALIDHANDLE error code.  Be aware that this 
  is the expected behavior and that you should not exit with an error 
  condition.

Notes on NATs (Network Address Translators):

- There are many different NAT solutions available, some of which are
  different from others. We have attempted to document some of the issues
  with various NAT solutions, but is not comprehensive.  Please refer to 
  the DirectX Developer Page at <http://msdn.microsoft.com/directx> for 
  further information on these issues.
- When performing the first operation over the TCP/IP service provider, 
  DirectPlay attempts to detect a Network Address Translator (NAT) or
  firewall. This increases the time that the Connect, Host, or EnumHosts
  calls take. If you are sure that your machine is not behind a NAT or
  firewall, you can set the following registry key values to 0 to disable
  the detection:

  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\DirectPlayNATHelp\DPNHPAST\DirectPlay8Priority

  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\DirectPlayNATHelp\DPNHUPnP\DirectPlay8Priority

  As with all registry modifications, only perform them if you are comfortable 
  doing so, and have a back up.

  DirectPlay Voice codecs via Windows ACM (Audio Compression Manager):

- DirectPlay Voice uses the following codecs via the Windows ACM:
   - Microsoft ADPCM
   - Microsoft GSM
   - DSP Group Truespeech

- Installation of these codecs with Windows is optional. If they 
  are not installed, then one or more of the compression types in 
  DirectPlay Voice will not function. If the application attempts 
  to use a compression type that is not functional due to a missing 
  ACM codec, it will receive the DVERR_COMPRESSIONNOTSUPPORTED 
  return code.

- On Windows 2000, the user can install these codecs through the 
  Add/Remove Hardware wizard. The user should select 
  "Add/Troubleshoot a device". When a list of hardware is presented, 
  the user should select "Add a new device", then on the next 
  page select "No, I want to select the hardware from a list". 
  This will bring up a list of hardware. Select "Sound, video, and 
  game controllers". The "Audio Codecs" item is listed under 
  "(Standard system devices)". 

- On Windows 9x, the user can install these codecs through the 
  Add New Hardware wizard. When prompted, the user should select 
  "No, I want to select the hardware from a list". In the list of 
  device types, codecs are listed under "Sound video and game 
  controllers". The codecs are listed under two manufacturers. 
  The ADPCM and GSM codecs are listed under "Microsoft Audio 
  Codecs" and the Truespeech codec is listed under "DSP Group".


=========================================
Broadcast Driver Architecture
=========================================

Known issues with Broadcast IP Data Reception under Windows 2000:

- After rebooting a Windows 2000 system with a BDA receiver card installed,
  the BDA NDIS IP NIC is not immediately active.  It becomes active only 
  when an application inserts the BDA IPSink filter into a graph and connects
  its input pin.  Once activated by connecting the IPSink filter's input pin,
  it takes 20 to 30 seconds for the BDA NDIS IP NIC to be assigned an IP 
  address.  The address assigned will be a Net 10 address.  Once activated, 
  the BDA NDIS IP NIC will remain active until the system is rebooted.

- There may be some applications that use the BDA NDIS IP NIC as the default 
  LAN adapter.  These apps will fail if the BDA NDIS IP NIC is active.  To 
  de-activate the BDA NDIS IP NIC, reboot the system and do not run a TV graph 
  or application.

- When viewing a BDA Network provider property page with GraphEdit, you cannot 
  enter a -1 value.  As a workaround, enter 0 and click the down arrow to set 
  the value to -1.

- Pin connections with mpe/slip with ipsink may fail if NO tuner is 
  installed post - DX8.1 installation.  This behavior is exactly as seen 
  in Windows XP.  This may be due to the TV tuner driver not being 
  properly installed. Automatic loading of BDA NDIS miniport is restricted 
  by design. The solution is to right click and install the bda.inf, followed by
  a machine reboot.


=========================================
DirectShow
=========================================

What's New in DirectShow for DirectX 8.1: 
There have been considerable improvements made to the DirectShow content
for the DirectX 8.1 SDK.  Below is a brief summary of the updates.

Improvements to existing samples
-------------------------------------
There have been many enhancements and refinements to the existing
DirectX 8.1 samples for this release, including:

- New features and functionality
- Numerous bug fixes, including Windows XP support issues
- UNICODE and IA64 support
- ASF-enabled versions of the PlayWnd and MFC Jukebox samples
- Clean compilation with Warning Level 4
- Improved video window repainting for Jukebox-style applications
- Updated resource files and version information blocks
- Digital Video sample (Capture\DVApp) was rewritten to take 
  advantage of the ICaptureGraphBuilder2 interface to build its graphs.
- Sample XTL files have been updated in 8.1 to reflect a change in the
  DES XML parser on Windows XP.  XML attributes must now be specified
  in *lowercase*.  Mixed-case attributes will fail on Windows XP.

New DirectShow samples
-------------------------------------
- Audio Capture         (Capture\AudioCap)
- DMO-enabled Player    (Players\PlayDMO)
- DMO Enumerator        (Misc\DMOEnum)
- Filter Mapper         (Misc\Mapper)
- Grabber Sample Filter (Filters\Grabber) (modified from DX8 SampleGrabber)
- JukeboxASF            (Players\JukeboxASF)
- PlayCap with Moniker  (Capture\PlayCapMoniker)
- PlayWndASF            (Players\PlayWndASF)
- Still Image Viewer    (Players\StillView)
- Utility source code and routines in the DirectShow\Common directory

New DirectShow samples for Windows XP
-------------------------------------
Windows XP provides new functionality that is not present in downlevel
operating systems (Windows 9x, Windows 2000).  The DirectShow_WinXP
directory provides samples to demonstrate using the new Video Mixing
Renderer and the new Microsoft Video Control (for use with the Windows
Driver Model and Broadcast Driver Architecture).

The new samples include:

- Video Control - C++, Visual Basic, and HTML samples of rendering
  ATSC, DVB, and Analog television in a Windows application.  The
  new Video Control encapsulates much of the new functionality and
  aids developers in creating television-enabled applications.

- Video Mixing Renderer (VMR)
  - Cube
  - Renderless
  - Text Player
  - VMR Mix
  - VMR Player
  - VMR Exclusive Mode (VMRXcl)

See the DirectShow_WinXP directory for more detailed information 
about these new samples.


Updated Documentation
-------------------------------------
The DirectShow documentation has been significantly enhanced for 
DirectX 8.1.  It adds information for the new features available in
Windows XP and expands on previously presented material for the
Microsoft TV Technologies.


Modified SampleGrabber filter source code
-------------------------------------
There have been several requests for Microsoft to publish the source
code for the popular SampleGrabber filter.  To that end, we now provide
a modified (simpler) version of the sample grabber filter, which uses 
a new GUID and new CLSID to prevent collisions with the original 
DirectX 8.0 filter.  You can review and modify the source code for this 
filter and use it in your own applications.  See the Filters\Grabber 
directory for more information.


Windows XP support in header files
-------------------------------------
DirectX 8.1 provides support for the following Windows platforms:
    - Windows 98, Windows 98 Second Edition, Windows Millennium Edition
    - Windows 2000
    - Windows XP

Because Windows XP adds new functionality which is not present on 
downlevel systems, many of the headers in the \DXSDK\include directory 
have been revised to allow developers to use new Windows XP functionality 
when desired, with full downlevel compatibility.  This is achieved by 
adding the following conditionally compiled sections to the relevant 
header files:

    #if (WINVER >= 0x501)
        // Windows XP content
    #else
        // Downlevel content
    #endif

To enable Windows XP features and to utilize the new portions of the
affected headers, set the Windows version to 0x501 (the Windows XP
version) in your Visual C++ project files.  If the Windows version is 
less than 0x501, the original DirectX 8.0 content will be used to allow
full backward compatibility with the DirectX 8.0 SDK.  Similar
improvements were also made to the Interface Definition Files (IDLs)
in the \DXSDK\include\DShowIDL directory.

See the Samples\DirectShow and Samples\DirectShow_WinXP directories
for more information.


Runtime improvements
-------------------------------------
The DirectX 8.1 binaries and redist files have received various improvements
since the DirectX 8.0 release, including a large subset of the modifications
that were made for Windows XP.


Revised GraphEdit application
-------------------------------------
The GraphEdit utility benefits from several improvements, particularly 
with respect to its user interface, menus, and toolbar.  Additionally,

- Graph resizing code (View->xxx %) handles all sizes correctly, and 
  you can resize your graphs using the menu, keyboard +/- keys, and 
  Ctrl+MouseWheel.  

- You can optionally disable the seek bar (and its timed updates), which 
  has been known to cause difficulty when debugging filters with GraphEdit.  

- To preserve screen real estate, source and file writer filters displayed 
  in GraphEdit will now only display their corresponding filenames as the 
  filter name, instead of displaying the fully qualified path name.  
  For example, a file source for the file 
  "C:\DXSDK\samples\Multimedia\Media\lake.mpg" would display "lake.mpg".


DirectShow Samples:
-------------------------------------
- DirectX 8.1 contains several new DirectShow samples and many updates
  to existing samples.  New C++ samples include:
  - Capture\AudioCap, Capture\PlayCapMoniker
  - Misc\DMOEnum, Misc\Mapper
  - Players\JukeboxASF, Players\PlayWndASF, Players\PlayDMO, Players\StillView

- In order to launch GraphEdit from the StillView or Jukebox applications, 
  GraphEdt.exe must be installed with the DirectX SDK or exist on your path

- There are several .XTL sample files in the \media folder.  These files 
  demonstrate some of the functionality available in the DirectShow Editing 
  Services.  You can render these files using the XTLTest C++ sample and 
  the XTLTestVB and DexterVB VisualBasic samples.  All provided XTL files 
  refer to media that ships in the SDK's media directory.  Rendering XTL 
  files is a great way to get an idea of what the new DirectShow Editing 
  Services APIs make possible.  For a simple demo, drag a few of the SDK 
  media directory's sample .XTL files onto the XTLTestVB sample's client area.

- The DirectShow Visual Basic samples demonstrate more of the DES functionality 
  than the C++ samples.  Be sure to run SlideShowVB and TrimmerVB if you are 
  interested in DES.  You can also use the DexterVB ("Dexter Explorer") 
  application as an XTL development/test tool.

- The Texture3D sample is known to fail on some systems (referencing
  invalid memory), for which you may see an error message.  We recommend 
  updating your video card drivers to the most current revision, as that 
  usually resolves the problem.

- The DirectShow IDL files are currently located in the
  \include\DShowIDL directory of the SDK for your convenience. 

- You will be required to build the \BaseClasses libraries before any sample
  that depends on them can be built or executed.  Please review the Readme.Txt 
  file in the DirectShow samples root directory for more information on 
  building the samples.

- If you select the Debug runtime when installing the SDK, then the video renderer 
  will display frame counters on DirectShow video windows.  These frame counters 
  will appear on all DirectShow applications, whether built as Debug or Release versions.  
  To disable the frame counters, switch to DirectX Release binaries by running "Install Retail"
  from the DirectX SDK's "Runtime Installs" Start Menu item.  Note that you must have 
  enabled the capability to switch between Debug and Release binaries when you installed
  the SDK, by checking the "Copy DirectX Setup Files" option on the install options page.
  This is not an issue on Windows XP, however, because the Video Mixing Renderer does
  not display these frame counters by default.



DirectShow 8.0 release notes:
-------------------------------------

- Application development using the DirectShow base classes libraries requires
  installation of the DirectShow samples.  The source code for the base classes
  is now included as a sample project, rather than as a separate Classes\Base 
  directory.  You must compile these base classes to generate strmbase.lib and 
  strmbasd.lib, which are no longer installed as binary files.   If you are
  linking with strmbase.lib or strmbasd.lib with your own application, be sure
  to build and link with the updated DirectX 8 libraries (instead of the older
  libraries that are may be installed with Visual C++ or the Microsoft Platform SDK).

- The samples included with the SDK are configured to compile and link with 
  the updated libraries, which are generated by building the BaseClasses project.  
  Further build instructions are provided in:
	  <DXSDKInstallFolder>\samples\Multimedia\DirectShow\readme.txt.

- The <streams.h> header file, along with many other DirectShow-specific header
  files, is now located in the DirectShow samples BaseClasses directory, 
  which includes the BaseClasses source code and header files.  You must add 
  the BaseClasses directory to the beginning of your project's include path 
  settings in order for Visual C++ to find all of the updated header files.

- To use the Teletext Decoder Fonts that are shipped with DX8, make sure that
  the WST_* fonts are installed by examining the \windows\fonts directory.

- There is a known problem with the MP3 decoder filter when used with
  Audio Effects DMOs.  If you add an audio effect DMO (like WavesReverb, Echo,
  or Flange) to an existing MP3 filter graph, the audio could be played back
  at double speed.  The problem would occur whenever the MP3 decoder is connected 
  to a downstream audio effect dmo (via the dmo filter wrapper) that offers a 
  default audio input type which doesn't match the native format of the 
  uncompressed MP3 stream.  For example, if you render a 22kHz MP3 file and 
  add a DMO Audio effect, the MP3 filter could misinterpret the input type as
  being 44kHz when it is actually 22kHz.  This would not be a problem for 44kHz 
  MP3 files.  This issue will be fixed in a future release.

- If you have the SDK on CD-ROM, the Extras\DirectShow\Media directory 
  contains additional media that can be used with DirectShow and the DirectShow
  Editing Services (DES) interfaces.  We have included DV-encoded AVI videos, 
  larger MPEG videos, and higher-quality JPEG images.  Since the DV files are 
  very large, you should copy them to your hard disk before playing them to 
  ensure the best performance.

- Remote graph spying with GraphEdit works only on Windows 2000.

- The DirectShow Editing Services (DES) Color Generator source filter may 
  generate blue or green components incorrectly if they have small values.  
  For best results, use solid primary colors or set Blue and Green 
  components to values of either 0xFF or 0x00.

- For this release, the WavesReverb DMO will only accept 16-bit audio input.
  If you attempt to connect the WavesReverb DMO to an 8-bit audio source,
  the connection will fail.

- The sample XTL files have been updated in 8.1 to reflect a change in the
  DES XML parser on Windows XP.  XML attributes must now be specified
  in *lowercase*.  Mixed-case attributes will fail on Windows XP.

- GraphEdit has been revised for 8.1 to improve its user interface and to
  correct problems with zoom levels.

- The IRenderEngine::SetRenderRange method does not work if there is an 
  effect on a source clip, where the source clip does not start at time 
  zero.

__________________________________________________________________


COMPILER SUPPORT

- All DirectX 8.1 SDK samples are developed and compiled with Visual Studio 6.0.  
  The DirectX 8.1 SDK samples are optimized to compile with Visual Studio 6.0, 
  and we have provided Visual Studio 6.0 DSP files for your convenience.  

- All DirectX 8.1 SDK samples have been compiled with Visual Studio 7.0, and 
  that development environment is supported, even though no explicit project 
  files are provided.
 
- The DirectX SDK no longer supports using Visual Studio 5.0. While this may 
  work, any issues with using Visual Studio 5.0 and the DirectX 8.1 SDK will 
  not be addressed in this release or future releases. The DirectX 8.0a SDK supports 
  VC 5.0 ( with the SP3 linker fix ) and Windows 95, so this may be an option for some 
  developers.


__________________________________________________________________


SPECIAL THANKS

Marlin Studios

Textures used in the Billboard sample, tree02S.tga, tree01S.tga & tree35S.tga,
are provided courtesy of Marlin Studios (http://www.marlinstudios.com).  
They are provided for use with this sample only and cannot be distributed 
with any application without prior written consent.  

Viewpoint Digital, Inc.

The model used by the Visual Basic AutoParts sample, engine1.x, is provided 
courtesy of Viewpoint Digital, Inc. (http://www.viewpoint.com).  It is provided 
for use with this sample only and cannot be distributed with any application 
without prior written consent.  V6 Engine Model copyright 1999 Viewpoint 
Digital, Inc..

3DCAFE and Platinum Pictures

The model in the Dolphin samples, Dolphin.x, is derived from the model files 
(dolphins.zip) provided courtesy of 3DCAFE and Platinum Pictures  
(http://www.3dcafe.com/).  It is provided for use with this sample only.  
For further terms of use please visit the 3DCAFE site or contact 3DCAFE.


__________________________________________________________________


ROLAND SOUNDCANVAS SOUND SET FOR GENERAL MIDI AND GS FORMAT

The Roland SoundCanvas Sound Set includes the necessary sounds to make a 
complete General MIDI compatible sound set.  It also includes additional 
sounds necessary to make a complete GS Format compatible sound set.

WHAT IS THE GS FORMAT?

The GS Format is a standardized set of specifications for sound sources 
that defines the manner in which multitimbral sound generating devices 
will respond to MIDI messages.  The GS Format also complies with General 
MIDI System Level 1.

The GS Format also defines a number of other details over and above the 
features of General MIDI.  These include unique specifications for sound 
and functions available for tone editing, effects, and other specifications 
concerning the manner in which sound sources will respond to MIDI messages.  
Any device that is equipped with GS Format sound sources can faithfully 
reproduce both General MIDI sound recordings and GS Format MIDI sound 
recordings.

HOW TO CONTACT ROLAND:

For more information, please contact:

Roland Corporation U.S.
7200 Dominion Circle
Los Angeles, CA  90040-3696

Phone:	323-685-5141
Fax:		323-722-7408

COPYRIGHT NOTICE AND LICENSE LIMITATIONS

Roland GS Sound Set/Microsoft (P) 1996 Roland Corporation U.S.

The Roland GS Sound Set/Microsoft, the "Sample Set", incorporates the 
Roland SoundCanvas Sound Set. Roland Corporation U.S. owns the Sample Set 
and all copyrights pertaining thereto and has licensed the Sample Set for 
use solely on Microsoft operating system products and Microsoft Application 
Products.  

Roland reserves all rights to the Sample Set not expressly granted by Roland 
Corporation U.S. or by Microsoft under the terms of Microsoft's End User 
License Agreement.

The Sample Set may only be redistributed with your software application as a 
part of a Microsoft operating system product installation and under the terms
of Microsoft's End User License Agreement. The Sample Set may not be
separated from the Microsoft operating system product; the Sample Set may
only be installed by the Microsoft installation procedure for the associated
Microsoft operating system product or for an associated software development
kit.  All other uses of the Sample Set require a separate written license
from Roland Corporation U.S.

The Sample Set may not be used, copied or redistributed in hardware ROM or 
similar technology of any device without prior express written permission
from an authorized representative of Roland Corporation U.S. The Sample
Set may not be redistributed with any Musical Instrument . A "Musical
Instrument" is any device that is capable of generating musical sounds
without being directly connected to a computer with a licensed Microsoft
operating system product. You may not modify the Sample Set under any
circumstances.

