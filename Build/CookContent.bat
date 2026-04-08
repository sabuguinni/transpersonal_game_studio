@echo off
setlocal enabledelayedexpansion

REM Transpersonal Game Studio - Content Cooking Script
REM Usage: CookContent.bat [Platform] [Configuration]
REM Example: CookContent.bat Windows Development

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set PROJECT_FILE=%PROJECT_ROOT%\TranspersonalGame.uproject

REM Default parameters
set COOK_PLATFORM=%1
if "%COOK_PLATFORM%"=="" set COOK_PLATFORM=Windows

set COOK_CONFIG=%2
if "%COOK_CONFIG%"=="" set COOK_CONFIG=Development

REM Find UE5 installation
set UE5_PATH=
for /f "tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\5.3" /v "InstalledDirectory" 2^>nul') do set UE5_PATH=%%b
if "%UE5_PATH%"=="" (
    echo ERROR: Unreal Engine 5.3 not found in registry
    exit /b 1
)

set EDITOR_CMD=%UE5_PATH%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe

echo ========================================
echo Transpersonal Game Studio - Cook Content
echo ========================================
echo Project: %PROJECT_FILE%
echo Platform: %COOK_PLATFORM%
echo Configuration: %COOK_CONFIG%
echo ========================================

REM Clean previous cooked content
set COOKED_DIR=%PROJECT_ROOT%\Saved\Cooked\%COOK_PLATFORM%
if exist "%COOKED_DIR%" (
    echo Cleaning previous cooked content...
    rmdir /s /q "%COOKED_DIR%"
)

REM Cook content
echo Starting content cooking...
"%EDITOR_CMD%" "%PROJECT_FILE%" -run=cook -targetplatform=%COOK_PLATFORM% -cookonthefly=false -compressed -iterate -unversioned -stdout -CrashForUAT -unattended -NoLogTimes -UTF8Output

if %ERRORLEVEL% neq 0 (
    echo ERROR: Content cooking failed with error code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo ========================================
echo Content cooking completed successfully!
echo Platform: %COOK_PLATFORM%
echo Output: %COOKED_DIR%
echo ========================================

endlocal