# SmartHire

**SmartHire** is a C++ based intelligent resume and job matching system.  
It models resumes, job listings, and candidate profiles using clean OOP principles, with intelligent matching, scoring, multi-format resume ingestion (`.txt` & `.pdf`), skill gap analysis, strength evaluation, job readiness scoring, and comprehensive report generation capabilities.

---

## Phase 1 — OOP Foundation

Phase 1 establishes the project foundation:

- **Project structure** — modular `backend/include` and `backend/src` layout
- **Resume model** — `Resume` class inheriting from `Profile`
- **Job model** — `Job` class with required and preferred skills
- **Supporting models** — `Skill`, `Education`, `Project`, `Experience`, `Certification`
- **OOP foundation** — classes, encapsulation, inheritance, STL vectors
- **C++17 build configuration** — CMake-based build system

---

## Phase 2 — Resume Analysis & Job Matching

Phase 2 transforms SmartHire into a functional resume analyzer and job matching engine.

### Core Classes

| Class | Purpose |
|---|---|
| `ResumeAnalyzer` | Analyzes a single resume against a single job listing |
| `JobMatcher` | Compares one resume against multiple jobs, ranks and sorts results |
| `JobMatchResult` | Lightweight struct holding job ID, title, company, and match score |

### Matching Algorithm

#### Skill Matching
- **Case-insensitive** comparison (e.g., `C++` matches `c++`)
- Required skills carry **70% weight**, preferred skills carry **30% weight**
- If only one skill category exists, it receives 100% weight
- Individual percentage = (matched skills / total skills) × 100

#### Education Matching
- Keyword-based matching between candidate's field of study and job requirement
- Recognized keywords: `computer engineering`, `computer science`, `information technology`, etc.
- Recognized degree levels: `diploma`, `b.tech`, `b.e.`, `master`, etc.
- Returns 100% for a match, 75% for partial, 0% for no match

#### Experience Matching
- Parses duration strings in multiple formats:
  - `"May 2025 - Jul 2025"` (month-name year range)
  - `"2 years"`, `"6 months"` (numeric + unit)
  - `"Jan 2025 - Present"` (ongoing)
- Formula: `min(candidateYears / requiredYears, 1.0) × 100%`
- If job requires 0 experience → 100%

### Scoring Formula

```
Overall Score = (SkillScore × 0.60) + (EducationScore × 0.20) + (ExperienceScore × 0.20)

Where:
  SkillScore = (RequiredMatch × 0.70) + (PreferredMatch × 0.30)
```

### Match Categories

| Score Range | Category |
|---|---|
| 80 – 100% | Strong Match |
| 60 – 79% | Good Match |
| 40 – 59% | Partial Match |
| 0 – 39% | Low Match |

---

## Phase 3 — Resume File Processing & Data Persistence

Phase 3 introduces file I/O capabilities, structured text file ingestion, and report generation:

### Core Classes

| Class | Purpose |
|---|---|
| `ResumeParser` | Reads and validates structured resume text files (`.txt`), populating a full `Resume` object |
| `JobDataLoader` | Reads multiple `[JOB]` entries from `jobs.txt`, populating a vector of `Job` objects |
| `ReportGenerator` | Saves candidate summary, ranked job matches, and statistical aggregates to a formatted text report |

---

## Phase 4 — Intelligent Resume Analysis & Skill Gap Engine

Phase 4 transforms SmartHire into an intelligent, data-driven career analytics and resume optimization system.

### Core Classes

| Class | Purpose |
|---|---|
| `SkillGapAnalyzer` | Identifies missing required/preferred skills, calculates skill coverage and skill gap percentages, and generates multi-job gap comparison metrics |
| `ResumeStrengthAnalyzer` | Evaluates granular section-by-section quality, detects data-backed candidate strengths, identifies improvement areas, and analyzes technology frequency |
| `RecommendationEngine` | Calculates job-specific readiness scores (50/20/20/10 weighted formula) and generates prioritized, actionable improvement recommendations |

### Scoring & Gap Formulas

- **Skill Coverage**: $\frac{\text{Matching Unique Skills}}{\text{Total Job Unique Skills}} \times 100\%$
- **Skill Gap**: $100\% - \text{Coverage}$
- **Resume Quality Score (100 Pts)**: Profile (15) + Skills (20) + Education (15) + Projects (20) + Experience (20) + Certifications (10)
- **Job Readiness**: $(\text{Skill Match} \times 0.50) + (\text{Education Match} \times 0.20) + (\text{Experience Match} \times 0.20) + (\text{Resume Quality} \times 0.10)$

---

## Phase 5 — Real Resume File Support & Document Extraction

Phase 5 elevates SmartHire to ingest real-world resume documents in both plain text (`.txt`) and PDF (`.pdf`) formats through a clean multi-tiered document reading abstraction.

### New Classes

| Class | Purpose |
|---|---|
| `ResumeDocumentReader` | Low-level document abstraction that handles format detection, file validation, text extraction from `.txt` and `.pdf`, and text normalization |
| `ResumeInputManager` | High-level facade providing a unified interface for loading, validating, and presenting resume loading results regardless of format |

### Architecture Pipeline

```text
                    SmartHire
                        |
             ResumeInputManager
                        |
             ResumeDocumentReader
                        |
                  Plain Text
                        |
                  ResumeParser (Structured & Semi-Structured)
                        |
                  Resume Object
                        |
        +---------------+---------------+
        |               |               |
 ResumeAnalyzer   SkillGapAnalyzer   JobMatcher
        |               |               |
        +---------------+---------------+
                        |
              RecommendationEngine
                        |
                ReportGenerator
```

### PDF Extraction Technology

- **Native C++ PDF Stream Parser**: SmartHire implements a self-contained, dependency-free PDF stream parser in `ResumeDocumentReader.cpp`.
- **Mechanism**: Reads standard PDF content objects and streams, scanning for `BT` (Begin Text) to `ET` (End Text) blocks. It decodes string literal operators (`(...) Tj`, `[...] TJ`, `'`), escape sequences (`\\`, `\(`, `\)`, `\n`, `\t`, octal literals), and advances line positioning cleanly.
- **Zero Heavy Dependencies**: Requires no heavy external libraries, no Python runtime dependencies, and no third-party DLLs. It is 100% standard C++17, compiling seamlessly across Windows (MinGW/MSVC), Linux, and macOS.
- **Defensive Fallback**: If an invalid, encrypted, or corrupted PDF is provided, the parser safely logs an error and returns an empty string without crashing or leaking memory.

