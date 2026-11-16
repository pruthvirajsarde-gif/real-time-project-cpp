#include "MatchingController.h"
#include "ApiResponse.h"
#include "ApiMiddleware.h"
#include "ResumeRepository.h"
#include "JobRepository.h"
#include "AdvancedJobMatcher.h"
#include <json.hpp>

void MatchingController::getMatches(
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

    if (!ResumeRepository::resumeExists(ctx.candidateId)) {
        ApiResponse::sendError(res, "RESUME_NOT_FOUND", "Please upload a resume first to run job matching.", 404, corsOrigin);
        return;
    }

    Resume resume = ResumeRepository::loadResume(ctx.candidateId);
    std::vector<Job> allJobs = JobRepository::getAllJobs();

    if (allJobs.empty()) {
        ApiResponse::sendSuccess(res, nlohmann::json::array(), "No jobs available for matching.", 200, corsOrigin);
        return;
    }

    auto rankings = AdvancedJobMatcher::rankJobsAdvanced(resume, allJobs);

    nlohmann::json matches = nlohmann::json::array();
    for (const auto& [job, expl] : rankings) {
        matches.push_back({
            {"jobId", job.getJobId()},
            {"title", job.getJobTitle()},
            {"company", job.getCompany()},
            {"location", job.getLocation()},
            {"overallScore", expl.overallScore},
            {"classification", expl.matchClassification}
        });
    }

    ApiResponse::sendSuccess(res, matches, "Job match rankings calculated successfully.", 200, corsOrigin);
}

void MatchingController::getMatchForJob(
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

    if (!ResumeRepository::resumeExists(ctx.candidateId)) {
        ApiResponse::sendError(res, "RESUME_NOT_FOUND", "Please upload a resume first to view job match.", 404, corsOrigin);
        return;
    }

    Resume resume = ResumeRepository::loadResume(ctx.candidateId);
    Job job = JobRepository::getJob(jobId);
    MatchExplanation expl = AdvancedJobMatcher::explainMatch(resume, job);

    nlohmann::json data = {
        {"jobId", job.getJobId()},
        {"title", job.getJobTitle()},
        {"company", job.getCompany()},
        {"overallScore", expl.overallScore},
        {"classification", expl.matchClassification}
    };

    ApiResponse::sendSuccess(res, data, "Match score retrieved.", 200, corsOrigin);
}

void MatchingController::getMatchExplanation(
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

    if (!ResumeRepository::resumeExists(ctx.candidateId)) {
        ApiResponse::sendError(res, "RESUME_NOT_FOUND", "Please upload a resume first to view match explanation.", 404, corsOrigin);
        return;
    }

    Resume resume = ResumeRepository::loadResume(ctx.candidateId);
    Job job = JobRepository::getJob(jobId);
    MatchExplanation expl = AdvancedJobMatcher::explainMatch(resume, job);

    nlohmann::json exactSkills = nlohmann::json::array();
    for (const auto& s : expl.exactMatches) exactSkills.push_back(s);

    nlohmann::json relatedSkills = nlohmann::json::array();
    for (const auto& r : expl.relatedMatches) {
        auto arrowPos = r.find(" -> ");
        if (arrowPos != std::string::npos) {
            nlohmann::json item;
            item["candidate"] = r.substr(0, arrowPos);
            item["job"] = r.substr(arrowPos + 4);
            relatedSkills.push_back(item);
        } else {
            nlohmann::json item;
            item["relation"] = r;
            relatedSkills.push_back(item);
        }
    }

    nlohmann::json missingRequired = nlohmann::json::array();
    for (const auto& s : expl.missingRequiredSkills) missingRequired.push_back(s);

    nlohmann::json missingPreferred = nlohmann::json::array();
    for (const auto& s : expl.missingPreferredSkills) missingPreferred.push_back(s);

    nlohmann::json strengths = nlohmann::json::array();
    for (const auto& s : expl.strengths) strengths.push_back(s);

    nlohmann::json improvementAreas = nlohmann::json::array();
    for (const auto& s : expl.improvementAreas) improvementAreas.push_back(s);

    nlohmann::json data = {
        {"overallScore", expl.overallScore},
        {"classification", expl.matchClassification},
        {"scores", {
            {"skill", expl.skillScore},
            {"text", expl.textScore},
            {"project", expl.projectScore},
            {"experience", expl.experienceScore},
            {"education", expl.educationScore},
            {"resumeQuality", expl.resumeQualityScore}
        }},
        {"exactSkills", exactSkills},
        {"relatedSkills", relatedSkills},
        {"missingRequired", missingRequired},
        {"missingPreferred", missingPreferred},
        {"strengths", strengths},
        {"improvementAreas", improvementAreas}
    };

    ApiResponse::sendSuccess(res, data, "Match explanation calculated.", 200, corsOrigin);
}
