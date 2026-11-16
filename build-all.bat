@echo off
setlocal
title SmartHire Unified Build System
echo ============================================================
echo       Building Complete SmartHire Production Application
echo ============================================================
echo.
echo [1/3] Configuring C++ Backend with CMake...
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo [FAILED] CMake configuration failed.
    pause
    exit /b 1
)

echo.
echo [2/3] Compiling C++ Executable and Test Suite...
cmake --build build -j4
if %errorlevel% neq 0 (
    echo [FAILED] C++ build failed.
    pause
    exit /b 1
)

echo.
echo [3/3] Compiling Production Frontend Bundle (Vite)...
cd frontend
call npm install
call npm run build
cd ..
if %errorlevel% neq 0 (
    echo [FAILED] Frontend build failed.
    pause
    exit /b 1
)

echo.
echo ============================================================
echo [SUCCESS] Complete SmartHire Build Succeeded!
echo   Backend:  build\SmartHire.exe
echo   Tests:    build\test_runner.exe
echo   Frontend: frontend\dist\
echo ============================================================
pause
