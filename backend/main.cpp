// ============================================================
// SmartHire — Phase 8: Secure Multi-User Recruitment Application
// ============================================================
// Phase 1-5: Ingestion, NLP, Advanced Matching, Readiness & Recommendations
// Phase 6: Persistent Database, Candidate Profiles & Application Tracking
// Phase 7: Semantic-Style Matching & Explainable Intelligence
// Phase 8: Authentication, RBAC, Session Management & Audit Logging
// ============================================================

#include "Resume.h"
#include "Job.h"
#include "ResumeAnalyzer.h"
#include "JobMatcher.h"
#include "ResumeParser.h"
#include "JobDataLoader.h"
#include "ReportGenerator.h"
#include "SkillGapAnalyzer.h"
#include "ResumeStrengthAnalyzer.h"
#include "RecommendationEngine.h"
#include "ResumeDocumentReader.h"
#include "ResumeInputManager.h"
#include "DatabaseManager.h"
#include "CandidateRepository.h"
#include "JobRepository.h"
#include "ResumeRepository.h"
#include "ApplicationManager.h"
#include "SavedJobManager.h"
#include "AdvancedJobMatcher.h"
#include "SecurityUtils.h"
#include "PasswordHasher.h"
#include "AccountRepository.h"
#include "SessionManager.h"
#include "AuthorizationManager.h"
#include "AuthenticationManager.h"
#include "AuditLogger.h"
#include "HttpServer.h"
#include "DemoDataManager.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <algorithm>

// ── Helper functions ─────────────────────────────────────────
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// ── Menus ────────────────────────────────────────────────────
void displayBanner();
void displayUnauthenticatedMenu();
void displayCandidateDashboardMenu();
void displayRecruiterDashboardMenu();

// ── Authentication Flows ─────────────────────────────────────
void handleLoginFlow(std::vector<Job>& jobs, bool& jobsLoaded);
void handleRegistrationFlow();
void handleForgotPasswordFlow();

// ── Workflows ────────────────────────────────────────────────
void candidateWorkflow(SecurityContext& context, std::vector<Job>& jobs, bool& jobsLoaded);
void recruiterWorkflow(SecurityContext& context, std::vector<Job>& jobs, bool& jobsLoaded);

// ── Candidate Actions ────────────────────────────────────────
void candidateProfileView(const SecurityContext& context);
void loadResumeAction(const SecurityContext& context, Resume& resume, bool& resumeLoaded, std::string& loadedResumePath);
void displayResumeAction(const Resume& resume, bool resumeLoaded);
void performSmartResumeAnalysis(const Resume& resume, const std::vector<Job>& jobs, bool resumeLoaded, bool jobsLoaded);
void viewJobMatches(const Resume& resume, const std::vector<Job>& jobs, bool resumeLoaded, bool jobsLoaded);
void performAdvancedMatchAnalysis(const Resume& resume, const std::vector<Job>& jobs, bool resumeLoaded, bool jobsLoaded);
void performCompareAlgorithms(const Resume& resume, const std::vector<Job>& jobs, bool resumeLoaded, bool jobsLoaded);
void browseJobsAction(const std::vector<Job>& jobs);
void saveJobAction(const SecurityContext& context, const std::vector<Job>& jobs);
void viewSavedJobsAction(const SecurityContext& context, const std::vector<Job>& jobs);
void applyForJobAction(const SecurityContext& context, const Resume& resume, const std::vector<Job>& jobs, bool resumeLoaded);
void viewMyApplicationsAction(const SecurityContext& context);
void withdrawApplicationAction(const SecurityContext& context);
void generateCandidateReportAction(const SecurityContext& context, const Resume& resume,
                                   const std::vector<Job>& jobs, bool resumeLoaded,
                                   bool jobsLoaded, const std::string& loadedResumePath);

// ── Recruiter Actions ────────────────────────────────────────
void recruiterDashboardOverview(const SecurityContext& context);
void createJobAction(const SecurityContext& context, std::vector<Job>& jobs);
void viewMyJobsAction(const SecurityContext& context, const std::vector<Job>& jobs);
void viewRecruiterApplicationsAction(const SecurityContext& context);
void updateApplicationStatusAction(const SecurityContext& context);
void viewCandidateDetailsAction(const SecurityContext& context);

// ── Common Actions ───────────────────────────────────────────
void changePasswordAction(SecurityContext& context);

