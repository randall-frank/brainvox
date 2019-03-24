!include "MUI.nsh"
!include "addtopath.nsh"

Name "Brainvox"
OutFile "bvox_win32_314.exe"

!define PRODUCT "brainvox"

InstallDir "$PROGRAMFILES\U of Iowa LCN\Brainvox"
SetCompressor lzma

!define MUI_ABORTWARNING
!define MUI_PAGE_HEADER_TEXT "Brainvox 3.14"
!define MUI_ICON "..\brainvox\brainvox.ico"
!define MUI_UNICON "..\brainvox\brainvox.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "License.rtf"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"


;-------------------------------- 
;Installer Sections   
  
Section "Brainvox 3.14" Brainvox
 
;Add files
  SetOutPath "$INSTDIR"
  File "License.rtf"
  File "..\docs\ReleaseNotes.txt"
  File "..\brainvox\brainvox.ico"
  SetOutPath "$INSTDIR\bin"
  File "..\bin\brainvox*.exe"
  
;create desktop shortcut
  CreateShortCut "$DESKTOP\${PRODUCT}.lnk" "$INSTDIR\bin\${PRODUCT}.exe" ""
 
;create start-menu items
  CreateDirectory "$SMPROGRAMS\Brainvox and tal_programs"
  CreateShortCut "$SMPROGRAMS\Brainvox and tal_programs\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Brainvox and tal_programs\${PRODUCT}.lnk" "$INSTDIR\bin\${PRODUCT}.exe" "" "$INSTDIR\bin\${PRODUCT}.exe" 0
  CreateShortCut "$SMPROGRAMS\Brainvox and tal_programs\Release notes.lnk" "$INSTDIR\ReleaseNotes.txt" "" "$INSTDIR\ReleaseNotes.txt" 0

;write uninstall information to the registry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayName" "${PRODUCT} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "UninstallString" "$INSTDIR\Uninstall.exe"
 
  WriteUninstaller "$INSTDIR\Uninstall.exe"

;file associations
!define Index "Line${__LINE__}"
  ReadRegStr $1 HKCR "._patient" ""
  StrCmp $1 "" "${Index}-NoBackup"
    StrCmp $1 "DatasetFile" "${Index}-NoBackup"
    WriteRegStr HKCR "._patient" "backup_val" $1
"${Index}-NoBackup:"
  WriteRegStr HKCR "._patient" "" "DatasetFile"
  ReadRegStr $0 HKCR "DatasetFile" ""
  StrCmp $0 "" 0 "${Index}-Skip"
	WriteRegStr HKCR "DatasetFile" "" "Brainvox dataset"
	WriteRegStr HKCR "DatasetFile\shell" "" "open"
	WriteRegStr HKCR "DatasetFile\DefaultIcon" "" "$INSTDIR\brainvox.ico"
"${Index}-Skip:"
  WriteRegStr HKCR "DatasetFile\shell\open\command" "" \
    '$INSTDIR\bin\${PRODUCT}.exe "%1"'
 
  System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
!undef Index

 
SectionEnd

Section "tal_programs 1.53" TalProgs
 
;Add files
  SetOutPath "$INSTDIR"
  File "License.rtf"
  SetOutPath "$INSTDIR\bin"
  File "..\bin\tal*.exe"
  File "..\bin\worsley.exe"
  File "..\bin\analyze2raw.exe"
  File "..\bin\raw2analyze.exe"
  File "..\bin\*.lib"
  SetOutPath "$INSTDIR\man"
  File "..\bin\man\*.txt"
  
;create start-menu items
  CreateDirectory "$SMPROGRAMS\Brainvox and tal_programs"
  CreateShortCut "$SMPROGRAMS\Brainvox and tal_programs\tal_programs.lnk" "$INSTDIR\man\tal_programs.txt" "" "$INSTDIR\man\tal_programs.txt" 0

;write uninstall information to the registry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayName" "${PRODUCT} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "UninstallString" "$INSTDIR\Uninstall.exe"
 
  WriteUninstaller "$INSTDIR\Uninstall.exe"

; PATH env var
  Push "$INSTDIR\\bin"
  Call AddToPath
 
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_Brainvox ${LANG_ENGLISH} "Brainvox applications, tools and desktop interface. Graphics card supporting OpenGL 1.5 and fragment shaders required to support hardware volume rendering"
  LangString DESC_TalProgs ${LANG_ENGLISH} "Tal_programs and man pages.  Requires MS C++ compiler for dynamic compilation tools."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${Brainvox} $(DESC_Brainvox)
    !insertmacro MUI_DESCRIPTION_TEXT ${TalProgs} $(DESC_TalProgs)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------    
;Uninstaller Section  
Section "Uninstall"
   
;Delete Files 
  RMDir /r "$INSTDIR\*.*"    
 
;Remove the installation directory
  RMDir "$INSTDIR"
  
;Delete Start Menu Shortcuts
  Delete "$DESKTOP\${PRODUCT}.lnk"
  Delete "$SMPROGRAMS\Brainvox and tal_programs\*.*"
  RmDir  "$SMPROGRAMS\Brainvox and tal_programs"
  
;Delete Uninstaller And Unistall Registry Entries
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT}"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"  

;File association
!define Index "Line${__LINE__}"
  ReadRegStr $1 HKCR "._patient" ""
  StrCmp $1 "DatasetFile" 0 "${Index}-NoOwn" ; only do this if we own it
    ReadRegStr $1 HKCR "._patient" "backup_val"
    StrCmp $1 "" 0 "${Index}-Restore" ; if backup="" then delete the whole key
      DeleteRegKey HKCR "._patient"
    Goto "${Index}-NoOwn"
"${Index}-Restore:"
      WriteRegStr HKCR "._patient" "" $1
      DeleteRegValue HKCR "._patient" "backup_val"
   
    DeleteRegKey HKCR "DatasetFile" ;Delete key with association settings
 
    System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
"${Index}-NoOwn:"
!undef Index

; PATH env var
  Push "$INSTDIR\\bin"
  Call un.RemoveFromPath

SectionEnd
 
 
;--------------------------------    
;MessageBox Section
 
 
;Function that calls a messagebox when installation finished correctly
Function .onInstSuccess
  MessageBox MB_OK "You have successfully installed ${PRODUCT}. Use the desktop icon to start the program."
FunctionEnd
 
 
Function un.onUninstSuccess
  MessageBox MB_OK "You have successfully uninstalled ${PRODUCT}."
FunctionEnd

