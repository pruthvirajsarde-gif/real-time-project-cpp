#include "ApplicationController.h"
#include "ApiResponse.h"
#include "ApiMiddleware.h"
#include "ApplicationManager.h"
#include "AuthorizationManager.h"
#include "JobRepository.h"
#include "ResumeRepository.h"
#include "AdvancedJobMatcher.h"
#include "RecommendationEngine.h"
#include "AuditLogger.h"
#include <json.hpp>

namespace {
nlohmann::json serializeApp(const ApplicationRecord& app) {
    return {
        {"id", app.id},
        {"candidateId", app.candidateId},
        {"candidateName", app.candidateName},
        {"jobId", app.jobId},
        {"jobTitle", app.jobTitle},
        {"company", app.company},
        {"matchScore", app.matchScore},
        {"readinessScore", app.readinessScore},
        {"status", app.status},
        {"appliedAt", app.appliedAt},
        {"updatedAt", app.updatedAt}
    };
}
} // anonymous namespace

void ApplicationController::applyForJob(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }
    if (!ApiMiddleware::requireCandidate(ctx, res, corsOrigin)) {
        return;
    }

    nlohmann::json body;
    try {
        body = nlohmann::json::parse(req.body);
    } catch (...) {
        ApiResponse::sendError(res, "BAD_REQUEST_MALFORMED_JSON", "Malformed JSON body.", 400, corsOrigin);
        return;
    }

    std::string jobId = body.value("jobId", "");
    if (jobId.empty()) {
        ApiResponse::sendError(res, "VALIDATION_FAILED", "Field 'jobId' is required.", 400, corsOrigin);
        return;
    }

    if (!JobRepository::jobExists(jobId)) {
        ApiResponse::sendError(res, "JOB_NOT_FOUND", "Job '" + jobId + "' does not exist.", 404, corsOrigin);
        return;
    }

    if (!ResumeRepository::resumeExists(ctx.candidateId)) {
        ApiResponse::sendError(res, "RESUME_REQUIRED", "Please upload a resume before applying for a job.", 400, corsOrigin);
        return;
    }

    if (ApplicationManager::hasActiveApplication(ctx.candidateId, jobId)) {
        ApiResponse::sendError(res, "DUPLICATE_APPLICATION", "You already have an active application for this job.", 409, corsOrigin);
        return;
    }

    Resume resume = ResumeRepository::loadResume(ctx.candidateId);
    Job job = JobRepository::getJob(jobId);

    double matchScore = AdvancedJobMatcher::calculateAdvancedMatchScore(resume, job);
    double readinessScore = RecommendationEngine::calculateJobReadinessScore(resume, job);

    int appId = ApplicationManager::applyForJob(ctx.candidateId, job, matchScore, readinessScore);
    if (appId <= 0) {
        ApiResponse::sendError(res, "APPLICATION_FAILED", "Failed to submit application.", 500, corsOrigin);
        return;
    }

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_APPLICATION_SUBMITTED, "Applied for job: " + jobId + " (App ID: " + std::to_string(appId) + ")");

    nlohmann::json data = {
        {"applicationId", appId},
        {"jobId", jobId},
        {"jobTitle", job.getJobTitle()},
        {"company", job.getCompany()},
        {"matchScore", matchScore},
        {"readinessScore", readinessScore},
        {"status", ApplicationManager::STATUS_APPLIED}
    };

    ApiResponse::sendSuccess(res, data, "Application submitted successfully.", 201, corsOrigin);
}

void ApplicationController::getMyApplications(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }
    if (!ApiMiddleware::requireCandidate(ctx, res, corsOrigin)) {
        return;
    }

    std::vector<ApplicationRecord> apps = ApplicationManager::getCandidateApplications(ctx.candidateId);
    nlohmann::json items = nlohmann::json::array();
    for (const auto& a : apps) {
        items.push_back(serializeApp(a));
    }

    ApiResponse::sendSuccess(res, items, "Candidate applications retrieved.", 200, corsOrigin);
}

void ApplicationController::getApplicationById(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }
    if (!ApiMiddleware::requireCandidate(ctx, res, corsOrigin)) {
        return;
    }

    int appId = 0;
    try {
        appId = std::stoi(req.matches[1]);
    } catch (...) {
        ApiResponse::sendError(res, "INVALID_ID", "Invalid application ID format.", 400, corsOrigin);
        return;
    }

    std::vector<ApplicationRecord> apps = ApplicationManager::getCandidateApplications(ctx.candidateId);
    for (const auto& a : apps) {
        if (a.id == appId) {
            ApiResponse::sendSuccess(res, serializeApp(a), "Application retrieved.", 200, corsOrigin);
            return;
        }
    }

    // Check if the application exists at all (ownership check)
    std::vector<ApplicationRecord> allApps = ApplicationManager::getAllApplications();
    for (const auto& a : allApps) {
        if (a.id == appId) {
            // Exists but belongs to another candidate
            ApiResponse::sendError(res, "FORBIDDEN_NOT_OWNER", "You are not authorized to view this application.", 403, corsOrigin);
            return;
        }
    }

    ApiResponse::sendError(res, "APPLICATION_NOT_FOUND", "Application not found.", 404, corsOrigin);
}

void ApplicationController::withdrawApplication(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }
    if (!ApiMiddleware::requireCandidate(ctx, res, corsOrigin)) {
        return;
    }

    int appId = 0;
    try {
        appId = std::stoi(req.matches[1]);
    } catch (...) {
        ApiResponse::sendError(res, "INVALID_ID", "Invalid application ID format.", 400, corsOrigin);
        return;
    }

    // Ownership validation via AuthorizationManager
    if (!AuthorizationManager::canWithdrawApplication(ctx, appId)) {
        ApiResponse::sendError(res, "FORBIDDEN_NOT_OWNER", "You are not authorized to withdraw this application.", 403, corsOrigin);
        return;
    }

    bool ok = ApplicationManager::withdrawApplication(appId);
    if (!ok) {
        ApiResponse::sendError(res, "WITHDRAWAL_FAILED", "Failed to withdraw application (already resolved or withdrawn).", 400, corsOrigin);
        return;
    }

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_APPLICATION_WITHDRAWN, "Withdrawn application ID: " + std::to_string(appId));
    ApiResponse::sendSuccess(res, {{"applicationId", appId}, {"status", ApplicationManager::STATUS_WITHDRAWN}}, "Application successfully withdrawn.", 200, corsOrigin);
}