// ============================================================
// Main Application Entry Point
// ============================================================
int main(int argc, char* argv[]) {
    // Check if running in REST API server mode or demo tooling
    bool serverMode = false;
    bool seedDemo = false;
    bool clearDemo = false;
    int portOverride = 0;
    std::string hostOverride;
    std::string configPath = "config/server.conf";
    std::string dbPath = "data/smarthire.db";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server" || arg == "-s") {
            serverMode = true;
        } else if (arg == "--seed-demo") {
            seedDemo = true;
        } else if (arg == "--clear-demo") {
            clearDemo = true;
        } else if (arg == "--port" && i + 1 < argc) {
            try { portOverride = std::stoi(argv[++i]); } catch (...) {}
        } else if (arg == "--host" && i + 1 < argc) {
            hostOverride = argv[++i];
        } else if (arg == "--config" && i + 1 < argc) {
            configPath = argv[++i];
        } else if (arg == "--db" && i + 1 < argc) {
            dbPath = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "============================================================\n";
            std::cout << " SmartHire — Intelligent Resume & Job Matching Platform\n";
            std::cout << "============================================================\n";
            std::cout << "Usage:\n";
            std::cout << "  SmartHire.exe                 Interactive console CLI mode\n";
            std::cout << "  SmartHire.exe --server        Launch REST API HTTP server (default: 8080)\n";
            std::cout << "  SmartHire.exe --seed-demo     Seed deterministic sample presentation data\n";
            std::cout << "  SmartHire.exe --clear-demo    Remove demo dataset from database\n\n";
            std::cout << "Options:\n";
            std::cout << "  --port <port>                 Override server port (e.g. 8080)\n";
            std::cout << "  --host <ip>                   Override server bind address (e.g. 127.0.0.1)\n";
            std::cout << "  --config <path>               Custom server configuration file\n";
            std::cout << "  --db <path>                   Custom SQLite database path\n";
            std::cout << "============================================================\n";
            return 0;
        }
    }

    if (seedDemo) {
        if (!DatabaseManager::initialize(dbPath, false)) {
            std::cerr << "  [CRITICAL] Could not initialize database: "
                      << DatabaseManager::getLastError() << "\n";
            return 1;
        }
        bool ok = DemoDataManager::seedDemoData(true);
        DatabaseManager::close();
        return ok ? 0 : 1;
    }

    if (clearDemo) {
        if (!DatabaseManager::initialize(dbPath, false)) {
            std::cerr << "  [CRITICAL] Could not initialize database: "
                      << DatabaseManager::getLastError() << "\n";
            return 1;
        }
        bool ok = DemoDataManager::clearDemoData(true);
        DatabaseManager::close();
        return ok ? 0 : 1;
    }

    if (serverMode) {
        HttpServer server;
        server.loadConfig(configPath);
        if (portOverride > 0) server.setPort(portOverride);
        if (!hostOverride.empty()) server.setHost(hostOverride);

        std::string effectiveDbPath = server.getConfig().databasePath.empty() ? dbPath : server.getConfig().databasePath;

        // Initialize Persistent Database (Phase 6/8 schema & migrations)
        if (!DatabaseManager::initialize(effectiveDbPath, true)) {
            std::cerr << "  [CRITICAL] Could not initialize database: "
                      << DatabaseManager::getLastError() << "\n";
            return 1;
        }

        std::filesystem::create_directories(server.getConfig().uploadDirectory);

        // Seed initial jobs if table is empty
        if (JobRepository::countJobs() == 0 && std::filesystem::exists("data/jobs.txt")) {
            JobRepository::syncJobsFromFile("data/jobs.txt");
        }

        server.start();
        DatabaseManager::close();
        return 0;
    }

    displayBanner();

    // Initialize Persistent Database (Phase 6/8 schema & migrations)
    if (!DatabaseManager::initialize(dbPath, true)) {
        std::cerr << "  [CRITICAL] Could not initialize database: "
                  << DatabaseManager::getLastError() << "\n";
    }

    // Load initial jobs
    std::vector<Job> jobs;
    bool jobsLoaded = false;
    if (JobRepository::countJobs() == 0 && std::filesystem::exists("data/jobs.txt")) {
        std::cout << "\n  [Sync] Importing initial jobs into SQLite database...\n";
        int imported = JobRepository::syncJobsFromFile("data/jobs.txt");
        std::cout << "  [Sync] " << imported << " jobs imported.\n";
    }
    jobs = JobRepository::getAllJobs();
    if (!jobs.empty()) {
        jobsLoaded = true;
    }

    // Top-level Authentication Loop
    int choice = 0;
    do {
        displayUnauthenticatedMenu();
        std::cout << "Enter choice: ";

        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "\n  [ERROR] Invalid input. Please enter 1, 2, 3, or 4.\n";
            continue;
        }
        std::cin.ignore(10000, '\n');

        switch (choice) {
            case 1:
                handleLoginFlow(jobs, jobsLoaded);
                break;
            case 2:
                handleRegistrationFlow();
                break;
            case 3:
                handleForgotPasswordFlow();
                break;
            case 4:
                std::cout << "\n  Closing database and exiting SmartHire...\n";
                DatabaseManager::close();
                std::cout << "  Thank you for using SmartHire!\n\n";
                break;
            default:
                std::cout << "\n  [ERROR] Invalid choice. Please select 1, 2, 3, or 4.\n";
                break;
        }
    } while (choice != 4);

    return 0;
}

// ============================================================
// Banner & Menu Displays
// ============================================================
void displayBanner() {
    std::cout << "\n";
    std::cout << "  ****************************************************\n";
    std::cout << "  *                                                  *\n";
    std::cout << "  *   SmartHire — Secure Multi-User Recruitment     *\n";
    std::cout << "  *                  Version 9.0                     *\n";
    std::cout << "  *                                                  *\n";
    std::cout << "  ****************************************************\n";
}

void displayUnauthenticatedMenu() {
    std::cout << "\n========================================\n";
    std::cout << "            SMART HIRE\n";
    std::cout << "       SECURE RECRUITMENT SYSTEM\n";
    std::cout << "========================================\n\n";
    std::cout << "1. Login\n";
    std::cout << "2. Register\n";
    std::cout << "3. Forgot Password / Local Reset\n";
    std::cout << "4. Exit\n\n";
}

