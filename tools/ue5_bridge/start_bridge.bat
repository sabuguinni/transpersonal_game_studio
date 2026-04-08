@echo off
title UE5 Bridge - Transpersonal Game Studio
color 0A

echo.
echo ========================================================
echo   UE5 BRIDGE - Transpersonal Game Studio
echo ========================================================
echo.

:: Check if Python is installed
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERRO: Python nao esta instalado ou nao esta no PATH!
    echo.
    echo Vai a https://www.python.org/downloads/ e instala o Python.
    echo IMPORTANTE: Marca a opcao "Add Python to PATH" durante a instalacao!
    echo.
    pause
    exit /b 1
)

:: Check if requests is installed
python -c "import requests" >nul 2>&1
if %errorlevel% neq 0 (
    echo A instalar o modulo 'requests'...
    pip install requests
    echo.
)

:: Run the bridge
echo A iniciar o UE5 Bridge...
echo Certifica-te que o Unreal Engine esta aberto!
echo.
python "%~dp0ue5_bridge.py"

pause
