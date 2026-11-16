#include "AuthorizationManager.h"
#include "SessionManager.h"
#include "AccountRepository.h"
#include "DatabaseManager.h"

#include <iostream>

bool AuthorizationManager::isSessionActive(const SecurityContext& ctx) {
    if (!ctx.authenticated || ctx.sessionToken.empty()) {
        return false;
    }
    SessionRecord session;
    return SessionManager::isSessionValid(ctx.sessionToken, session);
}

bool AuthorizationManager::canViewCandidateDashboard(const SecurityContext& ctx) {
    return isSessionActive(ctx) && ctx.role == "CANDIDATE";
}

bool AuthorizationManager::canViewRecruiterDashboard(const SecurityContext& ctx) {
    return isSessionActive(ctx) && ctx.role == "RECRUITER";
}

bool AuthorizationManager::canViewCandidateProfile(const SecurityContext& ctx, int targetCandidateId) {
    if (!isSessionActive(ctx)) return false;
    if (ctx.role == "CANDIDATE") {
        return ctx.candidateId == targetCandidateId;
    }
    if (ctx.role == "RECRUITER") {
        // Recruiter can view candidate profile for review
        return true;
    }
    return false;
}

bool AuthorizationManager::canEditCandidateProfile(const SecurityContext& ctx, int targetCandidateId) {
    if (!isSessionActive(ctx)) return false;
    return (ctx.role == "CANDIDATE" && ctx.candidateId == targetCandidateId);
}

bool AuthorizationManager::canUploadResume(const SecurityContext& ctx, int targetCandidateId) {
    if (!isSessionActive(ctx)) return false;
    return (ctx.role == "CANDIDATE" && ctx.candidateId == targetCandidateId);
}

bool AuthorizationManager::canApplyForJob(const SecurityContext& ctx, int candidateId) {
    if (!isSessionActive(ctx)) return false;
    return (ctx.role == "CANDIDATE" && ctx.candidateId == candidateId);
}

bool AuthorizationManager::canSaveJob(const SecurityContext& ctx, int candidateId) {
    if (!isSessionActive(ctx)) return false;
    return (ctx.role == "CANDIDATE" && ctx.candidateId == candidateId);
}

bool AuthorizationManager::canViewCandidateApplications(const SecurityContext& ctx, int candidateId) {
    if (!isSessionActive(ctx)) return false;
    return (ctx.role == "CANDIDATE" && ctx.candidateId == candidateId);
}

bool AuthorizationManager::canWithdrawApplication(const SecurityContext& ctx, int applicationId) {
    if (!isSessionActive(ctx) || ctx.role != "CANDIDATE") {
        return false;
    }

    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "SELECT candidate_id FROM applications WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, applicationId);
    int appCandidateId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        appCandidateId = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    return (appCandidateId == ctx.candidateId);
}

bool AuthorizationManager::canCreateJob(const SecurityContext& ctx) {
    return isSessionActive(ctx) && ctx.role == "RECRUITER";
}

bool AuthorizationManager::canEditJob(const SecurityContext& ctx, const std::string& jobId) {
    if (!isSessionActive(ctx) || ctx.role != "RECRUITER") {
        return false;
    }

    int ownerAccountId = AccountRepository::getRecruiterAccountIdForJob(jobId);
    // Unassigned legacy job or owned by this recruiter
    return (ownerAccountId == 0 || ownerAccountId == ctx.accountId);
}

bool AuthorizationManager::canViewApplicationsForJob(const SecurityContext& ctx, const std::string& jobId) {
    if (!isSessionActive(ctx) || ctx.role != "RECRUITER") {
        return false;
    }

    int ownerAccountId = AccountRepository::getRecruiterAccountIdForJob(jobId);
    return (ownerAccountId == 0 || ownerAccountId == ctx.accountId);
}

bool AuthorizationManager::canUpdateApplicationStatus(const SecurityContext& ctx, int applicationId) {
    if (!isSessionActive(ctx) || ctx.role != "RECRUITER") {
        return false;
    }

    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "SELECT job_id FROM applications WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, applicationId);
    std::string jobId;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* t = sqlite3_column_text(stmt, 0);
        if (t) jobId = reinterpret_cast<const char*>(t);
    }
    sqlite3_finalize(stmt);

    if (jobId.empty()) return false;

    int ownerAccountId = AccountRepository::getRecruiterAccountIdForJob(jobId);
    return (ownerAccountId == 0 || ownerAccountId == ctx.accountId);
}

bool AuthorizationManager::canChangePassword(const SecurityContext& ctx) {
    return isSessionActive(ctx);
}
