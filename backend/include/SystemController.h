#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// SystemController — Health check, API version metadata, and
// user-isolated activity audit logs.
// ============================================================
class SystemController {
public:
    static void getHealth(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getLiveness(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getReadiness(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getApiInfo(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getActivity(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
