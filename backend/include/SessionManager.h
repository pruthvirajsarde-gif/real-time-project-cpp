#pragma once

#include "AccountRepository.h"
#include <string>

// ============================================================
// SessionManager — Handles creation, validation, expiration,
// and revocation of user authentication sessions.
// Raw tokens are never stored in the database; only SHA-256 hashes.
// ============================================================
class SessionManager {
public:
    static constexpr int DEFAULT_SESSION_DURATION_MINUTES = 30;

    // Creates a new session for the given account, storing only its hash in DB,
    // and returns the unpredictable raw session token to the caller.
    static std::string createSession(
        int accountId,
        int durationMinutes = DEFAULT_SESSION_DURATION_MINUTES);

    // Validates that the raw session token exists, is not revoked, has not expired,
    // and that the associated account remains ACTIVE.
    static bool isSessionValid(
        const std::string& rawToken,
        SessionRecord& outSession);

    // Revokes an active session by token
    static bool revokeSession(const std::string& rawToken);

    // Revokes all active sessions for a user (e.g. on password change)
    static bool revokeAllUserSessions(int accountId);

    // Helper to extract account ID from a valid token
    static int getAccountIdFromToken(const std::string& rawToken);
};
