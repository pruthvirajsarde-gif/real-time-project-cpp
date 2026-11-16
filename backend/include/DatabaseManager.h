#pragma once

#include <string>
#include <vector>
#include "sqlite3.h"

// ============================================================
// DatabaseManager — Manages SQLite connection lifecycle,
// table creation, transactions, and execution.
// ============================================================
class DatabaseManager {
private:
    static sqlite3* db;
    static std::string currentDbPath;
    static std::string lastErrorMsg;

public:
    // Initialize SQLite database at the specified path
    static bool initialize(const std::string& databasePath, bool verbose = true);

    // Create all required schema tables and indexes
    static bool createTables();

    // Close the current database connection
    static void close();

    // Check if the database connection is currently open
    static bool isConnected();

    // Execute raw SQL command (e.g. PRAGMA, schema creation)
    static bool execute(const std::string& sql);

    // Transaction management
    static bool beginTransaction();
    static bool commitTransaction();
    static bool rollbackTransaction();

    // Direct SQLite connection pointer for prepared statements in repositories
    static sqlite3* getDb();

    // Retrieve the last recorded error message
    static std::string getLastError();

    // Retrieve the path of the currently open database
    static const std::string& getCurrentDatabasePath();
};
