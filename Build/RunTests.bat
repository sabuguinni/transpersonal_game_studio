@echo off
setlocal enabledelayedexpansion

REM Transpersonal Game Studio - Automated Testing Script
REM Usage: RunTests.bat [TestSuite] [Platform]
REM Example: RunTests.bat All Windows

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set PROJECT_FILE=%PROJECT_ROOT%\TranspersonalGame.uproject

REM Default parameters
set TEST_SUITE=%1
if "%TEST_SUITE%"=="" set TEST_SUITE=All

set TEST_PLATFORM=%2
if "%TEST_PLATFORM%"=="" set TEST_PLATFORM=Windows

REM Find UE5 installation
set UE5_PATH=
for /f "tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\5.3" /v "InstalledDirectory" 2^>nul') do set UE5_PATH=%%b
if "%UE5_PATH%"=="" (
    echo ERROR: Unreal Engine 5.3 not found in registry
    exit /b 1
)

set EDITOR_CMD=%UE5_PATH%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe
set UAT_PATH=%UE5_PATH%\Engine\Build\BatchFiles\RunUAT.bat

echo ========================================
echo Transpersonal Game Studio - Run Tests
echo ========================================
echo Project: %PROJECT_FILE%
echo Test Suite: %TEST_SUITE%
echo Platform: %TEST_PLATFORM%
echo ========================================

REM Create test results directory
set RESULTS_DIR=%PROJECT_ROOT%\TestResults\%DATE:~-4,4%-%DATE:~-10,2%-%DATE:~-7,2%_%TIME:~0,2%-%TIME:~3,2%-%TIME:~6,2%
set RESULTS_DIR=%RESULTS_DIR: =0%
if not exist "%RESULTS_DIR%" mkdir "%RESULTS_DIR%"

echo Test results will be saved to: %RESULTS_DIR%

REM Run unit tests
echo Running unit tests...
"%EDITOR_CMD%" "%PROJECT_FILE%" -ExecCmds="Automation RunTests TranspersonalGame; Quit" -TestExit="Automation Test Queue Empty" -ReportOutputPath="%RESULTS_DIR%\UnitTests.json" -log -unattended -nopause -nullrhi

if %ERRORLEVEL% neq 0 (
    echo WARNING: Unit tests completed with warnings or errors
)

REM Run functional tests if available
if exist "%PROJECT_ROOT%\Content\Tests" (
    echo Running functional tests...
    "%EDITOR_CMD%" "%PROJECT_FILE%" -ExecCmds="Automation RunTests Functional; Quit" -TestExit="Automation Test Queue Empty" -ReportOutputPath="%RESULTS_DIR%\FunctionalTests.json" -log -unattended -nopause -nullrhi
    
    if !ERRORLEVEL! neq 0 (
        echo WARNING: Functional tests completed with warnings or errors
    )
)

REM Run performance tests for non-debug builds
if not "%TEST_SUITE%"=="Debug" (
    echo Running performance benchmarks...
    "%EDITOR_CMD%" "%PROJECT_FILE%" -ExecCmds="Automation RunTests Performance; Quit" -TestExit="Automation Test Queue Empty" -ReportOutputPath="%RESULTS_DIR%\PerformanceTests.json" -log -unattended -nopause -nullrhi
    
    if !ERRORLEVEL! neq 0 (
        echo WARNING: Performance tests completed with warnings or errors
    )
)

REM Generate test summary
echo Generating test summary...
set SUMMARY_FILE=%RESULTS_DIR%\TestSummary.txt
echo Transpersonal Game Studio - Test Summary > "%SUMMARY_FILE%"
echo ======================================== >> "%SUMMARY_FILE%"
echo Test Date: %DATE% %TIME% >> "%SUMMARY_FILE%"
echo Test Suite: %TEST_SUITE% >> "%SUMMARY_FILE%"
echo Platform: %TEST_PLATFORM% >> "%SUMMARY_FILE%"
echo Project: TranspersonalGame >> "%SUMMARY_FILE%"
echo Cycle: PROD_JURASSIC_001 >> "%SUMMARY_FILE%"
echo ======================================== >> "%SUMMARY_FILE%"
echo. >> "%SUMMARY_FILE%"
echo Test results saved to: >> "%SUMMARY_FILE%"
echo %RESULTS_DIR% >> "%SUMMARY_FILE%"

echo ========================================
echo Testing completed!
echo Results saved to: %RESULTS_DIR%
echo ========================================

endlocal