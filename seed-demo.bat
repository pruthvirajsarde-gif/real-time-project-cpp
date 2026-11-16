@echo off
title SmartHire Demo Data Seeder
echo ============================================================
echo          Seeding SmartHire Presentation Demo Data
echo ============================================================
if not exist "build\SmartHire.exe" (
    echo [ERROR] build\SmartHire.exe not found. Build the project first.
    pause
    exit /b 1
)
.\build\SmartHire.exe --seed-demo
echo.
pause