### Text Normalization

Extracted raw text undergoes normalization before parsing:
- Line endings normalized across Windows (`\r\n`), Unix (`\n`), and legacy Mac (`\r`)
- Non-printable control characters removed while preserving whitespace and indentation
- Excessive blank lines collapsed to prevent section fragmentation

### Dual Parsing Modes in `ResumeParser`

`ResumeParser` seamlessly routes normalized text through two independent parsing engines:
1. **Structured Parser**: Reads bracketed sections (`[PROFILE]`, `[SKILLS]`, `[EDUCATION]`, `[PROJECT]`, `[EXPERIENCE]`, `[CERTIFICATION]`).
2. **Semi-Structured Parser**: Analyzes free-form headings and label pairs commonly found in exported PDFs:
   - Profile: `CONTACT`, `PERSONAL INFORMATION`, `Name:`, `Email:`, `Phone:`, `Location:`
   - Skills: `SKILLS`, `TECHNICAL SKILLS`, `TECHNOLOGIES`, `KEY SKILLS`
   - Education: `EDUCATION`, `ACADEMIC QUALIFICATIONS`, `Degree:`, `Institution:`
   - Projects: `PROJECTS`, `Project:`, `Technologies:`, `Description:`
   - Experience: `EXPERIENCE`, `WORK EXPERIENCE`, `Company:`, `Position:`
   - Certifications: `CERTIFICATIONS`, `CERTIFICATES`

---

## Application Menu & Workflow

SmartHire features a clean 9-option interactive console menu:

```text
========================================
              SMARTHIRE                
========================================

  1. Load Resume
  2. Analyze Resume
  3. View Job Matches
  4. Skill Gap Analysis
  5. Resume Strength Analysis
  6. Job Recommendations
  7. Generate Full Report
  8. Display Resume
  9. Exit

  Enter choice:
```

Option 1 prompts:
```text
Supported resume formats:
1. TXT
2. PDF

Enter resume file path (or press Enter for default: data/sample_resume.txt):
```

---

## Build & Run Instructions

### 1. Build SmartHire and Test Suite

Using CMake with MinGW:
```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

Or with Ninja:
```bash
cmake -S . -B build -G "Ninja"
cmake --build build
```

### 2. Run the Main Application

```bash
.\build\SmartHire.exe
```

### 3. Run the Automated Verification Suite

```bash
.\build\test_runner.exe
```

---

## Project Structure

```
SmartHire/
├── backend/
│   ├── include/          # Header files (.h)
│   │   ├── Profile.h
│   │   ├── Skill.h
│   │   ├── Education.h
│   │   ├── Project.h
│   │   ├── Experience.h
│   │   ├── Certification.h
│   │   ├── Resume.h
│   │   ├── Job.h
│   │   ├── ResumeAnalyzer.h         (Phase 2)
│   │   ├── JobMatcher.h             (Phase 2)
│   │   ├── ResumeParser.h           (Phase 3/5)
│   │   ├── JobDataLoader.h          (Phase 3)
│   │   ├── ReportGenerator.h        (Phase 3/4/5/6)
│   │   ├── SkillGapAnalyzer.h       (Phase 4)
│   │   ├── ResumeStrengthAnalyzer.h (Phase 4)
│   │   ├── RecommendationEngine.h   (Phase 4)
│   │   ├── ResumeDocumentReader.h   (Phase 5)
│   │   ├── ResumeInputManager.h     (Phase 5)
│   │   ├── DatabaseManager.h        (Phase 6)
│   │   ├── CandidateRepository.h    (Phase 6)
│   │   ├── JobRepository.h          (Phase 6)
│   │   ├── ResumeRepository.h       (Phase 6)
│   │   ├── ApplicationManager.h     (Phase 6)
│   │   └── SavedJobManager.h        (Phase 6)
│   ├── src/              # Source files (.cpp)
│   │   ├── Profile.cpp
│   │   ├── Skill.cpp
│   │   ├── Education.cpp
│   │   ├── Project.cpp
│   │   ├── Experience.cpp
│   │   ├── Certification.cpp
│   │   ├── Resume.cpp
│   │   ├── Job.cpp
│   │   ├── ResumeAnalyzer.cpp       (Phase 2)
│   │   ├── JobMatcher.cpp           (Phase 2)
│   │   ├── ResumeParser.cpp         (Phase 3/5)
│   │   ├── JobDataLoader.cpp        (Phase 3)
│   │   ├── ReportGenerator.cpp      (Phase 3/4/5/6)
│   │   ├── SkillGapAnalyzer.cpp     (Phase 4)
│   │   ├── ResumeStrengthAnalyzer.cpp (Phase 4)
│   │   ├── RecommendationEngine.cpp (Phase 4)
│   │   ├── ResumeDocumentReader.cpp (Phase 5)
│   │   ├── ResumeInputManager.cpp   (Phase 5)
│   │   ├── DatabaseManager.cpp      (Phase 6)
│   │   ├── CandidateRepository.cpp  (Phase 6)
│   │   ├── JobRepository.cpp        (Phase 6)
│   │   ├── ResumeRepository.cpp     (Phase 6)
│   │   ├── ApplicationManager.cpp   (Phase 6)
│   │   └── SavedJobManager.cpp      (Phase 6)
│   ├── third_party/      # Embedded dependencies
│   │   └── sqlite/       # SQLite3 C amalgamation (sqlite3.c, sqlite3.h)
│   └── main.cpp          # Dual-role console application (Candidate & Recruiter)
├── database/             # Database DDL & seeds
│   ├── schema.sql        # Full relational SQLite DDL schema
│   └── seed_jobs.sql     # Seed initial job records
├── data/                 # Data storage
│   ├── smarthire.db            # Persistent SQLite database file
│   ├── sample_resume.txt       # Structured candidate resume
│   ├── sample_resume.pdf       # Real PDF candidate resume test fixture
│   ├── jobs.txt                # Job seed database (5 positions)
│   ├── empty_resume.txt        # Edge-case test fixture
│   └── malformed_resume.txt    # Edge-case test fixture
├── reports/              # Output directory for generated reports
│   └── SmartHire_Report.txt    # Enhanced report with Database Summary
├── tests/                # Automated verification
│   ├── test_runner.cpp         # 178 automated assertions across 62 test suites
│   └── test_smarthire.db       # Isolated database for automated test suite
├── CMakeLists.txt        # CMake build configuration (C & C++17)
├── .gitignore
└── README.md
```

---

## Phase 6 — Persistent Database, Candidate Profiles & Application Tracking

SmartHire Phase 6 elevates the application from an in-memory resume analysis tool into an end-to-end local recruitment management system with persistent storage, candidate deduplication, application tracking workflows, bookmarked jobs, and dual-role console experiences.

### 1. SQLite Architecture & Technology
- **Engine**: Embedded SQLite 3 amalgamation compiled directly into the C++ runtime via CMake with zero external runtime package dependencies.
- **Portability**: Completely self-contained, multiplatform, and local (`data/smarthire.db`).
- **Connection Lifecycle**: Centralized through `DatabaseManager`, ensuring foreign key validation (`PRAGMA foreign_keys = ON;`), prepared statement caching, and clean teardown on application exit.
- **Isolation**: Automated test suite uses `tests/test_smarthire.db`, preserving the production database completely untainted.

### 2. Database Schema & Entity Relationship Diagram

```text
       +-----------------------+              +-----------------------+
       |       candidates      | 1          * |        resumes        |
       +-----------------------+--------------+-----------------------+
       | PK  id                |              | PK  id                |
       |     name              |              | FK  candidate_id      |
       |     email (UNIQUE)    |              |     source_path       |
       |     phone             |              |     file_format       |
       |     location          |              |     quality_score     |
       |     linkedin          |              +-----------------------+
       |     github            |
       |     portfolio         |
       +-----------+-----------+
                   | 1
       +-----------+-----------+
       |           |           |
       | *         | *         | *
