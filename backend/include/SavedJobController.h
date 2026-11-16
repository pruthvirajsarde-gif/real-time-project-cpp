#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// SavedJobController — Candidate bookmarked / saved jobs API
// ============================================================
class SavedJobController {
public:
    static void saveJob(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getSavedJobs(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void removeSavedJob(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
