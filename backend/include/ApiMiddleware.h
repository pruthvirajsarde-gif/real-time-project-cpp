#pragma once

#include "AuthorizationManager.h"
#include <httplib.h>
#include <string>
#include <chrono>
#include <unordered_map>
#include <mutex>

// ============================================================
// ApiMiddleware — Handles authentication, authorization checks,
// in-memory rate limiting, and privacy-preserving request logging.
// ============================================================
class ApiMiddleware {
public:
    // Extract and validate Bearer session token, populate SecurityContext.
    // Returns true on success, or delivers appropriate 401/403 and returns false.
    static bool authenticate(
        const httplib::Request& req,
        httplib::Response& res,
        SecurityContext& outCtx,
        const std::string& corsOrigin = "*"
    );

    // Require CANDIDATE role
    static bool requireCandidate(
        const SecurityContext& ctx,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    // Require RECRUITER role
    static bool requireRecruiter(
        const SecurityContext& ctx,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    // In-memory rate limiter for sensitive endpoints (e.g. 10 requests / min / IP)
    static bool checkRateLimit(
        const std::string& clientIp,
        const std::string& endpoint,
        int maxRequests = 10,
        int windowSeconds = 60
    );

    // Privacy-preserving request logging (no credentials, tokens, or resume text)
    static void logRequest(
        const httplib::Request& req,
        const httplib::Response& res,
        std::chrono::milliseconds duration,
        int accountId = 0
    );

    // Helper: extracts Bearer token from headers
    static std::string extractBearerToken(const httplib::Request& req);

    // Reset rate limiter state (useful for tests)
    static void resetRateLimiter();

private:
    struct RateLimitEntry {
        int count = 0;
        std::chrono::steady_clock::time_point windowStart;
    };

    static std::mutex rateLimitMutex_;
    static std::unordered_map<std::string, RateLimitEntry> rateLimits_;
};