+------+------+ +--+---+----+ ++-----+-------+
| cand_skills | | saved_jobs| | applications |
+-------------+ +-----------+ +--------------+
| FK cand_id  | | FK cand_id| | PK id        |
| FK skill_id | | FK job_id | | FK cand_id   |
+------+------+ +----+------+ | FK job_id    |
       | *           | *      |    status    |
+------+------+      |        |    scores    |
|   skills    |      |        +------+-------+
+-------------+      |               | *
| PK id       |      |               |
|    name     |      |               |
|    category |      |               |
+------+------+      |               |
       | *           |               |
+------+------+------+---------------+
|              jobs                  |
+------------------------------------+
| PK id                              |
|    job_id (UNIQUE)                 |
|    title                           |
|    company                         |
|    location                        |
|    employment_type                 |
|    education_requirement           |
|    minimum_experience              |
|    description                     |
+------------------------------------+
       | 1                     | 1
       | *                     | *
+------+---------------+ +-----+----------------+
|  job_required_skills | |  job_preferred_skills|
+----------------------+ +----------------------+
| FK job_id            | | FK job_id            |
| FK skill_id          | | FK skill_id          |
+----------------------+ +----------------------+
```

### 3. Candidate Persistence & Deduplication
- **Resume Ingestion Flow**: `Resume Document -> ResumeInputManager -> Resume -> CandidateRepository -> ResumeRepository -> SQLite`.
- **Deduplication by Email**: If candidate email exists, profile and skills are updated in place rather than creating redundant candidate entities.
- **Deep Resume Serialization**: Full resume object graphs (skills, education, professional experience, projects, certifications) are serialized into relational tables and reconstructed into clean C++ models.

### 4. Application Status Workflow

Applications strictly enforce valid state transitions:
```text
                  APPLIED
                     │
              UNDER_REVIEW
             ┌───────┴───────┐
        SHORTLISTED       REJECTED
             │
         INTERVIEW
             │
         SELECTED

   (Candidate can withdraw active applications: ANY ACTIVE -> WITHDRAWN)
```

- Allowed status values: `APPLIED`, `UNDER_REVIEW`, `SHORTLISTED`, `INTERVIEW`, `REJECTED`, `SELECTED`, `WITHDRAWN`.
- Duplicate active applications for the same candidate and job position are strictly prevented.

### 5. Dual-Role Experience

At startup, users select their role:
- **Candidate Mode**:
  - `Load Resume` (TXT / PDF ingestion + SQLite persistence)
  - `Analyze Resume` (Quality score, strengths, areas for improvement)
  - `View Job Matches` (Algorithmic match ranking)
  - `Skill Gap Analysis` (Coverage, missing required/preferred skills)
  - `Resume Strength Analysis` (Component weights, technology frequency)
  - `Job Recommendations` (Readiness score & targeted guidance)
  - `Apply for Job` (Interactive job selection, match preview, submission)
  - `My Applications` (Complete application history & withdrawal option)
  - `Saved Jobs` (Bookmark positions for later review)
  - `Candidate Dashboard` (Metrics on quality, skills, projects, apps, and top match)
  - `Generate Full Report` (Generates comprehensive report with Database Summary)
  - `Display Resume` (Displays parsed resume details and section completion)
- **Recruiter Mode**:
  - `Create Job` (Console prompt with validation, saves directly to SQLite)
  - `View Jobs` (Lists all active positions and required/preferred skills)
  - `View Applications` (Inspect applicants, match scores, readiness, and status)
  - `Update Application Status` (Transition candidate through recruitment pipeline)
  - `Recruiter Dashboard` (Aggregate metrics: total jobs, total applications, status breakdown)

### 6. Security & Transaction Integrity
- **Parameterized SQL**: All database operations use `sqlite3_prepare_v2` with `sqlite3_bind_*`, preventing SQL injection vulnerabilities.
- **ACID Transactions**: Multi-table insertions (such as jobs with skills, or candidates with resumes, education, and projects) execute inside `BEGIN TRANSACTION` / `COMMIT`, with automatic `ROLLBACK` on errors.

### 7. Verification Results (Phase 1–6)
Automated testing in `tests/test_runner.cpp` includes:
- **Phase 3 Tests (1–10)**: 32 assertions
- **Phase 4 Tests (11–20)**: 32 assertions
- **Phase 5 Tests (21–36)**: 42 assertions
- **Phase 6 Tests (37–62)**: 72 assertions
- **Total Assertions**: **178 / 178 PASSED**

---

## Phase 7 — Advanced NLP, Semantic-Style Matching & Explainable Recruitment Intelligence

Phase 7 upgrades SmartHire from a rule-based matching system into an advanced, offline, deterministic NLP-style resume analysis and explainable recruitment engine.

### 1. Architectural Pipeline

```text
                RESUME
                   │
                   ▼
          Text Preprocessor
                   │
                   ▼
          Keyword Extractor
                   │
                   ▼
          Skill Normalizer
                   │
                   ▼
        ┌──────────────────────┐
        │ Advanced Job Matcher │
        └──────────────────────┘
          /        │          \
         /         │           \
   Skill Match  Text Match  Context Match
         \         │           /
          \        │          /
             Match Explanation
                    │
                    ▼
             Recommendations
