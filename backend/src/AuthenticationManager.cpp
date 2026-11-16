#include "AuthenticationManager.h"
#include "AccountRepository.h"
#include "CandidateRepository.h"
#include "PasswordHasher.h"
#include "SessionManager.h"
#include "AuditLogger.h"
#include "SecurityUtils.h"
#include "Profile.h"

#include <iostream>

bool AuthenticationManager::registerCandidate(
    const std::string& fullName,
    const std::string& email,
    const std::string& phone,
    const std::string& username,
    const std::string& password,
    const std::string& confirmPassword,
    const std::string& location,
    std::string& outError,
    int& outAccountId,
    int& outCandidateId) 
{
    outAccountId = -1;
    outCandidateId = -1;

    // Validate Full Name
    auto nameVal = SecurityUtils::validateNonEmpty(fullName, "Full Name", 100);
    if (!nameVal.valid) {
        outError = nameVal.message;
        return false;
    }

    // Validate Username
    auto userVal = SecurityUtils::validateUsername(username);
    if (!userVal.valid) {
        outError = userVal.message;
        return false;
    }

    // Validate Email
    auto emailVal = SecurityUtils::validateEmail(email);
    if (!emailVal.valid) {
        outError = emailVal.message;
        return false;
    }

    // Validate Password Policy
    auto passVal = SecurityUtils::validatePassword(password);
    if (!passVal.valid) {
        outError = passVal.message;
        return false;
    }

    // Confirm Password Match
    if (password != confirmPassword) {
        outError = "Passwords do not match.";
        return false;
    }

    // Check Username Uniqueness
    Account existingUser;
    if (AccountRepository::findByUsername(username, existingUser)) {
        outError = "Username is already taken.";
        return false;
    }

    // Check Email Uniqueness in Accounts
    Account existingEmailAcc;
    if (AccountRepository::findByEmail(email, existingEmailAcc)) {
        outError = "Email is already registered.";
        return false;
    }

    // Check Email Uniqueness in Candidates
    if (CandidateRepository::findCandidateByEmail(email) != -1) {
        outError = "A candidate profile with this email already exists.";
        return false;
    }

    // Hash Password with cryptographically secure salt & PBKDF2
    std::string salt = PasswordHasher::generateSalt();
    std::string hash = PasswordHasher::hashPassword(password, salt);

    Account newAcc;
    newAcc.username = username;
    newAcc.email = email;
    newAcc.passwordHash = hash;
    newAcc.passwordSalt = salt;
    newAcc.role = "CANDIDATE";
    newAcc.status = "ACTIVE";

    int accId = AccountRepository::createAccount(newAcc);
    if (accId <= 0) {
        outError = "Failed to create account in database.";
        return false;
    }

    // Create Candidate Profile
    Profile candProfile;
    candProfile.setName(fullName);
    candProfile.setEmail(email);
    candProfile.setPhone(phone);
    candProfile.setLocation(location);

    int candId = CandidateRepository::createCandidate(candProfile);
    if (candId <= 0) {
        // Rollback account creation if candidate profile fails
        AccountRepository::updateStatus(accId, "DISABLED");
        outError = "Failed to create candidate profile.";
        return false;
    }

    // Link Account and Candidate
    AccountRepository::linkCandidateToAccount(candId, accId);

    outAccountId = accId;
    outCandidateId = candId;

    AuditLogger::log(accId, AuditLogger::ACTION_REGISTRATION_SUCCESS, "Candidate registration completed");
    return true;
}

