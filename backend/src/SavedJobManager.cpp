#include "SavedJobManager.h"
#include "DatabaseManager.h"
#include "JobRepository.h"
#include "ResumeRepository.h"
#include "ResumeAnalyzer.h"

#include <iostream>
#include <iomanip>

bool SavedJobManager::saveJob(int candidateId, const std::string& jobId) {
    if (candidateId <= 0 || jobId.empty()) return false;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    // Check if job exists
    if (!JobRepository::jobExists(jobId)) {
        return false;
    }

    const char* sql = "INSERT OR IGNORE INTO saved_jobs (candidate_id, job_id) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    sqlite3_bind_text(stmt, 2, jobId.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

bool SavedJobManager::removeSavedJob(int candidateId, const std::string& jobId) {
    if (candidateId <= 0 || jobId.empty()) return false;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "DELETE FROM saved_jobs WHERE candidate_id = ? AND job_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    sqlite3_bind_text(stmt, 2, jobId.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE && sqlite3_changes(db) > 0);
}

bool SavedJobManager::isJobSaved(int candidateId, const std::string& jobId) {
    if (candidateId <= 0 || jobId.empty()) return false;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "SELECT 1 FROM saved_jobs WHERE candidate_id = ? AND job_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    sqlite3_bind_text(stmt, 2, jobId.c_str(), -1, SQLITE_TRANSIENT);

    bool saved = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return saved;
}

std::vector<Job> SavedJobManager::getSavedJobs(int candidateId) {
    std::vector<Job> list;
    if (candidateId <= 0) return list;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return list;

    const char* sql = "SELECT job_id FROM saved_jobs WHERE candidate_id = ? ORDER BY saved_at DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return list;
    }

    sqlite3_bind_int(stmt, 1, candidateId);

    std::vector<std::string> jobIds;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* jId = sqlite3_column_text(stmt, 0);
        if (jId) {
            jobIds.emplace_back(reinterpret_cast<const char*>(jId));
        }
    }
    sqlite3_finalize(stmt);

    for (const auto& id : jobIds) {
        Job j = JobRepository::getJob(id);
        if (!j.getJobId().empty()) {
            list.push_back(j);
        }
    }

    return list;
}

int SavedJobManager::countSavedJobs(int candidateId) {
    if (candidateId <= 0) return 0;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return 0;

    const char* sql = "SELECT COUNT(*) FROM saved_jobs WHERE candidate_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, candidateId);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

void SavedJobManager::displaySavedJobs(int candidateId) {
    auto saved = getSavedJobs(candidateId);
    std::cout << "\n========================================\n";
    std::cout << "            SAVED JOBS\n";
    std::cout << "========================================\n\n";

    if (saved.empty()) {
        std::cout << "  No saved jobs found.\n\n";
        std::cout << "========================================\n";
        return;
    }

    // Try loading candidate resume to calculate match score
    Resume res = ResumeRepository::loadResume(candidateId);
    bool hasResume = !res.getName().empty() || !res.getSkills().empty();

    for (size_t i = 0; i < saved.size(); ++i) {
        const auto& j = saved[i];
        std::cout << j.getJobId() << "\n";
        std::cout << j.getJobTitle() << "\n";
        std::cout << j.getCompany() << "\n";
        if (hasResume) {
            double score = ResumeAnalyzer::calculateOverallMatchScore(res, j);
            std::cout << "Match Score: " << std::fixed << std::setprecision(2) << score << "%\n";
        }

        if (i + 1 < saved.size()) {
            std::cout << "\n";
        }
    }
    std::cout << "\n========================================\n";
}
