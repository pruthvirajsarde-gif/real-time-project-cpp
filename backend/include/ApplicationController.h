#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// ApplicationController — Candidate job applications, listings,
// details, and withdrawal with ownership validation.
// ============================================================
class ApplicationController {
public:
    static void applyForJob(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getMyApplications(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getApplicationById(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void withdrawApplication(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
