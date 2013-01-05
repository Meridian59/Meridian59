; install.msi
;
; Installer for Meridian 59: The Internet Quest Continues client
;--------------------------------

!include FontReg.nsh
!include FontName.nsh
!include WinMessages.nsh
!include UAC.nsh

!define SOURCEDIR "..\run\localclient"

; First is default
LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\German.nlf"

; The name of the installer
Name "Meridian 59: The Internet Quest Continues"

; The file to write
OutFile "meridian59-installer.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Meridian 59: The Internet Quest Continues"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Meridian 59: The Internet Quest Continues" "Install_Dir"

; Compress to the max
SetCompressor /SOLID LZMA

; We run an outer script with user permission, and an inner one with admin.  See
; http://nsis.sourceforge.net/UAC_plug-in
RequestExecutionLevel user

Function .OnInit
 
UAC_Elevate:
    !insertmacro UAC_RunElevated
    StrCmp 1223 $0 UAC_ElevationAborted ; UAC dialog aborted by user?
    StrCmp 0 $0 0 UAC_Err ; Error?
    StrCmp 1 $1 0 UAC_Success ;Are we the real deal or just the wrapper?
    Quit
 
UAC_Err:
    MessageBox mb_iconstop "Unable to elevate, error $0"
    Abort
 
UAC_ElevationAborted:
    # elevation was aborted, run as normal?
    MessageBox mb_iconstop "This installer requires admin access, aborting!"
    Abort
 
UAC_Success:
    StrCmp 1 $3 +4 ;Admin?
    StrCmp 3 $1 0 UAC_ElevationAborted ;Try again?
    MessageBox mb_iconstop "This installer requires admin access, try again"
    goto UAC_Elevate 
 
FunctionEnd

; Pages

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

LicenseData "license.rtf"

;--------------------------------

Function InstallWithUserPrivilege
  ; These files go to the local data directory, not Program Files
  SetOutPath "$LOCALAPPDATA\Meridian 59: The Internet Quest Continues\resource"
  File ${SOURCEDIR}\resource\*.*
  SetOutPath "$LOCALAPPDATA\Meridian 59: The Internet Quest Continues\mail"
  File "${SOURCEDIR}\mail\*.*"
  SetOutPath "$LOCALAPPDATA\Meridian 59: The Internet Quest Continues"
  File "${SOURCEDIR}\meridian.ini"

  ; Create download directory now; creating it on demand seems to fail
  ; under some UAC conditions.
  CreateDirectory "$LOCALAPPDATA\Meridian 59: The Internet Quest Continues\download"

   ; Copy settings from previous installation  
  ClearErrors
  Var /GLOBAL NDSDIR 
  ReadRegStr $NDSDIR HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\meridian.exe" "Path"
  IfErrors no_previous_install
    ; Copy mail and map from prior installation, if any
    CopyFiles /SILENT $NDSDIR\mail\*.* $OUTDIR\mail
  
    ; Copy ini files
    CopyFiles /SILENT $NDSDIR\*.ini $OUTDIR

    ; Look for files in VirtualStore on Vista or later (earlier Meridian installs
    ; on these OSes will cause meridian.ini, mail, etc. to be virtualized)
    StrCpy $2 $NDSDIR "" 3  ; Chop off "c:\", will fail on UNC names
    Var /GLOBAL VIRTUALDIR
    StrCpy $VIRTUALDIR "$LOCALAPPDATA\VirtualStore\$2"
    IfFileExists $VIRTUALDIR\mail 0 update_ini_files
      CopyFiles /SILENT $VIRTUALDIR\mail\*.* $OUTDIR\mail

  update_ini_files:

    IfFileExists $VIRTUALDIR\meridian.ini 0 update_ini_timestamp
      CopyFiles /SILENT $VIRTUALDIR\*.ini $OUTDIR

  update_ini_timestamp:

    ; Set download time to reflect new installer
    WriteINIStr $OUTDIR\meridian.ini Miscellaneous Download 176

  no_previous_install:

FunctionEnd

