@echo off
REM Quick Build Script for Transpersonal Game Studio
REM Integration & Build Agent (#19)
REM Cycle: PROD_JURASSIC_001

echo ========================================
echo Transpersonal Game Studio - Quick Build
echo Cycle: PROD_JURASSIC_001
echo ========================================

REM Set environment variables
set PROJECT_NAME=TranspersonalGame
set ENGINE_PATH=%~dp0..\..\Engine
set PROJECT_PATH=%~dp0..\..\%PROJECT_NAME%.uproject
set BUILD_CONFIG=Development
set TARGET_PLATFORM=Win64

echo.
echo [INFO] Starting Quick Build Process...
echo [INFO] Project: %PROJECT_NAME%
echo [INFO] Configuration: %BUILD_CONFIG%
echo [INFO] Platform: %TARGET_PLATFORM%
echo.

REM Check if Unreal Engine is available
if not exist "%ENGINE_PATH%\Binaries\Win64\UnrealBuildTool.exe" (
    echo [ERROR] Unreal Engine not found at: %ENGINE_PATH%
    echo [ERROR] Please verify engine installation
    pause
    exit /b 1
)

REM Check if project file exists
if not exist "%PROJECT_PATH%" (
    echo [ERROR] Project file not found: %PROJECT_PATH%
    echo [ERROR] Please verify project path
    pause
    exit /b 1
)

echo [INFO] Building game modules...
"%ENGINE_PATH%\Binaries\Win64\UnrealBuildTool.exe" %PROJECT_NAME% %TARGET_PLATFORM% %BUILD_CONFIG% -project="%PROJECT_PATH%" -rocket -progress

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed with error code: %ERRORLEVEL%
    echo [ERROR] Check build logs for details
    pause
    exit /b %ERRORLEVEL%
)

echo [INFO] Building editor modules...
"%ENGINE_PATH%\Binaries\Win64\UnrealBuildTool.exe" %PROJECT_NAME%Editor %TARGET_PLATFORM% %BUILD_CONFIG% -project="%PROJECT_PATH%" -rocket -progress

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Editor build failed with error code: %ERRORLEVEL%
    echo [ERROR] Check build logs for details
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ========================================
echo [SUCCESS] Quick Build Completed!
echo ========================================
echo.
echo Next steps:
echo 1. Run FullBuild.bat for complete build with tests
echo 2. Use DeployTest.bat to deploy to test environment
echo 3. Check Build\Logs\ for detailed build information
echo.

pause