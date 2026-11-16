#include "AuditLogger.h"
#include "DatabaseManager.h"
#include "SecurityUtils.h"

#include <iostream>
#include <iomanip>

static std::string getColText(sqlite3_stmt* stmt, int col) {
    const unsigned char* text = sqlite3_column_text(stmt, col);
    return text ? reinterpret_cast<const char*>(text) : "";
}

bool AuditLogger::log(int accountId, const std::string& action, const std::string& details) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = 
        "INSERT INTO audit_logs (account_id, action, details, created_at) "
        "VALUES (?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string now = SecurityUtils::getCurrentTimestamp();
    sqlite3_bind_int(stmt, 1, accountId);
    sqlite3_bind_text(stmt, 2, action.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, details.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, now.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

std::vector<AuditEntry> AuditLogger::getLogsForAccount(int accountId, int limit) {
    std::vector<AuditEntry> logs;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return logs;

    const char* sql = 
        "SELECT id, account_id, action, details, created_at "
        "FROM audit_logs WHERE account_id = ? "
        "ORDER BY id DESC LIMIT ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return logs;
    }

    sqlite3_bind_int(stmt, 1, accountId);
    sqlite3_bind_int(stmt, 2, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AuditEntry entry;
        entry.id = sqlite3_column_int(stmt, 0);
        entry.accountId = sqlite3_column_int(stmt, 1);
        entry.action = getColText(stmt, 2);
        entry.details = getColText(stmt, 3);
        entry.createdAt = getColText(stmt, 4);
        logs.push_back(entry);
    }

    sqlite3_finalize(stmt);
    return logs;
}

std::vector<AuditEntry> AuditLogger::getAllLogs(int limit) {
    std::vector<AuditEntry> logs;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return logs;

    const char* sql = 
        "SELECT id, account_id, action, details, created_at "
        "FROM audit_logs ORDER BY id DESC LIMIT ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return logs;
    }

    sqlite3_bind_int(stmt, 1, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AuditEntry entry;
        entry.id = sqlite3_column_int(stmt, 0);
        entry.accountId = sqlite3_column_int(stmt, 1);
        entry.action = getColText(stmt, 2);
        entry.details = getColText(stmt, 3);
        entry.createdAt = getColText(stmt, 4);
        logs.push_back(entry);
    }

    sqlite3_finalize(stmt);
    return logs;
}

void AuditLogger::displayAccountActivity(int accountId, const std::string& username) {
    std::cout << "\n========================================\n";
    std::cout << "          MY ACTIVITY                   \n";
    std::cout << "========================================\n\n";
    std::cout << "User: " << username << " (Account ID: " << accountId << ")\n\n";

    auto logs = getLogsForAccount(accountId, 20);
    if (logs.empty()) {
        std::cout << "No recorded activity yet.\n";
    } else {
        for (const auto& entry : logs) {
            std::cout << entry.createdAt << "\n";
            std::cout << entry.action;
            if (!entry.details.empty()) {
                std::cout << " (" << entry.details << ")";
            }
            std::cout << "\n\n";
        }
    }
    std::cout << "========================================\n";
}
