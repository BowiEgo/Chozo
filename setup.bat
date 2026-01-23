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

echo [Log] Environment setup complete.
pause