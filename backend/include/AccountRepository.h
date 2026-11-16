#pragma once

#include <string>
#include <vector>

// ============================================================
// Account — Core representation of an authenticated user
// ============================================================
struct Account {
    int id = 0;
    std::string username;
    std::string email;
    std::string passwordHash;
    std::string passwordSalt;
    std::string role;      // "CANDIDATE", "RECRUITER"
    std::string status;    // "ACTIVE", "LOCKED", "DISABLED"
    int failedLoginAttempts = 0;
    std::string createdAt;
    std::string lastLoginAt;
    std::string updatedAt;
};

// ============================================================
// SessionRecord — Database representation of an active session
// ============================================================
struct SessionRecord {
    int id = 0;
    int accountId = 0;
    std::string sessionTokenHash;
    std::string createdAt;
    std::string expiresAt;
    bool revoked = false;
};

// ============================================================
// AccountRepository — SQLite persistence for accounts, sessions,
// profile linkage, and password reset tokens.
// All queries use parameterized prepared statements.
// ============================================================
class AccountRepository {
public:
    // ── Account CRUD & Query Operations ─────────────────────────
    static int createAccount(const Account& account);
    static bool findByUsername(const std::string& username, Account& outAccount);
    static bool findByEmail(const std::string& email, Account& outAccount);
    static bool findById(int id, Account& outAccount);

    // ── Security State Updates ──────────────────────────────────
    static bool updatePassword(int accountId, const std::string& newHash, const std::string& newSalt);
    static bool updateStatus(int accountId, const std::string& status);
    static int incrementFailedAttempts(int accountId, int maxAttempts = 5);
    static bool resetFailedAttempts(int accountId);
    static bool updateLastLogin(int accountId);

    // ── Session Management ──────────────────────────────────────
    static bool createSession(const SessionRecord& session);
    static bool findSessionByTokenHash(const std::string& tokenHash, SessionRecord& outSession);
    static bool revokeSession(const std::string& tokenHash);
    static bool revokeAllSessionsForAccount(int accountId);
    static bool isSessionRevokedOrExpired(const std::string& tokenHash);

    // ── Candidate & Recruiter Ownership Linkage ─────────────────
    static bool linkCandidateToAccount(int candidateId, int accountId);
    static int getCandidateIdForAccount(int accountId);
    static int getAccountIdForCandidate(int candidateId);

    static bool linkJobToRecruiter(const std::string& jobId, int recruiterAccountId);
    static int getRecruiterAccountIdForJob(const std::string& jobId);
    static std::vector<std::string> getJobIdsForRecruiter(int recruiterAccountId);

    // ── Password Reset Verification Tokens ──────────────────────
    static bool storeResetCode(
        const std::string& username,
        const std::string& codeHash,
        const std::string& expiresAt);
    static bool verifyAndConsumeResetCode(
        const std::string& username,
        const std::string& codeHash);
};
