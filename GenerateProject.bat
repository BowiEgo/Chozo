@echo off
setlocal enabledelayedexpansion

REM Ensure xmake is available
xmake --version >nul 2>nul
if %errorlevel% neq 0 (
    echo [Error] xmake command not found.
    echo [Log] Please run setup.bat first or restart your CMD window.
    pause
    exit /b 1
)

:MENU
cls
echo ================================================================
echo   Chozo Engine Project Generator
echo ================================================================
echo   1. Clean Project
echo   2. Generate VS Project     (vsxmake - Recommended)
echo   3. Generate VSCode Project (compile_commands)
echo   4. Generate CLion Project  (xmake-config)
echo   0. Exit
echo ================================================================
set /p M=Select option (0-4):

if "%M%"=="1" goto CLEAN
if "%M%"=="2" goto VS
if "%M%"=="3" goto VSCODE
if "%M%"=="4" goto CLION
if "%M%"=="0" goto EXIT
goto MENU

:CLEAN
echo [Log] Cleaning...
xmake clean -a -v
if exist vsxmake rd /s /q vsxmake
pause
goto MENU

:VS
echo [Log] Generating VS2022 project...
xmake f -p windows -a x64 -m debug -y
xmake project -k vsxmake
echo [Log] Done. Open ./vsxmake/ChozoEngine.sln
pause
goto MENU

:VSCODE
xmake project -k compile_commands
echo [Log] compile_commands.json generated.
pause
goto MENU

:CLION
xmake f -y
echo [Log] Environment refreshed for CLion plugin.
pause
goto MENU

:EXIT
exit /b 0