void displayCandidateDashboardMenu() {
    std::cout << "\n========================================\n";
    std::cout << "       CANDIDATE DASHBOARD\n";
    std::cout << "========================================\n\n";
    std::cout << "1. My Profile\n";
    std::cout << "2. Load Resume\n";
    std::cout << "3. Display Resume\n";
    std::cout << "4. Analyze Resume\n";
    std::cout << "5. View Job Matches\n";
    std::cout << "6. Advanced Match Analysis\n";
    std::cout << "7. Compare Matching Algorithms\n";
    std::cout << "8. Browse Jobs\n";
    std::cout << "9. Save Job\n";
    std::cout << "10. Saved Jobs\n";
    std::cout << "11. Apply for Job\n";
    std::cout << "12. My Applications\n";
    std::cout << "13. Withdraw Application\n";
    std::cout << "14. My Activity\n";
    std::cout << "15. Change Password\n";
    std::cout << "16. Logout\n\n";
}

void displayRecruiterDashboardMenu() {
    std::cout << "\n========================================\n";
    std::cout << "       RECRUITER DASHBOARD\n";
    std::cout << "========================================\n\n";
    std::cout << "1. Dashboard\n";
    std::cout << "2. Create Job\n";
    std::cout << "3. My Jobs\n";
    std::cout << "4. View Applications\n";
    std::cout << "5. Update Application Status\n";
    std::cout << "6. View Candidate\n";
    std::cout << "7. My Activity\n";
    std::cout << "8. Change Password\n";
    std::cout << "9. Logout\n\n";
}

// ============================================================
// Authentication Flows
// ============================================================

void handleLoginFlow(std::vector<Job>& jobs, bool& jobsLoaded) {
    std::cout << "\n========================================\n";
    std::cout << "              USER LOGIN\n";
    std::cout << "========================================\n\n";

    std::string username, password;
    std::cout << "Username: ";
    std::getline(std::cin, username);
    username = trim(username);

    std::cout << "Password: ";
    std::getline(std::cin, password);

    SecurityContext context;
    std::string error;
    if (!AuthenticationManager::login(username, password, context, error)) {
        std::cout << "\n  [FAILED] " << error << "\n";
        return;
    }

    std::cout << "\n========================================\n";
    std::cout << "LOGIN SUCCESSFUL\n";
    std::cout << "========================================\n\n";
    std::cout << "Welcome, " << context.username << "!\n\n";
    std::cout << "Role:\n" << context.role << "\n\n";

    if (context.role == "CANDIDATE") {
        candidateWorkflow(context, jobs, jobsLoaded);
    } else if (context.role == "RECRUITER") {
        recruiterWorkflow(context, jobs, jobsLoaded);
    }
}

void handleRegistrationFlow() {
    std::cout << "\n========================================\n";
    std::cout << "             USER REGISTRATION\n";
    std::cout << "========================================\n\n";
    std::cout << "1. Register as Candidate\n";
    std::cout << "2. Register as Recruiter\n";
    std::cout << "3. Back\n\n";
    std::cout << "Enter choice: ";

    int choice = 0;
    if (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        return;
    }
    std::cin.ignore(10000, '\n');

    if (choice == 1) {
        std::string fullName, email, phone, location, username, password, confirmPassword;
        std::cout << "\nFull Name: ";
        std::getline(std::cin, fullName);
        std::cout << "Email: ";
        std::getline(std::cin, email);
        std::cout << "Phone: ";
        std::getline(std::cin, phone);
        std::cout << "Location: ";
        std::getline(std::cin, location);
        std::cout << "Username (3-30 letters/numbers/_): ";
        std::getline(std::cin, username);
        std::cout << "Password (min 8 chars, 1 uppercase, 1 lowercase, 1 digit): ";
        std::getline(std::cin, password);
        std::cout << "Confirm Password: ";
        std::getline(std::cin, confirmPassword);

        std::string err;
        int accId = 0, candId = 0;
        if (AuthenticationManager::registerCandidate(trim(fullName), trim(email), trim(phone),
                                                     trim(username), password, confirmPassword,
                                                     trim(location), err, accId, candId)) {
            std::cout << "\n  [SUCCESS] Candidate account registered successfully!\n";
            std::cout << "  You may now login with your credentials.\n";
        } else {
            std::cout << "\n  [REGISTRATION FAILED] " << err << "\n";
        }
    } else if (choice == 2) {
        std::string recName, company, email, username, password, confirmPassword;
        std::cout << "\nRecruiter Name: ";
        std::getline(std::cin, recName);
        std::cout << "Company: ";
        std::getline(std::cin, company);
        std::cout << "Email: ";
        std::getline(std::cin, email);
        std::cout << "Username (3-30 letters/numbers/_): ";
        std::getline(std::cin, username);
        std::cout << "Password (min 8 chars, 1 uppercase, 1 lowercase, 1 digit): ";
        std::getline(std::cin, password);
        std::cout << "Confirm Password: ";
        std::getline(std::cin, confirmPassword);

        std::string err;
        int accId = 0;
        if (AuthenticationManager::registerRecruiter(trim(recName), trim(company), trim(email),
                                                     trim(username), password, confirmPassword,
                                                     err, accId)) {
            std::cout << "\n  [SUCCESS] Recruiter account registered successfully!\n";
            std::cout << "  You may now login with your credentials.\n";
        } else {
            std::cout << "\n  [REGISTRATION FAILED] " << err << "\n";
        }
    }
}

