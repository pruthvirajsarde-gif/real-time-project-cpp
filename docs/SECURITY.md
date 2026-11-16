# SmartHire — Security Architecture & Threat Mitigation Guide

**Version:** 12.0 (Phase 12 Complete)  
**Security Classification:** Local & Enterprise-Ready Application Security  
**Audit Standard:** Authoritative Server-Side Defense-in-Depth  

---

## 1. Overview & Threat Model

SmartHire is designed with a **zero-trust, authoritative server-side security architecture**. The React frontend serves exclusively as a presentation layer and does not hold authoritative trust. All security checks, authorization bounds, input sanitization, and cryptographic operations are enforced in C++ backend services before executing operations against the SQLite database or filesystem.

### Threat Categories & Mitigation Matrix
| Threat Category | Potential Impact | SmartHire Defense Mechanism |
| :--- | :--- | :--- |
| **Credential Stuffing / Brute Force** | Account takeover | Sliding-window IP rate limiting + 5-attempt failed login lockout |
| **Password Database Leakage** | Plaintext exposure | PBKDF2-HMAC-SHA256 (100,000 iterations) with 16-byte unique salt per user |
| **Token Theft / DB Compromise** | Session hijacking | Session tokens stored as SHA-256 hashes; raw bearer tokens never persisted |
| **Horizontal Escalation** | Data breach between peers | Strict resource ownership checks comparing session identity to entity owner |
| **Vertical Privilege Escalation** | Recruiter actions by candidate | Route-level RBAC role guards returning `403 Forbidden` |
| **Cross-Tenant Analytics Tampering** | Viewing peer intelligence | Recruiter ownership validation on all job analytics and candidate comparisons |
| **SQL Injection (SQLi)** | Data tampering / extraction | 100% parameterized SQLite statements (`sqlite3_prepare_v2` / `sqlite3_bind_*`) |
| **Path Traversal / Arbitrary Write** | Filesystem corruption | Path normalization, base directory sanitization, randomized UUID file naming |
| **Malicious Payload / DoS** | Server crash / buffer overflow | 10MB payload size ceiling, strict JSON parser exception catching |
| **Clickjacking / MIME Sniffing** | Browser-side exploitation | HTTP security headers (`X-Frame-Options: DENY`, `X-Content-Type-Options: nosniff`) |

---

## 2. Authentication & Credential Storage

### 2.1 Password Hashing & Salting (Verified Configuration)
SmartHire avoids weak or single-iteration hashing (such as raw MD5 or plain SHA-256). Passwords undergo cryptographic stretching:
- **Algorithm:** PBKDF2 (Password-Based Key Derivation Function 2)
- **Pseudorandom Function (PRF):** HMAC-SHA256
- **Iteration Count:** 100,000 cycles (authoritatively configured in `PasswordHasher::DEFAULT_ITERATIONS`, verified by Test 238)
- **Salt Generation:** 16 bytes (128 bits) of cryptographically secure random bytes generated via platform entropy (`CryptGenRandom` / `std::random_device`), formatted as a 32-character hex string.
- **Derived Key Length:** 32 bytes (256 bits, 64-character hex representation).

### 2.2 Credential Protection & Scrubbing
- Password hashes and salts are **never included** in any REST API response payload.
- API models for accounts, candidates, and recruiters explicitly omit authentication secrets.
- Server logs strictly redact password fields, salts, and credential inputs.

### 2.3 Account Lockout Mechanism
To prevent offline and online dictionary attacks:
- Accounts track consecutive failed login attempts.
- After 5 consecutive failures, the account enters a temporary lockout state (15-minute default cooldown).
- Successful login automatically resets the failure counter to zero.

---

## 3. Session Security & Token Lifecycle

### 3.1 Bearer Token Generation
When a user successfully logs in:
1. The backend generates 32 bytes of secure random entropy (64 hex characters).
2. The raw token is returned to the client in the `Authorization: Bearer <token>` response header and JSON body.
3. The server computes the **SHA-256 digest** of the token and persists only the hash in the `sessions` table.
4. If an attacker gains read-only access to the SQLite database, they cannot use the stored session digests as bearer tokens.

