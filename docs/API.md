# SmartHire REST API Reference

**Version:** 11.0 (Production Ready)  
**Base URL:** `http://127.0.0.1:8080/api/v1`  
**Protocol:** HTTP/1.1  
**Format:** JSON (`application/json; charset=utf-8`)

---

## 1. Overview & Architecture

The SmartHire REST API exposes the full offline recruitment intelligence, role-based access control, matching engines, and persistence layers of SmartHire to external HTTP clients (web browsers, mobile apps, automation pipelines).

### Security & Protocol Standards
- **Authentication:** Bearer token authentication via `Authorization: Bearer <token>` HTTP header.
- **Sessions:** 32-byte cryptographically secure random session tokens (64 hex characters) stored as SHA-256 hashes in SQLite.
- **Access Control:** Role-Based Access Control (RBAC) separating `CANDIDATE` and `RECRUITER` privileges, combined with strict resource ownership validation.
- **Rate Limiting:** In-memory sliding window rate limiter (configurable via `config/server.conf`, defaults to 10 requests per minute per IP address on sensitive authentication routes).
- **Security Headers:** Every response emits `X-Content-Type-Options: nosniff`, `X-Frame-Options: DENY`, and appropriate `Cache-Control` directives.
- **CORS Support:** Full support for Cross-Origin Resource Sharing with configurable allowed origins and `OPTIONS` preflight handling.

---

## 2. Standard Response Format

All responses follow a consistent envelope structure.

### 2.1 Success Response (`200 OK`, `201 Created`, `204 No Content`)
```json
{
  "success": true,
  "message": "Human readable status description.",
  "data": { ... }
}
```

### 2.2 Error Response (`400`, `401`, `403`, `404`, `409`, `413`, `422`, `429`, `500`)
```json
{
  "success": false,
  "message": "Descriptive error message.",
  "error": {
    "code": "STANDARD_ERROR_CODE",
    "details": {}
  }
}
```

### Common Error Codes
| Error Code | HTTP Status | Meaning |
| :--- | :--- | :--- |
| `AUTH_MISSING_TOKEN` | 401 | Missing `Authorization` header |
| `AUTH_INVALID_TOKEN` | 401 | Unknown or revoked session token |
| `AUTH_SESSION_EXPIRED` | 401 | Session timestamp exceeded TTL |
| `AUTH_INVALID_CREDENTIALS` | 401 | Username or password incorrect |
| `FORBIDDEN_CANDIDATE_ONLY` | 403 | Recruiter attempted to call candidate route |
| `FORBIDDEN_RECRUITER_ONLY` | 403 | Candidate attempted to call recruiter route |
| `FORBIDDEN_NOT_OWNER` | 403 | Caller does not own the requested resource |
| `NOT_FOUND` | 404 | Resource does not exist |
| `CONFLICT_ALREADY_EXISTS` | 409 | Duplicate username, email, or unique entity |
| `DUPLICATE_APPLICATION` | 409 | Candidate already applied to this job |
| `BAD_REQUEST_MALFORMED_JSON`| 400 | Request body failed JSON parsing |
| `VALIDATION_FAILED` | 400 | Required fields missing or invalid format |
| `VALIDATION_MALICIOUS_PATH` | 400 | Path traversal characters detected |
| `UNSUPPORTED_FILE_TYPE` | 400 | Uploaded file is not `.txt` or `.pdf` |
| `PAYLOAD_TOO_LARGE` | 413 | Multipart body exceeds 10 MB limit |
| `RATE_LIMIT_EXCEEDED` | 429 | Sliding window threshold exceeded |
| `SERVICE_UNAVAILABLE` | 503 | Readiness check failed (database/uploads down) |
| `INTERNAL_ERROR` | 500 | Unhandled server exception |

---

## 3. Endpoints

### 3.0 System & Health Diagnostics

#### `GET /health`
Comprehensive system health diagnostic verifying database connectivity, storage write capability, and server status.
- **Access:** Public
- **Response (`200 OK` or `503 Service Unavailable`):**
```json
{
  "success": true,
  "message": "System is healthy.",
  "data": {
    "application": "SmartHire",
    "version": "11.0",
    "status": "UP",
    "database": "CONNECTED",
    "uploads": "READY",
    "uptime_seconds": 124
  }
}
```

