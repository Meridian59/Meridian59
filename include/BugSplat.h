
///////////////////////////////////////////////////////////////////////////////
//
// BugSplat.h
//
// This header file contains the code used to add BugSplat crash reporting to
// native Windows applications.
//
// For more information, see the documentation at https://www.bugsplat.com/docs/platforms/cplusplus
//
// Copyright 2003-2019, BugSplat 
// All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////

//! \mainpage
//! Integrating BugSplat crash reporting into your native Windows application is easy!
//! The following example code is from the myConsoleCrasher application:
//! \include  myConsoleCrasher.cpp


#ifndef BUGSPLAT_H
#define BUGSPLAT_H

#ifdef BUGSPLAT_EXPORTS
    #define MDS_EXPORT __declspec(dllexport)
#else
    #define MDS_EXPORT __declspec(dllimport)
#endif

#include <eh.h>
#include <new.h>
#include <signal.h>
#include <Windows.h>


//! If a callback function is registered with MiniDmpSender
//! it is notified when data or information is required from the main app
//! A list of callback codes appears at the end of this file...
typedef bool (__cdecl *MiniDmpSenderCallback)(UINT nCode, LPVOID lVal1, LPVOID lVal2);

// forward declare internal class
class MiniDumper;

//! The MiniDmpSender class is used to add BugSplat crash reporting to your app.
class MDS_EXPORT MiniDmpSender 
{
public:
    //! @param szDatabase database name on bugsplat.com
    //! @param szApp application name as supplied with PDB files on bugsplat.com
    //! @param szVersion version identifier as supplied with PDB files on bugsplat.com
    //! @param szAppIdentifier generic field to contain app details (locale, build #, etc...)
    //! @param dwFlags Option flags.  see MDSF_FLAG... defines
    MiniDmpSender(__wchar_t const *szDatabase,
                  __wchar_t const *szApp,
                  __wchar_t const *szVersion,
                  __wchar_t const *szAppIdentifier = NULL,
                  DWORD dwFlags = 0x04 /* MDSF_PREVENTHIJACKING */
                  );
    virtual ~MiniDmpSender();
public:
    //! To disable / enable the unhandled exception filter call this function with the desired mode
    //! This function is not valid when MDSF_PREVENTHIJACKING is used.
    bool enableExceptionFilter(bool enable = true);

    //! Returns the current state of the unhandled exception filter, ie the last value sent to enableExceptionFilter.
    bool isExceptionFilterEnabled() const;

	// the following typedef is identical to MINIDUMP_TYPE; copied here for convenience
	typedef enum _BS_MINIDUMP_TYPE {
		MiniDumpNormal = 0x00000000,
		MiniDumpWithDataSegs = 0x00000001,
		MiniDumpWithFullMemory = 0x00000002,
		MiniDumpWithHandleData = 0x00000004,
		MiniDumpFilterMemory = 0x00000008,
		MiniDumpScanMemory = 0x00000010,
		MiniDumpWithUnloadedModules = 0x00000020,
		MiniDumpWithIndirectlyReferencedMemory = 0x00000040,
		MiniDumpFilterModulePaths = 0x00000080,
		MiniDumpWithProcessThreadData = 0x00000100,
		MiniDumpWithPrivateReadWriteMemory = 0x00000200,
		MiniDumpWithoutOptionalData = 0x00000400,
		MiniDumpWithFullMemoryInfo = 0x00000800,
		MiniDumpWithThreadInfo = 0x00001000,
		MiniDumpWithCodeSegs = 0x00002000,
		MiniDumpWithoutAuxiliaryState = 0x00004000,
		MiniDumpWithFullAuxiliaryState = 0x00008000,
		MiniDumpWithPrivateWriteCopyMemory = 0x00010000,
		MiniDumpIgnoreInaccessibleMemory = 0x00020000,
		MiniDumpWithTokenInformation = 0x00040000,
		MiniDumpWithModuleHeaders = 0x00080000,
		MiniDumpFilterTriage = 0x00100000,
		MiniDumpWithAvxXStateContext = 0x00200000,
		MiniDumpValidTypeFlags = 0x003fffff,
	} BS_MINIDUMP_TYPE;

	//! Get the current minidump type
	MiniDmpSender::BS_MINIDUMP_TYPE getMiniDumpType() const;

