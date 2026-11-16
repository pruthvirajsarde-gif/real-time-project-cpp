#pragma once

#include <string>
#include <vector>

// ============================================================
// AuditEntry — Represents a recorded security/system event
// ============================================================
struct AuditEntry {
    int id = 0;
    int accountId = 0;
    std::string action;
    std::string details;
    std::string createdAt;
};

// ============================================================
// AuditLogger — Records and retrieves security and operational
// audit events. Never records sensitive secrets or tokens.
// ============================================================
class AuditLogger {
public:
    // Event Action Constants
    static constexpr const char* ACTION_REGISTRATION_SUCCESS   = "REGISTRATION_SUCCESS";
    static constexpr const char* ACTION_LOGIN_SUCCESS          = "LOGIN_SUCCESS";
    static constexpr const char* ACTION_LOGIN_FAILURE          = "LOGIN_FAILURE";
    static constexpr const char* ACTION_ACCOUNT_LOCKED         = "ACCOUNT_LOCKED";
    static constexpr const char* ACTION_LOGOUT                 = "LOGOUT";
    static constexpr const char* ACTION_PASSWORD_CHANGED       = "PASSWORD_CHANGED";
    static constexpr const char* ACTION_PASSWORD_RESET         = "PASSWORD_RESET";
    static constexpr const char* ACTION_PROFILE_UPDATED        = "PROFILE_UPDATED";
    static constexpr const char* ACTION_RESUME_UPLOADED        = "RESUME_UPLOADED";
    static constexpr const char* ACTION_JOB_CREATED            = "JOB_CREATED";
    static constexpr const char* ACTION_JOB_UPDATED            = "JOB_UPDATED";
    static constexpr const char* ACTION_APPLICATION_SUBMITTED  = "APPLICATION_SUBMITTED";
    static constexpr const char* ACTION_APPLICATION_WITHDRAWN  = "APPLICATION_WITHDRAWN";
    static constexpr const char* ACTION_APP_STATUS_CHANGED     = "APPLICATION_STATUS_CHANGED";
    static constexpr const char* ACTION_SAVED_JOB_ADDED        = "SAVED_JOB_ADDED";
    static constexpr const char* ACTION_SAVED_JOB_REMOVED      = "SAVED_JOB_REMOVED";
    static constexpr const char* ACTION_ANALYTICS_VIEWED       = "ANALYTICS_VIEWED";
    static constexpr const char* ACTION_CANDIDATE_COMP_VIEWED  = "CANDIDATE_COMP_VIEWED";

    // Log an event using prepared statement
    static bool log(int accountId, const std::string& action, const std::string& details);

    // Retrieve activity history for a specific account
    static std::vector<AuditEntry> getLogsForAccount(int accountId, int limit = 50);

    // Retrieve all audit logs (for administrative / test inspection)
    static std::vector<AuditEntry> getAllLogs(int limit = 100);

    // Display formatted user activity history to console
    static void displayAccountActivity(int accountId, const std::string& username);
};
