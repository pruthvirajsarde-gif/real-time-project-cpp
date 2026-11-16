#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// AuthController — REST handlers for registration, authentication,
// session termination, credential changes, and profile queries.
// ============================================================
class AuthController {
public:
    static void registerCandidate(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void registerRecruiter(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void login(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void logout(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void changePassword(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );

    static void getMe(
        const httplib::Request& req,
        httplib::Response& res,
        const std::string& corsOrigin = "*"
    );
};
