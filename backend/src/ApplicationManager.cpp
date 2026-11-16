#include "ApplicationManager.h"
#include "DatabaseManager.h"
#include "JobRepository.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

const std::string ApplicationManager::STATUS_APPLIED = "APPLIED";
const std::string ApplicationManager::STATUS_UNDER_REVIEW = "UNDER_REVIEW";
const std::string ApplicationManager::STATUS_SHORTLISTED = "SHORTLISTED";
const std::string ApplicationManager::STATUS_INTERVIEW = "INTERVIEW";
const std::string ApplicationManager::STATUS_REJECTED = "REJECTED";
const std::string ApplicationManager::STATUS_SELECTED = "SELECTED";
const std::string ApplicationManager::STATUS_WITHDRAWN = "WITHDRAWN";

bool ApplicationManager::isValidStatus(const std::string& status) {
    return (status == STATUS_APPLIED ||
            status == STATUS_UNDER_REVIEW ||
            status == STATUS_SHORTLISTED ||
            status == STATUS_INTERVIEW ||
            status == STATUS_REJECTED ||
            status == STATUS_SELECTED ||
            status == STATUS_WITHDRAWN);
}

bool ApplicationManager::hasActiveApplication(int candidateId, const std::string& jobId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db || candidateId <= 0 || jobId.empty()) return false;

    const char* sql = "SELECT 1 FROM applications WHERE candidate_id = ? AND job_id = ? AND status != 'WITHDRAWN';";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    sqlite3_bind_text(stmt, 2, jobId.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

int ApplicationManager::applyForJob(int candidateId, const Job& job,
                                    double matchScore, double readinessScore) {
    if (candidateId <= 0 || job.getJobId().empty()) return -1;
    if (hasActiveApplication(candidateId, job.getJobId())) {
        return -1; // Duplicate active application prevented
    }

    sqlite3* db = DatabaseManager::getDb();
    if (!db) return -1;

    // Ensure job is saved in repository
    if (!JobRepository::jobExists(job.getJobId())) {
        JobRepository::saveJob(job);
    }

    const char* sql = "INSERT INTO applications (candidate_id, job_id, match_score, readiness_score, status) "
                      "VALUES (?, ?, ?, ?, 'APPLIED');";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    sqlite3_bind_text(stmt, 2, job.getJobId().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, matchScore);
    sqlite3_bind_double(stmt, 4, readinessScore);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        return static_cast<int>(sqlite3_last_insert_rowid(db));
    }
    return -1;
}

bool ApplicationManager::updateStatus(int applicationId, const std::string& status) {
    if (applicationId <= 0 || !isValidStatus(status)) {
        return false;
    }

    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "UPDATE applications SET status = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, applicationId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE && sqlite3_changes(db) > 0);
}

bool ApplicationManager::withdrawApplication(int applicationId) {
    return updateStatus(applicationId, STATUS_WITHDRAWN);
}

std::vector<ApplicationRecord> ApplicationManager::getCandidateApplications(int candidateId) {
    std::vector<ApplicationRecord> list;
    sqlite3* db = DatabaseManager::getDb();
    if (!db || candidateId <= 0) return list;

    const char* sql = 
        "SELECT a.id, a.candidate_id, c.name, a.job_id, COALESCE(j.title, 'Unknown Job'), "
        "COALESCE(j.company, 'Unknown Company'), a.match_score, a.readiness_score, a.status, "
        "strftime('%Y-%m-%d', a.applied_at), strftime('%Y-%m-%d', a.updated_at) "
        "FROM applications a "
        "JOIN candidates c ON a.candidate_id = c.id "
        "LEFT JOIN jobs j ON a.job_id = j.job_id "
        "WHERE a.candidate_id = ? "
        "ORDER BY a.id DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return list;
    }

    sqlite3_bind_int(stmt, 1, candidateId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ApplicationRecord rec;
        rec.id = sqlite3_column_int(stmt, 0);
        rec.candidateId = sqlite3_column_int(stmt, 1);
        const unsigned char* cName = sqlite3_column_text(stmt, 2);
        const unsigned char* jId = sqlite3_column_text(stmt, 3);
        const unsigned char* jTitle = sqlite3_column_text(stmt, 4);
        const unsigned char* jComp = sqlite3_column_text(stmt, 5);
        rec.matchScore = sqlite3_column_double(stmt, 6);
        rec.readinessScore = sqlite3_column_double(stmt, 7);
        const unsigned char* st = sqlite3_column_text(stmt, 8);
        const unsigned char* appAt = sqlite3_column_text(stmt, 9);
        const unsigned char* updAt = sqlite3_column_text(stmt, 10);

        rec.candidateName = cName ? reinterpret_cast<const char*>(cName) : "";
        rec.jobId = jId ? reinterpret_cast<const char*>(jId) : "";
        rec.jobTitle = jTitle ? reinterpret_cast<const char*>(jTitle) : "";
        rec.company = jComp ? reinterpret_cast<const char*>(jComp) : "";
        rec.status = st ? reinterpret_cast<const char*>(st) : "";
        rec.appliedAt = appAt ? reinterpret_cast<const char*>(appAt) : "";
        rec.updatedAt = updAt ? reinterpret_cast<const char*>(updAt) : "";

        list.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return list;
}