```

### 2. Core Modules & Responsibilities

| Header / Source | Class / Component | Responsibilities |
|---|---|---|
| `TextPreprocessor.h/.cpp` | `TextPreprocessor` | Lowercasing, punctuation stripping, whitespace normalization, tokenization with technology preservation (`c++`, `c#`, `.net`, `node.js`), stop-word removal. |
| `KeywordExtractor.h/.cpp` | `KeywordExtractor` | Technical single-word terms and multi-word phrase extraction (`machine learning`, `database management`, `rest api`), frequency mapping. |
| `SkillNormalizer.h/.cpp` | `SkillNormalizer` | Alias normalization (`cpp` -> `C++`), skill taxonomy categorization, equivalence detection, related technology detection (`SQL` <-> `MySQL`, `Git` <-> `GitHub`). |
| `TextSimilarity.h/.cpp` | `TextSimilarity` | Jaccard token-set similarity, term-frequency Cosine similarity, keyword-weighted similarity, and local TF-IDF style relevance scoring. |
| `SemanticMatcher.h/.cpp` | `SemanticMatcher` | Multi-factor contextual matching: exact and related skill scoring, text relevance, project technology overlap, experience keyword overlap, and education alignment. |
| `MatchExplanation.h/.cpp` | `MatchExplanation` | Transparent struct capturing granular score breakdown, exact/related matches, missing required/preferred skills, data-driven strengths, and actionable suggestions. |
| `AdvancedJobMatcher.h/.cpp` | `AdvancedJobMatcher` | Top-level matching engine orchestrating ranking, 35/20/15/10/10/10 scoring formula, formatted explanation display, and classic-vs-advanced algorithm comparison. |

### 3. Intelligence & Matching Algorithms

#### A. Text Preprocessing & Technology Preservation
- Clean token stream extraction preserving symbols crucial to programming terminology: `C++`, `C#`, `.NET`, `Node.js`, `React.js`, `CI/CD`, `PL/SQL`.
- Stop-word dictionary removes high-frequency non-informative English words (`the`, `and`, `using`, `is`, `with`) while preserving technical keywords.

#### B. Skill Taxonomy & Alias Normalization
- Centralized knowledge dictionary (`data/skill_dictionary.txt`) classifying canonical technologies into domains (`Programming Languages`, `Databases`, `Web`, `Cloud`, `DevOps`, `Tools`).
- Strict distinction between:
  - **Equivalent Skills**: `cpp` == `C++`, `js` == `JavaScript`, `git hub` == `GitHub`.
  - **Related Technologies**: `SQL` <-> `MySQL`, `Git` <-> `GitHub`, `Node.js` <-> `Express`.
  - **Distinct Technologies**: `Java` != `JavaScript`, `C++` != `C#`.

#### C. Similarity Metrics
- **Jaccard Similarity**: $J(A, B) = \frac{|A \cap B|}{|A \cup B|}$, measuring token overlap on $[0.0, 1.0]$.
- **Cosine Similarity**: $\cos(\theta) = \frac{\mathbf{A} \cdot \mathbf{B}}{\|\mathbf{A}\| \|\mathbf{B}\|}$, computing term-frequency vector alignment with safe $0.0$ handling for zero-magnitude inputs.
- **TF-IDF Relevance**: Lightweight document scoring calculating term frequency normalized by document length combined with inverse document weighting downweighting generic vocabulary (`software`, `development`, `system`) and boosting domain-specific technologies (`Docker`, `Kubernetes`, `React`).

### 4. Advanced Score Formula

The compatibility score uses a comprehensive multi-factor breakdown totaling 100%:

```text
Overall Match Score = 
    (Skill Match          × 0.35) +
    (Text Relevance       × 0.20) +
    (Project Relevance    × 0.15) +
    (Experience Relevance × 0.10) +
    (Education Match      × 0.10) +
    (Resume Quality       × 0.10)
```

- **Skill Match (35%)**: Required skills carry 75% weight and preferred skills 25%. Exact matches receive 100% credit, related technologies receive 60% partial credit.
- **Text Relevance (20%)**: Combines keyword-weighted cosine similarity (50%) and TF-IDF relevance (50%).
- **Project Relevance (15%)**: Evaluates candidate project technology tags and descriptions against job requirements.
- **Experience Relevance (10%)**: Analyzes candidate professional experience and responsibilities against job requirements.
- **Education Match (10%)**: Degree and field of study alignment.
- **Resume Quality (10%)**: Profile completeness and structural quality score from Phase 4.

### 5. Match Classifications

| Score Range | Classification |
|---|---|
| 90.00% – 100.00% | Exceptional Match |
| 80.00% – 89.99% | Strong Match |
| 70.00% – 79.99% | Good Match |
| 60.00% – 69.99% | Moderate Match |
| Below 60.00% | Low Match |

### 6. Explainable Output & Diagnostics

- **Advanced Match Analysis (Option 13)**: Displays overall score, score breakdown, exact skills, related skills, missing required skills, missing preferred skills, relevant projects, strengths, and targeted improvement areas.
- **Compare Matching Algorithms (Option 14)**: Side-by-side comparison between the Phase 2 classic matcher and Phase 7 advanced matcher, detailing the score difference and explaining why the advanced engine provides superior recruitment intelligence.
- **Enhanced Reports**: Report files generated by `ReportGenerator` include the full `ADVANCED MATCH ANALYSIS` section.

### 7. Verification Results (Phase 7: 254 / 254 Passed)

Automated testing in `tests/test_runner.cpp` includes:
- **Phase 3 Tests (1–10)**: 32 assertions
- **Phase 4 Tests (11–20)**: 32 assertions
- **Phase 5 Tests (21–36)**: 42 assertions
- **Phase 6 Tests (37–62)**: 72 assertions
- **Phase 7 Tests (63–100)**: 76 assertions
- **Subtotal Assertions**: **254 / 254 PASSED**

