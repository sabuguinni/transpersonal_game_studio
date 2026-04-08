@echo off
setlocal enabledelayedexpansion

REM Transpersonal Game Studio - Game Packaging Script
REM Usage: PackageGame.bat [Platform] [Configuration] [OutputDir]
REM Example: PackageGame.bat Windows Development C:\Builds\TranspersonalGame

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set PROJECT_FILE=%PROJECT_ROOT%\TranspersonalGame.uproject

REM Default parameters
set PACKAGE_PLATFORM=%1
if "%PACKAGE_PLATFORM%"=="" set PACKAGE_PLATFORM=Windows

set PACKAGE_CONFIG=%2
if "%PACKAGE_CONFIG%"=="" set PACKAGE_CONFIG=Development

set OUTPUT_DIR=%3
if "%OUTPUT_DIR%"=="" set OUTPUT_DIR=%PROJECT_ROOT%\Builds\%PACKAGE_PLATFORM%_%PACKAGE_CONFIG%

REM Find UE5 installation
set UE5_PATH=
for /f "tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\5.3" /v "InstalledDirectory" 2^>nul') do set UE5_PATH=%%b
if "%UE5_PATH%"=="" (
    echo ERROR: Unreal Engine 5.3 not found in registry
    exit /b 1
)

set UAT_PATH=%UE5_PATH%\Engine\Build\BatchFiles\RunUAT.bat

echo ========================================
echo Transpersonal Game Studio - Package Game
echo ========================================
echo Project: %PROJECT_FILE%
echo Platform: %PACKAGE_PLATFORM%
echo Configuration: %PACKAGE_CONFIG%
echo Output Directory: %OUTPUT_DIR%
echo ========================================

REM Create output directory
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

REM Package the game using UAT
echo Starting game packaging...
call "%UAT_PATH%" BuildCookRun ^
    -project="%PROJECT_FILE%" ^
    -noP4 ^
    -platform=%PACKAGE_PLATFORM% ^
    -clientconfig=%PACKAGE_CONFIG% ^
    -cook ^
    -build ^
    -stage ^
    -package ^
    -archive ^
    -archivedirectory="%OUTPUT_DIR%" ^
    -pak ^
    -compressed ^
    -prereqs ^
    -nodebuginfo ^
    -utf8output

if %ERRORLEVEL% neq 0 (
    echo ERROR: Game packaging failed with error code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

REM Create version info file
echo Creating version info...
set VERSION_FILE=%OUTPUT_DIR%\version.txt
echo Transpersonal Game Studio > "%VERSION_FILE%"
echo Build Date: %DATE% %TIME% >> "%VERSION_FILE%"
echo Platform: %PACKAGE_PLATFORM% >> "%VERSION_FILE%"
echo Configuration: %PACKAGE_CONFIG% >> "%VERSION_FILE%"
echo Cycle: PROD_JURASSIC_001 >> "%VERSION_FILE%"

echo ========================================
echo Game packaging completed successfully!
echo Platform: %PACKAGE_PLATFORM%
echo Configuration: %PACKAGE_CONFIG%
echo Output: %OUTPUT_DIR%
echo ========================================

endlocal