bool AuthenticationManager::registerRecruiter(
    const std::string& recruiterName,
    const std::string& company,
    const std::string& email,
    const std::string& username,
    const std::string& password,
    const std::string& confirmPassword,
    std::string& outError,
    int& outAccountId) 
{
    outAccountId = -1;

    // Validate Recruiter Name
    auto nameVal = SecurityUtils::validateNonEmpty(recruiterName, "Recruiter Name", 100);
    if (!nameVal.valid) {
        outError = nameVal.message;
        return false;
    }

    // Validate Company
    auto compVal = SecurityUtils::validateNonEmpty(company, "Company", 100);
    if (!compVal.valid) {
        outError = compVal.message;
        return false;
    }

    // Validate Username
    auto userVal = SecurityUtils::validateUsername(username);
    if (!userVal.valid) {
        outError = userVal.message;
        return false;
    }

    // Validate Email
    auto emailVal = SecurityUtils::validateEmail(email);
    if (!emailVal.valid) {
        outError = emailVal.message;
        return false;
    }

    // Validate Password Policy
    auto passVal = SecurityUtils::validatePassword(password);
    if (!passVal.valid) {
        outError = passVal.message;
        return false;
    }

    // Confirm Password Match
    if (password != confirmPassword) {
        outError = "Passwords do not match.";
        return false;
    }

    // Check Username Uniqueness
    Account existingUser;
    if (AccountRepository::findByUsername(username, existingUser)) {
        outError = "Username is already taken.";
        return false;
    }

    // Check Email Uniqueness
    Account existingEmailAcc;
    if (AccountRepository::findByEmail(email, existingEmailAcc)) {
        outError = "Email is already registered.";
        return false;
    }

    // Hash Password
    std::string salt = PasswordHasher::generateSalt();
    std::string hash = PasswordHasher::hashPassword(password, salt);

    Account newAcc;
    newAcc.username = username;
    newAcc.email = email;
    newAcc.passwordHash = hash;
    newAcc.passwordSalt = salt;
    newAcc.role = "RECRUITER";
    newAcc.status = "ACTIVE";

    int accId = AccountRepository::createAccount(newAcc);
    if (accId <= 0) {
        outError = "Failed to create account in database.";
        return false;
    }

    outAccountId = accId;
    AuditLogger::log(accId, AuditLogger::ACTION_REGISTRATION_SUCCESS, "Recruiter registration completed: " + company);
    return true;
}

bool AuthenticationManager::login(
    const std::string& username,
    const std::string& password,
    SecurityContext& outContext,
    std::string& outError) 
{
    outContext = SecurityContext();

    if (username.empty() || password.empty()) {
        outError = "Username and password cannot be empty.";
        return false;
    }

    Account acc;
    if (!AccountRepository::findByUsername(username, acc)) {
        AuditLogger::log(0, AuditLogger::ACTION_LOGIN_FAILURE, "Unknown username attempt: " + username);
        outError = "Invalid username or password.";
        return false;
    }

    // Check Account Status
    if (acc.status == "LOCKED") {
        AuditLogger::log(acc.id, AuditLogger::ACTION_LOGIN_FAILURE, "Attempted login on locked account");
        outError = "Account locked due to multiple failed login attempts. Please contact the administrator.";
        return false;
    }
    if (acc.status == "DISABLED") {
        AuditLogger::log(acc.id, AuditLogger::ACTION_LOGIN_FAILURE, "Attempted login on disabled account");
        outError = "Account is disabled. Please contact the administrator.";
        return false;
    }

    // Verify Password in constant-time
    bool passwordMatches = PasswordHasher::verifyPassword(password, acc.passwordSalt, acc.passwordHash);
    if (!passwordMatches) {
        int failedAttempts = AccountRepository::incrementFailedAttempts(acc.id, 5);
        if (failedAttempts >= 5) {
            AuditLogger::log(acc.id, AuditLogger::ACTION_ACCOUNT_LOCKED, "Account locked after 5 failed login attempts");
            outError = "Account locked due to multiple failed login attempts. Please contact the administrator.";
        } else {
            AuditLogger::log(acc.id, AuditLogger::ACTION_LOGIN_FAILURE, "Password verification failed");
            outError = "Invalid username or password.";
        }
        return false;
    }

    // Login Successful: Reset failed attempts counter & update timestamp
    AccountRepository::resetFailedAttempts(acc.id);
    AccountRepository::updateLastLogin(acc.id);

    // Create Authenticated Session
    std::string sessionToken = SessionManager::createSession(acc.id, 30);
    if (sessionToken.empty()) {
        outError = "Failed to establish secure session.";
        return false;
    }

    outContext.accountId = acc.id;
    outContext.username = acc.username;
    outContext.email = acc.email;
    outContext.role = acc.role;
    outContext.sessionToken = sessionToken;
    outContext.authenticated = true;

    if (acc.role == "CANDIDATE") {
        outContext.candidateId = AccountRepository::getCandidateIdForAccount(acc.id);
    } else {
        outContext.candidateId = 0;
    }

    AuditLogger::log(acc.id, AuditLogger::ACTION_LOGIN_SUCCESS, 
                     acc.role == "CANDIDATE" ? "Candidate login" : "Recruiter login");
    return true;
}

bool AuthenticationManager::logout(SecurityContext& context) {
    if (!context.authenticated) return true;

    if (!context.sessionToken.empty()) {
        SessionManager::revokeSession(context.sessionToken);
    }

    AuditLogger::log(context.accountId, AuditLogger::ACTION_LOGOUT, "User logged out");
    context = SecurityContext();
    return true;
}