; The stuff to install
Section "Meridian 59: The Internet Quest Continues (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put executable files there
  File "${SOURCEDIR}\arq.exe"
  File "${SOURCEDIR}\club.exe"
  File "${SOURCEDIR}\heidelb1.ttf"
  File "${SOURCEDIR}\license.rtf"
  File "${SOURCEDIR}\m59bind.exe"
  File "${SOURCEDIR}\meridian.exe"
  File "${SOURCEDIR}\mss32.dll"
  File "${SOURCEDIR}\mss32midi.dll"
  File "${SOURCEDIR}\mssmp3.asi"
  File "${SOURCEDIR}\waveplay.dll"
  File "${SOURCEDIR}\wrap.dll"
  File "${SOURCEDIR}\zlib1.dll"

  ; Install font
  StrCpy $FONT_DIR $FONTS
  !insertmacro InstallTTFFont "${SOURCEDIR}\heidelb1.ttf"
  SendMessage ${HWND_BROADCAST} ${WM_FONTCHANGE} 0 0 /TIMEOUT=5000

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Meridian59 "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Meridian 59: The Internet Quest Continues" "DisplayName" "Meridian 59: The Internet Quest Continues"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Meridian 59: The Internet Quest Continues" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Meridian 59: The Internet Quest Continues" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Meridian 59: The Internet Quest Continues" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  ; Install resources and copy old settings at user level
  GetFunctionAddress $0 InstallWithUserPrivilege
  !insertmacro UAC_AsUser_Call function InstallWithUserPrivilege ${UAC_SYNCREGISTERS}|${UAC_SYNCINSTDIR}
SectionEnd

; Optional section (can be disabled by the user)
Section "Desktop Shortcut"
  ; This inherits the run directory from $OUTDIR, which is set to the local data dir
  ; by InstallWithUserPrivilege

  ; Set to run in data directory
  SetOutPath "$LOCALAPPDATA\Meridian 59: The Internet Quest Continues"
  CreateShortCut "$DESKTOP\Meridian 59: The Internet Quest Continues.lnk" "$INSTDIR\meridian.exe" "" "$INSTDIR\meridian.exe" 0
  
SectionEnd

; Optional section
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Meridian 59: The Internet Quest Continues"
  CreateShortCut "$SMPROGRAMS\Meridian 59: The Internet Quest Continues\Uninstall Meridian 59: The Internet Quest Continues.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

  ; Set to run in data directory
  SetOutPath "$LOCALAPPDATA\Meridian 59: The Internet Quest Continues"

  CreateShortCut "$SMPROGRAMS\Meridian 59: The Internet Quest Continues\Meridian 59: The Internet Quest Continues.lnk" "$INSTDIR\meridian.exe" "" "$INSTDIR\meridian.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Function un.OnInit
UAC_Elevate:
    !insertmacro UAC_RunElevated
    StrCmp 1223 $0 UAC_ElevationAborted ; UAC dialog aborted by user?
    StrCmp 0 $0 0 UAC_Err ; Error?
    StrCmp 1 $1 0 UAC_Success ;Are we the real deal or just the wrapper?
    Quit
 
UAC_Err:
    MessageBox mb_iconstop "Unable to elevate, error $0"
    Abort
 
UAC_ElevationAborted:
    # elevation was aborted, run as normal?
    MessageBox mb_iconstop "This installer requires admin access, aborting!"
    Abort
 
UAC_Success:
    StrCmp 1 $3 +4 ;Admin?
    StrCmp 3 $1 0 UAC_ElevationAborted ;Try again?
    MessageBox mb_iconstop "This installer requires admin access, try again"
    goto UAC_Elevate 
 
FunctionEnd

Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Meridian 59: The Internet Quest Continues"
  DeleteRegKey HKLM "SOFTWARE\Meridian 59: The Internet Quest Continues"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Meridian 59: The Internet Quest Continues\Meridian 59.lnk"
  Delete "$SMPROGRAMS\Meridian 59: The Internet Quest Continues\Uninstall Meridian 59.link"
  Delete "$DESKTOP\Meridian 59: The Internet Quest Continues.lnk"

  ; Remove font

  ;  StrCpy $FONT_DIR $INSTDIR
  ;  !insertmacro RemoveTTFFont "$INSTDIR\heidelb1.ttf"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Meridian 59: The Internet Quest Continues"
  RMDir /r "$INSTDIR"
  RMDir /r "$LOCALAPPDATA\Meridian 59: The Internet Quest Continues"

SectionEnd
