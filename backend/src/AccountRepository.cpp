#include "AccountRepository.h"
#include "DatabaseManager.h"
#include "SecurityUtils.h"

#include <iostream>

// Helper to safely extract string from SQLite statement column
static std::string getColText(sqlite3_stmt* stmt, int col) {
    const unsigned char* text = sqlite3_column_text(stmt, col);
    return text ? reinterpret_cast<const char*>(text) : "";
}

// ── Account CRUD & Query Operations ─────────────────────────

int AccountRepository::createAccount(const Account& account) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return -1;

    const char* sql = 
        "INSERT INTO accounts (username, email, password_hash, password_salt, role, status, "
        "failed_login_attempts, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, 0, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    std::string now = SecurityUtils::getCurrentTimestamp();
    std::string createdAt = account.createdAt.empty() ? now : account.createdAt;
    std::string updatedAt = account.updatedAt.empty() ? now : account.updatedAt;
    std::string role = account.role.empty() ? "CANDIDATE" : account.role;
    std::string status = account.status.empty() ? "ACTIVE" : account.status;

    sqlite3_bind_text(stmt, 1, account.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, account.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, account.passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, account.passwordSalt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, role.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, createdAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, updatedAt.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        return static_cast<int>(sqlite3_last_insert_rowid(db));
    }
    return -1;
}

bool AccountRepository::findByUsername(const std::string& username, Account& outAccount) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = 
        "SELECT id, username, email, password_hash, password_salt, role, status, "
        "failed_login_attempts, created_at, last_login_at, updated_at "
        "FROM accounts WHERE username = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outAccount.id = sqlite3_column_int(stmt, 0);
        outAccount.username = getColText(stmt, 1);
        outAccount.email = getColText(stmt, 2);
        outAccount.passwordHash = getColText(stmt, 3);
        outAccount.passwordSalt = getColText(stmt, 4);
        outAccount.role = getColText(stmt, 5);
        outAccount.status = getColText(stmt, 6);
        outAccount.failedLoginAttempts = sqlite3_column_int(stmt, 7);
        outAccount.createdAt = getColText(stmt, 8);
        outAccount.lastLoginAt = getColText(stmt, 9);
        outAccount.updatedAt = getColText(stmt, 10);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

bool AccountRepository::findByEmail(const std::string& email, Account& outAccount) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = 
        "SELECT id, username, email, password_hash, password_salt, role, status, "
        "failed_login_attempts, created_at, last_login_at, updated_at "
        "FROM accounts WHERE email = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outAccount.id = sqlite3_column_int(stmt, 0);
        outAccount.username = getColText(stmt, 1);
        outAccount.email = getColText(stmt, 2);
        outAccount.passwordHash = getColText(stmt, 3);
        outAccount.passwordSalt = getColText(stmt, 4);
        outAccount.role = getColText(stmt, 5);
        outAccount.status = getColText(stmt, 6);
        outAccount.failedLoginAttempts = sqlite3_column_int(stmt, 7);
        outAccount.createdAt = getColText(stmt, 8);
        outAccount.lastLoginAt = getColText(stmt, 9);
        outAccount.updatedAt = getColText(stmt, 10);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

bool AccountRepository::findById(int id, Account& outAccount) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = 
        "SELECT id, username, email, password_hash, password_salt, role, status, "
        "failed_login_attempts, created_at, last_login_at, updated_at "
        "FROM accounts WHERE id = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outAccount.id = sqlite3_column_int(stmt, 0);
        outAccount.username = getColText(stmt, 1);
        outAccount.email = getColText(stmt, 2);
        outAccount.passwordHash = getColText(stmt, 3);
        outAccount.passwordSalt = getColText(stmt, 4);
        outAccount.role = getColText(stmt, 5);
        outAccount.status = getColText(stmt, 6);
        outAccount.failedLoginAttempts = sqlite3_column_int(stmt, 7);
        outAccount.createdAt = getColText(stmt, 8);
        outAccount.lastLoginAt = getColText(stmt, 9);
        outAccount.updatedAt = getColText(stmt, 10);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

// ── Security State Updates ──────────────────────────────────

bool AccountRepository::updatePassword(int accountId, const std::string& newHash, const std::string& newSalt) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = 
        "UPDATE accounts SET password_hash = ?, password_salt = ?, updated_at = ? "
        "WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string now = SecurityUtils::getCurrentTimestamp();
    sqlite3_bind_text(stmt, 1, newHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, newSalt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, accountId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool AccountRepository::updateStatus(int accountId, const std::string& status) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "UPDATE accounts SET status = ?, updated_at = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string now = SecurityUtils::getCurrentTimestamp();
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, accountId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

int AccountRepository::incrementFailedAttempts(int accountId, int maxAttempts) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return -1;

    Account acc;
    if (!findById(accountId, acc)) return -1;

    int newAttempts = acc.failedLoginAttempts + 1;
    bool lock = (newAttempts >= maxAttempts);

    const char* sql = 
        lock ? "UPDATE accounts SET failed_login_attempts = ?, status = 'LOCKED', updated_at = ? WHERE id = ?;"
             : "UPDATE accounts SET failed_login_attempts = ?, updated_at = ? WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    std::string now = SecurityUtils::getCurrentTimestamp();
    sqlite3_bind_int(stmt, 1, newAttempts);
    sqlite3_bind_text(stmt, 2, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, accountId);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return newAttempts;
}

