DirectShow SDK Samples README
------------------------------

======================================================================
NOTE: See the bottom of this file for a list of new and updated
DirectShow features in the DirectX 8.1 SDK.
======================================================================

The DirectShow SDK samples are subdivided into directories according
to their major function:

	BaseClasses
	BDA
	Capture
	Common (shared source files)
	DMO
	DVD
	Editing
	Filters
	Misc
	Players

There are also several Visual Basic samples in the VBSamples\DirectShow
directory.  Even if you are exclusively a Visual C++ developer, we recommend
that you review the Visual Basic sample executables, which are accessible from
the SDK's "Visual Basic Samples" Start Menu folder.  Applications like
SlideShow and Trimmer demonstrate many DirectShow Editing Servies (DES)
capabilities that are not yet demonstrated in C++ samples.  The DexterVB tool
(Timeline Explorer) is also useful for exploring and visualizing XML timelines
used with DES.


Setting up the build environment
--------------------------------
In your Visual C++ build environment, be sure to set your Include and Lib 
directories to point to the DirectX 8 SDK's include and lib path.
The DirectX 8 SDK directories must be the first directories in the search path.
Otherwise, you may encounter linker errors, as you would continue using
the headers and libraries supplied with Visual C++ or with an earlier version
of the Platform SDK.

For more information, see "Setting Up The Build Environment" in the 
"Getting Started" section of the DirectX 8 documentation.


Building the Samples
--------------------

NOTE: The BaseClasses header file schedule.h has been renamed to dsschedule.h
to prevent conflicts with the Platform SDK <schedule.h>.  Only the refclock.h
header in the BaseClasses directory uses this header, so the impact should
be minimal.

There are two ways to build the SDK samples:
	1) Using the DShow.DSW Visual C++ workspace
	2) Building individual projects within their VC++ workspaces


Using the Workspace:

The easiest way to build the samples is to open the DShow.DSW workspace,
which is installed to the DirectShow samples root directory.  This workspace
includes references to each of the project files for the individual samples.
You can select a project (in the FileView pane) and right-click to display a
build menu for the project.  Select 'Build' to build a project and its 
dependencies.  Note that if you select 'Build (selection only), you will not
automatically build any dependencies (like the base classes, if required).  

You can also build or rebuild all of the projects together by selecting
'Build->Batch Build' from the Visual C++ main menu.  Projects included in
this workspace know their dependencies and will build them if needed.

Some of the projects listed in the FileView pane do not build applications,
but instead build support libraries used by other projects within the main
workspace.  For example, the GargleDMO sample depends on the 'ControlBase'
and 'MedParamBase' projects that are contained within the workspace.  
If you build GargleDMO, Visual C++ will automatically build the 
ControlBase and MedParamBase libraries if they need to be built, and those
libraries will be linked into the GargleDMO DLL.


Building Individual Projects:

You can open any of the provided project or workspace files in Visual C++
if you want to build, modify, or debug an individual sample.  If you build
outside of the DShow.DSW workspace, however, you might need to first build the
DirectShow Base Classes, which are installed to the "BaseClasses" directory.

The DirectMedia 6 SDK installed binaries for the BaseClasses
(as strmbase.lib and the debug build, strmbasd.lib), which were compiled versions
of the base classes previously installed into the SDK's classes\base directory.  
DirectX 8 ships the source code for these base classes as a sample project,
which allows you to modify the classes and build them with Visual C++.
Since many of the samples (especially the sample filters) need to link with 
STRMBASE.LIB (or STRMBASD.LIB), you must first build the base classes 
to allow you to build other sample filters and applications that use them.

Don't forget to build both Debug and Release versions of the BaseClasses
project if you intend to build both Debug and Release versions of the samples.


Building UNICODE
----------------

By default, the DirectShow samples build ANSI versions.  If you want to build 
UNICODE versions of the samples, then you must add defines for UNICODE and _UNICODE
to your project's compiler preprocessor settings.  For example, the preprocessor 
definitions line for a debug MFC application might look like this:
   
   _DEBUG,_WINDOWS,_AFXDLL,_MBCS,WIN32,UNICODE,_UNICODE
   
