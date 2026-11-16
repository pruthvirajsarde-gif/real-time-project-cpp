#include "AuthController.h"
#include "ApiResponse.h"
#include "ApiMiddleware.h"
#include "AuthenticationManager.h"
#include "AccountRepository.h"
#include <json.hpp>

void AuthController::registerCandidate(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    if (!ApiMiddleware::checkRateLimit(req.remote_addr, req.path)) {
        ApiResponse::sendError(res, "RATE_LIMIT_EXCEEDED", "Too many requests. Please slow down.", 429, corsOrigin);
        return;
    }

    nlohmann::json body;
    try {
        body = nlohmann::json::parse(req.body);
    } catch (...) {
        ApiResponse::sendError(res, "BAD_REQUEST_MALFORMED_JSON", "Malformed JSON body.", 400, corsOrigin);
        return;
    }

    std::string name = body.value("name", "");
    std::string email = body.value("email", "");
    std::string phone = body.value("phone", "");
    std::string location = body.value("location", "");
    std::string username = body.value("username", "");
    std::string password = body.value("password", "");

    if (name.empty() || email.empty() || username.empty() || password.empty()) {
        ApiResponse::sendError(res, "VALIDATION_FAILED", "Name, email, username, and password are required.", 400, corsOrigin);
        return;
    }

    std::string error;
    int accountId = 0;
    int candidateId = 0;

    bool success = AuthenticationManager::registerCandidate(
        name, email, phone, username, password, password, location, error, accountId, candidateId
    );

    if (!success) {
        int status = 400;
        std::string code = "REGISTRATION_FAILED";
        if (error.find("already exists") != std::string::npos || error.find("taken") != std::string::npos) {
            status = 409;
            code = "CONFLICT_ALREADY_EXISTS";
        }
        ApiResponse::sendError(res, code, error, status, corsOrigin);
        return;
    }

    nlohmann::json data = {
        {"accountId", accountId},
        {"candidateId", candidateId},
        {"role", "CANDIDATE"},
        {"username", username}
    };

    ApiResponse::sendSuccess(res, data, "Candidate account created successfully.", 201, corsOrigin);
}

void AuthController::registerRecruiter(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    if (!ApiMiddleware::checkRateLimit(req.remote_addr, req.path)) {
        ApiResponse::sendError(res, "RATE_LIMIT_EXCEEDED", "Too many requests. Please slow down.", 429, corsOrigin);
        return;
    }

    nlohmann::json body;
    try {
        body = nlohmann::json::parse(req.body);
    } catch (...) {
        ApiResponse::sendError(res, "BAD_REQUEST_MALFORMED_JSON", "Malformed JSON body.", 400, corsOrigin);
        return;
    }

    std::string name = body.value("name", "");
    std::string company = body.value("company", "");
    std::string email = body.value("email", "");
    std::string username = body.value("username", "");
    std::string password = body.value("password", "");

    if (name.empty() || company.empty() || email.empty() || username.empty() || password.empty()) {
        ApiResponse::sendError(res, "VALIDATION_FAILED", "Name, company, email, username, and password are required.", 400, corsOrigin);
        return;
    }

    std::string error;
    int accountId = 0;

    bool success = AuthenticationManager::registerRecruiter(
        name, company, email, username, password, password, error, accountId
    );

    if (!success) {
        int status = 400;
        std::string code = "REGISTRATION_FAILED";
        if (error.find("already exists") != std::string::npos || error.find("taken") != std::string::npos) {
            status = 409;
            code = "CONFLICT_ALREADY_EXISTS";
        }
        ApiResponse::sendError(res, code, error, status, corsOrigin);
        return;
    }

    nlohmann::json data = {
        {"accountId", accountId},
        {"role", "RECRUITER"},
        {"username", username}
    };

    ApiResponse::sendSuccess(res, data, "Recruiter account created successfully.", 201, corsOrigin);
}

void AuthController::login(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    if (!ApiMiddleware::checkRateLimit(req.remote_addr, req.path)) {
        ApiResponse::sendError(res, "RATE_LIMIT_EXCEEDED", "Too many requests. Please slow down.", 429, corsOrigin);
        return;
    }

    nlohmann::json body;
    try {
        body = nlohmann::json::parse(req.body);
    } catch (...) {
        ApiResponse::sendError(res, "BAD_REQUEST_MALFORMED_JSON", "Malformed JSON body.", 400, corsOrigin);
        return;
    }

    std::string username = body.value("username", "");
    std::string password = body.value("password", "");

    if (username.empty() || password.empty()) {
        ApiResponse::sendError(res, "AUTH_INVALID_CREDENTIALS", "Username and password are required.", 400, corsOrigin);
        return;
    }

    SecurityContext ctx;
    std::string error;
    bool success = AuthenticationManager::login(username, password, ctx, error);

    if (!success) {
        int status = 401;
        std::string code = "AUTH_INVALID_CREDENTIALS";
        if (error.find("locked") != std::string::npos) {
            status = 403;
            code = "AUTH_ACCOUNT_LOCKED";
        } else if (error.find("disabled") != std::string::npos) {
            status = 403;
            code = "AUTH_ACCOUNT_DISABLED";
        }
        ApiResponse::sendError(res, code, error, status, corsOrigin);
        return;
    }

    nlohmann::json data = {
        {"sessionToken", ctx.sessionToken},
        {"role", ctx.role},
        {"username", ctx.username},
        {"accountId", ctx.accountId}
    };
    if (ctx.role == "CANDIDATE") {
        data["candidateId"] = ctx.candidateId;
    }

    // Set noStoreCache = true for authentication tokens
    ApiResponse::sendSuccess(res, data, "Login successful.", 200, corsOrigin, true);
}

void AuthController::logout(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }

    AuthenticationManager::logout(ctx);
    ApiResponse::sendSuccess(res, nlohmann::json::object(), "Logout successful.", 200, corsOrigin, true);
}

void AuthController::changePassword(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }

    nlohmann::json body;
    try {
        body = nlohmann::json::parse(req.body);
    } catch (...) {
        ApiResponse::sendError(res, "BAD_REQUEST_MALFORMED_JSON", "Malformed JSON body.", 400, corsOrigin);
        return;
    }

    std::string currentPassword = body.value("currentPassword", "");
    std::string newPassword = body.value("newPassword", "");

    if (currentPassword.empty() || newPassword.empty()) {
        ApiResponse::sendError(res, "VALIDATION_FAILED", "Current password and new password are required.", 400, corsOrigin);
        return;
    }

    std::string error;
    bool success = AuthenticationManager::changePassword(ctx, currentPassword, newPassword, newPassword, error);
    if (!success) {
        ApiResponse::sendError(res, "PASSWORD_CHANGE_FAILED", error, 400, corsOrigin);
        return;
    }

    ApiResponse::sendSuccess(res, nlohmann::json::object(), "Password changed successfully. Please log in again.", 200, corsOrigin, true);
}

void AuthController::getMe(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }

    nlohmann::json data = {
        {"accountId", ctx.accountId},
        {"username", ctx.username},
        {"email", ctx.email},
        {"role", ctx.role}
    };
    if (ctx.role == "CANDIDATE") {
        data["candidateId"] = ctx.candidateId;
    }

    ApiResponse::sendSuccess(res, data, "Account profile retrieved.", 200, corsOrigin);
}
