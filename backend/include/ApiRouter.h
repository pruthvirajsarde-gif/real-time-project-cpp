#pragma once

#include <httplib.h>
#include <string>

// ============================================================
// ApiRouter — Registers all REST API v1 routes, route parameters,
// preflight CORS handlers, and error handlers on the HTTP server.
// ============================================================
class ApiRouter {
public:
    static void registerRoutes(httplib::Server& svr, const std::string& corsOrigin = "*");
};