void handleForgotPasswordFlow() {
    std::cout << "\n========================================\n";
    std::cout << "   FORGOT PASSWORD / LOCAL RESET (DEMO) \n";
    std::cout << "========================================\n\n";
    std::cout << "Note: This is an offline local development reset mechanism.\n\n";

    std::string username, email;
    std::cout << "Enter registered username: ";
    std::getline(std::cin, username);
    std::cout << "Enter registered email: ";
    std::getline(std::cin, email);

    std::string code, err;
    if (!AuthenticationManager::generatePasswordResetCode(trim(username), trim(email), code, err)) {
        std::cout << "\n  [ERROR] " << err << "\n";
        return;
    }

    std::cout << "\n  [VERIFICATION CODE GENERATED]\n";
    std::cout << "  Your one-time password reset code is: " << code << "\n";
    std::cout << "  (Valid for 15 minutes, single-use)\n\n";

    std::string enteredCode, newPassword, confirmPassword;
    std::cout << "Enter verification code: ";
    std::getline(std::cin, enteredCode);
    std::cout << "Enter new password: ";
    std::getline(std::cin, newPassword);
    std::cout << "Confirm new password: ";
    std::getline(std::cin, confirmPassword);

    if (AuthenticationManager::resetPasswordWithCode(trim(username), trim(enteredCode),
                                                    newPassword, confirmPassword, err)) {
        std::cout << "\n  [SUCCESS] Password has been reset successfully!\n";
        std::cout << "  Active sessions have been revoked. Please log in with your new password.\n";
    } else {
        std::cout << "\n  [RESET FAILED] " << err << "\n";
    }
}

// ============================================================
// Candidate Workflow Loop
// ============================================================
void candidateWorkflow(SecurityContext& context, std::vector<Job>& jobs, bool& jobsLoaded) {
    Resume resume;
    bool resumeLoaded = false;
    std::string loadedResumePath = "";

    // Pre-load candidate resume from SQLite if it exists
    if (context.candidateId > 0 && ResumeRepository::resumeExists(context.candidateId)) {
        resume = ResumeRepository::loadResume(context.candidateId);
        loadedResumePath = ResumeRepository::getResumeSourcePath(context.candidateId);
        if (!resume.getName().empty()) {
            resumeLoaded = true;
        }
    }

    int choice = 0;
    do {
        // Enforce session expiration check before every action
        if (!AuthenticationManager::validateContext(context)) {
            std::cout << "\n  [WARNING] Your session has expired. Please login again.\n";
            break;
        }

        displayCandidateDashboardMenu();
        std::cout << "Enter choice: ";

        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "\n  [ERROR] Invalid input. Please enter a number (1-16).\n";
            continue;
        }
        std::cin.ignore(10000, '\n');

        switch (choice) {
            case 1:
                candidateProfileView(context);
                break;
            case 2:
                loadResumeAction(context, resume, resumeLoaded, loadedResumePath);
                break;
            case 3:
                displayResumeAction(resume, resumeLoaded);
                break;
            case 4:
                performSmartResumeAnalysis(resume, jobs, resumeLoaded, jobsLoaded);
                break;
            case 5:
                viewJobMatches(resume, jobs, resumeLoaded, jobsLoaded);
                break;
            case 6:
                performAdvancedMatchAnalysis(resume, jobs, resumeLoaded, jobsLoaded);
                break;
            case 7:
                performCompareAlgorithms(resume, jobs, resumeLoaded, jobsLoaded);
                break;
            case 8:
                browseJobsAction(jobs);
                break;
            case 9:
                saveJobAction(context, jobs);
                break;
            case 10:
                viewSavedJobsAction(context, jobs);
                break;
            case 11:
                applyForJobAction(context, resume, jobs, resumeLoaded);
                break;
            case 12:
                viewMyApplicationsAction(context);
                break;
            case 13:
                withdrawApplicationAction(context);
                break;
            case 14:
                AuditLogger::displayAccountActivity(context.accountId, context.username);
                break;
            case 15:
                changePasswordAction(context);
                if (!context.authenticated) {
                    std::cout << "  Please log in again with your new password.\n";
                    return;
                }
                break;
            case 16:
                AuthenticationManager::logout(context);
                std::cout << "\n  Logged out successfully.\n";
                break;
            default:
                std::cout << "\n  [ERROR] Invalid choice. Please select 1-16.\n";
                break;
        }
    } while (choice != 16 && context.authenticated);
}