	//! Set the minidump type.   Use with care.  Changing the minidump type can result in large crash file 
	//! uploads that may be rejected by BugSplat. 
	void setMiniDumpType(MiniDmpSender::BS_MINIDUMP_TYPE eType);

    //! Disable network access from BugSplat library.  When a crash occurs, the minidump will be created 
	//! and a message box displayed indicating path to dump file; app will then exit w/o reporting crash to BugSplat.
    bool disableNetworkAccess(bool enable = true);
    //! Returns the current network access state 
    bool isNetworkAccessDisabled() const;

    //! Set the option flags; see MDSF_FLAG... defines below.
    bool setFlags( DWORD dwFlags );
    //! Get the option flags.
    DWORD getFlags() const;

    //! Use to change the version string at runtime.
    void resetVersionString(const __wchar_t * wszVersion);

    //! Use to change the generic app identifier field at runtime.
    void resetAppIdentifier(const __wchar_t * wszDescription);

	//! Use to set value for the Notes field in the web app
	void setNotes(const __wchar_t* wszNotes);

    //! Send additional file(s) along with the crash report; may be called repeatedly to send multiple files.
    void sendAdditionalFile(const __wchar_t * wszPath);

	//! Removes a file from the list created by sendAdditionalFile()
	bool removeAdditionalFile(const __wchar_t * wszPath);

	//! Set a custom crash attribute
	bool setAttribute(const __wchar_t* szName, const __wchar_t* szValue);
	
	//! Set full path for log file; default is bugsplat.log in the %TEMP% folder.
	void setLogFilePath(const __wchar_t * wszPath);
	
	//! Use to set full path for crash report zip file; default is a path in the %TEMP% folder.
    void setUserZipPath(const __wchar_t * wszPath);

    //! Use to set full path for BsSndRpt's resource DLL (allows dialog customizations, e.g. language); default is ./BugSplatRc.dll (or ./BugSplatRc64.dll).
    void setResourceDllPath(const __wchar_t * wszPath);

    //! Use to set the default user name.  Useful for quiet-mode applications that don't prompt for user/email/description at crash time.
    void setDefaultUserName(const __wchar_t * wszName);

    //! Use to set the default user email.  Useful for quiet-mode applications that don't prompt for user/email/description at crash time.
    void setDefaultUserEmail(const __wchar_t * wszEmail);

    //! Use to set the default user description.  Useful for quiet-mode applications that don't prompt for user/email/description at crash time.
    void setDefaultUserDescription(const __wchar_t * wszDescription);

    //! Use to send a BugSplat crash report outside of the unhandled exception filters.
    //! For example, you could send a report directly from your own try/catch clause.
    void createReport(EXCEPTION_POINTERS * pExcepInfo);

	//! Use to send an Address Sanitizer report to BugSplat.
	void createAsanReport(const char* asanMessage);

	//! Use to send a report to BugSplat, bypassing minidump creation.  
	//! See myConsoleCrasher for an example of the XML required schema.
	//! This function does not exit, normal program flow continues.
    void createReport(const __wchar_t * wszXmlReport);
	
    //! Use to send a report and exit
    inline void createReportAndExit() { setFlags(getFlags() | 0x02/*MDSF_FORCEEXIT*/); throw NULL; }

    //! Use with MDSF_CUSTOMEXCEPTIONFILTER flag.
    LONG unhandledExceptionHandler( PEXCEPTION_POINTERS pExceptionInfo );

    //! Use to assign a callback function for sharing additional information with the app after a crash occurs.
    void setCallback(MiniDmpSenderCallback fnCallback);

    //! Get path to minidump.  
    void getMinidumpPath(__wchar_t * buffer, size_t len);

	//! Set the size of the guard byte buffer.  See the MDSF_USEGUARDMEMORY flag
	int setGuardByteBufferSize(int nbytes);

	//! Set the timeout in ms used to determine if a process is hung. Default is 5000.
	int setHangDetectionTimeout(int ms);

    //! Internal method.
    LPVOID imp();

private:
    MiniDumper * m_pMiniDumper;
};


// ***************************
// Definitions for the dwFlags parameter of MiniDmpSender constructor and the getFlags/setFlags methods.
// The non-interactive flag is for applications that run unattended or without a user interface.  

//! This flag automatically submits a crash report, without prompting the user with a dialog box.
#define MDSF_NONINTERACTIVE     0x0001

