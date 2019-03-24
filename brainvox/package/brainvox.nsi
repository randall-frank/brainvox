
!define MUI_PRODUCT "brainvox"

Name "Brainvox"
Caption "Brainvox 3.14"
Icon "..\brainvox\brainvox.ico"
UninstallIcon "..\brainvox\brainvox.ico"
UninstallText "This will uninstall Brainvox."

XPStyle on
ShowInstDetails hide
ShowUninstDetails hide

CRCCheck On

OutFile "bvox_win32_314.exe"
ShowInstDetails hide
ShowUninstDetails hide
;SetCompressor "bzip2"
 
InstallDir "$PROGRAMFILES\U of Iowa LCN\Brainvox"
  
LicenseData "License.rtf"

Page license
Page directory
Page instfiles
 
UninstPage uninstConfirm
UninstPage instfiles

ShowInstDetails show

 
;-------------------------------- 
;Installer Sections   
  
Section "install" ; Installation info
 
;Add files
  SetOutPath "$INSTDIR"
  File "License.rtf"
  File "..\docs\ReleaseNotes.txt"
  File "..\brainvox\brainvox.ico"
  SetOutPath "$INSTDIR\bin"
  File "..\bin\*.exe"
  File "..\bin\*.lib"
  SetOutPath "$INSTDIR\man"
  File "..\bin\man\*.txt"
  
;create desktop shortcut
  CreateShortCut "$DESKTOP\${MUI_PRODUCT}.lnk" "$INSTDIR\bin\${MUI_PRODUCT}.exe" ""
 
;create start-menu items
  CreateDirectory "$SMPROGRAMS\Brainvox and tal_programs"
  CreateShortCut "$SMPROGRAMS\Brainvox and tal_programs\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Brainvox and tal_programs\${MUI_PRODUCT}.lnk" "$INSTDIR\bin\${MUI_PRODUCT}.exe" "" "$INSTDIR\bin\${MUI_PRODUCT}.exe" 0
  CreateShortCut "$SMPROGRAMS\Brainvox and tal_programs\Release notes.lnk" "$INSTDIR\ReleaseNotes.txt" "" "$INSTDIR\ReleaseNotes.txt" 0
  CreateShortCut "$SMPROGRAMS\Brainvox and tal_programs\tal_programs.lnk" "$INSTDIR\man\tal_programs.txt" "" "$INSTDIR\man\tal_programs.txt" 0

;write uninstall information to the registry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "DisplayName" "${MUI_PRODUCT} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "UninstallString" "$INSTDIR\Uninstall.exe"
 
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
    '$INSTDIR\bin\${MUI_PRODUCT}.exe "%1"'
 
  System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
!undef Index

 
SectionEnd
 
 
;--------------------------------    
;Uninstaller Section  
Section "Uninstall"
   
;Delete Files 
  RMDir /r "$INSTDIR\*.*"    
 
;Remove the installation directory
  RMDir "$INSTDIR"
  
;Delete Start Menu Shortcuts
  Delete "$DESKTOP\${MUI_PRODUCT}.lnk"
  Delete "$SMPROGRAMS\Brainvox and tal_programs\*.*"
  RmDir  "$SMPROGRAMS\Brainvox and tal_programs"
  
;Delete Uninstaller And Unistall Registry Entries
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${MUI_PRODUCT}"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}"  

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

SectionEnd
 
 
;--------------------------------    
;MessageBox Section
 
 
;Function that calls a messagebox when installation finished correctly
Function .onInstSuccess
  MessageBox MB_OK "You have successfully installed ${MUI_PRODUCT}. Use the desktop icon to start the program."
FunctionEnd
 
 
Function un.onUninstSuccess
  MessageBox MB_OK "You have successfully uninstalled ${MUI_PRODUCT}."
FunctionEnd

