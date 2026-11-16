#include "HttpServer.h"
#include "ApiRouter.h"
#include "DatabaseManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>

namespace {
std::string trim(const std::string& str) {
    auto first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    auto last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}
} // anonymous namespace

HttpServer::HttpServer() {
    setupServer();
}

HttpServer::HttpServer(const HttpServerConfig& config) : config_(config) {
    setupServer();
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::setupServer() {
    server_ = std::make_unique<httplib::Server>();

    // Set maximum request body payload length
    size_t maxBytes = static_cast<size_t>(config_.maxUploadMb) * 1024 * 1024;
    server_->set_payload_max_length(maxBytes);

    // Register all Phase 9 API routes
    ApiRouter::registerRoutes(*server_, config_.corsOrigin);

    // Serve static frontend files from frontend/dist/ if available
    server_->set_mount_point("/", "frontend/dist");
}

bool HttpServer::loadConfig(const std::string& configFilePath) {
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = trim(line.substr(0, pos));
        std::string val = trim(line.substr(pos + 1));

        if (key == "host") {
            config_.host = val;
        } else if (key == "port") {
            try { config_.port = std::stoi(val); } catch (...) {}
        } else if (key == "session_timeout_minutes") {
            try { config_.sessionTimeoutMinutes = std::stoi(val); } catch (...) {}
        } else if (key == "max_upload_mb") {
            try { config_.maxUploadMb = std::stoi(val); } catch (...) {}
        } else if (key == "cors_origin") {
            config_.corsOrigin = val;
        } else if (key == "database_path") {
            config_.databasePath = val;
        } else if (key == "upload_directory") {
            config_.uploadDirectory = val;
        } else if (key == "rate_limit_max_requests") {
            try { config_.rateLimitMaxRequests = std::stoi(val); } catch (...) {}
        } else if (key == "rate_limit_window_seconds") {
            try { config_.rateLimitWindowSeconds = std::stoi(val); } catch (...) {}
        }
    }

    // Check environment variable overrides if present
    if (const char* envHost = std::getenv("SMARTHIRE_HOST")) {
        config_.host = envHost;
    }
    if (const char* envPort = std::getenv("SMARTHIRE_PORT")) {
        try { config_.port = std::stoi(envPort); } catch (...) {}
    } else if (const char* envPort2 = std::getenv("PORT")) {
        try { config_.port = std::stoi(envPort2); } catch (...) {}
    }
    if (const char* envCors = std::getenv("SMARTHIRE_CORS_ORIGIN")) {
        config_.corsOrigin = envCors;
    }
    if (const char* envDb = std::getenv("SMARTHIRE_DB_PATH")) {
        config_.databasePath = envDb;
    }
    if (const char* envUpload = std::getenv("SMARTHIRE_UPLOAD_DIR")) {
        config_.uploadDirectory = envUpload;
    }

    // Re-apply configuration
    setupServer();
    return true;
}

bool HttpServer::start() {
    if (isRunning_) return true;

    std::cout << "\n========================================" << std::endl;
    std::cout << "          SMARTHIRE API SERVER" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\nVersion:\n9.0\n" << std::endl;
    std::cout << "Address:\nhttp://" << config_.host << ":" << config_.port << "\n" << std::endl;

    bool dbConnected = DatabaseManager::isConnected();
    std::cout << "Database:\n" << (dbConnected ? "CONNECTED" : "DISCONNECTED") << "\n" << std::endl;
    std::cout << "API:\nREADY\n" << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;
    std::cout << "========================================\n" << std::endl;

    isRunning_ = true;
    bool success = server_->listen(config_.host.c_str(), config_.port);
    isRunning_ = false;
    return success;
}

bool HttpServer::startAsync() {
    if (isRunning_) return true;

    isRunning_ = true;
    workerThread_ = std::make_unique<std::thread>([this]() {
        server_->listen(config_.host.c_str(), config_.port);
        isRunning_ = false;
    });

    // Wait until server has bound to port and is listening
    int attempts = 0;
    while (!server_->is_running() && attempts < 50) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        attempts++;
    }

    return server_->is_running();
}

void HttpServer::stop() {
    if (server_ && server_->is_running()) {
        server_->stop();
    }
    if (workerThread_ && workerThread_->joinable()) {
        workerThread_->join();
    }
    workerThread_.reset();
    isRunning_ = false;
}

bool HttpServer::isRunning() const {
    return isRunning_ && server_ && server_->is_running();
}

const HttpServerConfig& HttpServer::getConfig() const {
    return config_;
}

void HttpServer::setHost(const std::string& host) {
    config_.host = host;
    setupServer();
}

void HttpServer::setPort(int port) {
    config_.port = port;
    setupServer();
}

void HttpServer::setCorsOrigin(const std::string& origin) {
    config_.corsOrigin = origin;
    setupServer();
}

void HttpServer::setDatabasePath(const std::string& path) {
    config_.databasePath = path;
}

void HttpServer::setUploadDirectory(const std::string& dir) {
    config_.uploadDirectory = dir;
}

httplib::Server& HttpServer::getServer() {
    return *server_;
}
