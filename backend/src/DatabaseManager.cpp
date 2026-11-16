#include "DatabaseManager.h"

#include <iostream>
#include <filesystem>

sqlite3* DatabaseManager::db = nullptr;
std::string DatabaseManager::currentDbPath = "";
std::string DatabaseManager::lastErrorMsg = "";

bool DatabaseManager::initialize(const std::string& databasePath, bool verbose) {
    // If already connected to the same database, do not re-open
    if (db != nullptr && currentDbPath == databasePath) {
        return true;
    }

    if (db != nullptr) {
        close();
    }

    bool dbExisted = std::filesystem::exists(databasePath);

    // Ensure parent directory exists
    try {
        std::filesystem::path p(databasePath);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
    } catch (const std::exception& e) {
        lastErrorMsg = std::string("Filesystem error creating database directory: ") + e.what();
        return false;
    }

    if (verbose) {
        std::cout << "========================================\n";
        std::cout << "        DATABASE INITIALIZATION\n";
        std::cout << "========================================\n\n";
        std::cout << "Database:\n" << databasePath << "\n\n";
        std::cout << "Connecting...\n";
    }

    int rc = sqlite3_open(databasePath.c_str(), &db);
    if (rc != SQLITE_OK) {
        lastErrorMsg = sqlite3_errmsg(db);
        if (verbose) {
            std::cout << "FAILED: " << lastErrorMsg << "\n";
        }
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
        return false;
    }

    currentDbPath = databasePath;
    if (verbose) {
        std::cout << "SUCCESS\n\n";
        std::cout << "Checking tables...\n";
    }

    // Enable foreign keys
    execute("PRAGMA foreign_keys = ON;");

    if (!dbExisted && verbose) {
        std::cout << "Creating database...\n";
        std::cout << "Creating tables...\n";
    }

    if (!createTables()) {
        if (verbose) {
            std::cout << "FAILED: Could not create tables.\n";
        }
        return false;
    }

    if (verbose) {
        std::cout << "SUCCESS\n\n";
        if (!dbExisted) {
            std::cout << "Database initialized successfully.\n";
        }
        std::cout << "SmartHire database ready.\n";
        std::cout << "========================================\n";
    }

    return true;
}

