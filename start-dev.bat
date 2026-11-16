@echo off
setlocal enabledelayedexpansion
title SmartHire Launcher

echo ============================================================
echo           SmartHire Intelligent Recruitment Platform
echo                  Development Launcher (Phase 11)
echo ============================================================
echo.

:: 1. Check for backend binary
if not exist "build\SmartHire.exe" (
    echo [ERROR] Backend executable build\SmartHire.exe not found!
    echo         Please build the project first by running: build-all.bat
    pause
    exit /b 1
)

:: 2. Check for Node.js
where node >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] Node.js is not found in PATH. Please install Node.js v18+.
    pause
    exit /b 1
)

:: 3. Check for frontend dependencies
if not exist "frontend\node_modules" (
    echo [INFO] Installing frontend dependencies (npm install)...
    cd frontend
    call npm install
    cd ..
)

:: 4. Ensure data directories exist
if not exist "data\uploads" mkdir "data\uploads"

:: 5. Launch Backend Server in new terminal window
echo [STARTING] SmartHire Backend REST Server on http://127.0.0.1:8080...
start "SmartHire Backend Server" cmd /k ".\build\SmartHire.exe --server"

:: Brief pause to let backend initialize
timeout /t 2 /nobreak >nul

:: 6. Launch Frontend Dev Server in new terminal window
echo [STARTING] SmartHire React Frontend on http://localhost:5173...
start "SmartHire Web Frontend" cmd /k "cd frontend && npm run dev"

echo.
echo ============================================================
echo            SmartHire System Started Successfully
echo ============================================================
echo   SmartHire Backend API:  http://127.0.0.1:8080/api/v1
echo   SmartHire Health Check: http://127.0.0.1:8080/api/v1/health
echo   SmartHire Web Frontend: http://localhost:5173/
echo ============================================================
echo.
echo Both services are running in their respective background terminal windows.
echo To seed sample demo data, run: seed-demo.bat
echo Press any key to exit this launcher window.
pause >nul