#### `GET /health/live`
Lightweight process liveness probe for orchestrators and container healthchecks.
- **Access:** Public
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Server process is alive.",
  "data": {
    "status": "ALIVE",
    "uptime_seconds": 124
  }
}
```

#### `GET /health/ready`
Deep readiness probe verifying that the SQLite database executes live queries and the upload directory is writable. Returns HTTP 503 if any subsystem fails.
- **Access:** Public
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "System is ready to accept traffic.",
  "data": {
    "status": "READY",
    "database": "OK",
    "uploads": "OK"
  }
}
```

---

### 3.1 Authentication & Session

#### `POST /auth/register/candidate`
Registers a new candidate user account and links their candidate profile.
- **Access:** Public
- **Request Body:**
```json
{
  "name": "Elena Rostova",
  "email": "elena.api@example.com",
  "phone": "555-0199",
  "location": "Boston, MA",
  "username": "elena_rest_cand",
  "password": "SecureP@ssw0rd1"
}
```
- **Response (`201 Created`):**
```json
{
  "success": true,
  "message": "Candidate registered successfully.",
  "data": {
    "accountId": 12,
    "candidateId": 8,
    "role": "CANDIDATE"
  }
}
```

#### `POST /auth/register/recruiter`
Registers a new recruiter user account.
- **Access:** Public
- **Request Body:**
```json
{
  "name": "Marcus Vance",
  "company": "CyberDyne Systems",
  "email": "marcus.api@cyberdyne.com",
  "username": "marcus_rest_rec",
  "password": "RecruiterP@ss123"
}
```
- **Response (`201 Created`):**
```json
{
  "success": true,
  "message": "Recruiter registered successfully.",
  "data": {
    "accountId": 13,
    "role": "RECRUITER"
  }
}
```

#### `POST /auth/login`
Authenticates credentials and establishes a cryptographically secure session.
- **Access:** Public (Rate limited: 10 req/min/IP)
- **Request Body:**
```json
{
  "username": "elena_rest_cand",
  "password": "SecureP@ssw0rd1"
}
```
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Authentication successful.",
  "data": {
    "accountId": 12,
    "username": "elena_rest_cand",
    "role": "CANDIDATE",
    "candidateId": 8,
    "sessionToken": "64_HEX_CHARACTERS_RAW_TOKEN",
    "expiresInMinutes": 30
  }
}
```

#### `POST /auth/logout`
Revokes the caller's active session.
- **Access:** Authenticated
- **Headers:** `Authorization: Bearer <sessionToken>`
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Logged out successfully.",
  "data": {}
}
```

#### `GET /auth/me`
Retrieves information about the current authenticated caller.
- **Access:** Authenticated
- **Headers:** `Authorization: Bearer <sessionToken>`
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Caller profile retrieved.",
  "data": {
    "accountId": 12,
    "username": "elena_rest_cand",
    "email": "elena.api@example.com",
    "role": "CANDIDATE",
    "candidateId": 8
  }
}
```

#### `POST /auth/change-password`
Updates account password and invalidates all existing active sessions.
- **Access:** Authenticated
- **Request Body:**
```json
{
  "currentPassword": "SecureP@ssw0rd1",
  "newPassword": "NewSecureP@ssw0rd2",
  "confirmPassword": "NewSecureP@ssw0rd2"
}
```
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Password updated successfully. Please log in again.",
  "data": {}
}
```

---

### 3.2 Candidate Profile & Resume Management

