#include "ApiResponse.h"

void ApiResponse::setCorsHeaders(httplib::Response& res, const std::string& corsOrigin) {
    std::string origin = corsOrigin.empty() ? "*" : corsOrigin;
    res.set_header("Access-Control-Allow-Origin", origin);
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Authorization, Content-Type, Accept, X-Requested-With");
    res.set_header("Access-Control-Max-Age", "86400");
}

void ApiResponse::sendJson(
    httplib::Response& res,
    int statusCode,
    const nlohmann::json& body,
    const std::string& corsOrigin,
    bool noStoreCache
) {
    res.status = statusCode;
    res.set_content(body.dump(), "application/json; charset=utf-8");

    // Standard security headers
    res.set_header("X-Content-Type-Options", "nosniff");
    res.set_header("X-Frame-Options", "DENY");

    if (noStoreCache) {
        res.set_header("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
        res.set_header("Pragma", "no-cache");
    }

    setCorsHeaders(res, corsOrigin);
}

void ApiResponse::sendSuccess(
    httplib::Response& res,
    const nlohmann::json& data,
    const std::string& message,
    int statusCode,
    const std::string& corsOrigin,
    bool noStoreCache
) {
    nlohmann::json responseBody;
    responseBody["success"] = true;
    if (!message.empty()) {
        responseBody["message"] = message;
    }
    responseBody["data"] = data;

    sendJson(res, statusCode, responseBody, corsOrigin, noStoreCache);
}

void ApiResponse::sendError(
    httplib::Response& res,
    const std::string& errorCode,
    const std::string& errorMessage,
    int statusCode,
    const std::string& corsOrigin
) {
    nlohmann::json responseBody;
    responseBody["success"] = false;
    responseBody["message"] = errorMessage;
    responseBody["error"] = {
        {"code", errorCode}
    };

    sendJson(res, statusCode, responseBody, corsOrigin, true);
}
