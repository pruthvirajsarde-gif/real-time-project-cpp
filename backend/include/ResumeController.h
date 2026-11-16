#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// ResumeController — Endpoints for resume upload (.txt/.pdf),
// manual file import, resume retrieval, quality score breakdown,
// strength analysis, and career recommendations.
// ============================================================
class ResumeController {
public:
    static void uploadResume(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void importResume(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getResume(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getAnalysis(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getStrengths(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getRecommendations(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
