#include "ApiMiddleware.h"
#include "ApiResponse.h"
#include "SessionManager.h"
#include "AccountRepository.h"
#include "SecurityUtils.h"
#include <iostream>
#include <algorithm>

std::mutex ApiMiddleware::rateLimitMutex_;
std::unordered_map<std::string, ApiMiddleware::RateLimitEntry> ApiMiddleware::rateLimits_;

std::string ApiMiddleware::extractBearerToken(const httplib::Request& req) {
    if (!req.has_header("Authorization")) {
        return "";
    }
    std::string authHeader = req.get_header_value("Authorization");
    const std::string prefix = "Bearer ";
    if (authHeader.rfind(prefix, 0) == 0) {
        return authHeader.substr(prefix.length());
    }
    return "";
}

bool ApiMiddleware::authenticate(
    const httplib::Request& req,
    httplib::Response& res,
    SecurityContext& outCtx,
    const std::string& corsOrigin
) {
    outCtx = SecurityContext(); // Reset context

    std::string token = extractBearerToken(req);
    if (token.empty()) {
        ApiResponse::sendError(
            res,
            "AUTH_MISSING_TOKEN",
            "Authorization Bearer token is missing or malformed.",
            401,
            corsOrigin
        );
        return false;
    }

    SessionRecord session;
    if (!SessionManager::isSessionValid(token, session)) {
        ApiResponse::sendError(
            res,
            "AUTH_INVALID_TOKEN",
            "Session token is invalid, expired, or revoked.",
            401,
            corsOrigin
        );
        return false;
    }

    Account account;
    if (!AccountRepository::findById(session.accountId, account)) {
        ApiResponse::sendError(
            res,
            "AUTH_ACCOUNT_NOT_FOUND",
            "Associated account could not be found.",
            401,
            corsOrigin
        );
        return false;
    }

    if (account.status == "LOCKED") {
        ApiResponse::sendError(
            res,
            "AUTH_ACCOUNT_LOCKED",
            "Account is locked due to multiple failed login attempts. Please contact the administrator.",
            403,
            corsOrigin
        );
        return false;
    }

    if (account.status == "DISABLED") {
        ApiResponse::sendError(
            res,
            "AUTH_ACCOUNT_DISABLED",
            "Account is disabled. Please contact the administrator.",
            403,
            corsOrigin
        );
        return false;
    }

    // Populate validated SecurityContext
    outCtx.accountId = account.id;
    outCtx.username = account.username;
    outCtx.email = account.email;
    outCtx.role = account.role;
    outCtx.sessionToken = token;
    outCtx.authenticated = true;

    if (account.role == "CANDIDATE") {
        outCtx.candidateId = AccountRepository::getCandidateIdForAccount(account.id);
    } else {
        outCtx.candidateId = 0;
    }

    return true;
}

bool ApiMiddleware::requireCandidate(
    const SecurityContext& ctx,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    if (!ctx.authenticated || ctx.role != "CANDIDATE") {
        ApiResponse::sendError(
            res,
            "FORBIDDEN_CANDIDATE_ONLY",
            "Access forbidden. This resource requires a candidate account.",
            403,
            corsOrigin
        );
        return false;
    }
    return true;
}

bool ApiMiddleware::requireRecruiter(
    const SecurityContext& ctx,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    if (!ctx.authenticated || ctx.role != "RECRUITER") {
        ApiResponse::sendError(
            res,
            "FORBIDDEN_RECRUITER_ONLY",
            "Access forbidden. This resource requires a recruiter account.",
            403,
            corsOrigin
        );
        return false;
    }
    return true;
}

bool ApiMiddleware::checkRateLimit(
    const std::string& clientIp,
    const std::string& endpoint,
    int maxRequests,
    int windowSeconds
) {
    std::lock_guard<std::mutex> lock(rateLimitMutex_);
    std::string key = clientIp + ":" + endpoint;
    auto now = std::chrono::steady_clock::now();

    auto it = rateLimits_.find(key);
    if (it == rateLimits_.end()) {
        RateLimitEntry entry;
        entry.count = 1;
        entry.windowStart = now;
        rateLimits_[key] = entry;
        return true;
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.windowStart).count();
    if (elapsed >= windowSeconds) {
        // Window expired, reset
        it->second.count = 1;
        it->second.windowStart = now;
        return true;
    }

    // Within window
    if (it->second.count >= maxRequests) {
        return false; // Rate limit exceeded
    }

    it->second.count++;
    return true;
}

void ApiMiddleware::resetRateLimiter() {
    std::lock_guard<std::mutex> lock(rateLimitMutex_);
    rateLimits_.clear();
}

void ApiMiddleware::logRequest(
    const httplib::Request& req,
    const httplib::Response& res,
    std::chrono::milliseconds duration,
    int accountId
) {
    std::string timestamp = SecurityUtils::getCurrentTimestamp();
    std::cout << "[" << timestamp << "] "
              << req.method << " " << req.path << " -> "
              << res.status << " (" << duration.count() << "ms)";
    if (accountId > 0) {
        std::cout << " [Account: " << accountId << "]";
    }
    std::cout << std::endl;
}
