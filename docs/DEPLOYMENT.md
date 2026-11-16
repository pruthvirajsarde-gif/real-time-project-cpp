# SmartHire — Deployment & Operations Guide

**Version:** 11.0 (Production Ready)  
**Target Environment:** Local Workstations, On-Premises Servers & Intranet Hosts  
**Architecture:** Decoupled C++17 REST Backend + React 19 SPA Frontend  

---

## 1. System Requirements & Prerequisites

### 1.1 Hardware Specifications
| Component | Minimum Specification | Recommended Specification |
| :--- | :--- | :--- |
| **Processor** | Dual-core x86_64 CPU | Quad-core x86_64 CPU or higher |
| **Memory (RAM)** | 2 GB RAM | 4 GB RAM |
| **Disk Storage** | 500 MB free space | 2 GB free space (for resume uploads and DB growth) |
| **Operating System** | Windows 10/11, Windows Server, or Linux | Windows 11 or Linux x86_64 |

### 1.2 Software Toolchain
- **C++ Compiler:** MinGW-w64 GCC 13+ (bundled in `build_tools/w64devkit`) or Clang / MSVC with C++17 support.
- **Build System:** CMake 3.16 or later.
- **Node.js Environment:** Node.js 18.x or later with `npm` 9.x+.
- **Database Engine:** Embedded SQLite 3.x (no separate database server installation needed).

---

## 2. Configuration Management

SmartHire uses a hierarchical configuration strategy:
1. **Command-Line Arguments** (highest priority)
2. **Environment Variables**
3. **Configuration File** (`config/server.conf`)
4. **Compiled Fallback Defaults** (lowest priority)

### 2.1 Backend Server Configuration (`config/server.conf`)

```ini
# ===================================================================
# SmartHire Server Configuration
# ===================================================================

# Network Binding
host = 127.0.0.1
port = 8080

# Storage Paths
database_path = data/smarthire.db
upload_directory = data/uploads

# Security & Sessions
session_timeout = 86400
max_upload_mb = 10
cors_origin = http://localhost:5173

# Rate Limiting (Sliding Window on Auth Routes)
rate_limit_max_requests = 10
rate_limit_window_seconds = 60
```

### 2.2 Environment Variable Overrides
In containerized or automated deployment environments, specify configuration via environment variables:
- `SMARTHIRE_HOST`: Binds the backend socket (e.g., `0.0.0.0` or `127.0.0.1`).
- `SMARTHIRE_PORT`: Listening port (e.g., `8080`).
- `SMARTHIRE_DB_PATH`: Absolute or relative path to SQLite database file.
- `SMARTHIRE_UPLOAD_DIR`: Absolute or relative path to resume upload directory.
- `SMARTHIRE_CORS_ORIGIN`: Allowed HTTP origin header for browser CORS checks.

### 2.3 Frontend Environment Configuration (`frontend/.env`)
Copy `frontend/.env.example` to `frontend/.env`:
```ini
# SmartHire Frontend Environment Configuration
VITE_API_BASE_URL=http://127.0.0.1:8080/api/v1
```
For production deployments where the backend is hosted on a domain, set `VITE_API_BASE_URL=https://api.smarthire.internal/api/v1`.

---

## 3. Production Build Procedure

To compile and package the entire SmartHire application from source:

### 3.1 One-Click Windows Build (`build-all.bat`)
Run the unified build script from the repository root:
```cmd
build-all.bat
```
This script automatically:
1. Configures CMake in Release mode using the bundled toolchain.
2. Compiles `SmartHire.exe` and `test_runner.exe`.
3. Runs the 492 backend test assertions.
4. Installs frontend dependencies and executes `npm run build`.
5. Emits production assets into `frontend/dist/`.

### 3.2 Manual Step-by-Step Build

#### Step 1: Compile Backend
```powershell
$env:PATH = "$PWD\build_tools\w64devkit\bin;" + $env:PATH
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

#### Step 2: Verify Backend Regression
```powershell
cd build
.\test_runner.exe
cd ..
```
Verify all 492 assertions pass before continuing.

#### Step 3: Build Frontend SPA
```powershell
cd frontend
npm install
npm run build
cd ..
```
Production bundles will be compiled to `frontend/dist/`.

---

## 4. Local & Intranet Deployment Workflows

### 4.1 Development & Demonstration Startup (`start-dev.bat`)
To start both the C++ REST backend and the Vite development server with one command:
```cmd
start-dev.bat
```
This opens two independent console processes and displays the service endpoints:
- **Backend REST API:** `http://127.0.0.1:8080`
- **Frontend Web UI:** `http://localhost:5173`

### 4.2 Production-Grade Intranet Deployment
For production deployment on an intranet server or workstation:
1. **Start Backend Server:**
   ```powershell
   .\build\SmartHire.exe --server --config config/server.conf
   ```
2. **Serve Frontend Static Assets:**
   Serve `frontend/dist/` using any standard HTTP server (e.g. Nginx, Caddy, or Python):
   ```powershell
   cd frontend/dist
   python -m http.server 3000
   ```
   Or configure Nginx with an SPA fallback:
   ```nginx
   server {
       listen 80;
       server_name smarthire.local;
       root /opt/smarthire/frontend/dist;
       index index.html;
       location / {
           try_files $uri $uri/ /index.html;
       }
       location /api/ {
           proxy_pass http://127.0.0.1:8080/api/;
           proxy_set_header Host $host;
           proxy_set_header X-Real-IP $remote_addr;
       }
   }
   ```

---

## 5. Health Monitoring & Operational Probes

SmartHire provides standard Kubernetes/systemd-compatible diagnostic probes:

| Probe | Endpoint | Expected Status | Function |
| :--- | :--- | :--- | :--- |
| **Liveness** | `GET /api/v1/health/live` | `200 OK` | Confirms the C++ process is alive and responding. |
| **Readiness** | `GET /api/v1/health/ready` | `200 OK` (or `503`) | Executes active SQLite query and verifies upload folder write access. |
| **Health** | `GET /api/v1/health` | `200 OK` | Returns detailed uptime, database status, and subsystem health. |

Example Readiness Probe verification:
```powershell
curl -i http://127.0.0.1:8080/api/v1/health/ready
```
Output:
```http
HTTP/1.1 200 OK
Content-Type: application/json; charset=utf-8

{
  "success": true,
  "message": "System is ready to accept traffic.",
  "data": {
    "database": "OK",
    "status": "READY",
    "uploads": "OK"
  }
}
```

---

## 6. Backup, Restoration & Disaster Recovery

### 6.1 Database Backup
SmartHire persists all structured data in a single SQLite database (`data/smarthire.db`).
To perform an online safe backup while the server is running:
```powershell
# Using the SQLite CLI backup API
sqlite3 data/smarthire.db ".backup 'data/smarthire_backup.db'"
```
Or stop the backend server and perform a direct file copy of `data/smarthire.db`.

### 6.2 Upload Storage Backup
Archival copies of candidate resume uploads can be preserved by backing up the `data/uploads/` directory:
```powershell
Compress-Archive -Path data/uploads/* -DestinationPath uploads_backup.zip
```
