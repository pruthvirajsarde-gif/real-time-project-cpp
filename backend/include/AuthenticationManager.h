#pragma once

#include "AuthorizationManager.h"
#include <string>

// ============================================================
// AuthenticationManager — Coordinates account registration,
// credential verification, session creation, password resets,
// and audit logging.
// ============================================================
class AuthenticationManager {
public:
    // Candidate Registration
    static bool registerCandidate(
        const std::string& fullName,
        const std::string& email,
        const std::string& phone,
        const std::string& username,
        const std::string& password,
        const std::string& confirmPassword,
        const std::string& location,
        std::string& outError,
        int& outAccountId,
        int& outCandidateId);

    // Recruiter Registration
    static bool registerRecruiter(
        const std::string& recruiterName,
        const std::string& company,
        const std::string& email,
        const std::string& username,
        const std::string& password,
        const std::string& confirmPassword,
        std::string& outError,
        int& outAccountId);

    // User Login
    static bool login(
        const std::string& username,
        const std::string& password,
        SecurityContext& outContext,
        std::string& outError);

    // User Logout
    static bool logout(SecurityContext& context);

    // Password Change (authenticated user)
    static bool changePassword(
        SecurityContext& context,
        const std::string& currentPassword,
        const std::string& newPassword,
        const std::string& confirmPassword,
        std::string& outError);

    // Local Demo Password Reset: Step 1 — Request one-time code
    static bool generatePasswordResetCode(
        const std::string& username,
        const std::string& email,
        std::string& outResetCode,
        std::string& outError);

    // Local Demo Password Reset: Step 2 — Reset password using verified code
    static bool resetPasswordWithCode(
        const std::string& username,
        const std::string& resetCode,
        const std::string& newPassword,
        const std::string& confirmPassword,
        std::string& outError);

    // Verify if active context has valid non-expired session
    static bool validateContext(SecurityContext& context);
};