bool DatabaseManager::createTables() {
    if (!isConnected()) {
        lastErrorMsg = "Database is not connected.";
        return false;
    }

    const char* schema = 
        "PRAGMA foreign_keys = ON;\n"
        "CREATE TABLE IF NOT EXISTS candidates (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    name TEXT NOT NULL,\n"
        "    email TEXT UNIQUE NOT NULL,\n"
        "    phone TEXT,\n"
        "    location TEXT,\n"
        "    linkedin TEXT,\n"
        "    github TEXT,\n"
        "    portfolio TEXT,\n"
        "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,\n"
        "    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS resumes (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    candidate_id INTEGER NOT NULL,\n"
        "    source_path TEXT,\n"
        "    file_format TEXT,\n"
        "    resume_quality_score REAL DEFAULT 0.0,\n"
        "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,\n"
        "    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,\n"
        "    FOREIGN KEY (candidate_id) REFERENCES candidates(id) ON DELETE CASCADE\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS skills (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    name TEXT UNIQUE NOT NULL,\n"
        "    category TEXT\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS candidate_skills (\n"
        "    candidate_id INTEGER NOT NULL,\n"
        "    skill_id INTEGER NOT NULL,\n"
        "    PRIMARY KEY (candidate_id, skill_id),\n"
        "    FOREIGN KEY (candidate_id) REFERENCES candidates(id) ON DELETE CASCADE,\n"
        "    FOREIGN KEY (skill_id) REFERENCES skills(id) ON DELETE CASCADE\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS jobs (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    job_id TEXT UNIQUE NOT NULL,\n"
        "    title TEXT NOT NULL,\n"
        "    company TEXT NOT NULL,\n"
        "    location TEXT,\n"
        "    employment_type TEXT,\n"
        "    education_requirement TEXT,\n"
        "    minimum_experience INTEGER DEFAULT 0,\n"
        "    description TEXT,\n"
        "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS job_required_skills (\n"
        "    job_id TEXT NOT NULL,\n"
        "    skill_id INTEGER NOT NULL,\n"
        "    PRIMARY KEY (job_id, skill_id),\n"
        "    FOREIGN KEY (job_id) REFERENCES jobs(job_id) ON DELETE CASCADE,\n"
        "    FOREIGN KEY (skill_id) REFERENCES skills(id) ON DELETE CASCADE\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS job_preferred_skills (\n"
        "    job_id TEXT NOT NULL,\n"
        "    skill_id INTEGER NOT NULL,\n"
        "    PRIMARY KEY (job_id, skill_id),\n"
        "    FOREIGN KEY (job_id) REFERENCES jobs(job_id) ON DELETE CASCADE,\n"
        "    FOREIGN KEY (skill_id) REFERENCES skills(id) ON DELETE CASCADE\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS applications (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    candidate_id INTEGER NOT NULL,\n"
        "    job_id TEXT NOT NULL,\n"
        "    match_score REAL DEFAULT 0.0,\n"
        "    readiness_score REAL DEFAULT 0.0,\n"
        "    status TEXT NOT NULL CHECK(status IN ('APPLIED', 'UNDER_REVIEW', 'SHORTLISTED', 'INTERVIEW', 'REJECTED', 'SELECTED', 'WITHDRAWN')),\n"
        "    applied_at DATETIME DEFAULT CURRENT_TIMESTAMP,\n"
        "    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,\n"
        "    FOREIGN KEY (candidate_id) REFERENCES candidates(id) ON DELETE CASCADE,\n"
        "    FOREIGN KEY (job_id) REFERENCES jobs(job_id) ON DELETE CASCADE\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS saved_jobs (\n"
        "    candidate_id INTEGER NOT NULL,\n"
        "    job_id TEXT NOT NULL,\n"
        "    saved_at DATETIME DEFAULT CURRENT_TIMESTAMP,\n"
        "    PRIMARY KEY (candidate_id, job_id),\n"
        "    FOREIGN KEY (candidate_id) REFERENCES candidates(id) ON DELETE CASCADE,\n"
        "    FOREIGN KEY (job_id) REFERENCES jobs(job_id) ON DELETE CASCADE\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS candidate_education (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    candidate_id INTEGER NOT NULL,\n"
        "    degree TEXT NOT NULL,\n"
        "    institution TEXT NOT NULL,\n"
        "    field_of_study TEXT,\n"
        "    start_year INTEGER DEFAULT 0,\n"
        "    end_year INTEGER DEFAULT 0,\n"
        "    score REAL DEFAULT 0.0,\n"
        "    FOREIGN KEY (candidate_id) REFERENCES candidates(id) ON DELETE CASCADE\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS candidate_experience (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    candidate_id INTEGER NOT NULL,\n"
        "    company TEXT NOT NULL,\n"
        "    position TEXT NOT NULL,\n"
        "    duration TEXT,\n"
        "    responsibilities TEXT,\n"
        "    technologies TEXT,\n"
        "    FOREIGN KEY (candidate_id) REFERENCES candidates(id) ON DELETE CASCADE\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS candidate_projects (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    candidate_id INTEGER NOT NULL,\n"
        "    name TEXT NOT NULL,\n"
        "    description TEXT,\n"
        "    role TEXT,\n"
        "    duration TEXT,\n"
        "    technologies TEXT,\n"
        "    FOREIGN KEY (candidate_id) REFERENCES candidates(id) ON DELETE CASCADE\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS candidate_certifications (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    candidate_id INTEGER NOT NULL,\n"
        "    name TEXT NOT NULL,\n"
        "    issuer TEXT NOT NULL,\n"
        "    year INTEGER DEFAULT 0,\n"
        "    FOREIGN KEY (candidate_id) REFERENCES candidates(id) ON DELETE CASCADE\n"
        ");\n"
        "CREATE INDEX IF NOT EXISTS idx_applications_candidate ON applications(candidate_id);\n"
        "CREATE INDEX IF NOT EXISTS idx_applications_job ON applications(job_id);\n"
        "CREATE INDEX IF NOT EXISTS idx_saved_jobs_candidate ON saved_jobs(candidate_id);\n"
        "CREATE INDEX IF NOT EXISTS idx_candidates_email ON candidates(email);\n"
        "CREATE INDEX IF NOT EXISTS idx_jobs_job_id ON jobs(job_id);\n"
        "CREATE TABLE IF NOT EXISTS accounts (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    username TEXT UNIQUE NOT NULL,\n"
        "    email TEXT UNIQUE NOT NULL,\n"
        "    password_hash TEXT NOT NULL,\n"
        "    password_salt TEXT NOT NULL,\n"
        "    role TEXT NOT NULL CHECK(role IN ('CANDIDATE', 'RECRUITER')),\n"
        "    status TEXT NOT NULL DEFAULT 'ACTIVE' CHECK(status IN ('ACTIVE', 'LOCKED', 'DISABLED')),\n"
        "    failed_login_attempts INTEGER DEFAULT 0,\n"
        "    last_login_at DATETIME,\n"
        "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,\n"
        "    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS sessions (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    account_id INTEGER NOT NULL,\n"
        "    session_token_hash TEXT NOT NULL,\n"
        "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,\n"
        "    expires_at DATETIME NOT NULL,\n"
        "    revoked INTEGER DEFAULT 0,\n"
        "    FOREIGN KEY (account_id) REFERENCES accounts(id) ON DELETE CASCADE\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS audit_logs (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    account_id INTEGER,\n"
        "    action TEXT NOT NULL,\n"
        "    details TEXT,\n"
        "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP\n"
        ");\n"
        "CREATE TABLE IF NOT EXISTS password_reset_tokens (\n"
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
        "    username TEXT NOT NULL,\n"
        "    code_hash TEXT NOT NULL,\n"
        "    expires_at DATETIME NOT NULL,\n"
        "    used INTEGER DEFAULT 0,\n"
        "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP\n"
        ");\n"
        "CREATE INDEX IF NOT EXISTS idx_accounts_username ON accounts(username);\n"
        "CREATE INDEX IF NOT EXISTS idx_accounts_email ON accounts(email);\n"
        "CREATE INDEX IF NOT EXISTS idx_sessions_token_hash ON sessions(session_token_hash);\n"
        "CREATE INDEX IF NOT EXISTS idx_sessions_account ON sessions(account_id);\n"
        "CREATE INDEX IF NOT EXISTS idx_audit_logs_account ON audit_logs(account_id);\n"
        "CREATE INDEX IF NOT EXISTS idx_password_reset_username ON password_reset_tokens(username);\n";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, schema, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) {
            lastErrorMsg = errMsg;
            sqlite3_free(errMsg);
        } else {
            lastErrorMsg = sqlite3_errmsg(db);
        }
        return false;
    }

    // Safe migration: Add account_id to candidates if missing
    bool hasCandAccountId = false;
    sqlite3_stmt* candStmt = nullptr;
    if (sqlite3_prepare_v2(db, "PRAGMA table_info(candidates);", -1, &candStmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(candStmt) == SQLITE_ROW) {
            const unsigned char* colName = sqlite3_column_text(candStmt, 1);
            if (colName && std::string(reinterpret_cast<const char*>(colName)) == "account_id") {
                hasCandAccountId = true;
                break;
            }
        }
        sqlite3_finalize(candStmt);
    }
    if (!hasCandAccountId) {
        execute("ALTER TABLE candidates ADD COLUMN account_id INTEGER;");
    }

    // Safe migration: Add recruiter_id to jobs if missing
    bool hasJobRecruiterId = false;
    sqlite3_stmt* jobStmt = nullptr;
    if (sqlite3_prepare_v2(db, "PRAGMA table_info(jobs);", -1, &jobStmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(jobStmt) == SQLITE_ROW) {
            const unsigned char* colName = sqlite3_column_text(jobStmt, 1);
            if (colName && std::string(reinterpret_cast<const char*>(colName)) == "recruiter_id") {
                hasJobRecruiterId = true;
                break;
            }
        }
        sqlite3_finalize(jobStmt);
    }
    if (!hasJobRecruiterId) {
        execute("ALTER TABLE jobs ADD COLUMN recruiter_id INTEGER DEFAULT 0;");
    }

    return true;
}

void DatabaseManager::close() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
        currentDbPath.clear();
    }
}

bool DatabaseManager::isConnected() {
    return (db != nullptr);
}

bool DatabaseManager::execute(const std::string& sql) {
    if (!isConnected()) {
        lastErrorMsg = "Database is not connected.";
        return false;
    }
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) {
            lastErrorMsg = errMsg;
            sqlite3_free(errMsg);
        } else {
            lastErrorMsg = sqlite3_errmsg(db);
        }
        return false;
    }
    return true;
}

bool DatabaseManager::beginTransaction() {
    return execute("BEGIN TRANSACTION;");
}

bool DatabaseManager::commitTransaction() {
    return execute("COMMIT;");
}

bool DatabaseManager::rollbackTransaction() {
    return execute("ROLLBACK;");
}

sqlite3* DatabaseManager::getDb() {
    return db;
}

std::string DatabaseManager::getLastError() {
    return lastErrorMsg;
}

const std::string& DatabaseManager::getCurrentDatabasePath() {
    return currentDbPath;
}