// ============================================================
// Recruiter Workflow Loop
// ============================================================
void recruiterWorkflow(SecurityContext& context, std::vector<Job>& jobs, bool& jobsLoaded) {
    int choice = 0;
    do {
        // Enforce session expiration check before every action
        if (!AuthenticationManager::validateContext(context)) {
            std::cout << "\n  [WARNING] Your session has expired. Please login again.\n";
            break;
        }

        displayRecruiterDashboardMenu();
        std::cout << "Enter choice: ";

        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "\n  [ERROR] Invalid input. Please enter a number (1-9).\n";
            continue;
        }
        std::cin.ignore(10000, '\n');

        switch (choice) {
            case 1:
                recruiterDashboardOverview(context);
                break;
            case 2:
                createJobAction(context, jobs);
                jobsLoaded = !jobs.empty();
                break;
            case 3:
                viewMyJobsAction(context, jobs);
                break;
            case 4:
                viewRecruiterApplicationsAction(context);
                break;
            case 5:
                updateApplicationStatusAction(context);
                break;
            case 6:
                viewCandidateDetailsAction(context);
                break;
            case 7:
                AuditLogger::displayAccountActivity(context.accountId, context.username);
                break;
            case 8:
                changePasswordAction(context);
                if (!context.authenticated) {
                    std::cout << "  Please log in again with your new password.\n";
                    return;
                }
                break;
            case 9:
                AuthenticationManager::logout(context);
                std::cout << "\n  Logged out successfully.\n";
                break;
            default:
                std::cout << "\n  [ERROR] Invalid choice. Please select 1-9.\n";
                break;
        }
    } while (choice != 9 && context.authenticated);
}

// ============================================================
// Candidate Actions Implementation
// ============================================================

void candidateProfileView(const SecurityContext& context) {
    if (!AuthorizationManager::canViewCandidateProfile(context, context.candidateId)) {
        std::cout << "\n  [ERROR] Access denied. You do not have permission to view this profile.\n";
        return;
    }

    Profile profile = CandidateRepository::getCandidate(context.candidateId);
    std::cout << "\n========================================\n";
    std::cout << "              MY PROFILE                \n";
    std::cout << "========================================\n\n";
    std::cout << "Candidate ID: " << context.candidateId << "\n";
    std::cout << "Name:         " << profile.getName() << "\n";
    std::cout << "Email:        " << profile.getEmail() << "\n";
    std::cout << "Phone:        " << (profile.getPhone().empty() ? "(none)" : profile.getPhone()) << "\n";
    std::cout << "Location:     " << (profile.getLocation().empty() ? "(none)" : profile.getLocation()) << "\n";
    std::cout << "Role:         CANDIDATE\n";
    std::cout << "========================================\n";
}

void loadResumeAction(const SecurityContext& context, Resume& resume, bool& resumeLoaded, std::string& loadedResumePath) {
    if (!AuthorizationManager::canUploadResume(context, context.candidateId)) {
        std::cout << "\n  [ERROR] Access denied. You do not have permission to upload resume for this profile.\n";
        return;
    }

    std::cout << "\n  Supported resume formats:\n";
    std::cout << "  1. TXT\n";
    std::cout << "  2. PDF\n\n";
    std::cout << "  Enter resume file path (or press Enter for default: data/sample_resume.txt): ";

    std::string path;
    std::getline(std::cin, path);
    path = trim(path);

    if (path.empty()) {
        path = "data/sample_resume.txt";
    }

    resume = ResumeInputManager::loadResume(path);
    if (!resume.getName().empty()) {
        resumeLoaded = true;
        loadedResumePath = path;

        // Persist to SQLite under this candidate's account
        int savedId = ResumeRepository::saveResume(context.candidateId, resume, path);
        if (savedId > 0) {
            std::cout << "\n  [SQLite] Resume saved to database for candidate ID " << context.candidateId << ".\n";
            AuditLogger::log(context.accountId, AuditLogger::ACTION_RESUME_UPLOADED, "Uploaded: " + path);
        }
    } else {
        std::cout << "\n  [ERROR] Failed to load resume from " << path << ".\n";
    }
}

void displayResumeAction(const Resume& resume, bool resumeLoaded) {
    if (!resumeLoaded) {
        std::cout << "\n  [ERROR] No resume loaded. Please load a resume first (Option 2).\n";
        return;
    }
    resume.displayResume();
}

void performSmartResumeAnalysis(const Resume& resume, const std::vector<Job>& jobs, bool resumeLoaded, bool jobsLoaded) {
    if (!resumeLoaded) {
        std::cout << "\n  [ERROR] No resume loaded. Please load a resume first (Option 2).\n";
        return;
    }

    ResumeStrengthAnalyzer::displayResumeStrengths(resume);
    ResumeStrengthAnalyzer::displayTechnologyFrequency(resume);

    if (jobsLoaded && !jobs.empty()) {
        SkillGapAnalyzer::displayMultiJobGapComparison(resume, jobs);
        RecommendationEngine::displayRecommendations(resume, jobs[0]);
    }
}

void viewJobMatches(const Resume& resume, const std::vector<Job>& jobs, bool resumeLoaded, bool jobsLoaded) {
    if (!resumeLoaded) {
        std::cout << "\n  [ERROR] No resume loaded. Please load a resume first (Option 2).\n";
        return;
    }
    if (!jobsLoaded || jobs.empty()) {
        std::cout << "\n  [ERROR] No jobs available in database.\n";
        return;
    }

    JobMatcher::displayJobMatches(resume, jobs);
    JobMatcher::displayAnalysisSummary(resume, jobs);
}

void performAdvancedMatchAnalysis(const Resume& resume, const std::vector<Job>& jobs, bool resumeLoaded, bool jobsLoaded) {
    if (!resumeLoaded) {
        std::cout << "\n  [ERROR] No resume loaded. Please load a resume first (Option 2).\n";
        return;
    }
    if (!jobsLoaded || jobs.empty()) {
        std::cout << "\n  [ERROR] No jobs available in database.\n";
        return;
    }

    AdvancedJobMatcher::displayAdvancedJobRankings(resume, jobs);
}