bool AuthenticationManager::changePassword(
    SecurityContext& context,
    const std::string& currentPassword,
    const std::string& newPassword,
    const std::string& confirmPassword,
    std::string& outError) 
{
    if (!context.authenticated || context.accountId <= 0) {
        outError = "User is not authenticated.";
        return false;
    }

    // Verify session validity
    SessionRecord s;
    if (!SessionManager::isSessionValid(context.sessionToken, s)) {
        outError = "Session is invalid or expired. Please log in again.";
        return false;
    }

    Account acc;
    if (!AccountRepository::findById(context.accountId, acc)) {
        outError = "Account record not found.";
        return false;
    }

    // Verify current password
    if (!PasswordHasher::verifyPassword(currentPassword, acc.passwordSalt, acc.passwordHash)) {
        outError = "Current password does not match.";
        return false;
    }

    // Validate new password policy
    auto passVal = SecurityUtils::validatePassword(newPassword);
    if (!passVal.valid) {
        outError = passVal.message;
        return false;
    }

    if (newPassword != confirmPassword) {
        outError = "New passwords do not match.";
        return false;
    }

    if (currentPassword == newPassword) {
        outError = "New password must be different from current password.";
        return false;
    }

    // Generate new salt and hash
    std::string newSalt = PasswordHasher::generateSalt();
    std::string newHash = PasswordHasher::hashPassword(newPassword, newSalt);

    if (!AccountRepository::updatePassword(acc.id, newHash, newSalt)) {
        outError = "Failed to update password in database.";
        return false;
    }

    // Invalidate all active sessions for this account
    SessionManager::revokeAllUserSessions(acc.id);
    context.authenticated = false;
    context.sessionToken.clear();

    AuditLogger::log(acc.id, AuditLogger::ACTION_PASSWORD_CHANGED, "Password changed and active sessions invalidated");
    return true;
}

bool AuthenticationManager::generatePasswordResetCode(
    const std::string& username,
    const std::string& email,
    std::string& outResetCode,
    std::string& outError) 
{
    outResetCode.clear();

    if (username.empty() || email.empty()) {
        outError = "Username and email must be provided.";
        return false;
    }

    Account acc;
    if (!AccountRepository::findByUsername(username, acc)) {
        outError = "Invalid username or email.";
        return false;
    }

    if (acc.email != email) {
        outError = "Invalid username or email.";
        return false;
    }

    // Generate 6-digit random code
    std::string code = SecurityUtils::generateNumericCode(6);
    std::string codeHash = SecurityUtils::sha256Hex(code);
    std::string expiresAt = SecurityUtils::getExpirationTimestamp(15); // 15-minute expiration

    if (!AccountRepository::storeResetCode(username, codeHash, expiresAt)) {
        outError = "Failed to generate password reset request.";
        return false;
    }

    outResetCode = code;
    return true;
}

bool AuthenticationManager::resetPasswordWithCode(
    const std::string& username,
    const std::string& resetCode,
    const std::string& newPassword,
    const std::string& confirmPassword,
    std::string& outError) 
{
    if (username.empty() || resetCode.empty()) {
        outError = "Username and reset code must be provided.";
        return false;
    }

    Account acc;
    if (!AccountRepository::findByUsername(username, acc)) {
        outError = "Account not found.";
        return false;
    }

    std::string codeHash = SecurityUtils::sha256Hex(resetCode);
    if (!AccountRepository::verifyAndConsumeResetCode(username, codeHash)) {
        outError = "Invalid, expired, or already used reset code.";
        return false;
    }

    // Validate new password policy
    auto passVal = SecurityUtils::validatePassword(newPassword);
    if (!passVal.valid) {
        outError = passVal.message;
        return false;
    }

    if (newPassword != confirmPassword) {
        outError = "New passwords do not match.";
        return false;
    }

    // Generate new salt and hash
    std::string newSalt = PasswordHasher::generateSalt();
    std::string newHash = PasswordHasher::hashPassword(newPassword, newSalt);

    if (!AccountRepository::updatePassword(acc.id, newHash, newSalt)) {
        outError = "Failed to update password in database.";
        return false;
    }

    // Reset failed login attempts and unlock if locked
    AccountRepository::resetFailedAttempts(acc.id);
    if (acc.status == "LOCKED") {
        AccountRepository::updateStatus(acc.id, "ACTIVE");
    }

    // Revoke all existing sessions
    SessionManager::revokeAllUserSessions(acc.id);

    AuditLogger::log(acc.id, AuditLogger::ACTION_PASSWORD_RESET, "Password reset with single-use verification code");
    return true;
}

bool AuthenticationManager::validateContext(SecurityContext& context) {
    if (!context.authenticated || context.sessionToken.empty()) {
        context.authenticated = false;
        return false;
    }
    SessionRecord session;
    if (!SessionManager::isSessionValid(context.sessionToken, session)) {
        context.authenticated = false;
        return false;
    }
    return true;
}