### 3.2 Session Expiration & Revocation
- **Time-to-Live (TTL):** Sessions expire after a configurable duration (default: 86,400 seconds / 24 hours).
- **Explicit Logout:** Calling `POST /api/v1/auth/logout` immediately deletes the session hash from the database.
- **Password Reset Revocation:** Changing an account password invalidates all active sessions associated with that account.
- **Tampered / Expired Token Rejection:** Any request with an unparseable, altered, or expired bearer token returns `401 AUTH_INVALID_TOKEN` or `401 AUTH_SESSION_EXPIRED`.

---

## 4. Role-Based Access Control (RBAC) & Ownership

### 4.1 Role Hierarchy
SmartHire differentiates two first-class system roles:
- `CANDIDATE`: May manage personal resumes, view matching jobs, save jobs, submit job applications, and withdraw personal applications.
- `RECRUITER`: May create, update, and delete job requisitions, inspect applicant profiles, and update candidate application stages (`UNDER_REVIEW`, `SHORTLISTED`, `INTERVIEW`, `REJECTED`, `SELECTED`).

### 4.2 Vertical Role Boundaries
Every protected route executes an RBAC assertion before executing business logic:
- If a `CANDIDATE` calls `/api/v1/recruiter/jobs`, the server rejects the request with `403 FORBIDDEN_RECRUITER_ONLY`.
- If a `RECRUITER` calls `/api/v1/resume/upload` or `/api/v1/applications` (candidate submission), the server rejects the request with `403 FORBIDDEN_CANDIDATE_ONLY`.

### 4.3 Horizontal Tenant Isolation (Ownership Validation)
Possessing a valid role is necessary but not sufficient:
- **Candidate Isolation:** Candidate A cannot retrieve or modify Candidate B’s resume or profile data.
- **Recruiter Isolation:** Recruiter A cannot edit, delete, or transition applications for jobs owned by Recruiter B. Attempts return `403 FORBIDDEN_NOT_OWNER`.

---

## 5. Input Sanitization & Attack Mitigation

### 5.1 SQL Injection Prevention
All SQLite interactions utilize parameterized query interfaces. Dynamic SQL concatenation is strictly forbidden.
```cpp
// Secure parameter binding pattern used in SmartHire
sqlite3_stmt* stmt;
sqlite3_prepare_v2(db, "SELECT id, password_hash, salt FROM accounts WHERE username = ?;", -1, &stmt, nullptr);
sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
```

### 5.2 Upload Security & Directory Traversal Defense
Resume upload endpoints enforce strict isolation:
- Filenames containing path traversal characters (`..`, `/`, `\`) are stripped or rejected.
- Files are saved with server-generated unique filenames inside the dedicated `upload_directory`.
- File extensions are validated against a strict whitelist: only `.txt` and `.pdf` files are accepted.
- Total upload size is capped at 10 MB (configurable via `max_upload_mb` in `server.conf`).

### 5.3 HTTP Network Hardening
- **Security Headers:** Every HTTP response includes:
  - `X-Content-Type-Options: nosniff`
  - `X-Frame-Options: DENY`
  - `X-XSS-Protection: 1; mode=block`
  - `Cache-Control: no-store, no-cache, must-revalidate`
- **CORS Defense:** CORS headers are restricted to the configured `cors_origin` (default `http://localhost:5173` for development). Wildcard origins (`*`) are disallowed when credentialed bearer headers are used.
- **Rate Limiting:** Sliding-window limiter restricts brute-force attempts on `/api/v1/auth/login` and `/api/v1/auth/register/*` to 10 requests per minute per IP.

---

## 6. Audit Logging & System Accountability

All security-sensitive operations generate immutable entries in the `audit_logs` table:
- User registration and login attempts (success and failure)
- Password changes and session revocations
- Resume ingestion and deletion
- Job creation, updates, and removals
- Application submissions and status transitions

Audit logs record the actor account ID, timestamp, action type, IP address, and high-level description, without recording cleartext credentials or sensitive file contents.