#### `GET /candidates/me`
Fetches candidate profile details.
- **Access:** Candidate Only
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Candidate profile retrieved.",
  "data": {
    "candidateId": 8,
    "name": "Elena Rostova",
    "email": "elena.api@example.com",
    "phone": "555-0199",
    "location": "Boston, MA",
    "linkedin": "",
    "github": "",
    "portfolio": ""
  }
}
```

#### `PUT /candidates/me`
Updates profile contact information.
- **Access:** Candidate Only
- **Request Body:**
```json
{
  "phone": "555-9988",
  "location": "Cambridge, MA"
}
```
- **Response (`200 OK`):** Updated profile object.

#### `POST /resume/upload`
Multipart document upload (.txt or .pdf, max 10MB).
- **Access:** Candidate Only
- **Headers:** `Content-Type: multipart/form-data`
- **Form Field:** `resume` (file)
- **Response (`201 Created`):**
```json
{
  "success": true,
  "message": "Resume uploaded, parsed, and persisted successfully.",
  "data": {
    "candidateId": 8,
    "filename": "resume_cand_8_1727342890123.pdf",
    "format": "PDF",
    "qualityScore": 92.5,
    "skillsCount": 6,
    "projectsCount": 2,
    "experienceCount": 1,
    "educationCount": 1,
    "certificationsCount": 1
  }
}
```

#### `POST /resume/import`
Parses and loads a server-local resume file.
- **Access:** Candidate Only
- **Request Body:**
```json
{
  "filePath": "data/sample_resume.txt"
}
```
- **Response (`200 OK`):** Ingestion summary data.

#### `GET /resume`
Retrieves structured sections of the candidate's active resume.
- **Access:** Candidate Only
- **Response (`200 OK`):** Complete structured resume JSON.

#### `GET /resume/analysis`
Calculates quality score and breakdown across structure, skills, experience, education, projects, and certifications.
- **Access:** Candidate Only
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Resume quality analysis calculated.",
  "data": {
    "overallScore": 92.5,
    "classification": "Strong",
    "breakdown": {
      "skillsFactor": 95.0,
      "experienceFactor": 88.0,
      "educationFactor": 100.0,
      "projectsFactor": 90.0,
      "certificationsFactor": 90.0
    }
  }
}
```

#### `GET /resume/recommendations`
Provides deterministic, actionable recommendations to improve resume strength.
- **Access:** Candidate Only
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Recommendations generated.",
  "data": [
    "Consider adding verifiable metrics or quantified results to your project descriptions."
  ]
}
```

---

### 3.3 Public Jobs & Search

#### `GET /jobs`
Lists open job requisitions with filtering and pagination.
- **Access:** Public
- **Query Parameters:**
  - `page` (default: 1)
  - `limit` (default: 10, max: 100)
  - `title` (case-insensitive substring filter)
  - `company` (case-insensitive substring filter)
  - `location` (case-insensitive substring filter)
  - `skill` (matches required or preferred skill)
  - `employmentType` (e.g. "Full Time")
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Jobs retrieved successfully.",
  "data": {
    "page": 1,
    "limit": 10,
    "total": 6,
    "totalPages": 1,
    "items": [
      {
        "jobId": "J001",
        "title": "Software Engineer",
        "company": "Infosys Ltd.",
        "location": "Pune",
        "employmentType": "Full Time",
        "requiredSkills": ["C++", "SQL", "Git", "Data Structures"],
        "preferredSkills": ["Python", "Docker"],
        "minimumExperience": 1,
        "description": "Software development role in enterprise products team"
      }
    ]
  }
}
```

#### `GET /jobs/{id}`
Retrieves full details of a specific job requisition.
- **Access:** Public
- **Response (`200 OK`):** Single job object.

---

### 3.4 Matching & Explainable Intelligence

#### `GET /matches`
Computes hybrid multi-factor match rankings between the candidate's active resume and all open jobs.
- **Access:** Candidate Only
- **Query Parameters:**
  - `limit` (default: 10)
  - `minScore` (default: 0.0)
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Matches computed successfully.",
  "data": [
    {
      "jobId": "J001",
      "jobTitle": "Software Engineer",
      "company": "Infosys Ltd.",
      "overallScore": 88.5,
      "readinessScore": 84.0,
      "classification": "Strong Match"
    }
  ]
}
```

#### `GET /matches/{jobId}/explanation`
Generates an explainable intelligence report breaking down skill overlap, semantic similarity, missing skills, and readiness breakdown.
- **Access:** Candidate Only
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Match explanation generated.",
  "data": {
    "jobId": "J001",
    "scores": {
      "overall": 88.5,
      "skill": 90.0,
      "experience": 85.0,
      "education": 100.0,
      "text": 82.5
    },
    "skills": {
      "exact": ["C++", "SQL", "Git"],
      "missingRequired": ["Data Structures"],
      "missingPreferred": ["Docker"]
    },
    "recommendations": [
      "Acquire or demonstrate proficiency in Data Structures to fulfill required job criteria."
    ]
  }
}
```

