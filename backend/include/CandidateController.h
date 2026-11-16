#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// CandidateController — Candidate profile queries and updates
// ============================================================
class CandidateController {
public:
    static void getMyProfile(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void updateMyProfile(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
