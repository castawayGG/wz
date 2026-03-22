; WZ Virtual Gamepad - NSIS Installer Script
; Requires NSIS 3.0+ with Modern UI 2 plugin
; Build with: makensis installer.nsi

!include "MUI2.nsh"
!include "x64.nsh"
!include "LogicLib.nsh"

;--------------------------------
; General

Name "WZ Virtual Gamepad"
OutFile "WZVirtualGamepad_Setup.exe"
InstallDir "$PROGRAMFILES64\WZVirtualGamepad"
InstallDirRegKey HKLM "Software\WZVirtualGamepad" "InstallDir"
RequestExecutionLevel admin
Unicode True

;--------------------------------
; Version Information

VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName"      "WZ Virtual Gamepad"
VIAddVersionKey "ProductVersion"   "1.0.0"
VIAddVersionKey "CompanyName"      "WZ Virtual Gamepad Project"
VIAddVersionKey "LegalCopyright"   "MIT License"
VIAddVersionKey "FileDescription"  "WZ Virtual Gamepad Installer"
VIAddVersionKey "FileVersion"      "1.0.0.0"

;--------------------------------
; MUI Settings

!define MUI_ABORTWARNING

; NOTE: Place icon.ico and installer_banner.bmp in a 'resources\' directory
; next to the installer directory. These files are optional for the build.
; If they are missing, comment out the lines below.
!if /FileExists "..\resources\icon.ico"
    !define MUI_ICON "..\resources\icon.ico"
    !define MUI_UNICON "..\resources\icon.ico"
!endif
!if /FileExists "..\resources\installer_banner.bmp"
    !define MUI_WELCOMEFINISHPAGE_BITMAP "..\resources\installer_banner.bmp"
!endif

;--------------------------------
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\wz_gamepad.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch WZ Virtual Gamepad"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\docs\USER_GUIDE.md"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "View User Guide"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"

;--------------------------------
; Components

Section "Main Application" SecMain
    SectionIn RO  ; Required section
    SetOutPath "$INSTDIR"

    ; Main executable
    File "..\build\Release\wz_gamepad.exe"

    ; Create profiles directory
    SetOutPath "$INSTDIR\profiles"
    File "..\profiles\warzone_default.json"
    File "..\profiles\warzone_sniper.json"
    File "..\profiles\warzone_aggressive.json"
    File "..\profiles\warzone_vehicle.json"
    File "..\profiles\generic_fps.json"

    ; Documentation
    SetOutPath "$INSTDIR\docs"
    File "..\docs\USER_GUIDE.md"
    File "..\docs\ARCHITECTURE.md"
    File "..\docs\BUILDING.md"
    File "..\docs\DRIVER_SAFETY.md"
    File "..\docs\WARZONE_TUNING.md"

    ; Store install dir in registry
    WriteRegStr HKLM "Software\WZVirtualGamepad" "InstallDir" "$INSTDIR"
    WriteRegStr HKLM "Software\WZVirtualGamepad" "Version"    "1.0.0"

    ; Uninstall registry entries
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" \
        "DisplayName"     "WZ Virtual Gamepad"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" \
        "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" \
        "DisplayVersion"  "1.0.0"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" \
        "Publisher"       "WZ Virtual Gamepad Project"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" \
        "InstallLocation" "$INSTDIR"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" \
        "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" \
        "NoRepair"  1

    ; Write uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "ViGEmBus Driver" SecViGEm
    ; Check if ViGEmBus is already installed
    ReadRegStr $0 HKLM "SYSTEM\CurrentControlSet\Services\ViGEmBus" "ImagePath"
    ${If} $0 != ""
        DetailPrint "ViGEmBus driver already installed."
    ${Else}
        DetailPrint "Installing ViGEmBus driver..."
        ; Download ViGEmBus setup using PowerShell.
        ; The script verifies the Authenticode signature of the downloaded file
        ; before execution to prevent tampering.
        nsExec::ExecToLog 'powershell -NoProfile -ExecutionPolicy Bypass -Command "& { \
            $url = \"https://github.com/nefarius/ViGEmBus/releases/latest/download/ViGEmBus_Setup_x64.exe\"; \
            $out = Join-Path $env:TEMP \"ViGEmBus_Setup.exe\"; \
            Invoke-WebRequest $url -OutFile $out -UseBasicParsing; \
            $sig = Get-AuthenticodeSignature $out; \
            if ($sig.Status -ne \"Valid\") { Remove-Item $out -Force; throw \"Invalid signature on downloaded ViGEmBus installer.\"; } \
            Start-Process $out -ArgumentList \"/S\" -Wait \
        }"'
        Pop $0
        ${If} $0 == 0
            DetailPrint "ViGEmBus installed successfully."
        ${Else}
            MessageBox MB_OK|MB_ICONEXCLAMATION \
                "ViGEmBus installation failed. Please install it manually from:$\nhttps://github.com/nefarius/ViGEmBus/releases/latest"
        ${EndIf}
    ${EndIf}