//! This flag will force the application to exit after posting a BugSplat report.  The default behavior
//! is to continue calling the other unhandled exception filters in the unhandled exception chain.
#define MDSF_FORCEEXIT          0x0002

//! This flag prevents other threads in the process from hijacking the Exception filter.
//! It is useful when you want to prevent 3rd party libraries included with your application
//! or the Operating System from disabling BugSplat integration.
//! It is *recommended* that you use this setting
#define MDSF_PREVENTHIJACKING   0x0004

//! This flag launches the BugSplatHD.exe process with the current process id.
//! The process runs in low priority, and periodically performs tests on the main process id 
//! to determine if it has hung.
//! The burden on the OS is extremely low, and will only consume a few CPU cycles 
//! each minute.
#define MDSF_DETECTHANGS        0x0008

//! The guard memory flag causes the BugSplat library to pre-allocate a 4MB (default) chunk of data
//! on the heap. When a crash is encountered, this heap data is deleted and might be useful
//! in low memory scenarios.  The size of the buffer can be controlled using setGuardByteBufferSize();
#define MDSF_USEGUARDMEMORY     0x0010

//! BugSplat operates using the SetUnhandledExceptionFilter method. Use this flag if you want to
//! use your own exception handling mechanism instead.
//! To initiate a BugSplat crash report, you need to call unhandledExceptionHandler within your own
//! exception filter method
#define MDSF_CUSTOMEXCEPTIONFILTER 0x0020

//! EXPERIMENTAL
//! Use this flag to suspend all threads in the process prior to capturing the
//! crash report data. This might be used to prevent security libraries from detecting
//! the debugging commands used by BugSplat and terminating the app before the 
//! crash has a chance to be reported.
#define MDSF_SUSPENDALLTHREADS  0x0800

//! These flags control output of the BugSplat log output, typically used for development/debugging.
//! If MDSF_LOGCONSOLE is set, logging information is sent to the debugger console.
//! If MDSF_LOGFILE is set, logging information is added to a log file and is included in the crash report zip file.  
#define MDSF_LOGCONSOLE         0x1000
#define MDSF_LOGFILE            0x2000
#define MDSF_LOG_VERBOSE        0x4000

//! Definitions for the BugSplat exception call back codes.

//! nCode: MDSCB_EXCEPTIONCODE
//! lVal1: EXCEPTION_RECORD*, pointer to the exception record responsible for the crash
//! lVal2: Reserved
#define MDSCB_EXCEPTIONCODE            0x0050

//
// Helper functions used to set CRT state.
//
inline void terminator() { int*z = 0; *z = 13; }
inline void signal_handler(int) { terminator(); }
inline void __cdecl invalid_parameter_handler(const wchar_t *, const wchar_t *, const wchar_t *, unsigned int, uintptr_t)
{
	terminator();
}
inline int memory_depleted(size_t)
{
	terminator();
	return 0;
}

// This call should be made once from your app to enable collection of certain CRT exceptions
inline void SetGlobalCRTExceptionBehavior()
{
	// There is a single set_terminate handler for all dynamically linked DLLs or EXEs; 
	// even if you call set_terminate your handler may be replaced by another, 
	// or you may be replacing a handler set by another DLL or EXE.
	// See https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/set-terminate-crt?view=vs-2019
	set_terminate(&terminator);

	// Because there is only one _purecall_handler for each process, when you call _set_purecall_handler 
	// it immediately impacts all threads. The last caller on any thread sets the handler.
	// See https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/get-purecall-handler-set-purecall-handler?view=vs-2019
	_set_purecall_handler(&terminator);

	// Only one function can be specified as the global invalid argument handler at a time. 
	// See https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/set-invalid-parameter-handler-set-thread-local-invalid-parameter-handler?view=vs-2019
	_set_invalid_parameter_handler(&invalid_parameter_handler);

	// There is a single _set_new_handler handler for all dynamically linked DLLs or executables; 
	// even if you call _set_new_handler your handler might be replaced by another or that you are 
	// replacing a handler set by another DLL or executable.
	// See https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/set-new-handler?view=vs-2019
	_set_new_handler(&memory_depleted);
	_set_new_mode(1);
}


// This call should be made in each thread of your application to enable collection of certain CRT exceptions
inline void SetPerThreadCRTExceptionBehavior()
{
	// Signal handling, required for each thread, at least for SIGABRT
	signal(SIGABRT, signal_handler);
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
}

#endif //~BUGSPLAT_H