---

## Phase 8 — Authentication, RBAC & Account Management

Phase 8 elevates SmartHire from a local recruitment engine into a **secure multi-user recruitment application** with authentication, role-based access control (RBAC), session management, brute-force protection, ownership validation, and audit logging.

### 1. Security Architecture

```text
                  LOGIN / REGISTER
                         |
                         v
              +---------------------+
              | Authentication      |
              | Manager             |
              +----------+----------+
                         |
             +-----------+-----------+
             |                       |
             v                       v
      Password Hasher         Session Manager
             |                       |
             +-----------+-----------+
                         |
                         v
              +---------------------+
              | Security Context    |
              +----------+----------+
                         |
              +----------+----------+
              |                     |
              v                     v
         CANDIDATE             RECRUITER
              |                     |
              +----------+----------+
                         |
                         v
                 Authorization
                         |
                         v
                    SQLite DB
```

### 2. Security Modules

| Module | Header / Source | Primary Responsibility |
|---|---|---|
| **SecurityUtils** | `SecurityUtils.h` / `SecurityUtils.cpp` | SHA-256, HMAC-SHA256, PBKDF2-HMAC-SHA256, constant-time comparison, OS-level secure random byte generation, input validation, and time helpers. |
| **PasswordHasher** | `PasswordHasher.h` / `PasswordHasher.cpp` | PBKDF2 key derivation (100,000 iterations minimum, 16-byte random salt, 32-byte derived key) and constant-time password verification. |
| **AccountRepository** | `AccountRepository.h` / `AccountRepository.cpp` | Parameterized SQLite CRUD for accounts, sessions, password reset tokens, and recruiter/candidate linkages. |
| **SessionManager** | `SessionManager.h` / `SessionManager.cpp` | Cryptographically random 32-byte session tokens (stored hashed as SHA-256 in DB), 30-minute expiration, and session revocation. |
| **AuthorizationManager** | `AuthorizationManager.h` / `AuthorizationManager.cpp` | Role-Based Access Control and strict resource ownership validation for candidate applications and recruiter jobs. |
| **AuthenticationManager** | `AuthenticationManager.h` / `AuthenticationManager.cpp` | End-to-end registration, login, logout, password change, and local password recovery. |
| **AuditLogger** | `AuditLogger.h` / `AuditLogger.cpp` | Operational and security event logging (never logs passwords or tokens). |

### 3. Account Data Model & Roles

- **Roles**:
  - `CANDIDATE`: Job seekers managing their own profiles, uploading resumes, running AI matching, saving jobs, and applying.
  - `RECRUITER`: Hiring teams posting jobs, managing applicants, and updating statuses.
- **Account Statuses**:
  - `ACTIVE`: Normal operating status.
  - `LOCKED`: Automatically locked after 5 consecutive failed login attempts.
  - `DISABLED`: Administratively disabled account; cannot authenticate.

```text
accounts:
  id (INTEGER PRIMARY KEY)
  username (TEXT UNIQUE NOT NULL)
  email (TEXT UNIQUE NOT NULL)
  password_hash (TEXT NOT NULL)
  password_salt (TEXT NOT NULL)
  role (TEXT CHECK IN ('CANDIDATE', 'RECRUITER'))
  status (TEXT CHECK IN ('ACTIVE', 'LOCKED', 'DISABLED'))
  failed_login_attempts (INTEGER DEFAULT 0)
  last_login_at (DATETIME)
  created_at (DATETIME)
  updated_at (DATETIME)
```

### 4. Password Security & Cryptography

- **Algorithm**: `PBKDF2-HMAC-SHA256`
- **Iterations**: 100,000 iterations (conforming to modern cryptographic standards)
- **Salt**: 16 cryptographically secure random bytes generated via OS entropy (`CryptGenRandom` / `CryptAcquireContext` on Windows with `std::random_device` fallback)
- **Derived Key**: 32 bytes (64 hex characters)
- **Constant-Time Comparison**: Byte-by-byte XOR comparison preventing timing side-channel attacks
- **Password Policy**:
  - Minimum 8 characters
  - At least 1 uppercase letter (`A-Z`)
  - At least 1 lowercase letter (`a-z`)
  - At least 1 digit (`0-9`)
  - Rejection of weak common passwords (`password`, `12345678`, `password123`, `qwerty123`)

### 5. Session Management

- **Token Generation**: 32 cryptographically secure random bytes formatted as a 64-character hexadecimal string.
- **Storage Protection**: The raw token is **never** written to SQLite. SmartHire only persists the `SHA-256(rawToken)` hash in the `sessions` table.
- **Expiration Policy**: Active sessions automatically expire after 30 minutes.
- **Revocation**: Sessions are immediately revoked upon logout or password change.

### 6. Role-Based Access Control & Ownership Validation

- **Candidate Permissions**:
  - View & edit own profile (`candidates.account_id == ctx.accountId`)
  - Upload & analyze own resume
  - Browse jobs & run Advanced Semantic Matching
  - Save jobs & submit applications
  - Withdraw own applications (`application.candidate_id == ctx.candidateId`)
  - View own activity audit logs
  - *Strictly prohibited*: Creating jobs, modifying other candidate profiles, changing application review statuses, or viewing recruiter dashboards.
- **Recruiter Permissions**:
  - View recruiter dashboard metrics
  - Create job listings (automatically linked via `jobs.recruiter_id = ctx.accountId`)
  - Edit & view applications for owned jobs
  - Review candidate dossiers for applicants
  - Update application statuses (`APPLIED` -> `UNDER_REVIEW` -> `SHORTLISTED` -> `INTERVIEW` -> `SELECTED` / `REJECTED`)
  - View own activity audit logs
  - *Strictly prohibited*: Modifying candidate profiles, changing candidate passwords, or editing jobs created by other recruiters.

### 7. Brute-Force & Injection Protection

- **Failed Login Protection**: Tracking consecutive failed attempts. 5 failed attempts immediately transition account to `LOCKED`.
- **Generic Error Responses**: Generic messages (`"Invalid username or password."`) prevent account enumeration.
- **SQL Injection Prevention**: 100% of authentication, session, and repository database operations utilize parameterized prepared statements (`sqlite3_prepare_v2`, `sqlite3_bind_text`, `sqlite3_bind_int`). Raw string concatenation in queries is strictly avoided. Tested against payloads such as `' OR '1'='1`, `admin' --`, and `" OR ""=`.

