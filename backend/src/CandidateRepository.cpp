#include "CandidateRepository.h"
#include "DatabaseManager.h"
#include <iostream>

int CandidateRepository::createCandidate(const Profile& profile) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return -1;

    const char* sql = "INSERT INTO candidates (name, email, phone, location, linkedin, github, portfolio) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, profile.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, profile.getEmail().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, profile.getPhone().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, profile.getLocation().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, profile.getLinkedin().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, profile.getGithub().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, profile.getPortfolio().c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        return static_cast<int>(sqlite3_last_insert_rowid(db));
    }
    return -1;
}

Profile CandidateRepository::getCandidate(int candidateId) {
    Profile p;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return p;

    const char* sql = "SELECT name, email, phone, location, linkedin, github, portfolio FROM candidates WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return p;
    }

    sqlite3_bind_int(stmt, 1, candidateId);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* name = sqlite3_column_text(stmt, 0);
        const unsigned char* email = sqlite3_column_text(stmt, 1);
        const unsigned char* phone = sqlite3_column_text(stmt, 2);
        const unsigned char* location = sqlite3_column_text(stmt, 3);
        const unsigned char* linkedin = sqlite3_column_text(stmt, 4);
        const unsigned char* github = sqlite3_column_text(stmt, 5);
        const unsigned char* portfolio = sqlite3_column_text(stmt, 6);

        p.setName(name ? reinterpret_cast<const char*>(name) : "");
        p.setEmail(email ? reinterpret_cast<const char*>(email) : "");
        p.setPhone(phone ? reinterpret_cast<const char*>(phone) : "");
        p.setLocation(location ? reinterpret_cast<const char*>(location) : "");
        p.setLinkedin(linkedin ? reinterpret_cast<const char*>(linkedin) : "");
        p.setGithub(github ? reinterpret_cast<const char*>(github) : "");
        p.setPortfolio(portfolio ? reinterpret_cast<const char*>(portfolio) : "");
    }

    sqlite3_finalize(stmt);
    return p;
}

bool CandidateRepository::updateCandidate(int candidateId, const Profile& profile) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "UPDATE candidates SET name=?, email=?, phone=?, location=?, linkedin=?, github=?, portfolio=?, "
                      "updated_at=CURRENT_TIMESTAMP WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, profile.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, profile.getEmail().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, profile.getPhone().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, profile.getLocation().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, profile.getLinkedin().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, profile.getGithub().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, profile.getPortfolio().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, candidateId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool CandidateRepository::deleteCandidate(int candidateId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "DELETE FROM candidates WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

int CandidateRepository::findCandidateByEmail(const std::string& email) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return -1;

    const char* sql = "SELECT id FROM candidates WHERE email = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

    int candidateId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        candidateId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return candidateId;
}

int CandidateRepository::saveOrUpdateCandidate(const Profile& profile, bool& isNew) {
    int existingId = findCandidateByEmail(profile.getEmail());
    if (existingId > 0) {
        updateCandidate(existingId, profile);
        isNew = false;
        return existingId;
    } else {
        int newId = createCandidate(profile);
        isNew = true;
        return newId;
    }
}

std::vector<std::pair<int, Profile>> CandidateRepository::getAllCandidates() {
    std::vector<std::pair<int, Profile>> list;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return list;

    const char* sql = "SELECT id, name, email, phone, location, linkedin, github, portfolio FROM candidates ORDER BY id ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* email = sqlite3_column_text(stmt, 2);
        const unsigned char* phone = sqlite3_column_text(stmt, 3);
        const unsigned char* location = sqlite3_column_text(stmt, 4);
        const unsigned char* linkedin = sqlite3_column_text(stmt, 5);
        const unsigned char* github = sqlite3_column_text(stmt, 6);
        const unsigned char* portfolio = sqlite3_column_text(stmt, 7);

        Profile p(
            name ? reinterpret_cast<const char*>(name) : "",
            email ? reinterpret_cast<const char*>(email) : "",
            phone ? reinterpret_cast<const char*>(phone) : "",
            location ? reinterpret_cast<const char*>(location) : "",
            linkedin ? reinterpret_cast<const char*>(linkedin) : "",
            github ? reinterpret_cast<const char*>(github) : "",
            portfolio ? reinterpret_cast<const char*>(portfolio) : ""
        );
        list.emplace_back(id, p);
    }

    sqlite3_finalize(stmt);
    return list;
}

bool CandidateRepository::candidateExists(int candidateId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    const char* sql = "SELECT id FROM candidates WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}