std::vector<ApplicationRecord> ApplicationManager::getAllApplications() {
    std::vector<ApplicationRecord> list;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return list;

    const char* sql = 
        "SELECT a.id, a.candidate_id, c.name, a.job_id, COALESCE(j.title, 'Unknown Job'), "
        "COALESCE(j.company, 'Unknown Company'), a.match_score, a.readiness_score, a.status, "
        "strftime('%Y-%m-%d', a.applied_at), strftime('%Y-%m-%d', a.updated_at) "
        "FROM applications a "
        "JOIN candidates c ON a.candidate_id = c.id "
        "LEFT JOIN jobs j ON a.job_id = j.job_id "
        "ORDER BY a.id DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ApplicationRecord rec;
        rec.id = sqlite3_column_int(stmt, 0);
        rec.candidateId = sqlite3_column_int(stmt, 1);
        const unsigned char* cName = sqlite3_column_text(stmt, 2);
        const unsigned char* jId = sqlite3_column_text(stmt, 3);
        const unsigned char* jTitle = sqlite3_column_text(stmt, 4);
        const unsigned char* jComp = sqlite3_column_text(stmt, 5);
        rec.matchScore = sqlite3_column_double(stmt, 6);
        rec.readinessScore = sqlite3_column_double(stmt, 7);
        const unsigned char* st = sqlite3_column_text(stmt, 8);
        const unsigned char* appAt = sqlite3_column_text(stmt, 9);
        const unsigned char* updAt = sqlite3_column_text(stmt, 10);

        rec.candidateName = cName ? reinterpret_cast<const char*>(cName) : "";
        rec.jobId = jId ? reinterpret_cast<const char*>(jId) : "";
        rec.jobTitle = jTitle ? reinterpret_cast<const char*>(jTitle) : "";
        rec.company = jComp ? reinterpret_cast<const char*>(jComp) : "";
        rec.status = st ? reinterpret_cast<const char*>(st) : "";
        rec.appliedAt = appAt ? reinterpret_cast<const char*>(appAt) : "";
        rec.updatedAt = updAt ? reinterpret_cast<const char*>(updAt) : "";

        list.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return list;
}

