#pragma once

#include <string>

// ============================================================
// SecurityContext — Holds security and identity metadata for
// the currently active authenticated session.
// ============================================================
struct SecurityContext {
    int accountId = 0;
    std::string username;
    std::string email;
    std::string role;          // "CANDIDATE" or "RECRUITER"
    std::string sessionToken;  // In-memory raw session token
    bool authenticated = false;
    int candidateId = 0;       // Linked candidate profile ID (if CANDIDATE)
};

// ============================================================
// AuthorizationManager — Enforces Role-Based Access Control
// (RBAC) and resource ownership validation across SmartHire.
// ============================================================
class AuthorizationManager {
public:
    // ── Session & Role Dashboard Checks ─────────────────────────
    static bool isSessionActive(const SecurityContext& ctx);
    static bool canViewCandidateDashboard(const SecurityContext& ctx);
    static bool canViewRecruiterDashboard(const SecurityContext& ctx);

    // ── Candidate Permissions & Ownership ───────────────────────
    static bool canViewCandidateProfile(const SecurityContext& ctx, int targetCandidateId);
    static bool canEditCandidateProfile(const SecurityContext& ctx, int targetCandidateId);
    static bool canUploadResume(const SecurityContext& ctx, int targetCandidateId);
    static bool canApplyForJob(const SecurityContext& ctx, int candidateId);
    static bool canSaveJob(const SecurityContext& ctx, int candidateId);
    static bool canViewCandidateApplications(const SecurityContext& ctx, int candidateId);
    static bool canWithdrawApplication(const SecurityContext& ctx, int applicationId);

    // ── Recruiter Permissions & Ownership ───────────────────────
    static bool canCreateJob(const SecurityContext& ctx);
    static bool canEditJob(const SecurityContext& ctx, const std::string& jobId);
    static bool canViewApplicationsForJob(const SecurityContext& ctx, const std::string& jobId);
    static bool canUpdateApplicationStatus(const SecurityContext& ctx, int applicationId);

    // ── Common Account Permissions ──────────────────────────────
    static bool canChangePassword(const SecurityContext& ctx);
};
