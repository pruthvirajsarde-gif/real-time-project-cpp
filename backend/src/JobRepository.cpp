#include "JobRepository.h"
#include "DatabaseManager.h"
#include "JobDataLoader.h"
#include <iostream>
#include <algorithm>

int JobRepository::getOrCreateSkillId(const std::string& skillName, const std::string& category) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db || skillName.empty()) return -1;

    // Check if skill exists (case-insensitive)
    const char* selectSql = "SELECT id FROM skills WHERE LOWER(name) = LOWER(?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }
    sqlite3_bind_text(stmt, 1, skillName.c_str(), -1, SQLITE_TRANSIENT);

    int skillId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        skillId = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (skillId > 0) {
        return skillId;
    }

    // Insert new skill
    const char* insertSql = "INSERT INTO skills (name, category) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }
    sqlite3_bind_text(stmt, 1, skillName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, category.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        skillId = static_cast<int>(sqlite3_last_insert_rowid(db));
    }
    sqlite3_finalize(stmt);
    return skillId;
}

bool JobRepository::saveJob(const Job& job) {
    if (job.getJobId().empty() || job.getJobTitle().empty() || job.getCompany().empty()) {
        return false;
    }

    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    if (!DatabaseManager::beginTransaction()) {
        return false;
    }

    bool exists = jobExists(job.getJobId());
    sqlite3_stmt* stmt = nullptr;

    if (exists) {
        const char* updateSql = "UPDATE jobs SET title=?, company=?, location=?, employment_type=?, "
                                "education_requirement=?, minimum_experience=?, description=? WHERE job_id=?;";
        if (sqlite3_prepare_v2(db, updateSql, -1, &stmt, nullptr) != SQLITE_OK) {
            DatabaseManager::rollbackTransaction();
            return false;
        }
        sqlite3_bind_text(stmt, 1, job.getJobTitle().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, job.getCompany().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, job.getLocation().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, job.getEmploymentType().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, job.getEducationRequirement().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 6, job.getMinimumExperience());
        sqlite3_bind_text(stmt, 7, job.getDescription().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, job.getJobId().c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            DatabaseManager::rollbackTransaction();
            return false;
        }
        sqlite3_finalize(stmt);

        // Clear existing skill relations for refresh
        const char* delReq = "DELETE FROM job_required_skills WHERE job_id=?;";
        if (sqlite3_prepare_v2(db, delReq, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, job.getJobId().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        const char* delPref = "DELETE FROM job_preferred_skills WHERE job_id=?;";
        if (sqlite3_prepare_v2(db, delPref, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, job.getJobId().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    } else {
        const char* insertSql = "INSERT INTO jobs (job_id, title, company, location, employment_type, "
                                "education_requirement, minimum_experience, description) "
                                "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr) != SQLITE_OK) {
            DatabaseManager::rollbackTransaction();
            return false;
        }
        sqlite3_bind_text(stmt, 1, job.getJobId().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, job.getJobTitle().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, job.getCompany().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, job.getLocation().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, job.getEmploymentType().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, job.getEducationRequirement().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 7, job.getMinimumExperience());
        sqlite3_bind_text(stmt, 8, job.getDescription().c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            DatabaseManager::rollbackTransaction();
            return false;
        }
        sqlite3_finalize(stmt);
    }

    // Insert required skills
    const char* insReq = "INSERT OR IGNORE INTO job_required_skills (job_id, skill_id) VALUES (?, ?);";
    for (const auto& skill : job.getRequiredSkills()) {
        int sid = getOrCreateSkillId(skill, "Requirement");
        if (sid > 0) {
            if (sqlite3_prepare_v2(db, insReq, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, job.getJobId().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 2, sid);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
    }

    // Insert preferred skills
    const char* insPref = "INSERT OR IGNORE INTO job_preferred_skills (job_id, skill_id) VALUES (?, ?);";
    for (const auto& skill : job.getPreferredSkills()) {
        int sid = getOrCreateSkillId(skill, "Preference");
        if (sid > 0) {
            if (sqlite3_prepare_v2(db, insPref, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, job.getJobId().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 2, sid);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
    }

    return DatabaseManager::commitTransaction();
}

std::vector<Job> JobRepository::getAllJobs() {
    std::vector<Job> list;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return list;

    const char* sql = "SELECT job_id, title, company, location, employment_type, "
                      "education_requirement, minimum_experience, description FROM jobs ORDER BY id ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* jId = sqlite3_column_text(stmt, 0);
        const unsigned char* title = sqlite3_column_text(stmt, 1);
        const unsigned char* company = sqlite3_column_text(stmt, 2);
        const unsigned char* location = sqlite3_column_text(stmt, 3);
        const unsigned char* empType = sqlite3_column_text(stmt, 4);
        const unsigned char* edu = sqlite3_column_text(stmt, 5);
        int exp = sqlite3_column_int(stmt, 6);
        const unsigned char* desc = sqlite3_column_text(stmt, 7);

        Job job(
            jId ? reinterpret_cast<const char*>(jId) : "",
            title ? reinterpret_cast<const char*>(title) : "",
            company ? reinterpret_cast<const char*>(company) : "",
            location ? reinterpret_cast<const char*>(location) : "",
            empType ? reinterpret_cast<const char*>(empType) : "",
            edu ? reinterpret_cast<const char*>(edu) : "",
            exp,
            desc ? reinterpret_cast<const char*>(desc) : ""
        );

        // Fetch required skills
        const char* reqSql = "SELECT s.name FROM job_required_skills jrs JOIN skills s ON jrs.skill_id = s.id "
                             "WHERE jrs.job_id = ? ORDER BY s.id ASC;";
        sqlite3_stmt* reqStmt = nullptr;
        if (sqlite3_prepare_v2(db, reqSql, -1, &reqStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(reqStmt, 1, job.getJobId().c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(reqStmt) == SQLITE_ROW) {
                const unsigned char* sName = sqlite3_column_text(reqStmt, 0);
                if (sName) {
                    job.addRequiredSkill(reinterpret_cast<const char*>(sName));
                }
            }
            sqlite3_finalize(reqStmt);
        }

        // Fetch preferred skills
        const char* prefSql = "SELECT s.name FROM job_preferred_skills jps JOIN skills s ON jps.skill_id = s.id "
                              "WHERE jps.job_id = ? ORDER BY s.id ASC;";
        sqlite3_stmt* prefStmt = nullptr;
        if (sqlite3_prepare_v2(db, prefSql, -1, &prefStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(prefStmt, 1, job.getJobId().c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(prefStmt) == SQLITE_ROW) {
                const unsigned char* sName = sqlite3_column_text(prefStmt, 0);
                if (sName) {
                    job.addPreferredSkill(reinterpret_cast<const char*>(sName));
                }
            }
            sqlite3_finalize(prefStmt);
        }

        list.push_back(job);
    }

    sqlite3_finalize(stmt);
    return list;
}

Job JobRepository::getJob(const std::string& jobId) {
    Job job;
    sqlite3* db = DatabaseManager::getDb();
    if (!db || jobId.empty()) return job;

    const char* sql = "SELECT job_id, title, company, location, employment_type, "
                      "education_requirement, minimum_experience, description FROM jobs WHERE job_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return job;
    }

    sqlite3_bind_text(stmt, 1, jobId.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* jId = sqlite3_column_text(stmt, 0);
        const unsigned char* title = sqlite3_column_text(stmt, 1);
        const unsigned char* company = sqlite3_column_text(stmt, 2);
        const unsigned char* location = sqlite3_column_text(stmt, 3);
        const unsigned char* empType = sqlite3_column_text(stmt, 4);
        const unsigned char* edu = sqlite3_column_text(stmt, 5);
        int exp = sqlite3_column_int(stmt, 6);
        const unsigned char* desc = sqlite3_column_text(stmt, 7);

        job = Job(
            jId ? reinterpret_cast<const char*>(jId) : "",
            title ? reinterpret_cast<const char*>(title) : "",
            company ? reinterpret_cast<const char*>(company) : "",
            location ? reinterpret_cast<const char*>(location) : "",
            empType ? reinterpret_cast<const char*>(empType) : "",
            edu ? reinterpret_cast<const char*>(edu) : "",
            exp,
            desc ? reinterpret_cast<const char*>(desc) : ""
        );

        // Fetch required skills
        const char* reqSql = "SELECT s.name FROM job_required_skills jrs JOIN skills s ON jrs.skill_id = s.id "
                             "WHERE jrs.job_id = ? ORDER BY s.id ASC;";
        sqlite3_stmt* reqStmt = nullptr;
        if (sqlite3_prepare_v2(db, reqSql, -1, &reqStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(reqStmt, 1, jobId.c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(reqStmt) == SQLITE_ROW) {
                const unsigned char* sName = sqlite3_column_text(reqStmt, 0);
                if (sName) {
                    job.addRequiredSkill(reinterpret_cast<const char*>(sName));
                }
            }
            sqlite3_finalize(reqStmt);
        }

        // Fetch preferred skills
        const char* prefSql = "SELECT s.name FROM job_preferred_skills jps JOIN skills s ON jps.skill_id = s.id "
                              "WHERE jps.job_id = ? ORDER BY s.id ASC;";
        sqlite3_stmt* prefStmt = nullptr;
        if (sqlite3_prepare_v2(db, prefSql, -1, &prefStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(prefStmt, 1, jobId.c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(prefStmt) == SQLITE_ROW) {
                const unsigned char* sName = sqlite3_column_text(prefStmt, 0);
                if (sName) {
                    job.addPreferredSkill(reinterpret_cast<const char*>(sName));
                }
            }
            sqlite3_finalize(prefStmt);
        }
    }

    sqlite3_finalize(stmt);
    return job;
}

bool JobRepository::jobExists(const std::string& jobId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db || jobId.empty()) return false;

    const char* sql = "SELECT 1 FROM jobs WHERE job_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, jobId.c_str(), -1, SQLITE_TRANSIENT);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

bool JobRepository::deleteJob(const std::string& jobId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db || jobId.empty()) return false;

    const char* sql = "DELETE FROM jobs WHERE job_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, jobId.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

int JobRepository::countJobs() {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return 0;

    const char* sql = "SELECT COUNT(*) FROM jobs;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

int JobRepository::syncJobsFromFile(const std::string& filePath) {
    std::vector<Job> fileJobs = JobDataLoader::loadJobsFromFile(filePath);
    int insertedCount = 0;

    for (const auto& job : fileJobs) {
        if (!jobExists(job.getJobId())) {
            if (saveJob(job)) {
                insertedCount++;
            }
        }
    }
    return insertedCount;
}