---

### 3.5 Saved Jobs & Applications

#### `GET /saved-jobs`
Returns jobs bookmarked by the candidate.
- **Access:** Candidate Only

#### `POST /saved-jobs/{jobId}`
Bookmarks a job for later review.
- **Access:** Candidate Only
- **Response (`201 Created`):** Confirmation object.

#### `DELETE /saved-jobs/{jobId}`
Removes a job from saved list.
- **Access:** Candidate Only
- **Response (`200 OK`):** Confirmation object.

#### `GET /applications`
Lists applications submitted by the candidate.
- **Access:** Candidate Only

#### `POST /applications`
Submits an application for an open job requisition. Automatically calculates match and readiness scores.
- **Access:** Candidate Only
- **Request Body:**
```json
{
  "jobId": "J001"
}
```
- **Response (`201 Created`):**
```json
{
  "success": true,
  "message": "Application submitted successfully.",
  "data": {
    "applicationId": 14,
    "jobId": "J001",
    "status": "APPLIED",
    "matchScore": 88.5,
    "readinessScore": 84.0
  }
}
```

#### `GET /applications/{id}`
Retrieves application details. Enforces ownership: only the applying candidate or the job's recruiter can view.
- **Access:** Candidate Owner or Job Recruiter

#### `POST /applications/{id}/withdraw`
Withdraws a pending application.
- **Access:** Candidate Owner Only
- **Response (`200 OK`):** Updated status (`WITHDRAWN`).

---

### 3.6 Recruiter Endpoints

#### `POST /recruiter/jobs`
Posts a new job opening. Automatically links ownership to the calling recruiter.
- **Access:** Recruiter Only
- **Request Body:**
```json
{
  "jobId": "J_CYBER_REST_01",
  "title": "Robotics Systems Engineer",
  "company": "CyberDyne Systems",
  "location": "Boston, MA",
  "employmentType": "Full Time",
  "educationRequirement": "Computer Science",
  "minimumExperience": 2,
  "requiredSkills": ["C++", "Robotics", "SQL"],
  "preferredSkills": ["Python", "ROS"],
  "description": "Develop real-time robotics telemetry pipelines."
}
```
- **Response (`201 Created`):** Created job object.

#### `PUT /recruiter/jobs/{id}`
Updates an existing job requisition. Enforces recruiter ownership check.
- **Access:** Recruiter Owner Only

#### `DELETE /recruiter/jobs/{id}`
Deletes a job requisition. Enforces recruiter ownership check.
- **Access:** Recruiter Owner Only

#### `GET /recruiter/applications`
Lists all candidate applications submitted for jobs owned by the calling recruiter.
- **Access:** Recruiter Only

#### `PUT /recruiter/applications/{id}/status`
Transitions an application's lifecycle status (`UNDER_REVIEW`, `SHORTLISTED`, `INTERVIEW`, `REJECTED`, `SELECTED`). Enforces job ownership.
- **Access:** Recruiter Owner Only
- **Request Body:**
```json
{
  "status": "SHORTLISTED"
}
```
- **Response (`200 OK`):** Updated application object.

---

### 3.8 Advanced Career Intelligence & Recruiter Analytics (Phase 12)

