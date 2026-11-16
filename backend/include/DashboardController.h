#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// DashboardController — Candidate and Recruiter operational
// dashboard metrics and aggregate overview statistics.
// ============================================================
class DashboardController {
public:
    static void getCandidateDashboard(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getRecruiterDashboard(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