int ApplicationManager::countCandidateApplications(int candidateId, const std::string& status) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db || candidateId <= 0) return 0;

    sqlite3_stmt* stmt = nullptr;
    if (status.empty()) {
        const char* sql = "SELECT COUNT(*) FROM applications WHERE candidate_id = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;
        sqlite3_bind_int(stmt, 1, candidateId);
    } else {
        const char* sql = "SELECT COUNT(*) FROM applications WHERE candidate_id = ? AND status = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;
        sqlite3_bind_int(stmt, 1, candidateId);
        sqlite3_bind_text(stmt, 2, status.c_str(), -1, SQLITE_TRANSIENT);
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

int ApplicationManager::countAllApplications(const std::string& status) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return 0;

    sqlite3_stmt* stmt = nullptr;
    if (status.empty()) {
        const char* sql = "SELECT COUNT(*) FROM applications;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;
    } else {
        const char* sql = "SELECT COUNT(*) FROM applications WHERE status = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;
        sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

void ApplicationManager::displayApplicationHistory(int candidateId) {
    auto apps = getCandidateApplications(candidateId);
    std::cout << "\n========================================\n";
    std::cout << "        APPLICATION HISTORY\n";
    std::cout << "========================================\n\n";

    if (apps.empty()) {
        std::cout << "  No applications found for this candidate.\n\n";
        std::cout << "========================================\n";
        return;
    }

    for (size_t i = 0; i < apps.size(); ++i) {
        const auto& a = apps[i];
        std::cout << "Application ID: " << a.id << "\n";
        std::cout << "Job: " << a.jobTitle << "\n";
        std::cout << "Company: " << a.company << "\n";
        std::cout << "Match Score: " << std::fixed << std::setprecision(2) << a.matchScore << "%\n";
        std::cout << "Readiness: " << std::fixed << std::setprecision(2) << a.readinessScore << "%\n";
        std::cout << "Status: " << a.status << "\n";
        std::cout << "Applied: " << a.appliedAt << "\n";

        if (i + 1 < apps.size()) {
            std::cout << "\n----------------------------------------\n\n";
        }
    }
    std::cout << "\n========================================\n";
}

void ApplicationManager::displayRecruiterApplications() {
    auto apps = getAllApplications();
    std::cout << "\n========================================\n";
    std::cout << "        APPLICATIONS\n";
    std::cout << "========================================\n\n";

    if (apps.empty()) {
        std::cout << "  No applications submitted yet.\n\n";
        std::cout << "========================================\n";
        return;
    }

    for (size_t i = 0; i < apps.size(); ++i) {
        const auto& a = apps[i];
        std::cout << "Application ID: " << a.id << "\n\n";
        std::cout << "Candidate:\n" << a.candidateName << "\n\n";
        std::cout << "Job:\n" << a.jobTitle << " (" << a.jobId << " - " << a.company << ")\n\n";
        std::cout << "Match Score:\n" << std::fixed << std::setprecision(2) << a.matchScore << "%\n\n";
        std::cout << "Readiness:\n" << std::fixed << std::setprecision(2) << a.readinessScore << "%\n\n";
        std::cout << "Status:\n" << a.status << "\n";

        if (i + 1 < apps.size()) {
            std::cout << "\n----------------------------------------\n\n";
        }
    }
    std::cout << "\n========================================\n";
}

void ApplicationManager::displayRecruiterDashboard() {
    int totalJobs = JobRepository::countJobs();
    int totalApps = countAllApplications();
    int applied = countAllApplications(STATUS_APPLIED);
    int underReview = countAllApplications(STATUS_UNDER_REVIEW);
    int shortlisted = countAllApplications(STATUS_SHORTLISTED);
    int interview = countAllApplications(STATUS_INTERVIEW);
    int selected = countAllApplications(STATUS_SELECTED);
    int rejected = countAllApplications(STATUS_REJECTED);

    std::cout << "\n========================================\n";
    std::cout << "        RECRUITER DASHBOARD\n";
    std::cout << "========================================\n\n";
    std::cout << "Total Jobs:\n" << totalJobs << "\n\n";
    std::cout << "Total Applications:\n" << totalApps << "\n\n";
    std::cout << "Applied:\n" << applied << "\n\n";
    std::cout << "Under Review:\n" << underReview << "\n\n";
    std::cout << "Shortlisted:\n" << shortlisted << "\n\n";
    std::cout << "Interview:\n" << interview << "\n\n";
    std::cout << "Selected:\n" << selected << "\n\n";
    std::cout << "Rejected:\n" << rejected << "\n\n";
    std::cout << "========================================\n";
}

void ApplicationManager::displayCandidateDashboard(
    int candidateId,
    const std::string& candidateName,
    double resumeQuality,
    int totalSkills,
    int projectsCount,
    int experienceCount,
    int certsCount,
    int savedJobsCount,
    const std::string& topJobTitle,
    double topMatchScore) {

    int totalApps = countCandidateApplications(candidateId);
    int underReview = countCandidateApplications(candidateId, STATUS_UNDER_REVIEW);
    int shortlisted = countCandidateApplications(candidateId, STATUS_SHORTLISTED);
    int interview = countCandidateApplications(candidateId, STATUS_INTERVIEW);
    int selected = countCandidateApplications(candidateId, STATUS_SELECTED);
    int rejected = countCandidateApplications(candidateId, STATUS_REJECTED);

    std::cout << "\n========================================\n";
    std::cout << "        CANDIDATE DASHBOARD\n";
    std::cout << "========================================\n\n";
    std::cout << "Candidate:\n" << candidateName << "\n\n";
    std::cout << "Resume Quality:\n" << std::fixed << std::setprecision(2) << resumeQuality << "%\n\n";
    std::cout << "Total Skills:\n" << totalSkills << "\n\n";
    std::cout << "Projects:\n" << projectsCount << "\n\n";
    std::cout << "Experience:\n" << experienceCount << "\n\n";
    std::cout << "Certifications:\n" << certsCount << "\n\n";
    std::cout << "----------------------------------------\n";
    std::cout << "APPLICATIONS\n";
    std::cout << "----------------------------------------\n\n";
    std::cout << "Total Applications: " << totalApps << "\n";
    std::cout << "Under Review: " << underReview << "\n";
    std::cout << "Shortlisted: " << shortlisted << "\n";
    std::cout << "Interview: " << interview << "\n";
    std::cout << "Selected: " << selected << "\n";
    std::cout << "Rejected: " << rejected << "\n\n";
    std::cout << "----------------------------------------\n";
    std::cout << "SAVED JOBS\n";
    std::cout << "----------------------------------------\n\n";
    std::cout << "Saved Jobs: " << savedJobsCount << "\n\n";
    std::cout << "----------------------------------------\n";
    std::cout << "TOP JOB MATCH\n";
    std::cout << "----------------------------------------\n\n";
    std::cout << topJobTitle << "\n";
    if (topMatchScore > 0.0) {
        std::cout << std::fixed << std::setprecision(2) << topMatchScore << "%\n";
    }
    std::cout << "\n========================================\n";
}
