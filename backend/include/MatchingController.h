#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// MatchingController — Endpoints for advanced semantic job
// matching, ranking, and explainable compatibility breakdown.
// ============================================================
class MatchingController {
public:
    static void getMatches(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getMatchForJob(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getMatchExplanation(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
