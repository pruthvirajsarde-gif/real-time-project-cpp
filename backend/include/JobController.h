#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// JobController — Public and candidate job browsing, filtering,
// search query parameters, and paginated listing.
// ============================================================
class JobController {
public:
    static void getJobs(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getJobById(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
