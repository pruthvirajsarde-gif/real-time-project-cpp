#include "SystemController.h"
#include "ApiResponse.h"
#include "ApiMiddleware.h"
#include "AuditLogger.h"
#include "DatabaseManager.h"
#include <json.hpp>
#include <filesystem>

void SystemController::getHealth(
    const httplib::Request& /*req*/,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    bool dbConnected = DatabaseManager::isConnected();
    bool dbQueryOk = false;
    if (dbConnected && DatabaseManager::getDb()) {
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(DatabaseManager::getDb(), "SELECT 1;", -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                dbQueryOk = true;
            }
            sqlite3_finalize(stmt);
        }
    }
    bool uploadsOk = std::filesystem::exists("data/uploads") || std::filesystem::create_directories("data/uploads");

    nlohmann::json data = {
        {"status", "UP"},
        {"application", "SmartHire"},
        {"version", "9.0"},
        {"database", dbConnected ? "CONNECTED" : "DISCONNECTED"},
        {"liveness", "OK"},
        {"readiness", (dbConnected && dbQueryOk && uploadsOk) ? "READY" : "NOT_READY"},
        {"databaseQuery", dbQueryOk ? "OK" : "ERROR"},
        {"uploadsDirectory", uploadsOk ? "OK" : "ERROR"}
    };

    ApiResponse::sendSuccess(res, data, "System is healthy.", 200, corsOrigin);
}

void SystemController::getLiveness(
    const httplib::Request& /*req*/,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    nlohmann::json data = {
        {"status", "ok"},
        {"liveness", "OK"}
    };
    ApiResponse::sendSuccess(res, data, "Process is live.", 200, corsOrigin);
}

void SystemController::getReadiness(
    const httplib::Request& /*req*/,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    bool dbConnected = DatabaseManager::isConnected();
    bool dbQueryOk = false;
    if (dbConnected && DatabaseManager::getDb()) {
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(DatabaseManager::getDb(), "SELECT 1;", -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                dbQueryOk = true;
            }
            sqlite3_finalize(stmt);
        }
    }
    bool uploadsOk = std::filesystem::exists("data/uploads") || std::filesystem::create_directories("data/uploads");
    bool isReady = (dbConnected && dbQueryOk && uploadsOk);

    nlohmann::json data = {
        {"status", isReady ? "ok" : "degraded"},
        {"readiness", isReady ? "READY" : "NOT_READY"},
        {"database", dbConnected ? "ok" : "error"},
        {"databaseQuery", dbQueryOk ? "ok" : "error"},
        {"uploads", uploadsOk ? "ok" : "error"}
    };

    int statusCode = isReady ? 200 : 503;
    ApiResponse::sendSuccess(res, data, isReady ? "System is ready." : "System is degraded.", statusCode, corsOrigin);
}

void SystemController::getApiInfo(
    const httplib::Request& /*req*/,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    nlohmann::json data = {
        {"name", "SmartHire API"},
        {"version", "1.0"},
        {"applicationVersion", "9.0"},
        {"description", "Intelligent Resume & Job Matching REST API"}
    };

    ApiResponse::sendSuccess(res, data, "SmartHire API Information", 200, corsOrigin);
}

void SystemController::getActivity(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }

    std::vector<AuditEntry> logs = AuditLogger::getLogsForAccount(ctx.accountId, 50);
    nlohmann::json items = nlohmann::json::array();

    for (const auto& log : logs) {
        items.push_back({
            {"id", log.id},
            {"action", log.action},
            {"details", log.details},
            {"createdAt", log.createdAt}
        });
    }

    ApiResponse::sendSuccess(res, items, "User activity retrieved.", 200, corsOrigin);
}
