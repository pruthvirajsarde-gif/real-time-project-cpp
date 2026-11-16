# SmartHire — Professional Web Frontend (Phase 10)

SmartHire Web is a fast, responsive, and modern Single Page Application (SPA) built with **React**, **Vite**, and **Vanilla CSS**. It provides an intuitive, professional interface for candidates and recruiters, consuming the local C++17 REST API backend.

---

## 1. System Requirements

- **Node.js**: `v18.0.0` or higher (Tested and verified on `v24.12.0`)
- **Package Manager**: `npm` (v10+ recommended)
- **Modern Browser**: Chrome, Edge, Firefox, Safari (supports ES2022+ and Fetch API)
- **Backend**: SmartHire Phase 9 C++ REST Server (`SmartHire.exe --server`) running on `http://127.0.0.1:8080`

---

## 2. Technology Stack

- **Core**: React 19 (`react`, `react-dom`)
- **Tooling & Bundler**: Vite 8 (`vite`, `@vitejs/plugin-react`)
- **Routing**: React Router 7 (`react-router-dom`)
- **Styling**: Modern Vanilla CSS Design System with dark-mode glassmorphic aesthetics, HSL color tokens, micro-animations, and responsive breakpoints
- **Testing**: Vitest (`vitest`) for zero-dependency unit and contract verification
- **Icons**: Lucide React (`lucide-react`)

---

## 3. Architecture & Security Model

```text
Browser Client (React + Vite)
      │
      │ HTTP / JSON / Multipart
      ▼
C++ REST Server (cpp-httplib @ 127.0.0.1:8080)
      │
      ▼
SmartHire Core (NLP / Matcher / Auth / Audit)
      │
      ▼
SQLite Database (smarthire.db)
```

- **Authentication**: Bearer tokens are acquired via `POST /api/v1/auth/login` and kept in browser `sessionStorage`.
- **Zero Secrets**: Passwords, password hashes, PBKDF2 salts, and server keys are never stored in client browser storage.
- **Client Route Protection**: `ProtectedRoute` validates role ownership (`CANDIDATE` vs `RECRUITER`) as a UX presentation layer, while all backend endpoints independently enforce role-based access control (RBAC).
- **Session Expiration**: Automatic 401 interceptor clears expired tokens and redirects the user to `/login` with an informational notice.

---

## 4. Environment Variables

Create or modify `.env` in the `frontend/` directory:

```env
# Base URL pointing to the C++ REST API Server
VITE_API_BASE_URL=http://127.0.0.1:8080/api/v1
```

If not specified, the client defaults to `http://127.0.0.1:8080/api/v1`.

---

## 5. Getting Started & Installation

### Step 1: Start the C++ Backend REST Server
In the root directory of the project:

```bash
# Start backend in HTTP REST server mode
./build/SmartHire.exe --server
```
*Expected Server Output:*
```text
[SmartHire Server] Listening on http://127.0.0.1:8080
[SmartHire Server] CORS origin allowed: http://localhost:5173
```

### Step 2: Install Frontend Dependencies
In the `frontend/` directory:

```bash
cd frontend
npm install
```

### Step 3: Run the Development Server
```bash
npm run dev
```

*Expected Terminal Output:*
```text
  VITE v8.3.1  ready in 180 ms

  ➜  Local:   http://localhost:5173/
  ➜  Network: use --host to expose
```

Open `http://localhost:5173/` in your browser.

---

## 6. Production Build & Preview

To compile the optimized, production-ready static assets:

```bash
npm run build
```

To preview the built production bundle locally:

```bash
npm run preview
```

---

## 7. Automated Testing

Run the automated Vitest test suite:

```bash
npm test
```

### Test Coverage:
- **`authAndUtils.test.js`**: Token storage, cached user management, score color assignments, status badges, date formatting, email validation, and password strength regex.
- **`apiClientAndServices.test.js`**: Bearer token injection, standardized envelope parsing (`{ success: true, data }`), standardized error handling (`{ success: false, error }`), 401 session expiration handling, offline server helper message, and all 9 endpoint services.
- **`workflowAndRouting.test.js`**: Role-based access control matrix, candidate/recruiter route boundaries, and full E2E journey sequence progression.

---

## 8. Application Routes & Portals

### Public Routes
- `/`: Modern Landing Page highlighting Intelligent Matching, NLP Analysis, and Platform Capabilities.
- `/login`: Secure Login with masked password and role detection.
- `/register`: Tabbed Registration for Candidates and Recruiters.
- `/jobs`: Public Job Board with live search and filtering.
- `/jobs/:id`: Detailed Job Requisition view.

### Candidate Portal (`/candidate/*`)
- `/candidate`: Overview Dashboard (Resume Quality, Applications, Saved Jobs, Best Match).
- `/candidate/resume`: Resume Document Viewer with `.txt`/`.pdf` drag-and-drop upload.
- `/candidate/resume-analysis`: Detailed Quality Scoring (Profile, Skills, Experience, Education, Projects).
- `/candidate/jobs`: Job Search with keyword, company, location, and skill filters.
- `/candidate/jobs/:id`: Job details with Apply and Save actions.
- `/candidate/matches/:id`: Advanced Explainable Match Analysis (Exact Skills, Related Skills, Missing Required/Preferred, Recommendations).
- `/candidate/saved-jobs`: Bookmarked Job Requisitions.
- `/candidate/applications`: Application Pipeline tracking with stage progress bar and withdrawal option.
- `/candidate/profile`: View and update Candidate Profile.
- `/candidate/activity`: Audited activity log.
- `/candidate/settings`: Password change and live server diagnostic health check.

### Recruiter Console (`/recruiter/*`)
- `/recruiter`: Hiring Dashboard (Jobs posted, applications received, pipeline stage breakdown).
- `/recruiter/jobs`: Requisition management table with Create, Edit, and Delete actions.
- `/recruiter/jobs/new`: Job Posting Form.
- `/recruiter/applicants`: Applicant pipeline with match score review, candidate cards, and status stage transitions.
- `/recruiter/activity`: Recruiter operational audit log.
- `/recruiter/settings`: Recruiter account settings and backend health diagnostics.

---

## 9. Troubleshooting

| Issue | Cause | Solution |
| :--- | :--- | :--- |
| **"SmartHire server is unavailable"** | C++ REST server is not running | Start the backend executable using `./build/SmartHire.exe --server` |
| **CORS Error in Browser Console** | Backend `server.conf` origin mismatch | Verify `config/server.conf` contains `cors_origin=http://localhost:5173` |
| **401 Unauthorized / Redirection to Login** | Session timed out (30m inactivity limit) | Log in again at `/login` to acquire a fresh Bearer token |
| **Upload Rejected (HTTP 400)** | Unsupported file type or size > 10MB | Upload only `.pdf` or `.txt` resumes under 10MB |
| **Duplicate Application (HTTP 409)** | You already applied for this position | Each candidate can only apply once per job ID |
