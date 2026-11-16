#pragma once

#include <httplib.h>
#include <string>
#include <memory>
#include <thread>
#include <atomic>

// ============================================================
// HttpServerConfig — Configuration parameters for the HTTP server
// ============================================================
struct HttpServerConfig {
    std::string host = "127.0.0.1";
    int port = 8080;
    int sessionTimeoutMinutes = 30;
    int maxUploadMb = 10;
    std::string corsOrigin = "http://localhost:5173";
    std::string databasePath = "data/smarthire.db";
    std::string uploadDirectory = "data/uploads/";
    int rateLimitMaxRequests = 100;
    int rateLimitWindowSeconds = 60;
};

// ============================================================
// HttpServer — Embedded C++17 HTTP/REST Server for SmartHire.
// Handles lifecycle, configuration, route registration, and
// asynchronous execution support for automated testing.
// ============================================================
class HttpServer {
public:
    HttpServer();
    explicit HttpServer(const HttpServerConfig& config);
    ~HttpServer();

    // Load server settings from a key=value configuration file
    bool loadConfig(const std::string& configFilePath = "config/server.conf");

    // Starts the server and blocks until stopped (for SmartHire.exe --server)
    bool start();

    // Starts the server in a background thread (for automated integration tests)
    bool startAsync();

    // Stops the running server
    void stop();

    // Checks whether server is currently running
    bool isRunning() const;

    // Configuration getters & setters
    const HttpServerConfig& getConfig() const;
    void setHost(const std::string& host);
    void setPort(int port);
    void setCorsOrigin(const std::string& origin);
    void setDatabasePath(const std::string& path);
    void setUploadDirectory(const std::string& dir);

    // Direct access to underlying httplib server instance
    httplib::Server& getServer();

private:
    void setupServer();

    HttpServerConfig config_;
    std::unique_ptr<httplib::Server> server_;
    std::unique_ptr<std::thread> workerThread_;
    std::atomic<bool> isRunning_{false};
};
