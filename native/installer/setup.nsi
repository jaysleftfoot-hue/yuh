Unicode true
!include "MUI2.nsh"
!include "x64.nsh"
Name "Big League Bowling"
OutFile "../../dist/Big-League-Bowling-Setup.exe"
InstallDir "$LOCALAPPDATA\Programs\Big League Bowling"
InstallDirRegKey HKCU "Software\BigLeagueBowling" "InstallDir"
RequestExecutionLevel user
SetCompressor /SOLID lzma
!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_RUN "$INSTDIR\Big League Bowling.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Play Big League Bowling"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
Function .onInit
 ${IfNot} ${RunningX64}
  MessageBox MB_ICONSTOP "This build requires 64-bit Windows."
  Abort
 ${EndIf}
FunctionEnd
Section "Game"
 SetShellVarContext current
 SetOutPath "$INSTDIR"
 File /r "../../dist/native-windows/*"
 WriteUninstaller "$INSTDIR\Uninstall.exe"
 CreateDirectory "$SMPROGRAMS\Big League Bowling"
 CreateShortcut "$SMPROGRAMS\Big League Bowling\Play.lnk" "$INSTDIR\Big League Bowling.exe"
 CreateShortcut "$SMPROGRAMS\Big League Bowling\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
 CreateShortcut "$DESKTOP\Big League Bowling.lnk" "$INSTDIR\Big League Bowling.exe"
 WriteRegStr HKCU "Software\BigLeagueBowling" "InstallDir" "$INSTDIR"
 WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BigLeagueBowling" "DisplayName" "Big League Bowling"
 WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BigLeagueBowling" "UninstallString" '"$INSTDIR\Uninstall.exe"'
 WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BigLeagueBowling" "DisplayIcon" "$INSTDIR\Big League Bowling.exe"
 WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BigLeagueBowling" "NoModify" 1
 WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BigLeagueBowling" "NoRepair" 1
SectionEnd
Section "Uninstall"
 SetShellVarContext current
 Delete "$DESKTOP\Big League Bowling.lnk"
 Delete "$SMPROGRAMS\Big League Bowling\Play.lnk"
 Delete "$SMPROGRAMS\Big League Bowling\Uninstall.lnk"
 RMDir "$SMPROGRAMS\Big League Bowling"
 ; Only remove shipped game files. Career saves outside this directory survive.
 Delete "$INSTDIR\Big League Bowling.exe"
 Delete "$INSTDIR\README.txt"
 Delete "$INSTDIR\ART_DIRECTION.md"
 Delete "$INSTDIR\raylib-LICENSE.txt"
 RMDir /r "$INSTDIR\assets"
 Delete "$INSTDIR\Uninstall.exe"
 RMDir "$INSTDIR"
 DeleteRegKey HKCU "Software\BigLeagueBowling"
 DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BigLeagueBowling"
SectionEnd
