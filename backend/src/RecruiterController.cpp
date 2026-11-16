#include "RecruiterController.h"
#include "ApiResponse.h"
#include "ApiMiddleware.h"
#include "AuthorizationManager.h"
#include "JobRepository.h"
#include "AccountRepository.h"
#include "ApplicationManager.h"
#include "AuditLogger.h"
#include <json.hpp>
#include <algorithm>

namespace {
nlohmann::json serializeJob(const Job& job) {
    nlohmann::json reqSkills = nlohmann::json::array();
    for (const auto& s : job.getRequiredSkills()) reqSkills.push_back(s);

    nlohmann::json prefSkills = nlohmann::json::array();
    for (const auto& s : job.getPreferredSkills()) prefSkills.push_back(s);

    return {
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
    };
}

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

void RecruiterController::createJob(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }
    if (!ApiMiddleware::requireRecruiter(ctx, res, corsOrigin)) {
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
    std::string title = body.value("title", "");
    std::string company = body.value("company", "");
    std::string location = body.value("location", "");
    std::string employmentType = body.value("employmentType", "Full Time");
    std::string educationReq = body.value("educationRequirement", "Any Degree");
    int minExp = body.value("minimumExperience", 0);
    std::string description = body.value("description", "");

    if (jobId.empty() || title.empty() || company.empty()) {
        ApiResponse::sendError(res, "VALIDATION_FAILED", "Job ID, title, and company are required.", 400, corsOrigin);
        return;
    }

    if (JobRepository::jobExists(jobId)) {
        ApiResponse::sendError(res, "CONFLICT_JOB_EXISTS", "A job with ID '" + jobId + "' already exists.", 409, corsOrigin);
        return;
    }

    Job job(jobId, title, company, location, employmentType, educationReq, minExp, description);

    if (body.contains("requiredSkills") && body["requiredSkills"].is_array()) {
        for (const auto& s : body["requiredSkills"]) {
            if (s.is_string()) job.addRequiredSkill(s.get<std::string>());
        }
    }
    if (body.contains("preferredSkills") && body["preferredSkills"].is_array()) {
        for (const auto& s : body["preferredSkills"]) {
            if (s.is_string()) job.addPreferredSkill(s.get<std::string>());
        }
    }

    bool saved = JobRepository::saveJob(job);
    if (!saved) {
        ApiResponse::sendError(res, "DATABASE_ERROR", "Failed to save job listing.", 500, corsOrigin);
        return;
    }

    // Link job to authenticated recruiter account
    AccountRepository::linkJobToRecruiter(jobId, ctx.accountId);
    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_JOB_CREATED, "Created job: " + jobId + " (" + title + ")");

    ApiResponse::sendSuccess(res, serializeJob(job), "Job created successfully.", 201, corsOrigin);
}

void RecruiterController::getMyJobs(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }
    if (!ApiMiddleware::requireRecruiter(ctx, res, corsOrigin)) {
        return;
    }

    std::vector<std::string> jobIds = AccountRepository::getJobIdsForRecruiter(ctx.accountId);
    nlohmann::json items = nlohmann::json::array();
    for (const auto& id : jobIds) {
        if (JobRepository::jobExists(id)) {
            items.push_back(serializeJob(JobRepository::getJob(id)));
        }
    }

    ApiResponse::sendSuccess(res, items, "Recruiter jobs retrieved.", 200, corsOrigin);
}

void RecruiterController::updateJob(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }
    if (!ApiMiddleware::requireRecruiter(ctx, res, corsOrigin)) {
        return;
    }

    std::string jobId = req.matches[1];
    if (!JobRepository::jobExists(jobId)) {
        ApiResponse::sendError(res, "JOB_NOT_FOUND", "Job '" + jobId + "' not found.", 404, corsOrigin);
        return;
    }

    if (!AuthorizationManager::canEditJob(ctx, jobId)) {
        ApiResponse::sendError(res, "FORBIDDEN_NOT_OWNER", "You are not authorized to modify this job posting.", 403, corsOrigin);
        return;
    }

    nlohmann::json body;
    try {
        body = nlohmann::json::parse(req.body);
    } catch (...) {
        ApiResponse::sendError(res, "BAD_REQUEST_MALFORMED_JSON", "Malformed JSON body.", 400, corsOrigin);
        return;
    }

    Job job = JobRepository::getJob(jobId);
    if (body.contains("title")) job.setJobTitle(body["title"].get<std::string>());
    if (body.contains("company")) job.setCompany(body["company"].get<std::string>());
    if (body.contains("location")) job.setLocation(body["location"].get<std::string>());
    if (body.contains("employmentType")) job.setEmploymentType(body["employmentType"].get<std::string>());
    if (body.contains("educationRequirement")) job.setEducationRequirement(body["educationRequirement"].get<std::string>());
    if (body.contains("minimumExperience")) job.setMinimumExperience(body["minimumExperience"].get<int>());
    if (body.contains("description")) job.setDescription(body["description"].get<std::string>());

    bool ok = JobRepository::saveJob(job);
    if (!ok) {
        ApiResponse::sendError(res, "DATABASE_ERROR", "Failed to update job listing.", 500, corsOrigin);
        return;
    }

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_JOB_UPDATED, "Updated job: " + jobId);
    ApiResponse::sendSuccess(res, serializeJob(job), "Job updated successfully.", 200, corsOrigin);
}