If you want to build one of the MFC samples as UNICODE, there are two 
additional steps:

1) Add the appropriate MFC unicode library to the project's linker settings.
If you are building a release version, you might use MFC42u.lib.  If you are
building a debug version, you might use MFC42ud.lib.  Open the project settings
dialog, navigate to the Link tab, choose the "General" category, and add the 
appropriate MFC library to the "Object/library modules" field.

2) Set the entry point to wWinMainCRTStartup in the project's linker settings.
Navigate to the Link tab, choose the "Output" category, and add wWinMainCRTStartup
to the "Entry-point symbol" field.  The following information from MSDN provides
more detail:

    Note   MFC Unicode applications use wWinMain as the entry point. 
    In this case, CWinApp::m_lpCmdLine is a Unicode string. Be sure to set 
    wWinMainCRTStartup as the Entry Point symbol in the Output category of 
    the Link tab in the Project Settings dialog box.


Building Windows Media-related samples
--------------------------------------

Building the Windows Media-enabled samples (ASFCopy, JukeboxASF, PlayWndASF)
additionally requires the Microsoft Windows Media Format SDK and a valid 
software certificate.  The SDK download page is located at
http://msdn.microsoft.com/workshop/imedia/windowsmedia/sdk/wmsdk.asp, with
links to the SDK itself and information for obtaining a certificate.  Once 
installed, the project link settings for these samples (or alternatively,
the Visual C++ Tools->Options->Directories library path) must be updated 
to add the path for WMStub.lib and WMVCore.lib, which are provided with 
the Windows Media Format SDK.

Because of the dependency on the Windows Media Format SDK, the project files
for ASFCopy, JukeboxASF, and PlayWndASF are omitted from the main
DirectShow Visual C++ workspace file (dshow.dsw) to prevent build errors.


Building for Windows XP
-----------------------
If you want to target Windows XP specifically to use its new features, 
you must set WINVER=0x501 in the sample project file.  You must also 
install the Windows XP Platform SDK, however, to ensure that you have the 
latest header files.  For example, wxutil.cpp in the BaseClasses directory
uses the new TIME_KILL_SYNCHRONOUS flag only if (WINVER >= 0x501).  This flag 
is conditionally defined in the Windows XP Platform SDK in mmsystem.h, 
but only if WINVER is also set to 0x501 when compiling.  To prevent build 
issues, upgrade to the latest version of the Platform SDK.



=============================================================================
What's New in DirectShow for DirectX 8.1
=============================================================================
September, 2001
=============================================================================

There have been considerable improvements made to the DirectShow content
for the DirectX 8.1 SDK.  Below is a brief summary of the updates.


Improvements to existing samples
--------------------------------
There have been many enhancements and refinements to the existing
DirectX 8.0 samples for this release, including:

- New features and functionality
- Numerous bug fixes, including Windows XP support issues
- UNICODE and IA64 support
- ASF-enabled versions of the PlayWnd and Jukebox samples
- Clean compilation with Warning Level 4
- Improved video window repainting for Jukebox-style applications
- Updated resource files and version information blocks
- Digital Video sample (Capture\DVApp) was rewritten to take 
  advantage of the ICaptureGraphBuilder2 interface to build its graphs.


New DirectShow samples
----------------------
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
---------------------
The DirectShow documentation has been significantly enhanced for 
DirectX 8.1.  It adds information for the new features available in
Windows XP and expands on previously presented material for the
Microsoft TV Technologies.


Modified SampleGrabber filter source code
-----------------------------------------
There have been several requests for Microsoft to publish the source
code for the popular SampleGrabber filter.  To that end, we now provide
a modified (simpler) version of the sample grabber filter, which uses 
a new GUID and new CLSID to prevent collisions with the original 
DirectX 8.0 filter.  You can review and modify the source code for this 
filter and use it in your own applications.  See the Filters\Grabber 
directory for more information.


Windows XP support in header files
----------------------------------
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
--------------------
The DirectX 8.1 binaries and redist files have received various bug fixes
since the DirectX 8.0 release, including a large subset of the modifications
that were made for Windows XP.


Revised GraphEdit application
-----------------------------
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

