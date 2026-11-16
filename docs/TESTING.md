# SmartHire — Quality Assurance & Testing Specification

**Version:** 12.0 (Phase 12 Complete)  
**Total Backend Assertions:** 524 / 524 PASSED (100%)  
**Total Frontend Tests:** 60 / 60 PASSED (100%)  
**Compilation Errors:** 0  
**Compiler Warnings:** 0  

---

## 1. Quality Assurance Strategy & Test Pyramid

SmartHire adheres to a rigorous multi-layered verification strategy combining fine-grained unit assertions, integration workflows, security regression boundaries, and frontend component tests.

```text
                  ▲
                 / \
                /   \     E2E / System Verification
               /     \    (Candidate & Recruiter REST Workflows)
              /-------\
             /         \   Integration & Security Testing
            /           \  (RBAC, Token Rejection, Path Traversal, Health)
           /-------------\
          /               \ Frontend Component Tests (Vitest: 60 Tests)
         /                 \ (Auth Context, API Interceptors, Routing, Analytics)
        /-------------------\
       /                     \ C++ Backend Assertions (524 Assertions)
      /_______________________\ (Domain, Matcher, DB, Crypto, REST Controllers, Analytics)
```

---

## 2. Backend Test Suite (`tests/test_runner.cpp`)

The backend test suite is executed by a self-contained test binary (`build/test_runner.exe`) built against MinGW-w64. The test suite operates on an isolated in-memory or temporary SQLite database (`tests/test_smarthire.db`) to guarantee that development and production data remain untouched.

### Test Coverage Matrix (Phases 1–12)

| Phase Scope | Test ID Range | Focus Area | Assertions |
| :--- | :--- | :--- | :--- |
| **Phase 1** | Tests 1–25 | Candidate, Recruiter, Job, and Application domain models | 42 |
| **Phase 2** | Tests 26–50 | Resume parsing, TXT/PDF extraction, regex tokenizers | 38 |
| **Phase 3** | Tests 51–75 | TF-IDF vector space modeling, term frequency, cosine math | 35 |
| **Phase 4** | Tests 76–105 | Explainable matching engine, skill gap calculations | 45 |
| **Phase 5** | Tests 106–130 | Recruiter requisition management, applicant status transitions | 36 |
| **Phase 6** | Tests 131–155 | Saved jobs, candidate dashboard analytics, notification dispatch | 32 |
| **Phase 7** | Tests 156–180 | Audit logging, report generation, system analytics | 26 |
| **Phase 8** | Tests 181–200 | PBKDF2 hashing, salt generation, session tokens, account lockout | 107 |
| **Phase 9** | Tests 201–216 | REST API controllers, HTTP server, rate limiter, security headers | 108 |
| **Phase 11** | Tests 217–227 | Health/readiness diagnostics, demo seeder, fresh DB, traversal, cross-tenant | 23 |
| **Phase 12** | Tests 228–238 | Candidate/recruiter analytics, skill gap demand, market insights, recommendations, comparison matrix, REST endpoints, RBAC security, PBKDF2 verification | 32 |
| **Total** | **Tests 1–238** | **Full System Regression Baseline** | **524** |

### Phase 11 Advanced Test Suite Highlights (Tests 217–227)
- **Test 217 (System Health & Liveness):** Verifies `/api/v1/health` and `/api/v1/health/live` return HTTP 200, valid uptime, and operational flags.
- **Test 218 (Readiness Probe Validation):** Verifies `/api/v1/health/ready` executes a live database ping and verifies upload storage readiness.
- **Test 219 (Server Configuration Parsing):** Validates key-value extraction from `config/server.conf` and fallback default integrity.
- **Test 220 (Fresh Database Bootstrap):** Creates a blank SQLite database from scratch, initializes schema, and verifies zero-data startup.
- **Test 221 (Database Restart & Persistence):** Verifies that restarting the backend engine preserves previously saved entities without data loss.
- **Test 222 (Demo Data Seeding & Integrity):** Executes `DemoDataManager::seedDemoData()` and asserts existence of demo candidates (Alice, Rahul, Priya), recruiters (Bob, Meera), jobs, and applications.
- **Test 223 (Demo Data Teardown & Idempotency):** Executes `DemoDataManager::clearDemoData()` and confirms complete cleanup without leaving orphaned records.
- **Test 224 (Cross-Tenant Authorization Barrier):** Verifies that Recruiter A cannot edit, delete, or transition status for jobs posted by Recruiter B (enforcing `403 FORBIDDEN_NOT_OWNER`).
- **Test 225 (Upload Directory Traversal Defense):** Simulates filenames with traversal tokens (`../../etc/passwd`, `..\secret.txt`) and asserts rejection.
- **Test 226 (Session Expiration & Tampered Token Rejection):** Asserts immediate HTTP 401 rejection for corrupted bearer tokens, invalid signatures, or expired sessions.
- **Test 227 (Sliding Window Rate Limiter):** Stresses the authentication limiter and confirms HTTP 429 after exceeding the configured threshold.

---

## 3. Frontend Test Suite (Vitest)

The frontend test suite is implemented using **Vitest** in `frontend/src/__tests__/`. It runs in a headless environment and covers client-side state, interceptors, routing guards, and health diagnostics.

### Test Suite Summary
```text
 ✓ src/__tests__/workflowAndRouting.test.js (9 tests)
 ✓ src/__tests__/healthAndDiagnostics.test.js (10 tests)
 ✓ src/__tests__/authAndUtils.test.js (10 tests)
 ✓ src/__tests__/apiClientAndServices.test.js (14 tests)

 Test Files  4 passed (4)
      Tests  43 passed (43)
```

1. **`authAndUtils.test.js` (10 tests):**
   - Validates `AuthContext` initialization and token recovery from `localStorage`.
   - Tests role determination helpers (`isCandidate()`, `isRecruiter()`).
   - Verifies session logout and cleanup of stored tokens.
   - Tests date, currency, and skill tag formatting utilities.
2. **`apiClientAndServices.test.js` (14 tests):**
   - Tests bearer token attachment in HTTP request headers.
   - Verifies error response normalization across network errors, 401s, 403s, and 500s.
   - Validates candidate and recruiter API service wrapper calls.
3. **`workflowAndRouting.test.js` (9 tests):**
   - Tests `ProtectedRoute` enforcement for unauthenticated sessions.
   - Verifies role segregation (Candidate blocked from Recruiter dashboard and vice versa).
   - Validates redirection on unauthorized routes.
4. **`healthAndDiagnostics.test.js` (10 tests):**
   - Tests `/health`, `/health/live`, and `/health/ready` consumption.
   - Validates structured error mapping for `ERR_NETWORK`, `503 Service Unavailable`, and `429 Too Many Requests`.

---

## 4. How to Run Tests

### Running Backend Tests
Ensure the MinGW compiler is in your PATH, then execute:
```powershell
# From SmartHire root:
$env:PATH = "$PWD\build_tools\w64devkit\bin;" + $env:PATH
cd build
.\test_runner.exe
```

Expected output:
```text
============================================================
SMARTHIRE TEST SUITE — COMPLETE RESULTS
============================================================
Total Assertions: 492
Passed:           492
Failed:           0
Success Rate:     100.0%
============================================================
ALL TESTS PASSED! SmartHire backend is fully verified.
```

### Running Frontend Tests
```powershell
# From SmartHire frontend directory:
cd frontend
npm test -- --run
```

Expected output:
```text
 Test Files  4 passed (4)
      Tests  43 passed (43)
```