### 8. Audit Logging

Every critical event is recorded in the `audit_logs` table with account ID, action, details, and timestamp:
- `REGISTRATION_SUCCESS`
- `LOGIN_SUCCESS`
- `LOGIN_FAILURE`
- `ACCOUNT_LOCKED`
- `LOGOUT`
- `PASSWORD_CHANGED`
- `PASSWORD_RESET`
- `PROFILE_UPDATED`
- `RESUME_UPLOADED`
- `JOB_CREATED`
- `JOB_UPDATED`
- `APPLICATION_SUBMITTED`
- `APPLICATION_WITHDRAWN`
- `APPLICATION_STATUS_CHANGED`
- `SAVED_JOB_ADDED`
- `SAVED_JOB_REMOVED`

*Privacy Guarantee*: Passwords, password hashes, salts, and session tokens are strictly excluded from audit logs.

### 9. Local Password Reset (Demo Mode)

Provides a controlled, offline password recovery workflow:
1. Verify username and registered email.
2. Generate single-use, 6-digit numeric reset token with 15-minute expiration (persisted as SHA-256 hash).
3. Validate reset code, update password, unlock account if locked, reset failed attempt counters, and invalidate all existing active sessions.

*Academic Project Note*: This is a local development and educational password recovery mechanism, not an email-based production identity recovery system.

### 10. Database Migration

Opening an existing Phase 6/7 database triggers automated, non-destructive migration in `DatabaseManager::createTables()`:
- Missing tables (`accounts`, `sessions`, `audit_logs`, `password_reset_tokens`) are safely created.
- Foreign key linkage columns (`candidates.account_id`, `jobs.recruiter_id`) are added via `ALTER TABLE` if missing.
- Existing candidates, resumes, jobs, and application tracking records are 100% preserved.

### 11. Complete Test Suite (361 / 361 Passed)

Automated testing in `tests/test_runner.cpp` includes:
- **Phase 3 Tests (1–10)**: 32 assertions
- **Phase 4 Tests (11–20)**: 32 assertions
- **Phase 5 Tests (21–36)**: 42 assertions
- **Phase 6 Tests (37–62)**: 72 assertions
- **Phase 7 Tests (63–100)**: 76 assertions
- **Phase 8 Tests (101–158)**: 107 new assertions
- **TOTAL TEST ASSERTIONS (Phase 8)**: **361 / 361 PASSED**

---

## Phase 9 — REST API, HTTP Server & Web-Ready Backend Architecture

Phase 9 transforms SmartHire into a web-ready, multi-client backend by embedding a lightweight C++17 HTTP/REST server while keeping the interactive CLI fully functional.

```text
+-------------------------------------------------------------------------+
|                         Clients & Integrations                          |
|   - Web SPA (React/Vue)    - Mobile (Android)    - Postman / curl       |
+------------------------------------+------------------------------------+
                                     |  HTTP/1.1 (JSON & Multipart)
                                     v
+-------------------------------------------------------------------------+
|                  Embedded HTTP Server (cpp-httplib)                     |
|                                                                         |
|  [Security & Middleware Layer]                                          |
|   - In-Memory Sliding-Window Rate Limiter (10 req/min/IP)               |
|   - CORS Preflight & Headers (Access-Control-Allow-Origin: ...)         |
|   - Bearer Session Token Extractor & Authenticator                      |
|   - Role-Based Access Control (RBAC: CANDIDATE vs RECRUITER)            |
|   - Resource Ownership Guards (Applications, Resumes, Jobs)             |
|   - Security Headers (X-Content-Type-Options: nosniff, no-store)        |
|   - Privacy-Preserving Audit Logger (No passwords or tokens logged)     |
|                                                                         |
|  [REST Controllers]                                                     |
|   - AuthController          - CandidateController - ResumeController    |
|   - JobController           - MatchingController  - SavedJobController  |
|   - ApplicationController   - RecruiterController - DashboardController |
|   - SystemController                                                    |
+------------------------------------+------------------------------------+
                                     |  Native C++ Method Invocations
                                     v
+-------------------------------------------------------------------------+
|                       Core SmartHire Services                           |
|   - AuthenticationManager  - AuthorizationManager   - SessionManager    |
|   - AdvancedJobMatcher     - SemanticMatcher        - SkillGapAnalyzer  |
|   - ResumeStrengthAnalyzer - RecommendationEngine   - ApplicationManager|
|   - ResumeInputManager     - ResumeParser           - AuditLogger       |
+------------------------------------+------------------------------------+
                                     |  Parameterized Prepared Statements
                                     v
+-------------------------------------------------------------------------+
|                  Persistence Layer (SQLite3 Engine)                     |
|   - accounts  - sessions   - candidates  - resumes  - jobs              |
|   - applications  - saved_jobs  - candidate_skills  - audit_logs        |
+-------------------------------------------------------------------------+
```

### 1. Dual Execution Modes

SmartHire supports two distinct run modes from the exact same compiled binary:

```bash
# 1. Interactive Console Mode (Phases 1-8 CLI)
./build/SmartHire.exe

# 2. REST API Server Mode (Phase 9 Embedded Server)
./build/SmartHire.exe --server

# Optional command-line parameter overrides:
./build/SmartHire.exe --server --port 8080 --host 127.0.0.1
```

### 2. Embedded Server Configuration (`config/server.conf`)

```ini
# SmartHire Server Configuration
host=127.0.0.1
port=8080
threads=4
session_timeout_minutes=30
max_upload_mb=10
cors_origin=http://localhost:5173
```

### 3. REST API Endpoint Matrix