bool AccountRepository::resetFailedAttempts(int accountId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "UPDATE accounts SET failed_login_attempts = 0, updated_at = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string now = SecurityUtils::getCurrentTimestamp();
    sqlite3_bind_text(stmt, 1, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, accountId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool AccountRepository::updateLastLogin(int accountId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "UPDATE accounts SET last_login_at = ?, updated_at = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string now = SecurityUtils::getCurrentTimestamp();
    sqlite3_bind_text(stmt, 1, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, accountId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

// ── Session Management ──────────────────────────────────────

bool AccountRepository::createSession(const SessionRecord& session) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = 
        "INSERT INTO sessions (account_id, session_token_hash, created_at, expires_at, revoked) "
        "VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string createdAt = session.createdAt.empty() ? SecurityUtils::getCurrentTimestamp() : session.createdAt;

    sqlite3_bind_int(stmt, 1, session.accountId);
    sqlite3_bind_text(stmt, 2, session.sessionTokenHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, createdAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, session.expiresAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, session.revoked ? 1 : 0);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool AccountRepository::findSessionByTokenHash(const std::string& tokenHash, SessionRecord& outSession) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = 
        "SELECT id, account_id, session_token_hash, created_at, expires_at, revoked "
        "FROM sessions WHERE session_token_hash = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, tokenHash.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outSession.id = sqlite3_column_int(stmt, 0);
        outSession.accountId = sqlite3_column_int(stmt, 1);
        outSession.sessionTokenHash = getColText(stmt, 2);
        outSession.createdAt = getColText(stmt, 3);
        outSession.expiresAt = getColText(stmt, 4);
        outSession.revoked = (sqlite3_column_int(stmt, 5) != 0);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

bool AccountRepository::revokeSession(const std::string& tokenHash) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "UPDATE sessions SET revoked = 1 WHERE session_token_hash = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, tokenHash.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool AccountRepository::revokeAllSessionsForAccount(int accountId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "UPDATE sessions SET revoked = 1 WHERE account_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, accountId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool AccountRepository::isSessionRevokedOrExpired(const std::string& tokenHash) {
    SessionRecord s;
    if (!findSessionByTokenHash(tokenHash, s)) {
        return true; // Not found -> invalid
    }
    if (s.revoked) {
        return true;
    }
    return SecurityUtils::isTimestampExpired(s.expiresAt);
}

// ── Candidate & Recruiter Ownership Linkage ─────────────────

bool AccountRepository::linkCandidateToAccount(int candidateId, int accountId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "UPDATE candidates SET account_id = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, accountId);
    sqlite3_bind_int(stmt, 2, candidateId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

int AccountRepository::getCandidateIdForAccount(int accountId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return 0;

    const char* sql = "SELECT id FROM candidates WHERE account_id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, accountId);

    int candidateId = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        candidateId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return candidateId;
}

int AccountRepository::getAccountIdForCandidate(int candidateId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return 0;

    const char* sql = "SELECT account_id FROM candidates WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, candidateId);

    int accountId = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        accountId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return accountId;
}

bool AccountRepository::linkJobToRecruiter(const std::string& jobId, int recruiterAccountId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "UPDATE jobs SET recruiter_id = ? WHERE job_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, recruiterAccountId);
    sqlite3_bind_text(stmt, 2, jobId.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

int AccountRepository::getRecruiterAccountIdForJob(const std::string& jobId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return 0;

    const char* sql = "SELECT recruiter_id FROM jobs WHERE job_id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, jobId.c_str(), -1, SQLITE_TRANSIENT);

    int recruiterId = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        recruiterId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return recruiterId;
}

std::vector<std::string> AccountRepository::getJobIdsForRecruiter(int recruiterAccountId) {
    std::vector<std::string> jobIds;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return jobIds;

    const char* sql = "SELECT job_id FROM jobs WHERE recruiter_id = ? ORDER BY id ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return jobIds;
    }

    sqlite3_bind_int(stmt, 1, recruiterAccountId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        jobIds.push_back(getColText(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return jobIds;
}

// ── Password Reset Verification Tokens ──────────────────────

bool AccountRepository::storeResetCode(
    const std::string& username,
    const std::string& codeHash,
    const std::string& expiresAt) 
{
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    // Mark previous unused reset codes for this username as used
    {
        const char* invalidateSql = "UPDATE password_reset_tokens SET used = 1 WHERE username = ?;";
        sqlite3_stmt* invStmt = nullptr;
        if (sqlite3_prepare_v2(db, invalidateSql, -1, &invStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(invStmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(invStmt);
            sqlite3_finalize(invStmt);
        }
    }

    const char* sql = 
        "INSERT INTO password_reset_tokens (username, code_hash, expires_at, used) "
        "VALUES (?, ?, ?, 0);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, codeHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, expiresAt.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool AccountRepository::verifyAndConsumeResetCode(
    const std::string& username,
    const std::string& codeHash) 
{
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = 
        "SELECT id, expires_at, used FROM password_reset_tokens "
        "WHERE username = ? AND code_hash = ? AND used = 0 "
        "ORDER BY id DESC LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, codeHash.c_str(), -1, SQLITE_TRANSIENT);

    int tokenId = 0;
    bool valid = false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        tokenId = sqlite3_column_int(stmt, 0);
        std::string expiresAt = getColText(stmt, 1);
        int used = sqlite3_column_int(stmt, 2);

        if (used == 0 && !SecurityUtils::isTimestampExpired(expiresAt)) {
            valid = true;
        }
    }
    sqlite3_finalize(stmt);

    if (valid && tokenId > 0) {
        // Mark as used (single-use)
        const char* useSql = "UPDATE password_reset_tokens SET used = 1 WHERE id = ?;";
        sqlite3_stmt* useStmt = nullptr;
        if (sqlite3_prepare_v2(db, useSql, -1, &useStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(useStmt, 1, tokenId);
            sqlite3_step(useStmt);
            sqlite3_finalize(useStmt);
        }
        return true;
    }

    return false;
}
