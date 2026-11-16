#include "SavedJobController.h"
#include "ApiResponse.h"
#include "ApiMiddleware.h"
#include "SavedJobManager.h"
#include "JobRepository.h"
#include "AuditLogger.h"
#include <json.hpp>

void SavedJobController::saveJob(
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

    std::string jobId = req.matches[1];
    if (!JobRepository::jobExists(jobId)) {
        ApiResponse::sendError(res, "JOB_NOT_FOUND", "Job '" + jobId + "' not found.", 404, corsOrigin);
        return;
    }

    if (SavedJobManager::isJobSaved(ctx.candidateId, jobId)) {
        ApiResponse::sendSuccess(res, {{"jobId", jobId}}, "Job is already saved.", 200, corsOrigin);
        return;
    }

    bool ok = SavedJobManager::saveJob(ctx.candidateId, jobId);
    if (!ok) {
        ApiResponse::sendError(res, "DATABASE_ERROR", "Failed to bookmark job.", 500, corsOrigin);
        return;
    }

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_SAVED_JOB_ADDED, "Saved job: " + jobId);
    ApiResponse::sendSuccess(res, {{"jobId", jobId}}, "Job saved successfully.", 201, corsOrigin);
}

void SavedJobController::getSavedJobs(
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

    std::vector<Job> savedJobs = SavedJobManager::getSavedJobs(ctx.candidateId);
    nlohmann::json items = nlohmann::json::array();

    for (const auto& job : savedJobs) {
        nlohmann::json reqSkills = nlohmann::json::array();
        for (const auto& s : job.getRequiredSkills()) reqSkills.push_back(s);

        nlohmann::json prefSkills = nlohmann::json::array();
        for (const auto& s : job.getPreferredSkills()) prefSkills.push_back(s);

        items.push_back({
            {"jobId", job.getJobId()},
            {"title", job.getJobTitle()},
            {"company", job.getCompany()},
            {"location", job.getLocation()},
            {"employmentType", job.getEmploymentType()},
            {"educationRequirement", job.getEducationRequirement()},
            {"minimumExperience", job.getMinimumExperience()},
            {"description", job.getDescription()},
            {"requiredSkills", reqSkills},
            {"preferredSkills", prefSkills}
        });
    }

    ApiResponse::sendSuccess(res, items, "Saved jobs retrieved.", 200, corsOrigin);
}

void SavedJobController::removeSavedJob(
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

    std::string jobId = req.matches[1];
    if (!SavedJobManager::isJobSaved(ctx.candidateId, jobId)) {
        ApiResponse::sendError(res, "SAVED_JOB_NOT_FOUND", "Job '" + jobId + "' is not in candidate's saved list.", 404, corsOrigin);
        return;
    }

    bool ok = SavedJobManager::removeSavedJob(ctx.candidateId, jobId);
    if (!ok) {
        ApiResponse::sendError(res, "DATABASE_ERROR", "Failed to remove saved job.", 500, corsOrigin);
        return;
    }

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_SAVED_JOB_REMOVED, "Removed saved job: " + jobId);
    ApiResponse::sendSuccess(res, {{"jobId", jobId}}, "Saved job removed.", 200, corsOrigin);
}
