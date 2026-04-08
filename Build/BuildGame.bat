@echo off
REM Copyright Transpersonal Game Studio. All Rights Reserved.
REM Build script for Transpersonal Game

setlocal enabledelayedexpansion

REM Configuration
set PROJECT_NAME=TranspersonalGame
set ENGINE_PATH=%UE5_ROOT%
set PROJECT_PATH=%~dp0..
set BUILD_CONFIG=%1

REM Default to Development if no config specified
if "%BUILD_CONFIG%"=="" set BUILD_CONFIG=Development

REM Validate engine path
if not exist "%ENGINE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" (
    echo ERROR: Unreal Engine not found at %ENGINE_PATH%
    echo Please set UE5_ROOT environment variable to your UE5 installation
    exit /b 1
)

echo ========================================
echo Building %PROJECT_NAME% - %BUILD_CONFIG%
echo ========================================
echo Project Path: %PROJECT_PATH%
echo Engine Path: %ENGINE_PATH%
echo.

REM Generate project files if they don't exist
if not exist "%PROJECT_PATH%\%PROJECT_NAME%.sln" (
    echo Generating project files...
    "%ENGINE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="%PROJECT_PATH%\%PROJECT_NAME%.uproject" -game -rocket -progress
    if !errorlevel! neq 0 (
        echo ERROR: Failed to generate project files
        exit /b !errorlevel!
    )
)

REM Build the game
echo Building game...
"%ENGINE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" %PROJECT_NAME% Win64 %BUILD_CONFIG% -project="%PROJECT_PATH%\%PROJECT_NAME%.uproject" -progress -NoHotReloadFromIDE

if %errorlevel% neq 0 (
    echo ERROR: Build failed with error code %errorlevel%
    exit /b %errorlevel%
)

echo.
echo ========================================
echo Build completed successfully!
echo Configuration: %BUILD_CONFIG%
echo ========================================

REM Optional: Run basic validation
if "%2"=="validate" (
    echo Running build validation...
    REM Add validation steps here
)

endlocal
exit /b 0