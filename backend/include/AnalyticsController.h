#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// AnalyticsController — REST endpoints for candidate career intelligence,
// skill gap prioritization, market insights, recruiter pipeline analytics,
// and multi-candidate comparison tables.
// ============================================================
class AnalyticsController {
public:
    // Candidate Analytics
    static void getCandidateAnalytics(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getCandidateSkillsAnalytics(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getCandidateMarketInsights(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getCandidateRecommendations(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getResumeJobComparison(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    // Recruiter Analytics
    static void getRecruiterAnalytics(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getRecruiterJobAnalytics(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getRecruiterJobCandidateComparison(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