| Method | Endpoint | Access Level | Description |
| :--- | :--- | :--- | :--- |
| `GET` | `/api/v1/health` | Public | System status and database connectivity |
| `POST`| `/api/v1/auth/register/candidate` | Public | Register new candidate account |
| `POST`| `/api/v1/auth/register/recruiter` | Public | Register new recruiter account |
| `POST`| `/api/v1/auth/login` | Public (Rate Limited) | Obtain 64-char Bearer session token |
| `POST`| `/api/v1/auth/logout` | Authenticated | Revoke caller's active session |
| `GET` | `/api/v1/auth/me` | Authenticated | Current user identity and role |
| `POST`| `/api/v1/auth/change-password` | Authenticated | Change password and revoke all sessions |
| `GET` | `/api/v1/candidates/me` | Candidate Only | Candidate profile data |
| `PUT` | `/api/v1/candidates/me` | Candidate Only | Update profile phone or location |
| `POST`| `/api/v1/resume/upload` | Candidate Only | Multipart `.txt`/`.pdf` upload (max 10MB) |
| `POST`| `/api/v1/resume/import` | Candidate Only | Import server-local resume file |
| `GET` | `/api/v1/resume` | Candidate Only | Full parsed structured resume |
| `GET` | `/api/v1/resume/analysis` | Candidate Only | Quality scoring & factor breakdown |
| `GET` | `/api/v1/resume/strengths` | Candidate Only | Core strengths & technology frequency |
| `GET` | `/api/v1/resume/recommendations` | Candidate Only | Prioritized improvement recommendations |
| `GET` | `/api/v1/jobs` | Public | Search and paginate jobs (limit <= 100) |
| `GET` | `/api/v1/jobs/{id}` | Public | Detailed job requisition |
| `GET` | `/api/v1/matches` | Candidate Only | Top match rankings against open jobs |
| `GET` | `/api/v1/matches/{id}/explanation` | Candidate Only | Detailed score and skill gap breakdown |
| `GET` | `/api/v1/saved-jobs` | Candidate Only | List bookmarked jobs |
| `POST`| `/api/v1/saved-jobs/{id}` | Candidate Only | Bookmark job |
| `DELETE`| `/api/v1/saved-jobs/{id}` | Candidate Only | Remove saved bookmark |
| `GET` | `/api/v1/applications` | Candidate Only | List candidate's submitted applications |
| `POST`| `/api/v1/applications` | Candidate Only | Apply to job with match computation |
| `GET` | `/api/v1/applications/{id}` | Owner Only | Get application status |
| `POST`| `/api/v1/applications/{id}/withdraw` | Owner Only | Withdraw pending application |
| `POST`| `/api/v1/recruiter/jobs` | Recruiter Only | Post new job opening |
| `PUT` | `/api/v1/recruiter/jobs/{id}` | Recruiter Owner | Update owned job opening |
| `DELETE`| `/api/v1/recruiter/jobs/{id}` | Recruiter Owner | Remove owned job opening |
| `GET` | `/api/v1/recruiter/applications` | Recruiter Only | View applicants for owned jobs |
| `PUT` | `/api/v1/recruiter/applications/{id}/status` | Recruiter Owner | Update applicant hiring status |
| `GET` | `/api/v1/dashboard/candidate` | Candidate Only | Candidate overview metrics |
| `GET` | `/api/v1/dashboard/recruiter` | Recruiter Only | Recruiter hiring metrics |
| `GET` | `/api/v1/activity` | Authenticated | User-isolated audit trail |

### 4. Build & Compilation Instructions

SmartHire uses CMake and MinGW-w64 with standard C++17:

```bash
# Configure build
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Compile CLI and test suite
cmake --build build -j4

# Execute comprehensive automated test suite
./build/test_runner.exe
```

### 5. Verification & Test Suite Results

```text
========================================
TEST RESULTS: 469 / 469 PASSED
========================================

Compilation errors: 0
Compiler warnings: 0
Total test suites: 48 (Tests 1 to 216)
Total assertions verified: 469
Failed assertions: 0
Assertion pass rate: 100.0%
```

- **Phase 3 Tests (1–10)**: 32 assertions
- **Phase 4 Tests (11–20)**: 32 assertions
- **Phase 5 Tests (21–36)**: 42 assertions
- **Phase 6 Tests (37–62)**: 72 assertions
- **Phase 7 Tests (63–100)**: 76 assertions
- **Phase 8 Tests (101–158)**: 107 assertions
- **Phase 9 Tests (159–216)**: 108 assertions
- **Phase 11 Tests (217–227)**: 23 assertions
- **TOTAL BACKEND ASSERTIONS**: **492 / 492 PASSED**
- **TOTAL FRONTEND TESTS (Vitest)**: **43 / 43 PASSED**

---

## Phase 10 — Professional Web Frontend, Candidate Portal & Recruiter Portal

Phase 10 introduces a responsive, modern web application built with **React 19**, **Vite**, and **Vanilla CSS** that communicates directly with the C++ REST API.

### Web Architecture

```text
                SmartHire Web
                     │
               React 19 + Vite
                     │
             REST API (/api/v1/)
                     │
          C++ Embedded HTTP Server
                     │
              SmartHire Core
       (NLP, Matching, Auth, RBAC)
                     │
                  SQLite
```

---

## Phase 11 — Production Readiness, Deployment, Advanced QA & Final Packaging

Phase 11 transforms SmartHire into an enterprise-grade, production-ready software project with robust configuration management, automated health diagnostics, demo seeding tooling, complete integration testing, and comprehensive documentation.

### 1. Unified Launchers & Workflow Automation
- **One-Command Dev Startup (`start-dev.bat` / `start-dev.sh`)**: Starts both the C++ REST server (`:8080`) and Vite dev server (`:5173`) simultaneously with environment verification and URL output.
- **One-Click Production Build (`build-all.bat`)**: Compiles backend in Release mode, runs the 492 backend assertions, installs frontend packages, and generates static production bundles in `frontend/dist/`.
- **Demo Data Tooling (`seed-demo.bat` / `clear-demo.bat`)**: Deterministically populates or removes realistic test accounts, multi-stage applications, and technical job requisitions.

### 2. Configuration Management
SmartHire eliminates hardcoded host/port assumptions with a clean 4-tier configuration hierarchy:
`CLI Flags` > `Environment Variables` > `config/server.conf` > `Compiled Defaults`.
- Configurable settings: `host`, `port`, `database_path`, `upload_directory`, `session_timeout`, `max_upload_mb`, `cors_origin`, and sliding-window rate limit parameters.
- Environment variables: `SMARTHIRE_HOST`, `SMARTHIRE_PORT`, `SMARTHIRE_DB_PATH`, `SMARTHIRE_UPLOAD_DIR`, `SMARTHIRE_CORS_ORIGIN`.
- Frontend environment decoupling: `VITE_API_BASE_URL` with provided `.env.example`.

### 3. Health & Readiness Diagnostics
Diagnostic probes compatible with standard orchestration engines:
- **Liveness Probe (`GET /api/v1/health/live`)**: Ultra-fast process liveness verification.
- **Readiness Probe (`GET /api/v1/health/ready`)**: Verifies active SQLite execution (`SELECT 1`) and filesystem write accessibility.
- **Comprehensive Health (`GET /api/v1/health`)**: Reports overall system state, database connectivity, storage status, and server uptime.

