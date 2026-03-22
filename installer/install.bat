@echo off
setlocal EnableDelayedExpansion

echo ============================================================
echo   WZ Virtual Gamepad - Installer
echo ============================================================
echo.

:: Check for administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator.
    echo Right-click install.bat and select "Run as administrator".
    pause
    exit /b 1
)

:: Set installation directory
set "INSTALL_DIR=%ProgramFiles%\WZVirtualGamepad"
set "SCRIPT_DIR=%~dp0"

echo Installation directory: %INSTALL_DIR%
echo.

:: Create installation directory
if not exist "%INSTALL_DIR%" (
    mkdir "%INSTALL_DIR%"
    if !errorLevel! neq 0 (
        echo ERROR: Failed to create installation directory.
        pause
        exit /b 1
    )
)

:: Copy executable
if exist "%SCRIPT_DIR%wz_gamepad.exe" (
    copy /Y "%SCRIPT_DIR%wz_gamepad.exe" "%INSTALL_DIR%\" >nul
    echo [OK] Copied wz_gamepad.exe
) else (
    echo WARNING: wz_gamepad.exe not found in %SCRIPT_DIR%
    echo          Please build the project first or run from the bin directory.
)

:: Copy profiles
if not exist "%INSTALL_DIR%\profiles" mkdir "%INSTALL_DIR%\profiles"
if exist "%SCRIPT_DIR%profiles\" (
    xcopy /Y /Q "%SCRIPT_DIR%profiles\*" "%INSTALL_DIR%\profiles\" >nul
    echo [OK] Copied profiles
)

:: Copy documentation
if not exist "%INSTALL_DIR%\docs" mkdir "%INSTALL_DIR%\docs"
if exist "%SCRIPT_DIR%..\docs\" (
    xcopy /Y /Q "%SCRIPT_DIR%..\docs\*" "%INSTALL_DIR%\docs\" >nul
    echo [OK] Copied documentation
)

:: ── ViGEmBus Driver Installation ─────────────────────────────────────────
echo.
echo Checking for ViGEmBus driver...

:: Check if ViGEmBus is already installed
reg query "HKLM\SYSTEM\CurrentControlSet\Services\ViGEmBus" >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] ViGEmBus driver is already installed.
) else (
    echo ViGEmBus driver not found. Attempting to download and install...
    echo.

    :: Check if winget is available
    where winget >nul 2>&1
    if %errorLevel% equ 0 (
        echo Using winget to install ViGEmBus...
        winget install --id nefarius.ViGEmBus --accept-source-agreements --accept-package-agreements
        if !errorLevel! neq 0 (
            echo WARNING: winget install failed. Please install ViGEmBus manually.
            echo          Download from: https://github.com/nefarius/ViGEmBus/releases/latest
        ) else (
            echo [OK] ViGEmBus installed via winget.
        )
    ) else (
        echo winget not available. Please install ViGEmBus manually:
        echo   https://github.com/nefarius/ViGEmBus/releases/latest
        echo   Download and run ViGEmBus_Setup_x64.exe
    )
)

:: ── Create Start Menu Shortcut ────────────────────────────────────────────
echo.
echo Creating Start Menu shortcut...
set "START_MENU=%ProgramData%\Microsoft\Windows\Start Menu\Programs\WZ Virtual Gamepad"
if not exist "%START_MENU%" mkdir "%START_MENU%"

if exist "%INSTALL_DIR%\wz_gamepad.exe" (
    powershell -Command ^
        "$ws = New-Object -ComObject WScript.Shell; ^
         $sc = $ws.CreateShortcut('%START_MENU%\WZ Virtual Gamepad.lnk'); ^
         $sc.TargetPath = '%INSTALL_DIR%\wz_gamepad.exe'; ^
         $sc.WorkingDirectory = '%INSTALL_DIR%'; ^
         $sc.Description = 'WZ Virtual Gamepad - KB/Mouse to Controller Mapper'; ^
         $sc.Save()" >nul 2>&1
    if !errorLevel! equ 0 (
        echo [OK] Start Menu shortcut created.
    ) else (
        echo WARNING: Could not create Start Menu shortcut.
    )
)

:: ── Create Desktop Shortcut ───────────────────────────────────────────────
if exist "%INSTALL_DIR%\wz_gamepad.exe" (
    powershell -Command ^
        "$ws = New-Object -ComObject WScript.Shell; ^
         $sc = $ws.CreateShortcut([Environment]::GetFolderPath('Desktop') + '\WZ Virtual Gamepad.lnk'); ^
         $sc.TargetPath = '%INSTALL_DIR%\wz_gamepad.exe'; ^
         $sc.WorkingDirectory = '%INSTALL_DIR%'; ^
         $sc.Description = 'WZ Virtual Gamepad - KB/Mouse to Controller Mapper'; ^
         $sc.Save()" >nul 2>&1
    if !errorLevel! equ 0 (
        echo [OK] Desktop shortcut created.
    )
)

:: ── Add to Windows Firewall exception (optional) ─────────────────────────
if exist "%INSTALL_DIR%\wz_gamepad.exe" (
    netsh advfirewall firewall add rule name="WZ Virtual Gamepad" ^
        dir=in action=allow program="%INSTALL_DIR%\wz_gamepad.exe" enable=yes >nul 2>&1
)

:: ── Write uninstall registry key ──────────────────────────────────────────
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" ^
    /v "DisplayName"     /t REG_SZ /d "WZ Virtual Gamepad"          /f >nul 2>&1
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" ^
    /v "DisplayVersion"  /t REG_SZ /d "1.0.0"                        /f >nul 2>&1
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" ^
    /v "Publisher"       /t REG_SZ /d "WZ Virtual Gamepad Project"   /f >nul 2>&1
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" ^
    /v "InstallLocation" /t REG_SZ /d "%INSTALL_DIR%"                /f >nul 2>&1
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WZVirtualGamepad" ^
    /v "UninstallString" /t REG_SZ /d "%INSTALL_DIR%\uninstall.bat"  /f >nul 2>&1

:: Copy uninstaller
if exist "%SCRIPT_DIR%uninstall.bat" (
    copy /Y "%SCRIPT_DIR%uninstall.bat" "%INSTALL_DIR%\" >nul
)

echo.
echo ============================================================
echo   Installation complete!
echo.
echo   WZ Virtual Gamepad has been installed to:
echo   %INSTALL_DIR%
echo.
echo   IMPORTANT: ViGEmBus driver must be installed for the
echo   virtual gamepad to function. If not already installed,
echo   download from:
echo   https://github.com/nefarius/ViGEmBus/releases/latest
echo.
echo   Launch the application from the Start Menu or Desktop
echo   shortcut, or run:
echo   "%INSTALL_DIR%\wz_gamepad.exe"
echo ============================================================
echo.
pause
endlocal
