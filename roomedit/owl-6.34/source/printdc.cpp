//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TPrintDC
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/dc.h>
#include <owl/module.h>

//
// Supply missing function definition and macros for newer Win32 function
//
  extern "C" {
    int  WINAPI DeviceCapabilitiesExA(LPCSTR, LPCSTR, LPCSTR, WORD,
                                      LPSTR, CONST DEVMODEA*);
    int  WINAPI DeviceCapabilitiesExW(LPCWSTR, LPCWSTR, LPCWSTR, WORD,
                                      LPWSTR, CONST DEVMODEW*);
    #if defined(UNICODE)
      #define DeviceCapabilitiesEx  DeviceCapabilitiesExW
    #else
      #define DeviceCapabilitiesEx  DeviceCapabilitiesExA
    #endif // !UNICODE
  }

  #define PROC_DEVICECAPABILITIES MAKEINTRESOURCE(91)

namespace owl {

OWL_DIAGINFO;

//
//
//
class TPrinterDriverLibrary : public TModule {
  public:
    TPrinterDriverLibrary(const tstring& drvName);

    TModuleProc5<int,LPCTSTR,LPCTSTR,WORD,LPTSTR,const DEVMODE*> DeviceCapabilities;
};

//
//
//
TPrinterDriverLibrary::TPrinterDriverLibrary(const tstring& drvName)
:
  TModule(drvName, true, true, false), // shouldLoad, mustLoad and !addToList
  DeviceCapabilities(*this,(LPCSTR)PROC_DEVICECAPABILITIES)
{
}

//
/// Creates a TPrint object for the DC given by handle.
//
TPrintDC::TPrintDC(HDC handle, TAutoDelete autoDelete)
:
  TCreatedDC(handle, autoDelete)
{
  memset(&DocInfo, 0, sizeof(DocInfo));
  DocInfo.cbSize = sizeof(DocInfo);
  DocInfo.lpszOutput = _T("");
  DocInfo.lpszDatatype = _T("");
}

//
/// Creates a TPrint object given print driver, device, output, and data from the
/// DEVMODE structure.
//
TPrintDC::TPrintDC(LPCTSTR driver, LPCTSTR device,
                   LPCTSTR output, const DEVMODE * initData)
:
  TCreatedDC(driver, device, output, initData)
{
  memset(&DocInfo, 0, sizeof(DocInfo));
  DocInfo.cbSize = sizeof(DocInfo);
  DocInfo.lpszOutput = _T("");
  DocInfo.lpszDatatype = _T("");
}

//
/// String-aware overload
//
TPrintDC::TPrintDC(const tstring& driver, const tstring& device, const tstring& output, const DEVMODE* initData)
  : TCreatedDC(driver, device, output, initData)
{
  memset(&DocInfo, 0, sizeof(DocInfo));
  DocInfo.cbSize = sizeof(DocInfo);
  DocInfo.lpszOutput = _T("");
  DocInfo.lpszDatatype = _T("");
}

//
/// Retrieves data about the specified capability of the named printer driver,
/// device, and port, and places the results in the output char array. The driver,
/// device, and port names must be zero-terminated strings. The devmode argument
/// points to a DEVMODE struct. If devmode is 0 (the default), DeviceCapabilities
/// retrieves the current default initialization values for the specified printer
/// driver; otherwise, it retrieves the values contained in the DEVMODE structure.
/// The format of the output array depends on the capability being queried. If
/// output is 0 (the default), DeviceCapabilities returns the number of bytes
/// required in the output array. Possible values for capability are as follows:
/// - \c \b  DC_BINNAMES	The function enumerates the paper bins on the given device. If a
/// device driver supports this constant, the output array is a data structure that
/// contains two members. The first member is an array identifying valid paper bins:
/// \code
/// short BinList[cBinMax]
/// \endcode
/// 	The second member is an array of character strings specifying the bin names:
/// \code
/// char PaperNames[cBinMax][cchBinName]
/// \endcode
/// 	If a device driver does not support this value, the output array is empty and
/// the return value is NULL.
/// 	If output is NULL, the return value specifies the number of bins supported.
/// - \c \b  DC_BINS	The function retrieves a list of constants that identify the available
/// bins and copies the list to the output array. If this array is NULL, the
/// function returns the number of supported bins. The following bin identifiers can
/// be returned:
/// \code
/// DMBIN_AUTO
/// DMBIN_CASSETTE
/// DMBIN_ENVELOPE
/// DMBIN_ENVMANUAL
/// DMBIN_FIRST
/// DMBIN_LARGECAPACITY
/// DMBIN_LARGEFMT
/// DMBIN_LAST
/// DMBIN_LOWER
/// DMBIN_MANUAL
/// DMBIN_MIDDLE
/// DMBIN_ONLYONE
/// DMBIN_SMALLFMT
/// DMBIN_TRACTOR
/// DMBIN_UPPER
/// \endcode
/// - \c \b  DC_DRIVER	The function returns the driver version number.
/// - \c \b  DC_DUPLEX	The function returns the level of duplex support. The return value is
/// 1 if the function supports duplex output; otherwise it is 0.
/// - \c \b  DC_ENUMRESOLUTIONS
/// The function copies a list of available printer resolutions to the output array.
/// The resolutions are copied as pairs of int32 integers; the first value of the
/// pair specifies the horizontal resolution and the second value specifies the
/// vertical resolution. If output is 0, the function returns the number of
/// supported resolutions.
/// - \c \b  DC_EXTRA	The function returns the number of bytes required for the
/// device-specific data that is appended to the DEVMODE structure.
/// - \c \b  DC_FIELDS	The function returns a value indicating which members of the DEVMODE
/// structure are set by the device driver. This value can be one or more of the
/// following constants:
/// \code
/// DM_ORIENTATION
/// DM_PAPERSIZE
/// DM_PAPERLENGTH
/// DM_PAPERWIDTH
/// DM_SCALE
/// DM_COPIES
/// DM_DEFAULTSOURCE
/// DM_PRINTQUALITY
/// DM_COLOR
/// DM_DUPLEX
/// DM_YRESOLUTION
/// DM_TTOPTION
/// DC_FILEDEPENDENCIES
/// \endcode
/// The function returns a list of files that must be loaded when the device driver
/// is installed. If output is 0 and this value is specified, the function returns
/// the number of file names that must be loaded. If output is nonzero, the function
/// returns the specified number of 64-character file names.
/// - \c \b  DC_MAXEXTENT	The function returns the maximum supported paper-size. These
/// dimensions are returned in a POINT structure; the x member gives the maximum
/// paper width and the y member gives the maximum paper length.
/// - \c \b  DC_MINEXTENT	The function returns the minimum supported paper-size. These
/// dimensions are returned in a POINT structure; the x member gives the minimum
/// paper width and the y member gives the minimum paper length.
/// - \c \b  DC_ORIENTATION	This function returns the number of degrees that a
/// portrait-oriented paper is rotated counterclockwise to produce landscape
/// orientation. if the return value is 0, there is no landscape orientation. If the
/// return value is 90, the portrait-oriented paper is rotated 90 degrees (as is the
/// case when HP laser printers are used). if the return value is 270, the
/// portrait-oriented paper is rotated 270 degrees (as is the case when dot-matrix
/// printers are used).
/// - \c \b  DC_PAPERNAMES	This function returns a list of supported paper names such as
/// Letter size or Legal size. The output array points to an array containing the
/// paper names. If the output array is 0, the function returns the number of
/// available paper sizes.
/// - \c \b  DC_PAPERS	The function retrieves a list of supported paper sizes and copies it
/// to the output array. The function returns the number of sizes identified in the
/// array. If output is 0, the function returns the number of supported paper sizes.
/// - \c \b  DC_PAPERSIZE	The function retrieves the supported paper sizes (specified in .1
/// millimeter units) and copies them to the output array.
/// - \c \b  DC_SIZE	The function returns the size of the DEVMODE structure required by the
/// given device driver.
/// - \c \b  DC_TRUETYPE	This function returns the printer driver's TrueType font
/// capabilities. The values returned can be one or more of the following constants:
/// - \c \b  DCTT_BITMAP	Device supports printing TrueType fonts as graphics. (Dot-matrix and
/// PCL printers)
/// - \c \b  DCTT_DOWNLOAD	Device supports downloading TryeType fonts. (PostScript and PCL
/// printers)
/// - \c \b  DCTT_SUBDEV	Device supports substituting device fonts for TrueType fonts.
/// (PostScript printers)
/// - \c \b  DC_VERSION	The function returns the device driver version number.
/// If DeviceCapabilities succeeds, the return value depends on the value of
/// capability, as noted above. Otherwise, the return value is GDI_ERROR.
//
uint32
TPrintDC::DeviceCapabilities(LPCTSTR driver,
                             LPCTSTR device,
                             LPCTSTR port,
                             int     capability,
                             LPTSTR  output,
                             LPDEVMODE devmode)
{
  // Hand call DeviceCapabilities due to Win32s missing function!
  //
  uint32 caps = 0;

#if !defined(WINELIB)
  // Try the Win32 DeviceCapabilitiesEx function
  //
  caps = ::DeviceCapabilitiesEx(driver, device, port, (uint16)capability, output, devmode);
  if (GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
    return caps;
#endif

  // Locate & call the DeviceCapabilities function within the printer driver
  // itself.
  //
  try {
    TPrinterDriverLibrary driverLib(driver);
    caps = driverLib.DeviceCapabilities(device, port, (uint16)capability, output, devmode);
  }
  catch (...) {
  }
  return caps;
}

//
/// String-aware overload
//
uint32 TPrintDC::DeviceCapabilities(
      const tstring& driver,
      const tstring& device,
      const tstring& port,
      int capability, 
      LPTSTR output,
      LPDEVMODE devmode)
{
  return DeviceCapabilities(
    driver.empty() ? 0 : driver.c_str(),
    device.empty() ? 0 : device.c_str(),
    port.empty() ? 0 : port.c_str(),
    capability,
    output,
    devmode);
}

} // OWL namespace
/* ========================================================================== */