void performCompareAlgorithms(const Resume& resume, const std::vector<Job>& jobs, bool resumeLoaded, bool jobsLoaded) {
    if (!resumeLoaded) {
        std::cout << "\n  [ERROR] No resume loaded. Please load a resume first (Option 2).\n";
        return;
    }
    if (!jobsLoaded || jobs.empty()) {
        std::cout << "\n  [ERROR] No jobs available in database.\n";
        return;
    }

    std::cout << "\nSelect a job to compare:\n";
    for (size_t i = 0; i < jobs.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << jobs[i].getJobId() << " — " << jobs[i].getJobTitle() << "\n";
    }
    std::cout << "Enter choice (1-" << jobs.size() << "): ";
    int jobChoice = 0;
    if (!(std::cin >> jobChoice) || jobChoice < 1 || jobChoice > static_cast<int>(jobs.size())) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "\n  [ERROR] Invalid job choice.\n";
        return;
    }
    std::cin.ignore(10000, '\n');

    AdvancedJobMatcher::displayComparisonWithClassic(resume, jobs[jobChoice - 1]);
}

void browseJobsAction(const std::vector<Job>& jobs) {
    std::cout << "\n========================================\n";
    std::cout << "             AVAILABLE JOBS             \n";
    std::cout << "========================================\n\n";

    if (jobs.empty()) {
        std::cout << "No jobs available.\n";
        return;
    }

    for (const auto& job : jobs) {
        std::cout << "Job ID:          " << job.getJobId() << "\n";
        std::cout << "Title:           " << job.getJobTitle() << "\n";
        std::cout << "Company:         " << job.getCompany() << "\n";
        std::cout << "Location:        " << job.getLocation() << "\n";
        std::cout << "Employment Type: " << job.getEmploymentType() << "\n";
        std::cout << "Min Experience:  " << job.getMinimumExperience() << " years\n";
        std::cout << "----------------------------------------\n";
    }
}

void saveJobAction(const SecurityContext& context, const std::vector<Job>& jobs) {
    if (!AuthorizationManager::canSaveJob(context, context.candidateId)) {
        std::cout << "\n  [ERROR] Access denied. You do not have permission to save jobs.\n";
        return;
    }

    if (jobs.empty()) {
        std::cout << "\n  [ERROR] No jobs available.\n";
        return;
    }

    std::cout << "\nEnter Job ID to save: ";
    std::string jobId;
    std::getline(std::cin, jobId);
    jobId = trim(jobId);

    if (jobId.empty()) {
        std::cout << "  [ERROR] Job ID cannot be empty.\n";
        return;
    }

    if (SavedJobManager::saveJob(context.candidateId, jobId)) {
        std::cout << "\n  [SUCCESS] Job " << jobId << " saved successfully.\n";
        AuditLogger::log(context.accountId, AuditLogger::ACTION_SAVED_JOB_ADDED, "Saved Job: " + jobId);
    } else {
        std::cout << "\n  [NOTE] Job already saved or could not be saved.\n";
    }
}

void viewSavedJobsAction(const SecurityContext& context, const std::vector<Job>& jobs) {
    (void)jobs;
    SavedJobManager::displaySavedJobs(context.candidateId);
}

void applyForJobAction(const SecurityContext& context, const Resume& resume,
                       const std::vector<Job>& jobs, bool resumeLoaded) {
    if (!AuthorizationManager::canApplyForJob(context, context.candidateId)) {
        std::cout << "\n  [ERROR] Access denied. You do not have permission to apply for jobs.\n";
        return;
    }
    if (!resumeLoaded) {
        std::cout << "\n  [ERROR] No resume loaded. Please load a resume first (Option 2).\n";
        return;
    }

    std::cout << "\nEnter Job ID to apply for: ";
    std::string jobId;
    std::getline(std::cin, jobId);
    jobId = trim(jobId);

    const Job* targetJob = nullptr;
    for (const auto& j : jobs) {
        if (j.getJobId() == jobId) {
            targetJob = &j;
            break;
        }
    }

    if (!targetJob) {
        std::cout << "\n  [ERROR] Job ID '" << jobId << "' not found.\n";
        return;
    }

    double matchScore = AdvancedJobMatcher::calculateAdvancedMatchScore(resume, *targetJob);
    double readinessScore = RecommendationEngine::calculateJobReadinessScore(resume, *targetJob);

    int appId = ApplicationManager::applyForJob(context.candidateId, *targetJob, matchScore, readinessScore);
    if (appId > 0) {
        std::cout << "\n  [SUCCESS] Application submitted successfully (Application ID: " << appId << ")!\n";
        std::cout << "  Match Score:     " << std::fixed << std::setprecision(1) << matchScore << "%\n";
        std::cout << "  Readiness Score: " << std::fixed << std::setprecision(1) << readinessScore << "%\n";
        AuditLogger::log(context.accountId, AuditLogger::ACTION_APPLICATION_SUBMITTED,
                         "Applied to " + jobId + " (App ID: " + std::to_string(appId) + ")");
    } else {
        std::cout << "\n  [NOTE] You already have an active application for job " << jobId << ".\n";
    }
}

void viewMyApplicationsAction(const SecurityContext& context) {
    if (!AuthorizationManager::canViewCandidateApplications(context, context.candidateId)) {
        std::cout << "\n  [ERROR] Access denied.\n";
        return;
    }
    ApplicationManager::displayApplicationHistory(context.candidateId);
}

