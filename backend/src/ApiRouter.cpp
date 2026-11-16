#include "ApiRouter.h"
#include "ApiResponse.h"
#include <fstream>
#include "AuthController.h"
#include "CandidateController.h"
#include "ResumeController.h"
#include "JobController.h"
#include "MatchingController.h"
#include "SavedJobController.h"
#include "ApplicationController.h"
#include "RecruiterController.h"
#include "DashboardController.h"
#include "SystemController.h"
#include "AnalyticsController.h"

void ApiRouter::registerRoutes(httplib::Server& svr, const std::string& corsOrigin) {
    // ── Preflight CORS Handler ──────────────────────────────────
    svr.Options(R"(/.*)", [corsOrigin](const httplib::Request&, httplib::Response& res) {
        ApiResponse::setCorsHeaders(res, corsOrigin);
        res.status = 204;
    });

    // ── System & Health Endpoints ───────────────────────────────
    svr.Get("/api/v1/health", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        SystemController::getHealth(req, res, corsOrigin);
    });

    svr.Get("/api/v1/health/live", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        SystemController::getLiveness(req, res, corsOrigin);
    });

    svr.Get("/api/v1/health/ready", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        SystemController::getReadiness(req, res, corsOrigin);
    });

    svr.Get("/api/v1", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        SystemController::getApiInfo(req, res, corsOrigin);
    });

    svr.Get("/api/v1/activity", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        SystemController::getActivity(req, res, corsOrigin);
    });

    // ── Authentication Endpoints ────────────────────────────────
    svr.Post("/api/v1/auth/register/candidate", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AuthController::registerCandidate(req, res, corsOrigin);
    });

    svr.Post("/api/v1/auth/register/recruiter", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AuthController::registerRecruiter(req, res, corsOrigin);
    });

    svr.Post("/api/v1/auth/login", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AuthController::login(req, res, corsOrigin);
    });

    svr.Post("/api/v1/auth/logout", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AuthController::logout(req, res, corsOrigin);
    });

    svr.Post("/api/v1/auth/change-password", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AuthController::changePassword(req, res, corsOrigin);
    });

    svr.Get("/api/v1/auth/me", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AuthController::getMe(req, res, corsOrigin);
    });

    // ── Candidate Profile Endpoints ─────────────────────────────
    svr.Get("/api/v1/candidates/me", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        CandidateController::getMyProfile(req, res, corsOrigin);
    });

    svr.Put("/api/v1/candidates/me", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        CandidateController::updateMyProfile(req, res, corsOrigin);
    });

    // ── Resume Endpoints ────────────────────────────────────────
    svr.Post("/api/v1/resume/upload", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        ResumeController::uploadResume(req, res, corsOrigin);
    });

    svr.Post("/api/v1/resume/import", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        ResumeController::importResume(req, res, corsOrigin);
    });

    svr.Get("/api/v1/resume", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        ResumeController::getResume(req, res, corsOrigin);
    });

    svr.Get("/api/v1/resume/analysis", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        ResumeController::getAnalysis(req, res, corsOrigin);
    });

    svr.Get("/api/v1/resume/strengths", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        ResumeController::getStrengths(req, res, corsOrigin);
    });

    svr.Get("/api/v1/resume/recommendations", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        ResumeController::getRecommendations(req, res, corsOrigin);
    });

    // ── Jobs Endpoints (Public & Search) ────────────────────────
    svr.Get("/api/v1/jobs", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        JobController::getJobs(req, res, corsOrigin);
    });

    svr.Get(R"(/api/v1/jobs/([^/]+))", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        JobController::getJobById(req, res, corsOrigin);
    });

    // ── Matching Endpoints ──────────────────────────────────────
    svr.Get("/api/v1/matches", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        MatchingController::getMatches(req, res, corsOrigin);
    });

    svr.Get(R"(/api/v1/matches/([^/]+)/explanation)", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        MatchingController::getMatchExplanation(req, res, corsOrigin);
    });

    svr.Get(R"(/api/v1/matches/([^/]+))", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        MatchingController::getMatchForJob(req, res, corsOrigin);
    });

    // ── Saved Jobs Endpoints ────────────────────────────────────
    svr.Get("/api/v1/saved-jobs", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        SavedJobController::getSavedJobs(req, res, corsOrigin);
    });

    svr.Post(R"(/api/v1/saved-jobs/([^/]+))", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        SavedJobController::saveJob(req, res, corsOrigin);
    });

    svr.Delete(R"(/api/v1/saved-jobs/([^/]+))", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        SavedJobController::removeSavedJob(req, res, corsOrigin);
    });

    // ── Applications Endpoints (Candidate) ──────────────────────
    svr.Get("/api/v1/applications", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        ApplicationController::getMyApplications(req, res, corsOrigin);
    });

    svr.Post("/api/v1/applications", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        ApplicationController::applyForJob(req, res, corsOrigin);
    });

    svr.Post(R"(/api/v1/applications/([0-9]+)/withdraw)", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        ApplicationController::withdrawApplication(req, res, corsOrigin);
    });

    svr.Get(R"(/api/v1/applications/([0-9]+))", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        ApplicationController::getApplicationById(req, res, corsOrigin);
    });

    // ── Recruiter Endpoints ─────────────────────────────────────
    svr.Post("/api/v1/recruiter/jobs", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        RecruiterController::createJob(req, res, corsOrigin);
    });

    svr.Get("/api/v1/recruiter/jobs", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        RecruiterController::getMyJobs(req, res, corsOrigin);
    });

    svr.Put(R"(/api/v1/recruiter/jobs/([^/]+))", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        RecruiterController::updateJob(req, res, corsOrigin);
    });

    svr.Delete(R"(/api/v1/recruiter/jobs/([^/]+))", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        RecruiterController::deleteJob(req, res, corsOrigin);
    });

    svr.Get("/api/v1/recruiter/applications", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        RecruiterController::getApplications(req, res, corsOrigin);
    });

    svr.Put(R"(/api/v1/recruiter/applications/([0-9]+)/status)", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        RecruiterController::updateApplicationStatus(req, res, corsOrigin);
    });

    svr.Get(R"(/api/v1/recruiter/applications/([0-9]+))", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        RecruiterController::getApplicationById(req, res, corsOrigin);
    });

    // ── Dashboard Endpoints ─────────────────────────────────────
    svr.Get("/api/v1/dashboard/candidate", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        DashboardController::getCandidateDashboard(req, res, corsOrigin);
    });

    svr.Get("/api/v1/dashboard/recruiter", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        DashboardController::getRecruiterDashboard(req, res, corsOrigin);
    });

    // ── Analytics & Intelligence Endpoints (Phase 12) ───────────
    svr.Get("/api/v1/analytics/candidate", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AnalyticsController::getCandidateAnalytics(req, res, corsOrigin);
    });

    svr.Get("/api/v1/analytics/candidate/skills", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AnalyticsController::getCandidateSkillsAnalytics(req, res, corsOrigin);
    });

    svr.Get("/api/v1/analytics/candidate/market", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AnalyticsController::getCandidateMarketInsights(req, res, corsOrigin);
    });

    svr.Get("/api/v1/analytics/candidate/recommendations", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AnalyticsController::getCandidateRecommendations(req, res, corsOrigin);
    });

    svr.Get(R"(/api/v1/analytics/candidate/compare/([^/]+))", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AnalyticsController::getResumeJobComparison(req, res, corsOrigin);
    });

    svr.Get("/api/v1/analytics/candidate/compare", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AnalyticsController::getResumeJobComparison(req, res, corsOrigin);
    });

    svr.Get("/api/v1/analytics/recruiter", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AnalyticsController::getRecruiterAnalytics(req, res, corsOrigin);
    });

    svr.Get(R"(/api/v1/recruiter/jobs/([^/]+)/analytics)", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AnalyticsController::getRecruiterJobAnalytics(req, res, corsOrigin);
    });

    svr.Get(R"(/api/v1/recruiter/jobs/([^/]+)/candidates/compare)", [corsOrigin](const httplib::Request& req, httplib::Response& res) {
        AnalyticsController::getRecruiterJobCandidateComparison(req, res, corsOrigin);
    });

    // ── 404 Not Found Handler ───────────────────────────────────
    svr.set_error_handler([corsOrigin](const httplib::Request& req, httplib::Response& res) {
        if (res.status == 404 && res.body.empty()) {
            if (req.path.rfind("/api/", 0) == 0) {
                ApiResponse::sendError(res, "ROUTE_NOT_FOUND", "The requested API endpoint was not found.", 404, corsOrigin);
            } else {
                std::ifstream indexFile("frontend/dist/index.html");
                if (indexFile.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(indexFile)), std::istreambuf_iterator<char>());
                    res.set_content(content, "text/html");
                    res.status = 200;
                    return;
                }
                ApiResponse::sendError(res, "ROUTE_NOT_FOUND", "The requested API endpoint was not found.", 404, corsOrigin);
            }
        }
    });

    // ── 500 Internal Exception Handler ──────────────────────────
    svr.set_exception_handler([corsOrigin](const httplib::Request&, httplib::Response& res, std::exception_ptr) {
        ApiResponse::sendError(res, "INTERNAL_SERVER_ERROR", "An unexpected internal server error occurred.", 500, corsOrigin);
    });
}