### 4. Demo Data Suite & Presentation Accounts
Deterministic demo dataset ready for immediate evaluator presentation:
- **Candidate 1**: `cand_alice` / `AliceP@ss123` (Alice Smith — Senior C++ Developer, Pune)
- **Candidate 2**: `cand_rahul` / `RahulP@ss123` (Rahul Sharma — Frontend Engineer, Pune)
- **Candidate 3**: `cand_priya` / `PriyaP@ss123` (Priya Nair — Fullstack Engineer, Pune)
- **Recruiter 1**: `rec_techcorp` / `RecruiterP@ss123` (Bob Vance — TechCorp India)
- **Recruiter 2**: `rec_innovate` / `RecruiterP@ss123` (Meera Patel — InnovateAI Labs)
- **Jobs**: `J101` through `J105` (Senior C++ Engineer, Systems Engineer, React Specialist, Python Fullstack, QA Automation).

### 5. Final Quality Assurance & Verification
- **Backend Test Suite**: **492 / 492 assertions PASSED (100%)**
- **Frontend Test Suite**: **43 / 43 Vitest tests PASSED (100%)**
- **Compiler Warnings / Errors**: **0**

---

## Phase 12 — Advanced Intelligence, Analytics & Final Showcase

Phase 12 transforms SmartHire from a feature-complete recruitment platform into a sophisticated, deterministic **career decision-support system**.

### 1. Deterministic Career Intelligence Engine
- **Career Readiness Indicators**: Evaluates profile completeness, skill coverage, experience alignment, and resume structure into a standardized **Readiness Index** and explainable diagnostic factors (no opaque black-box AI).
- **Skill Gap Demand Analytics (`/candidate/skill-gap`)**: Decomposes candidate competencies into exact matches, related matches, and missing requirements categorized by priority (`REQUIRED` vs `PREFERRED`) with demand frequency across open requisitions.
- **Job Market Insights & Skill Co-occurrence (`/candidate/market-insights`)**: Analyzes the active job database to report top requested technical skills, geographic concentrations (e.g. Pune, Mumbai, Bangalore), employment type splits, and deterministic skill co-occurrence pairs (`C++ + STL`, `React + TypeScript`).
- **Explainable Job Recommendations**: Generates ranked job suggestions with transparent, human-readable rationale ("✓ 8/10 required skills matched", "✓ Experience requirement satisfied", "✓ Strong semantic similarity").
- **"Why This Job?" Feature (`/candidate/jobs/:id`)**: Comprehensive breakdown on job details showing strong matches, related competencies, missing requirements, and experience thresholds.
- **Resume vs. Job Detailed Comparison (`/candidate/compare`)**: Side-by-side interactive matrix comparing candidate qualifications against specific job requisitions.
- **Candidate Application Analytics (`/candidate/analytics`)**: Lifetime application volume, active vs. withdrawn applications, status breakdown, and applied job match score averages.

### 2. Recruiter Intelligence & Hiring Analytics
- **Recruiter Hiring Analytics (`/recruiter/analytics`)**: Requisition overview, multi-stage application pipeline funnel (`APPLIED` ➔ `UNDER_REVIEW` ➔ `SHORTLISTED` ➔ `INTERVIEW` ➔ `SELECTED`), and a 5-tier applicant match score histogram (90–100%, 80–89%, 70–79%, 60–69%, <60%).
- **Job Analytics (`/recruiter/jobs/:id/analytics`)**: Per-job analytics detailing applicant volume, average match score, highest match score, and stage distribution.
- **Candidate Comparison Matrix (`/recruiter/jobs/:id/compare`)**: Factual, non-evaluative applicant comparison table (scores, required skill counts, experience years, current stage) empowering human recruiters to make informed final decisions without automated labelling.

### 3. Security Verification & Cryptography Consistency
- **PBKDF2-HMAC-SHA256**: Confirmed and verified **100,000 iterations**, 16-byte salt, and 32-byte derived key (verified in Test 238).
- **Strict RBAC & Cross-Tenant Isolation**: Verified that candidates cannot access recruiter analytics, and recruiters cannot access another recruiter's private analytics or job comparisons.

### 4. Verified System Test Baseline
- **Backend Test Suite**: **524 / 524 assertions PASSED (100%)** (Tests 1–238)
- **Frontend Test Suite**: **60 / 60 Vitest tests PASSED (100%)** (5 test suites)
- **Compiler Warnings / Errors**: **0**
- **Frontend Production Build**: **SUCCESS** (`dist/` 460 kB)

### 5. Documentation Suite
Detailed technical documentation is provided in the `docs/` directory:
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — System topology, component interactions, and data model.
- [docs/SECURITY.md](docs/SECURITY.md) — Threat model, PBKDF2 cryptography, session security, and defense-in-depth.
- [docs/TESTING.md](docs/TESTING.md) — Test pyramid, assertion matrix, Vitest specifications, and test commands.
- [docs/DEPLOYMENT.md](docs/DEPLOYMENT.md) — Prerequisites, build workflows, local deployment, and disaster recovery.
- [docs/DEMO_GUIDE.md](docs/DEMO_GUIDE.md) — Step-by-step evaluator presentation sequence.
- [docs/API.md](docs/API.md) — Comprehensive REST API reference with request/response schemas.
- [docs/SmartHire_API.postman_collection.json](docs/SmartHire_API.postman_collection.json) — Ready-to-import Postman collection.

---

## Quick Start Summary

### 1. Build Everything
```cmd
build-all.bat
```

### 2. Populate Demo Accounts & Jobs
```cmd
seed-demo.bat
```

### 3. Launch Development Servers
```cmd
start-dev.bat
```
Visit `http://localhost:5173/` in your browser.

---

## Known Limitations & Academic Scope
- **Offline / Local Deployment Focus**: The application is tailored for on-premises, local workstation, or intranet deployment with SQLite. Enterprise clustered deployments with PostgreSQL/MySQL are outside current scope.
- **Deterministic NLP**: The matching engine uses an explainable vector-space TF-IDF algorithm rather than non-deterministic deep learning transformers, intentionally prioritizing low latency, zero cloud egress, and explainability.

---

## License

This project is developed for educational and academic evaluation purposes.