void withdrawApplicationAction(const SecurityContext& context) {
    std::cout << "\nEnter Application ID to withdraw: ";
    int appId = 0;
    if (!(std::cin >> appId)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "  [ERROR] Invalid application ID.\n";
        return;
    }
    std::cin.ignore(10000, '\n');

    // Ownership check: only application owner may withdraw
    if (!AuthorizationManager::canWithdrawApplication(context, appId)) {
        std::cout << "\n  [ERROR] Access denied. You do not own this application.\n";
        return;
    }

    if (ApplicationManager::withdrawApplication(appId)) {
        std::cout << "\n  [SUCCESS] Application " << appId << " has been withdrawn.\n";
        AuditLogger::log(context.accountId, AuditLogger::ACTION_APPLICATION_WITHDRAWN,
                         "Withdrawn application ID: " + std::to_string(appId));
    } else {
        std::cout << "\n  [ERROR] Could not withdraw application " << appId << ".\n";
    }
}

// ============================================================
// Recruiter Actions Implementation
// ============================================================

void recruiterDashboardOverview(const SecurityContext& context) {
    if (!AuthorizationManager::canViewRecruiterDashboard(context)) {
        std::cout << "\n  [ERROR] Access denied.\n";
        return;
    }
    ApplicationManager::displayRecruiterDashboard();
}

void createJobAction(const SecurityContext& context, std::vector<Job>& jobs) {
    if (!AuthorizationManager::canCreateJob(context)) {
        std::cout << "\n  [ERROR] Access denied. Only recruiters can create jobs.\n";
        return;
    }

    std::cout << "\n========================================\n";
    std::cout << "          CREATE NEW JOB LISTING        \n";
    std::cout << "========================================\n\n";

    std::string jobId, title, company, location, empType, reqSkillsStr, prefSkillsStr, eduReq, desc;
    int minExp = 0;

    std::cout << "Job ID: ";
    std::getline(std::cin, jobId);
    jobId = trim(jobId);
    if (jobId.empty()) {
        std::cout << "  [ERROR] Job ID cannot be empty.\n";
        return;
    }
    if (JobRepository::jobExists(jobId)) {
        std::cout << "  [ERROR] Job ID '" << jobId << "' already exists in database.\n";
        return;
    }

    std::cout << "Job Title: ";
    std::getline(std::cin, title);
    std::cout << "Company: ";
    std::getline(std::cin, company);
    std::cout << "Location: ";
    std::getline(std::cin, location);
    std::cout << "Employment Type: ";
    std::getline(std::cin, empType);
    std::cout << "Minimum Experience (Years): ";
    std::cin >> minExp;
    std::cin.ignore(10000, '\n');
    std::cout << "Education Requirement: ";
    std::getline(std::cin, eduReq);
    std::cout << "Required Skills (comma-separated): ";
    std::getline(std::cin, reqSkillsStr);
    std::cout << "Preferred Skills (comma-separated): ";
    std::getline(std::cin, prefSkillsStr);
    std::cout << "Job Description: ";
    std::getline(std::cin, desc);

    Job newJob;
    newJob.setJobId(jobId);
    newJob.setJobTitle(trim(title));
    newJob.setCompany(trim(company));
    newJob.setLocation(trim(location));
    newJob.setEmploymentType(trim(empType));
    newJob.setMinimumExperience(minExp);
    newJob.setEducationRequirement(trim(eduReq));
    newJob.setDescription(trim(desc));

    std::stringstream reqSs(reqSkillsStr);
    std::string s;
    while (std::getline(reqSs, s, ',')) {
        s = trim(s);
        if (!s.empty()) newJob.addRequiredSkill(s);
    }

    std::stringstream prefSs(prefSkillsStr);
    while (std::getline(prefSs, s, ',')) {
        s = trim(s);
        if (!s.empty()) newJob.addPreferredSkill(s);
    }

    if (JobRepository::saveJob(newJob)) {
        // Link job ownership to this recruiter
        AccountRepository::linkJobToRecruiter(jobId, context.accountId);
        jobs.push_back(newJob);
        std::cout << "\n  [SUCCESS] Job " << jobId << " created and linked to your recruiter account.\n";
        AuditLogger::log(context.accountId, AuditLogger::ACTION_JOB_CREATED, "Created job: " + jobId);
    } else {
        std::cout << "\n  [ERROR] Failed to save job in database.\n";
    }
}

void viewMyJobsAction(const SecurityContext& context, const std::vector<Job>& jobs) {
    std::cout << "\n========================================\n";
    std::cout << "              MY POSTED JOBS            \n";
    std::cout << "========================================\n\n";

    std::vector<std::string> myJobIds = AccountRepository::getJobIdsForRecruiter(context.accountId);
    if (myJobIds.empty()) {
        std::cout << "You have not posted any jobs yet.\n";
        return;
    }

    for (const auto& jId : myJobIds) {
        Job j = JobRepository::getJob(jId);
        std::cout << "Job ID:   " << j.getJobId() << "\n";
        std::cout << "Title:    " << j.getJobTitle() << "\n";
        std::cout << "Company:  " << j.getCompany() << "\n";
        std::cout << "Location: " << j.getLocation() << "\n";
        std::cout << "----------------------------------------\n";
    }
}

