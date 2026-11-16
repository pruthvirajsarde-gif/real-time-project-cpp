#pragma once

#include <httplib.h>
#include <json.hpp>
#include <string>

// ============================================================
// ApiResponse — Formats and delivers uniform JSON responses
// according to the SmartHire Phase 9 REST specification.
// ============================================================
class ApiResponse {
public:
    // Sends raw JSON response with appropriate security and CORS headers
    static void sendJson(
        httplib::Response& res,
        int statusCode,
        const nlohmann::json& body,
        const std::string& corsOrigin = "*",
        bool noStoreCache = false
    );

    // Sends standardized success response:
    // { "success": true, "message": message, "data": data }
    static void sendSuccess(
        httplib::Response& res,
        const nlohmann::json& data = nlohmann::json::object(),
        const std::string& message = "",
        int statusCode = 200,
        const std::string& corsOrigin = "*",
        bool noStoreCache = false
    );

    // Sends standardized error response:
    // { "success": false, "message": errorMessage, "error": { "code": errorCode } }
    static void sendError(
        httplib::Response& res,
        const std::string& errorCode,
        const std::string& errorMessage,
        int statusCode = 400,
        const std::string& corsOrigin = "*"
    );

    // Sets standard CORS headers on preflight OPTIONS requests
    static void setCorsHeaders(
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