void RecruiterController::deleteJob(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }
    if (!ApiMiddleware::requireRecruiter(ctx, res, corsOrigin)) {
        return;
    }

    std::string jobId = req.matches[1];
    if (!JobRepository::jobExists(jobId)) {
        ApiResponse::sendError(res, "JOB_NOT_FOUND", "Job '" + jobId + "' not found.", 404, corsOrigin);
        return;
    }

    if (!AuthorizationManager::canEditJob(ctx, jobId)) {
        ApiResponse::sendError(res, "FORBIDDEN_NOT_OWNER", "You are not authorized to delete this job posting.", 403, corsOrigin);
        return;
    }

    bool ok = JobRepository::deleteJob(jobId);
    if (!ok) {
        ApiResponse::sendError(res, "DATABASE_ERROR", "Failed to delete job.", 500, corsOrigin);
        return;
    }

    ApiResponse::sendSuccess(res, {{"jobId", jobId}}, "Job deleted successfully.", 200, corsOrigin);
}

void RecruiterController::getApplications(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }
    if (!ApiMiddleware::requireRecruiter(ctx, res, corsOrigin)) {
        return;
    }

    std::vector<std::string> ownedJobIds = AccountRepository::getJobIdsForRecruiter(ctx.accountId);
    std::vector<ApplicationRecord> allApps = ApplicationManager::getAllApplications();
    nlohmann::json items = nlohmann::json::array();

    for (const auto& a : allApps) {
        if (std::find(ownedJobIds.begin(), ownedJobIds.end(), a.jobId) != ownedJobIds.end()) {
            items.push_back(serializeApp(a));
        }
    }

    ApiResponse::sendSuccess(res, items, "Recruiter job applications retrieved.", 200, corsOrigin);
}

void RecruiterController::getApplicationById(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }
    if (!ApiMiddleware::requireRecruiter(ctx, res, corsOrigin)) {
        return;
    }

    int appId = 0;
    try {
        appId = std::stoi(req.matches[1]);
    } catch (...) {
        ApiResponse::sendError(res, "INVALID_ID", "Invalid application ID format.", 400, corsOrigin);
        return;
    }

    std::vector<std::string> ownedJobIds = AccountRepository::getJobIdsForRecruiter(ctx.accountId);
    std::vector<ApplicationRecord> allApps = ApplicationManager::getAllApplications();

    for (const auto& a : allApps) {
        if (a.id == appId) {
            if (std::find(ownedJobIds.begin(), ownedJobIds.end(), a.jobId) == ownedJobIds.end()) {
                ApiResponse::sendError(res, "FORBIDDEN_NOT_OWNER", "You are not authorized to view applications for other recruiters' jobs.", 403, corsOrigin);
                return;
            }
            ApiResponse::sendSuccess(res, serializeApp(a), "Application retrieved.", 200, corsOrigin);
            return;
        }
    }

    ApiResponse::sendError(res, "APPLICATION_NOT_FOUND", "Application not found.", 404, corsOrigin);
}

void RecruiterController::updateApplicationStatus(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) {
        return;
    }
    if (!ApiMiddleware::requireRecruiter(ctx, res, corsOrigin)) {
        return;
    }

    int appId = 0;
    try {
        appId = std::stoi(req.matches[1]);
    } catch (...) {
        ApiResponse::sendError(res, "INVALID_ID", "Invalid application ID format.", 400, corsOrigin);
        return;
    }

    if (!AuthorizationManager::canUpdateApplicationStatus(ctx, appId)) {
        ApiResponse::sendError(res, "FORBIDDEN_NOT_OWNER", "You are not authorized to update status for this application.", 403, corsOrigin);
        return;
    }

    nlohmann::json body;
    try {
        body = nlohmann::json::parse(req.body);
    } catch (...) {
        ApiResponse::sendError(res, "BAD_REQUEST_MALFORMED_JSON", "Malformed JSON body.", 400, corsOrigin);
        return;
    }

    std::string status = body.value("status", "");
    if (status.empty()) {
        ApiResponse::sendError(res, "VALIDATION_FAILED", "Field 'status' is required.", 400, corsOrigin);
        return;
    }

    if (!ApplicationManager::isValidStatus(status)) {
        ApiResponse::sendError(res, "INVALID_STATUS", "Invalid status value: '" + status + "'.", 400, corsOrigin);
        return;
    }

    bool ok = ApplicationManager::updateStatus(appId, status);
    if (!ok) {
        ApiResponse::sendError(res, "STATUS_UPDATE_FAILED", "Failed to update application status.", 500, corsOrigin);
        return;
    }

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_APP_STATUS_CHANGED, "App " + std::to_string(appId) + " -> " + status);
    ApiResponse::sendSuccess(res, {{"applicationId", appId}, {"status", status}}, "Application status updated successfully.", 200, corsOrigin);
}
