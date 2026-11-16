#include "SessionManager.h"
#include "SecurityUtils.h"

std::string SessionManager::createSession(int accountId, int durationMinutes) {
    if (accountId <= 0) return "";

    // Generate 32 bytes cryptographically secure random token (64 hex characters)
    std::string rawToken = SecurityUtils::generateSecureRandomHex(32);
    std::string tokenHash = SecurityUtils::sha256Hex(rawToken);

    std::string createdAt = SecurityUtils::getCurrentTimestamp();
    std::string expiresAt = SecurityUtils::getExpirationTimestamp(durationMinutes);

    SessionRecord session;
    session.accountId = accountId;
    session.sessionTokenHash = tokenHash;
    session.createdAt = createdAt;
    session.expiresAt = expiresAt;
    session.revoked = false;

    if (!AccountRepository::createSession(session)) {
        return "";
    }

    return rawToken;
}

bool SessionManager::isSessionValid(
    const std::string& rawToken,
    SessionRecord& outSession) 
{
    if (rawToken.empty()) return false;

    std::string tokenHash = SecurityUtils::sha256Hex(rawToken);

    if (!AccountRepository::findSessionByTokenHash(tokenHash, outSession)) {
        return false;
    }

    if (outSession.revoked) {
        return false;
    }

    if (SecurityUtils::isTimestampExpired(outSession.expiresAt)) {
        return false;
    }

    // Verify associated account is still ACTIVE
    Account acc;
    if (!AccountRepository::findById(outSession.accountId, acc)) {
        return false;
    }

    if (acc.status != "ACTIVE") {
        return false;
    }

    return true;
}

bool SessionManager::revokeSession(const std::string& rawToken) {
    if (rawToken.empty()) return false;
    std::string tokenHash = SecurityUtils::sha256Hex(rawToken);
    return AccountRepository::revokeSession(tokenHash);
}

bool SessionManager::revokeAllUserSessions(int accountId) {
    if (accountId <= 0) return false;
    return AccountRepository::revokeAllSessionsForAccount(accountId);
}

int SessionManager::getAccountIdFromToken(const std::string& rawToken) {
    SessionRecord session;
    if (isSessionValid(rawToken, session)) {
        return session.accountId;
    }
    return 0;
}