SectionEnd

Section "Start Menu Shortcuts" SecStartMenu
    CreateDirectory "$SMPROGRAMS\WZ Virtual Gamepad"
    CreateShortcut "$SMPROGRAMS\WZ Virtual Gamepad\WZ Virtual Gamepad.lnk" \
        "$INSTDIR\wz_gamepad.exe" "" "$INSTDIR\wz_gamepad.exe" 0
    CreateShortcut "$SMPROGRAMS\WZ Virtual Gamepad\User Guide.lnk" \
        "$INSTDIR\docs\USER_GUIDE.md"
    CreateShortcut "$SMPROGRAMS\WZ Virtual Gamepad\Uninstall.lnk" \
        "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Desktop Shortcut" SecDesktop
    CreateShortcut "$DESKTOP\WZ Virtual Gamepad.lnk" \
        "$INSTDIR\wz_gamepad.exe" "" "$INSTDIR\wz_gamepad.exe" 0
SectionEnd

;--------------------------------
; Section Descriptions

LangString DESC_SecMain     ${LANG_ENGLISH} "WZ Virtual Gamepad application (required)"
LangString DESC_SecViGEm    ${LANG_ENGLISH} "Install ViGEmBus virtual gamepad driver (required for virtual controller)"
LangString DESC_SecStartMenu ${LANG_ENGLISH} "Create Start Menu shortcuts"
LangString DESC_SecDesktop  ${LANG_ENGLISH} "Create Desktop shortcut"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain}      $(DESC_SecMain)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecViGEm}     $(DESC_SecViGEm)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop}   $(DESC_SecDesktop)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Uninstaller

Section "Uninstall"
    ; Stop running instances
    nsExec::ExecToLog 'taskkill /F /IM wz_gamepad.exe'

    ; Remove files
    Delete "$INSTDIR\wz_gamepad.exe"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir /r "$INSTDIR\profiles"
    RMDir /r "$INSTDIR\docs"

    ; Ask about user data
    MessageBox MB_YESNO "Remove user profiles and configuration data?" IDNO skip_userdata
        RMDir /r "$APPDATA\WZVirtualGamepad"
    skip_userdata:

    ; Remove shortcuts
    RMDir /r "$SMPROGRAMS\WZ Virtual Gamepad"
    Delete "$DESKTOP\WZ Virtual Gamepad.lnk"

    ; Remove registry entries
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad"
    DeleteRegKey HKLM "Software\WZVirtualGamepad"

    RMDir "$INSTDIR"

    MessageBox MB_OK "WZ Virtual Gamepad has been uninstalled.$\n$\nNote: The ViGEmBus driver was NOT removed.$\nUse 'Add or Remove Programs' to uninstall it if desired."
SectionEnd

;--------------------------------
; Functions

Function .onInit
    ; Check Windows version (require Windows 10+)
    ${IfNot} ${AtLeastWin10}
        MessageBox MB_OK|MB_ICONSTOP "WZ Virtual Gamepad requires Windows 10 or later."
        Abort
    ${EndIf}

    ; Check 64-bit
    ${IfNot} ${RunningX64}
        MessageBox MB_OK|MB_ICONSTOP "WZ Virtual Gamepad requires a 64-bit Windows installation."
        Abort
    ${EndIf}
FunctionEnd