void viewRecruiterApplicationsAction(const SecurityContext& context) {
    if (!AuthorizationManager::canViewRecruiterDashboard(context)) {
        std::cout << "\n  [ERROR] Access denied.\n";
        return;
    }
    ApplicationManager::displayRecruiterApplications();
}

void updateApplicationStatusAction(const SecurityContext& context) {
    std::cout << "\nEnter Application ID to update: ";
    int appId = 0;
    if (!(std::cin >> appId)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "  [ERROR] Invalid application ID.\n";
        return;
    }
    std::cin.ignore(10000, '\n');

    // Ownership check: recruiter must own the job for this application
    if (!AuthorizationManager::canUpdateApplicationStatus(context, appId)) {
        std::cout << "\n  [ERROR] Access denied. You do not manage the job for this application.\n";
        return;
    }

    std::cout << "\nAvailable Status Options:\n";
    std::cout << "  1. " << ApplicationManager::STATUS_UNDER_REVIEW << "\n";
    std::cout << "  2. " << ApplicationManager::STATUS_SHORTLISTED << "\n";
    std::cout << "  3. " << ApplicationManager::STATUS_INTERVIEW << "\n";
    std::cout << "  4. " << ApplicationManager::STATUS_SELECTED << "\n";
    std::cout << "  5. " << ApplicationManager::STATUS_REJECTED << "\n\n";
    std::cout << "Select new status (1-5): ";

    int sChoice = 0;
    if (!(std::cin >> sChoice)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "  [ERROR] Invalid choice.\n";
        return;
    }
    std::cin.ignore(10000, '\n');

    std::string newStatus;
    switch (sChoice) {
        case 1: newStatus = ApplicationManager::STATUS_UNDER_REVIEW; break;
        case 2: newStatus = ApplicationManager::STATUS_SHORTLISTED; break;
        case 3: newStatus = ApplicationManager::STATUS_INTERVIEW; break;
        case 4: newStatus = ApplicationManager::STATUS_SELECTED; break;
        case 5: newStatus = ApplicationManager::STATUS_REJECTED; break;
        default:
            std::cout << "  [ERROR] Invalid selection.\n";
            return;
    }

    if (ApplicationManager::updateStatus(appId, newStatus)) {
        std::cout << "\n  [SUCCESS] Application " << appId << " status updated to " << newStatus << ".\n";
        AuditLogger::log(context.accountId, AuditLogger::ACTION_APP_STATUS_CHANGED,
                         "App ID " + std::to_string(appId) + " -> " + newStatus);
    } else {
        std::cout << "\n  [ERROR] Could not update status for application " << appId << ".\n";
    }
}

void viewCandidateDetailsAction(const SecurityContext& context) {
    std::cout << "\nEnter Candidate ID: ";
    int candId = 0;
    if (!(std::cin >> candId)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "  [ERROR] Invalid candidate ID.\n";
        return;
    }
    std::cin.ignore(10000, '\n');

    if (!AuthorizationManager::canViewCandidateProfile(context, candId)) {
        std::cout << "\n  [ERROR] Access denied.\n";
        return;
    }

    if (!CandidateRepository::candidateExists(candId)) {
        std::cout << "\n  [ERROR] Candidate ID " << candId << " does not exist.\n";
        return;
    }

    Profile profile = CandidateRepository::getCandidate(candId);
    std::cout << "\n========================================\n";
    std::cout << "           CANDIDATE DOSSIER            \n";
    std::cout << "========================================\n\n";
    std::cout << "ID:       " << candId << "\n";
    std::cout << "Name:     " << profile.getName() << "\n";
    std::cout << "Email:    " << profile.getEmail() << "\n";
    std::cout << "Phone:    " << profile.getPhone() << "\n";
    std::cout << "Location: " << profile.getLocation() << "\n";

    if (ResumeRepository::resumeExists(candId)) {
        Resume res = ResumeRepository::loadResume(candId);
        std::cout << "\nSkills: ";
        for (const auto& sk : res.getSkills()) std::cout << sk.getName() << ", ";
        std::cout << "\nResume Score: " << std::fixed << std::setprecision(1)
                  << ResumeStrengthAnalyzer::calculateResumeQualityScore(res) << "%\n";
    }
    std::cout << "========================================\n";
}

// ============================================================
// Common Actions Implementation
// ============================================================

void changePasswordAction(SecurityContext& context) {
    if (!AuthorizationManager::canChangePassword(context)) {
        std::cout << "\n  [ERROR] Session invalid or expired.\n";
        return;
    }

    std::cout << "\n========================================\n";
    std::cout << "             CHANGE PASSWORD            \n";
    std::cout << "========================================\n\n";

    std::string currentPass, newPass, confirmPass;
    std::cout << "Current Password: ";
    std::getline(std::cin, currentPass);
    std::cout << "New Password (min 8 chars, 1 uppercase, 1 lowercase, 1 digit): ";
    std::getline(std::cin, newPass);
    std::cout << "Confirm New Password: ";
    std::getline(std::cin, confirmPass);

    std::string err;
    if (AuthenticationManager::changePassword(context, currentPass, newPass, confirmPass, err)) {
        std::cout << "\n  [SUCCESS] Password changed successfully!\n";
        std::cout << "  All active sessions have been invalidated.\n";
    } else {
        std::cout << "\n  [FAILED] " << err << "\n";
    }
}
