@echo off
setlocal enabledelayedexpansion

REM Transpersonal Game Studio - Build Script
REM Usage: BuildGame.bat [Configuration] [Platform]
REM Example: BuildGame.bat Development Win64

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set PROJECT_FILE=%PROJECT_ROOT%\TranspersonalGame.uproject

REM Default parameters
set BUILD_CONFIG=%1
if "%BUILD_CONFIG%"=="" set BUILD_CONFIG=Development

set BUILD_PLATFORM=%2
if "%BUILD_PLATFORM%"=="" set BUILD_PLATFORM=Win64

REM Find UE5 installation
set UE5_PATH=
for /f "tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\5.3" /v "InstalledDirectory" 2^>nul') do set UE5_PATH=%%b
if "%UE5_PATH%"=="" (
    echo ERROR: Unreal Engine 5.3 not found in registry
    exit /b 1
)

set UBT_PATH=%UE5_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe

echo ========================================
echo Transpersonal Game Studio - Build System
echo ========================================
echo Project: %PROJECT_FILE%
echo Configuration: %BUILD_CONFIG%
echo Platform: %BUILD_PLATFORM%
echo UE5 Path: %UE5_PATH%
echo ========================================

REM Clean previous build artifacts
echo Cleaning previous build artifacts...
if exist "%PROJECT_ROOT%\Binaries" rmdir /s /q "%PROJECT_ROOT%\Binaries"
if exist "%PROJECT_ROOT%\Intermediate" rmdir /s /q "%PROJECT_ROOT%\Intermediate"

REM Generate project files
echo Generating project files...
"%UE5_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="%PROJECT_FILE%" -game -rocket -progress

REM Build the project
echo Building TranspersonalGame...
"%UBT_PATH%" TranspersonalGame %BUILD_PLATFORM% %BUILD_CONFIG% -project="%PROJECT_FILE%" -progress -NoHotReloadFromIDE

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed with error code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

REM Build the editor target if not shipping
if not "%BUILD_CONFIG%"=="Shipping" (
    echo Building TranspersonalGameEditor...
    "%UBT_PATH%" TranspersonalGameEditor %BUILD_PLATFORM% %BUILD_CONFIG% -project="%PROJECT_FILE%" -progress -NoHotReloadFromIDE
    
    if !ERRORLEVEL! neq 0 (
        echo ERROR: Editor build failed with error code !ERRORLEVEL!
        exit /b !ERRORLEVEL!
    )
)

echo ========================================
echo Build completed successfully!
echo Configuration: %BUILD_CONFIG%
echo Platform: %BUILD_PLATFORM%
echo ========================================

endlocal