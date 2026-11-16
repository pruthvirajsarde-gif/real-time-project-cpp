# SmartHire — Evaluator & Presentation Demo Guide

**Version:** 12.0 (Phase 12 Complete)  
**Target Audience:** Academic Evaluators, Project Reviewers & Technical Demonstrators  
**Demonstration Duration:** 7–12 Minutes  

---

## 1. Quick Start & Demonstration Preparation

SmartHire includes an automated deterministic demo data suite designed to make academic review and live presentations seamless.

### Step 1: Seed Demo Data
Before launching the servers, populate the database with realistic demo accounts, requisitions, structured resumes, and multi-stage applications:
```cmd
seed-demo.bat
```
*(Or via CLI: `.\build\SmartHire.exe --seed-demo`)*

### Step 2: Launch SmartHire
Launch the dual-service local environment with one command:
```cmd
start-dev.bat
```
This launches:
- **SmartHire REST Backend:** `http://127.0.0.1:8080`
- **SmartHire Web UI:** `http://localhost:5173`

---

## 2. Pre-Configured Demo Accounts

All demo accounts are created deterministically with secure credentials:

| Role | Username | Password | Full Name / Organization | Profile Focus |
| :--- | :--- | :--- | :--- | :--- |
| **Candidate** | `cand_alice` | `AliceP@ss123` | Alice Smith | Senior C++ & Systems Developer (Pune) |
| **Candidate** | `cand_rahul` | `RahulP@ss123` | Rahul Sharma | Frontend & UI Developer (Pune) |
| **Candidate** | `cand_priya` | `PriyaP@ss123` | Priya Nair | Fullstack & Backend Engineer (Pune) |
| **Recruiter** | `rec_techcorp` | `RecruiterP@ss123` | Bob Vance (TechCorp) | Enterprise Software & Infrastructure |
| **Recruiter** | `rec_innovate` | `RecruiterP@ss123` | Meera Patel (InnovateAI) | Distributed Systems & Web Platforms |

---

## 3. Step-by-Step Demonstration Script (Showcase Flow)

### Step 1: Project Introduction (30–45 seconds)
> *"SmartHire is an offline-capable, high-performance intelligent career decision-support platform. Rather than relying on opaque black-box AI models, SmartHire is engineered in modern C++17 with deterministic vector-space TF-IDF semantic matching, explainable career readiness indicators, multi-job skill trend analytics, and factual candidate comparison matrices."*

### Step 2: The Candidate Experience
1. **Login as Candidate:**
   - Navigate to: `http://localhost:5173`
   - Sign in as `cand_alice` / `AliceP@ss123`.
2. **Candidate Workspace & Career Readiness (`/candidate`):**
   - Point out the **Career Readiness Indicators** panel showing **Readiness Index (84%)**, **Readiness Tier** ("Strong Fit / High Readiness"), Skill Coverage (80%), Experience Coverage (90%), Education Alignment (85%), and Profile Completeness (92%).
   - Review the deterministic diagnostic factors and action items.
3. **Inspect Resume & Structure Analysis (`/candidate/resume` & `/candidate/resume-analysis`):**
   - View extracted contact details, education, technical skills, and structured experience chronologies.
   - Show the Quality Score and structural section completeness.
4. **Skill Gap Analytics (`/candidate/skill-gap`):**
   - Show candidate's matched skills vs. missing required and preferred competencies.
   - Highlight market demand frequencies across active jobs (e.g., Docker 80% demand).
5. **Job Recommendations & "Why This Job?" (`/candidate/jobs/:id`):**
   - Navigate to open job requisition.
   - Expand the **"Why This Job Matches You"** intelligence card:
     - Matched skills breakdown
     - Related competency matches
     - Missing requirements
     - Experience threshold satisfaction
6. **Resume vs. Job Side-by-Side Comparison (`/candidate/compare`):**
   - Show the side-by-side comparison matrix evaluating skill compatibility, education, and experience requirements with overall match breakdown.
7. **Submit Application:**
   - Click **Apply Now** to submit an application.
8. **Application History Analytics (`/candidate/analytics`):**
   - View active vs. withdrawn applications, status breakdown, and average applied match score.
9. **SmartHire Job Market Insights (`/candidate/market-insights`):**
   - Show database-derived market trends: Top In-Demand Skills, Jobs by Location (Pune, Bangalore), Employment Type distribution, and frequent skill co-occurrence pairs (e.g. `C++ + STL`, `React + TypeScript`).
10. **Sign Out:** Sign out from candidate session.

### Step 3: The Recruiter Experience
1. **Login as Recruiter:**
   - Sign in as `rec_techcorp` / `RecruiterP@ss123`.
2. **Recruiter Hiring Analytics (`/recruiter/analytics`):**
   - Point out the Recruitment Overview cards (Active Jobs, Total Applicants, Shortlisted, Interview, Selected).
   - Review the **Application Pipeline Funnel** (`APPLIED` ➔ `UNDER_REVIEW` ➔ `SHORTLISTED` ➔ `INTERVIEW` ➔ `SELECTED`).
   - Review the **Match Score Distribution Histogram** (applicant volume grouped into 5 score tiers: 90–100%, 80–89%, 70–79%, 60–69%, <60%).
3. **Requisition Management (`/recruiter/jobs`):**
   - View posted job requisitions.
   - Click **📊 Analytics** on a job row to view **Job Analytics (`/recruiter/jobs/:id/analytics`)**:
     - Applicant volume, Average Match Score, Highest Match Score, Stage Distribution.
4. **Candidate Comparison Matrix (`/recruiter/jobs/:id/compare`):**
   - Review the factual, non-evaluative applicant comparison table: Match Score, Required Skills Matched, Experience Years, Stage.
   - Highlight that SmartHire supports human decision-making rather than making automated hiring choices.
5. **Update Candidate Stage:**
   - Advance candidate from `UNDER_REVIEW` to `SHORTLISTED` or `INTERVIEW`.
   - Show instant stage persistence.

### Step 4: Technical & Security Verification (1–2 minutes)
1. **Authoritative Server-Side RBAC:**
   - Demonstrate that candidate routes are strictly blocked for recruiters and vice versa (`403 Forbidden`).
   - Demonstrate cross-tenant isolation: recruiters cannot view jobs or candidate analytics belonging to another recruiter.
2. **PBKDF2 Cryptographic Configuration:**
   - Documented and verified configuration: **100,000 iterations**, 16-byte salt, 32-byte derived key (Test 238).
3. **Regression Test Verification:**
   - Highlight verified test results:
     - Backend: **524 / 524 assertions passed (100%)**
     - Frontend: **60 / 60 Vitest tests passed (100%)**

---

## 4. Resetting & Teardown

To return the system to an empty, clean development state after a presentation:
```cmd
clear-demo.bat
```
*(Or via CLI: `.\build\SmartHire.exe --clear-demo`)*

This safely removes demo accounts, demo applications, and demo jobs without destroying user-created production accounts.
