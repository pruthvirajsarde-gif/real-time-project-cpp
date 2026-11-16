#!/usr/bin/env bash
# SmartHire Development Environment Launcher for Linux / macOS

set -e

echo "============================================================"
echo "          SmartHire Intelligent Recruitment Platform        "
echo "                 Development Launcher (Phase 11)            "
echo "============================================================"
echo

if [ ! -f "build/SmartHire" ] && [ ! -f "build/SmartHire.exe" ]; then
    echo "[ERROR] Backend binary not found. Build the project first:"
    echo "        cmake -B build && cmake --build build"
    exit 1
fi

if ! command -v node &> /dev/null; then
    echo "[ERROR] Node.js is not installed or not in PATH."
    exit 1
fi

mkdir -p data/uploads

echo "[STARTING] Launching SmartHire Backend Server (port 8080)..."
if [ -f "build/SmartHire.exe" ]; then
    ./build/SmartHire.exe --server &
else
    ./build/SmartHire --server &
fi
BACKEND_PID=$!

sleep 2

echo "[STARTING] Launching React Frontend (port 5173)..."
cd frontend
npm run dev &
FRONTEND_PID=$!
cd ..

echo
echo "============================================================"
echo "   SmartHire Backend API:  http://127.0.0.1:8080/api/v1"
echo "   SmartHire Health Check: http://127.0.0.1:8080/api/v1/health"
echo "   SmartHire Web Frontend: http://localhost:5173/"
echo "============================================================"
echo "Press Ctrl+C to terminate both servers."

trap "kill $BACKEND_PID $FRONTEND_PID 2>/dev/null || true" EXIT INT TERM
wait
