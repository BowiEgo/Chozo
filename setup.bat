@echo off
REM ================================================================
REM  Chozo Engine Environment Setup Script (Windows)
REM ================================================================

echo [Log] Checking for xmake...

:CHECK
xmake --version >nul 2>nul
if %errorlevel% neq 0 (
    echo [Log] xmake not found. Starting installation...

    REM Install xmake using official script
    powershell -NoProfile -ExecutionPolicy Bypass -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; iwr -useb https://raw.githubusercontent.com/xmake-io/xmake/master/scripts/get.ps1 | iex"

    echo [Log] Refreshing environment variables for current session...

    REM Use a clever trick to refresh PATH from Registry without restarting CMD
    for /f "tokens=2*" %%A in ('reg query "HKLM\System\CurrentControlSet\Control\Session Manager\Environment" /v Path') do set "syspath=%%B"
    for /f "tokens=2*" %%A in ('reg query "HKCU\Environment" /v Path') do set "usrpath=%%B"
    set "PATH=%syspath%;%usrpath%"

    REM Final verification
    xmake --version >nul 2>nul
    if %errorlevel% neq 0 (
        echo [Warning] xmake might be installed but PATH is not updated.
        echo [Log] Please RESTART your CMD window and run GenerateProject.bat
        pause
        exit /b 0
    )
    echo [Log] xmake installed and session PATH updated.
) else (
    echo [Log] xmake is already installed.
)

@REM :: Vulkan SDK Setup
@REM setlocal enabledelayedexpansion

@REM :: 1. check if VULKAN_SDK is already set
@REM if defined VULKAN_SDK (
@REM     echo [Log] Vulkan SDK is already installed at: %VULKAN_SDK%
@REM     goto :EXIT
@REM )

@REM echo [Log] Fetching latest Vulkan SDK version...

@REM :: 2. get latest version number
@REM for /f "delims=" %%i in ('curl -s https://vulkan.lunarg.com/sdk/latest/windows.txt') do set SDK_VERSION=%%i

@REM if "!SDK_VERSION!"=="" (
@REM     echo [ERROR] Could not fetch latest version info.
@REM     pause
@REM     exit /b 1
@REM )

@REM echo [Log] Latest version is: !SDK_VERSION!

@REM :: 3. prepare download URL
@REM set SDK_URL=https://sdk.lunarg.com/sdk/download/latest/windows/vulkan_sdk.exe
@REM set INSTALLER_NAME=vulkan_sdk_installer.exe

@REM :: 4. download the installer
@REM echo [Log] Downloading Vulkan SDK (290MB+)...
@REM curl -L "%SDK_URL%" -o %INSTALLER_NAME% --progress-bar

@REM if %ERRORLEVEL% NEQ 0 (
@REM     echo.
@REM     echo [ERROR] Failed to download Vulkan SDK. 
@REM     echo [TIP] Please check if version %VULKAN_VERSION% still exists or your internet connection.
@REM     echo [TIP] You can manually download it from: https://vulkan.lunarg.com/
@REM     pause
@REM     exit /b %ERRORLEVEL%
@REM )

@REM :: verify file size (should be larger than 1MB)
@REM for %%I in (%INSTALLER_NAME%) do if %%~zI LSS 104857600 (
@REM     echo [ERROR] Downloaded file is incomplete or corrupted.
@REM     del %INSTALLER_NAME%
@REM     pause
@REM     exit /b 1
@REM )

@REM :: 3. silent install
@REM echo [Log] Installing Vulkan SDK (This may take a minute)...
@REM echo [Log] PLEASE NOTE: You may see a UAC prompt.
@REM :: /S for silent install --alldpys to install all components(optional)
@REM start /wait %INSTALLER_NAME% /S

@REM if %ERRORLEVEL% NEQ 0 (
@REM     echo [ERROR] Installer exited with error code: %ERRORLEVEL%
@REM ) else (
@REM     echo [Log] Installation completed successfully!
@REM     echo [Log] Done. Please RESTART your terminal/IDE to apply environment variables.
@REM     del %INSTALLER_NAME%
@REM )

@REM :EXIT
@REM pause
@REM endlocal

echo [Log] Environment setup complete.
pause