@echo off
setlocal EnableDelayedExpansion

echo ============================================================
echo   WZ Virtual Gamepad - Uninstaller
echo ============================================================
echo.

:: Check for administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator.
    echo Right-click uninstall.bat and select "Run as administrator".
    pause
    exit /b 1
)

set "INSTALL_DIR=%ProgramFiles%\WZVirtualGamepad"

echo This will remove WZ Virtual Gamepad from your system.
echo Installation directory: %INSTALL_DIR%
echo.
set /p CONFIRM=Are you sure you want to uninstall? (Y/N): 
if /i "%CONFIRM%" neq "Y" (
    echo Uninstall cancelled.
    pause
    exit /b 0
)

:: ── Stop running instance ─────────────────────────────────────────────────
echo.
echo Stopping any running instances...
taskkill /F /IM wz_gamepad.exe >nul 2>&1
timeout /t 1 /nobreak >nul

:: ── Remove Start Menu shortcut ────────────────────────────────────────────
set "START_MENU=%ProgramData%\Microsoft\Windows\Start Menu\Programs\WZ Virtual Gamepad"
if exist "%START_MENU%" (
    rmdir /S /Q "%START_MENU%" >nul 2>&1
    echo [OK] Removed Start Menu shortcut.
)

:: ── Remove Desktop shortcut ───────────────────────────────────────────────
set "DESKTOP_LINK=%USERPROFILE%\Desktop\WZ Virtual Gamepad.lnk"
if exist "%DESKTOP_LINK%" (
    del /F /Q "%DESKTOP_LINK%" >nul 2>&1
    echo [OK] Removed Desktop shortcut.
)

:: Also check Public Desktop
set "PUBLIC_LINK=%PUBLIC%\Desktop\WZ Virtual Gamepad.lnk"
if exist "%PUBLIC_LINK%" (
    del /F /Q "%PUBLIC_LINK%" >nul 2>&1
)

:: ── Remove Firewall rule ──────────────────────────────────────────────────
netsh advfirewall firewall delete rule name="WZ Virtual Gamepad" >nul 2>&1

:: ── Remove installation directory ────────────────────────────────────────
if exist "%INSTALL_DIR%" (
    rmdir /S /Q "%INSTALL_DIR%" >nul 2>&1
    if !errorLevel! equ 0 (
        echo [OK] Removed installation directory.
    ) else (
        echo WARNING: Could not fully remove %INSTALL_DIR%
        echo          Some files may be in use. Please reboot and delete manually.
    )
)

:: ── Remove registry uninstall entry ──────────────────────────────────────
reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" /f >nul 2>&1
echo [OK] Removed registry entries.

:: ── Remove user configuration (optional) ─────────────────────────────────
set "USER_CFG=%APPDATA%\WZVirtualGamepad"
if exist "%USER_CFG%" (
    set /p REMOVE_CFG=Remove user configuration and profiles? (Y/N): 
    if /i "!REMOVE_CFG!" equ "Y" (
        rmdir /S /Q "%USER_CFG%" >nul 2>&1
        echo [OK] Removed user configuration.
    ) else (
        echo User configuration kept at: %USER_CFG%
    )
)

:: ── ViGEmBus note ─────────────────────────────────────────────────────────
echo.
echo NOTE: The ViGEmBus driver has NOT been uninstalled.
echo       If you want to remove it, use:
echo       winget uninstall --id nefarius.ViGEmBus
echo       or use "Add or Remove Programs" in Windows Settings.

echo.
echo ============================================================
echo   WZ Virtual Gamepad has been successfully uninstalled.
echo ============================================================
echo.
pause
endlocal
