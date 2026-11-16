#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// RecruiterController — Recruiter job postings, updates, deletion,
// application reviews, and candidate status transitions.
// Enforces strict recruiter resource ownership.
// ============================================================
class RecruiterController {
public:
    static void createJob(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getMyJobs(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void updateJob(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void deleteJob(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getApplications(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getApplicationById(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void updateApplicationStatus(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
