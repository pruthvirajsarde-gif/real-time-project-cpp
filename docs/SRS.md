# Software Requirements Specification (SRS)
## SmartHire — Intelligent Resume Analysis & Explainable Job Matching Platform

**Document Version:** 1.0  
**Standard Compliance:** IEEE Std 830-1998 (Recommended Practice for Software Requirements Specifications)  
**System Classification:** Multi-Tier Offline-Resilient Recruitment Automation Platform  
**Date of Release:** November 2026  
**Status:** Approved / Production Specification  

---

## Table of Contents
1. [Introduction](#1-introduction)
   - 1.1 Purpose
   - 1.2 Document Conventions
   - 1.3 Intended Audience & Reading Suggestions
   - 1.4 Product Scope
   - 1.5 References & Definitions
2. [Overall Description](#2-overall-description)
   - 2.1 Product Perspective
   - 2.2 Product Functions (High-Level Overview)
   - 2.3 User Classes & Characteristics
   - 2.4 Operating Environment
   - 2.5 Design & Implementation Constraints
   - 2.6 User Documentation & Assistance
   - 2.7 Assumptions & Dependencies
3. [External Interface Requirements](#3-external-interface-requirements)
   - 3.1 User Interfaces (UI/UX Design Tokens)
   - 3.2 Hardware Interfaces
   - 3.3 Software Interfaces (C++ Backend, SQLite3, React SPA)
   - 3.4 Communications Interfaces (RESTful JSON, HTTP/1.1)
4. [System Features & Functional Requirements](#4-system-features--functional-requirements)
   - 4.1 Module 1: Authentication & Identity Management
   - 4.2 Module 2: Resume Ingestion & Parsing Engine
   - 4.3 Module 3: Job Requisition Lifecycle Management
   - 4.4 Module 4: Deterministic Explainable AI Matching Engine
   - 4.5 Module 5: Candidate Career Readiness & Skill Gap Analytics
   - 4.6 Module 6: Job Market Insights & Skill Co-occurrence
   - 4.7 Module 7: Application Tracking System (ATS) Workflow
   - 4.8 Module 8: Recruiter Pipeline & Candidate Comparison Matrix
   - 4.9 Module 9: System Diagnostics & Health Probes
   - 4.10 Module 10: Deterministic Sample Data Seeding
5. [Non-Functional Requirements (NFR)](#5-non-functional-requirements-nfr)
   - 5.1 Performance Requirements
   - 5.2 Safety & Security Requirements
   - 5.3 Software Quality Attributes (Reliability, Availability, Maintainability, Portability)
   - 5.4 Data Integrity & Compliance
6. [Data Requirements & Schema Specification](#6-data-requirements--schema-specification)
   - 6.1 Entity-Relationship Model
   - 6.2 Data Dictionary & Table Definitions
7. [Verification & Traceability Matrix](#7-verification--traceability-matrix)

---

## 1. Introduction

### 1.1 Purpose
This Software Requirements Specification (SRS) comprehensively defines the functional and non-functional requirements for the **SmartHire** Intelligent Resume Analysis & Explainable Job Matching Platform. SmartHire is designed to solve inefficiencies, algorithmic opacity, and privacy concerns associated with cloud-dependent black-box recruiting software by offering a deterministic, explainable, high-performance, and offline-capable solution.

### 1.2 Document Conventions
- Requirements are uniquely identified using alphanumeric codes:
  - **FR-[MODULE]-[NUMBER]**: Functional Requirement
  - **NFR-[CATEGORY]-[NUMBER]**: Non-Functional Requirement
  - **IF-[TYPE]-[NUMBER]**: Interface Requirement
- Priority classifications:
  - **[MANDATORY]**: Essential for minimum viable production operation.
  - **[HIGH]**: Core operational enhancement required for primary workflows.
  - **[MEDIUM]**: Advanced analytical or convenience capability.
- Requirement statements use IEEE RFC 2119 keywords: *MUST*, *MUST NOT*, *REQUIRED*, *SHALL*, *SHOULD*, *RECOMMENDED*, and *MAY*.

### 1.3 Intended Audience & Reading Suggestions
- **Software Engineers & Architects:** Review Sections 2, 3, 4, 5, and 6 for architecture, data contracts, and implementation constraints.
- **Quality Assurance & Test Engineers:** Reference Section 4 and Section 7 for test case derivation and verification criteria.
- **Academic Evaluators & Project Stakeholders:** Review Sections 1, 2, and 4 for project scope, innovative features, and system capabilities.

### 1.4 Product Scope
SmartHire provides a unified, full-stack recruitment platform comprising:
1. **A High-Throughput C++17 Backend Engine:** Delivering high-speed mathematical analysis (TF-IDF vector space modeling, cosine angle computation, skill ontology mapping, and PBKDF2 cryptography).
2. **An Accessible React 19 Frontend Single Page Application (SPA):** Providing candidate self-service dashboards, interactive skill-gap visualizations, ATS workflows, and recruiter comparison matrices.
3. **An Embedded SQLite3 Persistence Layer:** Enabling single-binary deployments with zero cloud dependency and strict horizontal tenant isolation.

### 1.5 References & Definitions
- **IEEE Std 830-1998:** IEEE Recommended Practice for Software Requirements Specifications.
- **TF-IDF:** Term Frequency-Inverse Document Frequency numerical statistic reflecting term importance within a document corpus.
- **Cosine Similarity:** Mathematical metric measuring the cosine of the angle between two multi-dimensional feature vectors.
- **RBAC:** Role-Based Access Control enforcing privilege separation between Candidates and Recruiters.
- **PBKDF2-HMAC-SHA256:** Password-Based Key Derivation Function 2 applying SHA-256 with 100,000 iterations and cryptographic salts.
- **ATS:** Applicant Tracking System managing candidate progress across recruitment pipeline stages.

---

## 2. Overall Description

### 2.1 Product Perspective
SmartHire operates as a self-contained, multi-tier client-server system. Unlike conventional recruitment platforms that send candidate resumes to third-party proprietary LLM APIs (introducing latency, data leakage, and nondeterministic hallucinations), SmartHire performs all linguistic parsing, scoring, and analytics locally on bare-metal hardware.

```
+------------------------------------------------------------------------+
|                     SmartHire Deployment Boundary                      |
|                                                                        |
|  [ Web Browser / Mobile Device ]                                       |
|               |                                                        |
|               | HTTP/1.1 REST (JSON API / Multipart Form-Data)         |
|               v                                                        |
|  +------------------------------------------------------------------+  |
|  | Presentation Tier: React 19 / Vite SPA                           |  |
|  | (AuthContext, HashRouter, Responsive Glassmorphic UI)            |  |
|  +------------------------------------------------------------------+  |
|               |                                                        |
|               | http://127.0.0.1:8080/api/v1 (or Cloud API Host)       |
|               v                                                        |
|  +------------------------------------------------------------------+  |
|  | Application Tier: C++17 Core Engine (Single Native Binary)       |  |
|  | - cpp-httplib Multithreaded HTTP Server                          |  |
|  | - ApiRouter & Middleware Pipeline (CORS, Rate Limit, Sec Headers)|  |
|  | - MatchingEngine (Vector Space TF-IDF & Skill Gap Matrix)        |  |
|  | - AnalyticsService (Market Insights, Co-occurrence, Funnels)    |  |
|  | - AuthManager (PBKDF2-HMAC-SHA256 & Cryptographic Sessions)      |  |
|  +------------------------------------------------------------------+  |
|               |                                                        |
|               | In-Process Direct C-API Calls                          |
|               v                                                        |
|  +------------------------------------------------------------------+  |
|  | Persistence Tier: Embedded SQLite 3.x Engine                     |  |
|  | - Write-Ahead Logging (WAL) / ACIDs Transactions                 |  |
|  | - Filesystem: data/uploads/ (Hex UUID Document Store)            |  |
|  +------------------------------------------------------------------+  |
+------------------------------------------------------------------------+
```

### 2.2 Product Functions (High-Level Overview)
- **Role-Based Portals:** Independent, customized workflows for Candidates and Recruiters.
- **Document Ingestion:** Automated text parsing of raw resumes (`.txt`, `.pdf`) with regex pattern extraction for contact info, skills, education, and work chronologies.
- **Explainable Job Matching:** Multi-faceted scoring decomposing candidate suitability into Keyword Overlap, Skill Match Ratio, and Semantic Experience Alignment.
- **Skill Gap Diagnosis:** Categorization of missing skills into Required vs Preferred with concrete recommendations for interview readiness.
- **Recruiter ATS Pipeline:** Multi-stage application status transitions (`APPLIED` → `UNDER_REVIEW` → `SHORTLISTED` → `INTERVIEW` → `SELECTED` / `REJECTED`).
- **Recruiter Decision Matrix:** Side-by-side comparative scoring of all candidates applied to a specific opening.
- **Deterministic Live Data Demo:** One-click seeding and reset utility for presentation and academic defense demonstration.

### 2.3 User Classes & Characteristics
1. **Candidate:**
   - *Technical Proficiency:* Novice to intermediate.
   - *Goals:* Upload resumes, analyze skill deficiencies against live job openings, inspect explainable match scores, track application statuses, and bookmark jobs.
2. **Recruiter:**
   - *Technical Proficiency:* Intermediate.
   - *Goals:* Create and manage job openings, view matched candidates, evaluate applicant comparison matrices, advance application stages, and review candidate pipeline analytics.
3. **System Administrator:**
   - *Technical Proficiency:* Advanced.
   - *Goals:* Execute automated health probes (`/health/live`, `/health/ready`), inspect tamper-resistant audit logs, manage database migrations, and seed or clear demo datasets.

### 2.4 Operating Environment
- **Server OS:** Cross-platform (Windows 10/11 x64, Linux Ubuntu 20.04+, Alpine Linux 3.20+ via Docker).
- **Client OS & Browsers:** Modern evergreen web browsers (Google Chrome 110+, Mozilla Firefox 110+, Apple Safari 16+, Microsoft Edge).
- **Embedded Database:** SQLite v3.35+ with foreign keys enabled.
- **Network Protocol:** HTTP/1.1 TCP/IP; Port 8080 (backend default), Port 5173 (local dev frontend), or static CDN (GitHub Pages / Vercel).

### 2.5 Design & Implementation Constraints
1. **No External AI APIs:** All natural language processing, vectorization, and matching algorithms must execute entirely in C++ without relying on OpenAI, Google Gemini, or external cloud webhooks.
2. **Zero-Latency In-Memory Math:** TF-IDF matrix generation and cosine angle calculations must be computed in under 50 milliseconds for typical resume-job pairs.
3. **Horizontal Multi-Tenant Security:** A candidate must never access recruiter-restricted endpoints or inspect another candidate's private resume and contact data.

### 2.6 User Documentation & Assistance
- Comprehensive documentation includes:
  - System Architecture & Component Design ([docs/ARCHITECTURE.md](file:///d:/cpp%20activity%20real%20time%20p/SmartHire/docs/ARCHITECTURE.md))
  - RESTful API Contract & Endpoints ([docs/API.md](file:///d:/cpp%20activity%20real%20time%20p/SmartHire/docs/API.md))
  - Demonstration & Defense Walkthrough Guide ([docs/DEMO_GUIDE.md](file:///d:/cpp%20activity%20real%20time%20p/SmartHire/docs/DEMO_GUIDE.md))
  - Complete Postman API Collection ([docs/SmartHire_API.postman_collection.json](file:///d:/cpp%20activity%20real%20time%20p/SmartHire/docs/SmartHire_API.postman_collection.json))

---

## 3. External Interface Requirements

### 3.1 User Interfaces (UI)
- **Design System:** Responsive, dark-mode glassmorphic theme with CSS variables (`--primary`, `--surface-card`, `--text-primary`, `--border-card`).
- **Responsive Layouts:** Desktop navigation bar with collapsing mobile drawer for handheld devices.
- **Visual Feedback:** Deterministic health status badge, SVG progress meters, dynamic color-coded match tiers (Green for High 80-100%, Yellow for Medium 60-79%, Red for Low <60%).

### 3.2 Hardware Interfaces
- No proprietary hardware interfaces required. The system runs on standard x86-64 and ARM64 computing architectures with minimum 512 MB RAM and 100 MB disk storage.

### 3.3 Software Interfaces
- **C++ Standard Library:** C++17 STL containers (`std::unordered_map`, `std::vector`, `std::regex`, `std::chrono`).
- **SQLite3 C-API:** Prepared statement bindings (`sqlite3_prepare_v2`, `sqlite3_bind_*`, `sqlite3_step`).
- **nlohmann/json:** High-fidelity JSON parsing, serialization, and envelope construction.
- **cpp-httplib:** Multithreaded socket listener supporting concurrent worker threads.

### 3.4 Communications Interfaces
- **Standard Protocol:** HTTP/1.1 over TCP.
- **Content Types:** `application/json; charset=utf-8` for API exchanges, and `multipart/form-data` for resume file uploads.
- **Authentication Scheme:** Bearer Token via standard HTTP `Authorization: Bearer <token>` header.

---

## 4. System Features & Functional Requirements

### 4.1 Module 1: Authentication & Identity Management
- **FR-AUTH-01 [MANDATORY]: User Registration**
  - The system SHALL accept registration payloads containing `username`, `email`, `password`, `role` (`CANDIDATE` or `RECRUITER`), and personal/company metadata.
  - The system SHALL reject duplicate usernames or email addresses with HTTP `409 CONFLICT`.
- **FR-AUTH-02 [MANDATORY]: Password Security & Key Derivation**
  - Passwords MUST NOT be stored in plain text.
  - The system SHALL hash passwords using PBKDF2-HMAC-SHA256 with a unique 16-byte cryptographically random salt and 100,000 iterations.
- **FR-AUTH-03 [MANDATORY]: Session Issuance & Revocation**
  - Upon successful credential verification, the system SHALL issue a 32-byte cryptographic session token.
  - Session tokens stored in the database MUST be hashed with SHA-256 to prevent token database compromise.
  - Logout requests SHALL immediately invalidate and delete the active session token.
- **FR-AUTH-04 [HIGH]: Brute-Force Rate Limiting**
  - The system SHALL lock user accounts temporarily after 5 consecutive failed login attempts within a 15-minute sliding window.

### 4.2 Module 2: Resume Ingestion & Parsing Engine
- **FR-RES-01 [MANDATORY]: Document Upload**
  - The system SHALL support resume uploads in `.txt` and basic `.pdf` formats via `multipart/form-data`.
  - The system SHALL sanitize uploaded filenames, assigning an immutable UUID v4 filename stored in `data/uploads/`.
- **FR-RES-02 [MANDATORY]: Text Extraction & Skill Normalization**
  - The system SHALL extract raw text from uploaded resumes and execute regex parsers for phone numbers, email addresses, and educational degrees.
  - Extracted skills SHALL be matched against a curated canonical taxonomy (`skill_dictionary.txt`), normalizing synonyms (e.g., "JS" → "JavaScript", "C++17" → "C++").
- **FR-RES-03 [HIGH]: Resume Strength Scoring**
  - The system SHALL calculate an objective Resume Completeness Score (0–100%) based on the presence of contact info, structured experience, education, and at least 5 recognized technical skills.

### 4.3 Module 3: Job Requisition Lifecycle Management
- **FR-JOB-01 [MANDATORY]: Job Posting Creation**
  - Authenticated Recruiters SHALL have the capability to create job postings with `title`, `company`, `location`, `salary_range`, `description`, `required_skills`, and `preferred_skills`.
- **FR-JOB-02 [MANDATORY]: Job Directory & Search**
  - The system SHALL provide a public and candidate-accessible searchable directory of all active jobs.
  - Search queries SHALL support filtering by keyword, location, skill requirements, and title.
- **FR-JOB-03 [HIGH]: Job Requisition Lifecycle**
  - Recruiters SHALL be able to update job parameters, toggle statuses between `ACTIVE`, `PAUSED`, and `CLOSED`, or delete their own postings.

### 4.4 Module 4: Deterministic Explainable AI Matching Engine
- **FR-MATCH-01 [MANDATORY]: Vector-Space TF-IDF Representation**
  - The system SHALL construct normalized TF-IDF feature vectors for both candidate resume profiles and job descriptions in C++.
- **FR-MATCH-02 [MANDATORY]: Cosine Similarity Score**
  - The system SHALL calculate the mathematical cosine angle between resume vectors and job requisition vectors, outputting a normalized similarity coefficient between `0.00` and `1.00`.
- **FR-MATCH-03 [MANDATORY]: Explainability Decomposition**
  - Match results MUST NOT be presented as a simple opaque number.
  - The system SHALL return an explainable score decomposition:
    $$\text{Final Match Score} = (0.50 \times \text{Skill Match Ratio}) + (0.35 \times \text{TF-IDF Cosine}) + (0.15 \times \text{Experience Alignment})$$
  - The payload SHALL include explicit lists of `matched_skills`, `missing_required_skills`, and `missing_preferred_skills`.

### 4.5 Module 5: Candidate Career Readiness & Skill Gap Analytics
- **FR-ANL-01 [HIGH]: Career Readiness Index**
  - The system SHALL generate a composite Career Readiness Index for candidates categorizing them into:
    - `EXECUTIVE` (Score $\ge 85$)
    - `INTERMEDIATE` (Score $65 - 84$)
    - `EMERGING` (Score $< 65$)
- **FR-ANL-02 [HIGH]: Market-Weighted Skill Gap Analysis**
  - The system SHALL analyze candidate missing skills against all open jobs in the database and rank missing skills by market frequency and wage demand.

### 4.6 Module 6: Job Market Insights & Skill Co-occurrence
- **FR-MKT-01 [MEDIUM]: In-Demand Skills Aggregation**
  - The system SHALL aggregate requirement frequencies across all active job postings to produce real-time rankings of top technical competencies.
- **FR-MKT-02 [MEDIUM]: Deterministic Skill Co-occurrence**
  - The system SHALL compute frequency pairs of skills frequently required together in the same requisition (e.g., C++ & Multithreading, React & Vite).

### 4.7 Module 7: Application Tracking System (ATS) Workflow
- **FR-APP-01 [MANDATORY]: One-Click Application Submission**
  - Authenticated Candidates SHALL submit applications to any active job opening with their active resume profile.
  - Duplicate applications by the same candidate to the same job SHALL be rejected with `409 CONFLICT`.
- **FR-APP-02 [MANDATORY]: Application Status Tracking**
  - Candidates SHALL be able to view all submitted applications with timestamps, active stage, and recruiter updates.
- **FR-APP-03 [MANDATORY]: Recruiter Stage Advancement**
  - Recruiters SHALL be authorized to advance application statuses through the validated workflow:
    `APPLIED` $\rightarrow$ `UNDER_REVIEW` $\rightarrow$ `SHORTLISTED` $\rightarrow$ `INTERVIEW` $\rightarrow$ `SELECTED` or `REJECTED`.

### 4.8 Module 8: Recruiter Pipeline & Candidate Comparison Matrix
- **FR-REC-01 [HIGH]: Recruiter Hiring Funnel**
  - The system SHALL compute applicant counts at each funnel stage for any job opening owned by the recruiter.
- **FR-REC-02 [HIGH]: Side-by-Side Candidate Comparison Matrix**
  - The system SHALL generate a multi-applicant tabular comparison ranking applicants by overall match score, required skill count, years of experience, and stage.

### 4.9 Module 9: System Diagnostics & Health Probes
- **FR-SYS-01 [HIGH]: Liveness Health Probe (`/api/v1/health/live`)**
  - The system SHALL return HTTP `200 OK` with JSON `{ "status": "UP", "liveness": "OK" }` within 5ms to indicate the HTTP listener is operational.
- **FR-SYS-02 [HIGH]: Readiness Health Probe (`/api/v1/health/ready`)**
  - The system SHALL test database connectivity (`SELECT 1`) and verify write access to `data/uploads/`.
  - If any subsystem fails, it SHALL return HTTP `503 SERVICE UNAVAILABLE`.

### 4.10 Module 10: Deterministic Sample Data Seeding
- **FR-DEMO-01 [HIGH]: CLI Demo Data Seeder**
  - The CLI argument `--seed-demo` SHALL populate the database with predetermined accounts (`candidate1`, `recruiter1`), 10 industry jobs, 5 structured resumes, and active application pipelines.
- **FR-DEMO-02 [HIGH]: CLI Demo Reset**
  - The CLI argument `--clear-demo` SHALL cleanly remove seeded presentation data without corrupting user accounts.

---

## 5. Non-Functional Requirements (NFR)

### 5.1 Performance Requirements
- **NFR-PERF-01 [LATENCY]:** 95% of standard REST API queries (Jobs, Profiles, Dashboard) SHALL respond in under **25 milliseconds**.
- **NFR-PERF-02 [MATCHING SPEED]:** The C++ TF-IDF vectorization and cosine calculation for a candidate against a catalog of 100 jobs SHALL execute in under **15 milliseconds**.
- **NFR-PERF-03 [CONCURRENCY]:** The multithreaded backend server SHALL handle at least **50 concurrent requests per second** without connection dropping on modern dual-core consumer hardware.

### 5.2 Safety & Security Requirements
- **NFR-SEC-01 [HORIZONTAL ISOLATION]:** Users SHALL only access their own private records. Any attempt to modify another user's candidate profile or job posting SHALL return `403 FORBIDDEN_NOT_OWNER`.
- **NFR-SEC-02 [SQL INJECTION DEFENSE]:** All database queries MUST use parameterized prepared statements (`sqlite3_bind_*`). Zero dynamic string concatenation is permitted in SQL queries.
- **NFR-SEC-03 [HTTP SECURITY HEADERS]:** The server SHALL inject security headers on every response:
  - `X-Content-Type-Options: nosniff`
  - `X-Frame-Options: DENY`
  - `X-XSS-Protection: 1; mode=block`
- **NFR-SEC-04 [FILE PATH SANITIZATION]:** Uploaded file paths MUST be checked against path traversal tokens (`..`, `/`, `\`) to prevent directory escape attacks.

### 5.3 Software Quality Attributes
- **NFR-REL-01 [RELIABILITY]:** The backend process SHALL maintain 99.9% uptime in local environments and cleanly recover from database locks through automatic retry logic.
- **NFR-PORT-01 [PORTABILITY]:** The frontend SPA MUST run interchangeably on local `localhost:5173`, static CDN (GitHub Pages), and edge hosts (Vercel) without code modifications.
- **NFR-MAINT-01 [MODULARITY]:** All C++ business logic must remain decoupled from HTTP routing, adhering strictly to the Controller-Service-Repository pattern.

---

## 6. Data Requirements & Schema Specification

### 6.1 Entity-Relationship Model

```
+---------------+       1:1       +----------------+
|   ACCOUNTS    | <-------------> |   CANDIDATES   |
|---------------|                 |----------------|
| id (PK)       |                 | id (PK)        |
| username (UK) |                 | account_id(FK) |
| email (UK)    |                 | name, phone    |
| password_hash |                 +----------------+
| salt, role    |                         | 1:N
+---------------+                         v
        | 1:N                     +----------------+
        |                         |  APPLICATIONS  |
        v                         |----------------|
+---------------+       1:N       | id (PK)        |
|   RECRUITERS  | --------------> | candidate_id   |
|---------------|                 | job_id (FK)    |
| id (PK)       |                 | status         |
| account_id(FK)|                 +----------------+
| company_name  |                         ^
+---------------+                         | 1:N
        | 1:N                             |
        v                         +----------------+
+---------------+                 |      JOBS      |
|     JOBS      | <---------------+----------------|
|---------------|                 | id (PK, String)|
| id (PK)       |                 | recruiter_id   |
| recruiter_id  |                 | title, skills  |
+---------------+                 +----------------+
```

### 6.2 Data Dictionary & Table Definitions

#### 1. Table: `accounts`
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `id` | INTEGER | PRIMARY KEY AUTOINCREMENT | Unique account identifier |
| `username` | TEXT | UNIQUE NOT NULL | Alphanumeric login identifier |
| `email` | TEXT | UNIQUE NOT NULL | Standard format email address |
| `password_hash` | TEXT | NOT NULL | PBKDF2-HMAC-SHA256 hex digest |
| `salt` | TEXT | NOT NULL | 16-byte random hex salt |
| `role` | TEXT | NOT NULL CHECK(role IN ('CANDIDATE', 'RECRUITER')) | User privilege level |
| `failed_attempts`| INTEGER | DEFAULT 0 | Failed login attempt counter |
| `lockout_until` | INTEGER | DEFAULT 0 | Epoch timestamp for lockout expiration |
| `created_at` | DATETIME| DEFAULT CURRENT_TIMESTAMP | Account creation timestamp |

#### 2. Table: `candidates`
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `id` | INTEGER | PRIMARY KEY AUTOINCREMENT | Unique candidate identifier |
| `account_id` | INTEGER | UNIQUE NOT NULL, FK(`accounts.id`) | Owning authentication account |
| `name` | TEXT | NOT NULL | Candidate full name |
| `phone` | TEXT | NULLABLE | Contact telephone |
| `location` | TEXT | NULLABLE | Geographic location |
| `bio` | TEXT | NULLABLE | Professional summary |

#### 3. Table: `recruiters`
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `id` | INTEGER | PRIMARY KEY AUTOINCREMENT | Unique recruiter identifier |
| `account_id` | INTEGER | UNIQUE NOT NULL, FK(`accounts.id`) | Owning authentication account |
| `name` | TEXT | NOT NULL | Recruiter full name |
| `company` | TEXT | NOT NULL | Hiring organization name |
| `email` | TEXT | NOT NULL | Official contact email |

#### 4. Table: `jobs`
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `id` | TEXT | PRIMARY KEY | Unique job alphanumeric code (e.g. `JOB-101`) |
| `recruiter_id` | INTEGER | NOT NULL, FK(`recruiters.id`) | Posting recruiter |
| `title` | TEXT | NOT NULL | Job position title |
| `company` | TEXT | NOT NULL | Company name |
| `location` | TEXT | NOT NULL | Work location / Remote |
| `salary` | TEXT | NOT NULL | Compensation range string |
| `description` | TEXT | NOT NULL | Full job specification |
| `required_skills`| TEXT | NOT NULL | Comma-separated canonical skills |
| `status` | TEXT | DEFAULT 'ACTIVE' | `ACTIVE`, `PAUSED`, or `CLOSED` |
| `posted_date` | DATETIME| DEFAULT CURRENT_TIMESTAMP | Publication date |

#### 5. Table: `applications`
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `id` | INTEGER | PRIMARY KEY AUTOINCREMENT | Unique application record ID |
| `job_id` | TEXT | NOT NULL, FK(`jobs.id`) | Target job position |
| `candidate_id` | INTEGER | NOT NULL, FK(`candidates.id`) | Applicant profile ID |
| `status` | TEXT | DEFAULT 'APPLIED' | Pipeline stage (`APPLIED`, `UNDER_REVIEW`, `SHORTLISTED`, `INTERVIEW`, `SELECTED`, `REJECTED`) |
| `applied_date` | DATETIME| DEFAULT CURRENT_TIMESTAMP | Application submission timestamp |
| `updated_date` | DATETIME| DEFAULT CURRENT_TIMESTAMP | Stage transition timestamp |

#### 6. Table: `sessions`
| Column | Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `id` | INTEGER | PRIMARY KEY AUTOINCREMENT | Session entry ID |
| `account_id` | INTEGER | NOT NULL, FK(`accounts.id`) | Associated account |
| `token_hash` | TEXT | UNIQUE NOT NULL | SHA-256 hash of active Bearer token |
| `expires_at` | INTEGER | NOT NULL | Unix epoch expiration timestamp |
| `created_at` | DATETIME| DEFAULT CURRENT_TIMESTAMP | Login timestamp |

---

## 7. Verification & Traceability Matrix

| Requirement ID | Description | Verifying Component / Test Suite | Acceptance Criteria |
| :--- | :--- | :--- | :--- |
| **FR-AUTH-01** | Account Registration | `tests/test_runner.cpp` (Suite 1) | HTTP 201 Created; duplicate returns 409 |
| **FR-AUTH-02** | PBKDF2 Password Hashing | `tests/test_runner.cpp` (Test 238) | 100,000 iterations verified, unique salt |
| **FR-RES-01** | Resume File Ingestion | `ResumeController.cpp` | Sanitized UUID stored in `data/uploads/` |
| **FR-MATCH-01** | Vector Space TF-IDF | `MatchingEngine.cpp` & `test_runner.cpp` | Deterministic score within [0.0, 1.0] |
| **FR-MATCH-03** | Explainability Breakdown | `MatchingController.cpp` | Explanations output matched/missing skills |
| **FR-APP-03** | ATS Workflow Transitions | `ApplicationManager.cpp` | Validated state progression; illegal transitions rejected |
| **FR-REC-02** | Candidate Comparison Matrix | `RecruiterAnalyticsPage.jsx` | All applicants sorted by score & skills |
| **FR-SYS-01** | Liveness Probe | `test_runner.cpp` | HTTP 200 within 5ms |
| **FR-SYS-02** | Readiness Probe | `test_runner.cpp` | Verifies DB query and upload directory write |
| **NFR-SEC-01** | Horizontal Tenant Isolation | `ApiMiddleware.cpp` | Cross-tenant access yields 403 Forbidden |
| **NFR-SEC-02** | SQL Injection Defense | `DatabaseManager.cpp` | 100% prepared statement parameterization |

---

## 8. Approval & Sign-Off

| Role | Name | Title | Date | Signature |
| :--- | :--- | :--- | :--- | :--- |
| **Lead Developer** | Pruthviraj Sarde | Core Systems Engineer | Nov 17, 2026 | *P. Sarde* |
| **System Architect** | Academic Project Lead | System Reviewer | Nov 17, 2026 | *Reviewed* |