#### `GET /analytics/candidate`
Generates comprehensive career intelligence for the candidate, including Career Readiness Indicators (Readiness Index, Readiness Tier, Skill/Experience/Education Coverage, Profile Completeness) and application history analytics.
- **Access:** Candidate Only
- **Response (`200 OK`):**
```json
{
  "success": true,
  "message": "Candidate career analytics retrieved.",
  "data": {
    "careerReadiness": {
      "readinessIndex": 84,
      "readinessTier": "Strong Fit / High Readiness",
      "resumeQualityScore": 85,
      "skillCoverage": 80,
      "experienceCoverage": 90,
      "educationCoverage": 85,
      "profileCompleteness": 92,
      "readinessIndicators": [
        "Profile completeness meets competitive thresholds (92%)",
        "Skill coverage spans 80% across active market vacancies"
      ],
      "recommendations": ["Consider adding cloud deployment projects"]
    },
    "applicationMetrics": {
      "totalSubmitted": 5,
      "active": 4,
      "withdrawn": 1,
      "savedJobs": 3,
      "averageMatchScore": 79.5,
      "highestMatchScore": 92.0,
      "byStatus": { "APPLIED": 2, "SHORTLISTED": 1, "INTERVIEW": 1 }
    }
  }
}
```

#### `GET /analytics/candidate/skills`
Calculates comprehensive skill gap analytics with priority classifications (`REQUIRED` vs `PREFERRED`) and market demand frequency across open jobs.
- **Access:** Candidate Only
- **Query Parameters:** `jobId` (optional specific job context)
- **Response (`200 OK`):** Exact matches, related matches, missing required/preferred skills with market demand percentages.

#### `GET /analytics/candidate/market`
Aggregates job market trends deterministically from the current SmartHire job database.
- **Access:** Candidate Only
- **Response (`200 OK`):**
```json
{
  "success": true,
  "data": {
    "sampleSize": 12,
    "note": "Based on current SmartHire job dataset",
    "topSkills": [{ "skill": "C++", "count": 8, "frequencyPct": 66.7 }],
    "jobsByLocation": [{ "location": "Pune", "count": 6 }],
    "jobsByEmploymentType": [{ "type": "Full Time", "count": 10 }],
    "frequentSkillPairs": [{ "pair": "C++ + STL", "count": 5 }]
  }
}
```

#### `GET /analytics/candidate/recommendations`
Provides explainable job recommendations based on multi-factor deterministic compatibility with clear rationale points.
- **Access:** Candidate Only
- **Query Parameters:** `limit` (default: 5)

#### `GET /analytics/candidate/compare/{jobId}` (or `?jobId={jobId}`)
Performs side-by-side comparison between candidate resume and job requirements (skills, experience, education, score breakdown).
- **Access:** Candidate Only

#### `GET /analytics/recruiter`
Aggregates recruiter hiring metrics across all posted requisitions, including application pipeline funnel and 5-tier match score distribution histogram.
- **Access:** Recruiter Only

#### `GET /recruiter/jobs/{id}/analytics`
Detailed analytical metrics for a specific recruiter job (applicant volume, average match, highest match, stage distribution).
- **Access:** Recruiter Only (Strict ownership validated)

#### `GET /recruiter/jobs/{id}/candidates/compare`
Produces a factual side-by-side comparison table of all applicants for a job (match score, required skills matched, experience years, current stage). Recruiter makes the final hiring decision without automated labelling.
- **Access:** Recruiter Only (Strict ownership validated)

---

## 4. Example Workflows

### Candidate Journey
```bash
# 1. Register candidate
curl -X POST http://127.0.0.1:8080/api/v1/auth/register/candidate \
  -H "Content-Type: application/json" \
  -d '{"name":"Elena","email":"elena@test.com","phone":"555-1234","location":"Boston","username":"elena","password":"SecureP@ssw0rd1"}'

# 2. Login to obtain session token
curl -X POST http://127.0.0.1:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"elena","password":"SecureP@ssw0rd1"}'

# 3. Upload resume (.pdf)
curl -X POST http://127.0.0.1:8080/api/v1/resume/upload \
  -H "Authorization: Bearer <TOKEN>" \
  -F "resume=@sample_resume.pdf"

# 4. View match rankings
curl -X GET http://127.0.0.1:8080/api/v1/matches \
  -H "Authorization: Bearer <TOKEN>"

# 5. Apply for top job
curl -X POST http://127.0.0.1:8080/api/v1/applications \
  -H "Authorization: Bearer <TOKEN>" \
  -H "Content-Type: application/json" \
  -d '{"jobId":"J001"}'
```
