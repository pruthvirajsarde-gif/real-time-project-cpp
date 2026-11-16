#include "AnalyticsController.h"
#include "AnalyticsService.h"
#include "ApiResponse.h"
#include "ApiMiddleware.h"
#include "AuditLogger.h"
#include <json.hpp>

void AnalyticsController::getCandidateAnalytics(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) return;
    if (!ApiMiddleware::requireCandidate(ctx, res, corsOrigin)) return;

    CandidateAnalytics ca = AnalyticsService::getCandidateAnalytics(ctx.candidateId);

    nlohmann::json statusObj = nlohmann::json::object();
    for (const auto& [st, cnt] : ca.statusBreakdown) {
        statusObj[st] = cnt;
    }

    nlohmann::json strengthsArr = nlohmann::json::array();
    for (const auto& s : ca.readinessStrengths) strengthsArr.push_back(s);

    nlohmann::json improvementsArr = nlohmann::json::array();
    for (const auto& s : ca.readinessImprovements) improvementsArr.push_back(s);

    nlohmann::json data = {
        {"applications", {
            {"total", ca.totalApplications},
            {"active", ca.activeApplications},
            {"withdrawn", ca.withdrawnApplications},
            {"byStatus", statusObj},
            {"savedJobs", ca.savedJobsCount},
            {"avgMatchScore", ca.avgMatchScore},
            {"highestMatchScore", ca.highestMatchScore}
        }},
        {"resumeHealth", {
            {"qualityScore", ca.resumeQualityScore},
            {"skillCoverage", ca.skillCoverage},
            {"experienceCoverage", ca.experienceCoverage},
            {"educationCoverage", ca.educationCoverage},
            {"profileCompleteness", ca.profileCompleteness}
        }},
        {"careerReadiness", {
            {"overallReadiness", ca.overallReadiness},
            {"skillReadiness", ca.skillReadiness},
            {"experienceReadiness", ca.experienceReadiness},
            {"educationReadiness", ca.educationReadiness},
            {"readinessLevel", ca.readinessLevel},
            {"strengths", strengthsArr},
            {"improvementAreas", improvementsArr}
        }}
    };

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_ANALYTICS_VIEWED, "Viewed candidate career analytics");
    ApiResponse::sendSuccess(res, data, "Candidate analytics retrieved successfully.", 200, corsOrigin);
}

void AnalyticsController::getCandidateSkillsAnalytics(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) return;
    if (!ApiMiddleware::requireCandidate(ctx, res, corsOrigin)) return;

    CandidateSkillGapAnalytics sga = AnalyticsService::getCandidateSkillGapAnalytics(ctx.candidateId);

    nlohmann::json candSkillsArr = nlohmann::json::array();
    for (const auto& s : sga.candidateSkills) candSkillsArr.push_back(s);

    nlohmann::json exactArr = nlohmann::json::array();
    for (const auto& s : sga.exactMatchesAcrossJobs) exactArr.push_back(s);

    nlohmann::json relArr = nlohmann::json::array();
    for (const auto& s : sga.relatedSkills) relArr.push_back(s);

    nlohmann::json missingReqArr = nlohmann::json::array();
    for (const auto& m : sga.prioritizedMissingRequired) {
        missingReqArr.push_back({
            {"skill", m.skill},
            {"jobCount", m.jobCount},
            {"demandPercentage", m.demandPercentage},
            {"priority", m.priority}
        });
    }

    nlohmann::json missingPrefArr = nlohmann::json::array();
    for (const auto& m : sga.prioritizedMissingPreferred) {
        missingPrefArr.push_back({
            {"skill", m.skill},
            {"jobCount", m.jobCount},
            {"demandPercentage", m.demandPercentage},
            {"priority", m.priority}
        });
    }

    nlohmann::json data = {
        {"catalogSkillCoverage", sga.catalogSkillCoverage},
        {"candidateSkills", candSkillsArr},
        {"exactMatches", exactArr},
        {"relatedSkills", relArr},
        {"missingRequired", missingReqArr},
        {"missingPreferred", missingPrefArr}
    };

    ApiResponse::sendSuccess(res, data, "Candidate skill gap analytics retrieved.", 200, corsOrigin);
}

void AnalyticsController::getCandidateMarketInsights(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) return;

    MarketInsights mi = AnalyticsService::getMarketInsights();

    nlohmann::json topSkillsArr = nlohmann::json::array();
    for (const auto& s : mi.topSkills) {
        topSkillsArr.push_back({
            {"skill", s.skill},
            {"totalCount", s.totalCount},
            {"jobCount", s.totalCount},
            {"count", s.totalCount},
            {"requiredCount", s.requiredCount},
            {"preferredCount", s.preferredCount},
            {"percentage", s.percentage},
            {"frequencyPercentage", s.percentage},
            {"frequencyPct", s.percentage}
        });
    }

    nlohmann::json locObj = nlohmann::json::object();
    nlohmann::json locArr = nlohmann::json::array();
    for (const auto& [loc, cnt] : mi.jobsByLocation) {
        locObj[loc] = cnt;
        locArr.push_back({
            {"location", loc},
            {"jobCount", cnt},
            {"count", cnt}
        });
    }

    nlohmann::json empObj = nlohmann::json::object();
    nlohmann::json empArr = nlohmann::json::array();
    for (const auto& [emp, cnt] : mi.jobsByEmploymentType) {
        empObj[emp] = cnt;
        empArr.push_back({
            {"employmentType", emp},
            {"jobCount", cnt},
            {"count", cnt}
        });
    }

    nlohmann::json coOccurArr = nlohmann::json::array();
    for (const auto& c : mi.skillCoOccurrences) {
        coOccurArr.push_back({
            {"skillA", c.skillA},
            {"skillB", c.skillB},
            {"pair", c.skillA + " + " + c.skillB},
            {"count", c.count},
            {"coOccurrenceCount", c.count}
        });
    }

    nlohmann::json data = {
        {"totalJobs", mi.totalJobs},
        {"totalJobsAnalyzed", mi.totalJobs},
        {"sampleSize", mi.totalJobs},
        {"topSkills", topSkillsArr},
        {"jobsByLocation", locObj},
        {"jobsByLocationList", locArr},
        {"jobsByEmploymentType", empObj},
        {"jobsByEmploymentTypeList", empArr},
        {"skillCoOccurrences", coOccurArr},
        {"frequentlyCoOccurringSkills", coOccurArr},
        {"frequentSkillPairs", coOccurArr}
    };

    ApiResponse::sendSuccess(res, data, "Job market insights calculated from current dataset.", 200, corsOrigin);
}

void AnalyticsController::getCandidateRecommendations(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) return;
    if (!ApiMiddleware::requireCandidate(ctx, res, corsOrigin)) return;

    size_t limit = 10;
    if (req.has_param("limit")) {
        try {
            int parsed = std::stoi(req.get_param_value("limit"));
            if (parsed > 0 && parsed <= 50) limit = static_cast<size_t>(parsed);
        } catch (...) {}
    }

    std::vector<RecommendedJob> recs = AnalyticsService::getCandidateRecommendations(ctx.candidateId, limit);

    nlohmann::json recsArr = nlohmann::json::array();
    for (const auto& r : recs) {
        nlohmann::json reasonsArr = nlohmann::json::array();
        for (const auto& reason : r.explainableReasons) reasonsArr.push_back(reason);

        nlohmann::json matchedSkillsArr = nlohmann::json::array();
        for (const auto& s : r.topMatchedSkills) matchedSkillsArr.push_back(s);

        nlohmann::json missingSkillsArr = nlohmann::json::array();
        for (const auto& s : r.missingRequiredSkills) missingSkillsArr.push_back(s);

        recsArr.push_back({
            {"jobId", r.jobId},
            {"title", r.jobTitle},
            {"company", r.company},
            {"location", r.location},
            {"matchScore", r.matchScore},
            {"readinessScore", r.readinessScore},
            {"explainableReasons", reasonsArr},
            {"matchedSkills", matchedSkillsArr},
            {"missingRequiredSkills", missingSkillsArr}
        });
    }

    ApiResponse::sendSuccess(res, recsArr, "Candidate recommendations generated.", 200, corsOrigin);
}

void AnalyticsController::getResumeJobComparison(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) return;
    if (!ApiMiddleware::requireCandidate(ctx, res, corsOrigin)) return;

    std::string jobId;
    if (req.matches.size() > 1 && !req.matches[1].str().empty()) {
        jobId = req.matches[1].str();
    } else if (req.has_param("jobId")) {
        jobId = req.get_param_value("jobId");
    }
    if (jobId.empty()) {
        ApiResponse::sendError(res, "VALIDATION_ERROR", "jobId is required for resume comparison.", 400, corsOrigin);
        return;
    }
    bool found = false;
    ResumeJobComparison cmp = AnalyticsService::compareResumeToJob(ctx.candidateId, jobId, found);

    if (!found) {
        ApiResponse::sendError(res, "JOB_NOT_FOUND", "Job '" + jobId + "' not found.", 404, corsOrigin);
        return;
    }

    nlohmann::json matchedArr = nlohmann::json::array();
    for (const auto& s : cmp.matchedSkills) matchedArr.push_back(s);

    nlohmann::json relatedArr = nlohmann::json::array();
    for (const auto& s : cmp.relatedSkills) relatedArr.push_back(s);

    nlohmann::json missingReqArr = nlohmann::json::array();
    for (const auto& s : cmp.missingRequiredSkills) missingReqArr.push_back(s);

    nlohmann::json missingPrefArr = nlohmann::json::array();
    for (const auto& s : cmp.missingPreferredSkills) missingPrefArr.push_back(s);

    nlohmann::json data = {
        {"jobId", cmp.jobId},
        {"jobTitle", cmp.jobTitle},
        {"company", cmp.company},
        {"location", cmp.location},
        {"overallScore", cmp.overallScore},
        {"classification", cmp.classification},
        {"scores", {
            {"skill", cmp.skillScore},
            {"text", cmp.textScore},
            {"project", cmp.projectScore},
            {"experience", cmp.experienceScore},
            {"education", cmp.educationScore},
            {"resumeQuality", cmp.resumeQualityScore}
        }},
        {"skills", {
            {"matched", matchedArr},
            {"related", relatedArr},
            {"missingRequired", missingReqArr},
            {"missingPreferred", missingPrefArr}
        }},
        {"education", {
            {"required", cmp.requiredEducation},
            {"candidate", cmp.candidateEducation},
            {"compatibility", cmp.educationCompatibility}
        }},
        {"experience", {
            {"required", cmp.requiredExperience},
            {"candidate", cmp.candidateExperience},
            {"compatibility", cmp.experienceCompatibility}
        }}
    };

    ApiResponse::sendSuccess(res, data, "Resume-to-job comparison calculated.", 200, corsOrigin);
}

void AnalyticsController::getRecruiterAnalytics(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) return;
    if (!ApiMiddleware::requireRecruiter(ctx, res, corsOrigin)) return;

    RecruiterAnalytics ra = AnalyticsService::getRecruiterAnalytics(ctx.accountId);

    nlohmann::json funnelObj = nlohmann::json::object();
    for (const auto& [stage, count] : ra.pipelineFunnel) {
        funnelObj[stage] = count;
    }

    nlohmann::json distObj = nlohmann::json::object();
    for (const auto& [bucket, count] : ra.matchScoreDistribution) {
        distObj[bucket] = count;
    }

    nlohmann::json data = {
        {"activeJobs", ra.activeJobs},
        {"totalApplications", ra.totalApplications},
        {"averageMatchScore", ra.averageMatchScore},
        {"pipelineFunnel", funnelObj},
        {"matchScoreDistribution", distObj}
    };

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_ANALYTICS_VIEWED, "Viewed recruiter hiring analytics");
    ApiResponse::sendSuccess(res, data, "Recruiter analytics retrieved.", 200, corsOrigin);
}

void AnalyticsController::getRecruiterJobAnalytics(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) return;
    if (!ApiMiddleware::requireRecruiter(ctx, res, corsOrigin)) return;

    std::string jobId = req.matches[1];
    bool found = false;
    bool owned = false;
    RecruiterJobAnalytics rja = AnalyticsService::getRecruiterJobAnalytics(ctx.accountId, jobId, found, owned);

    if (!found) {
        ApiResponse::sendError(res, "JOB_NOT_FOUND", "Job '" + jobId + "' not found.", 404, corsOrigin);
        return;
    }
    if (!owned) {
        ApiResponse::sendError(res, "FORBIDDEN_NOT_OWNER", "You are not authorized to view analytics for this job requisition.", 403, corsOrigin);
        return;
    }

    nlohmann::json stageObj = nlohmann::json::object();
    for (const auto& [stage, count] : rja.stageDistribution) {
        stageObj[stage] = count;
    }

    nlohmann::json distObj = nlohmann::json::object();
    for (const auto& [bucket, count] : rja.matchDistribution) {
        distObj[bucket] = count;
    }

    nlohmann::json data = {
        {"jobId", rja.jobId},
        {"title", rja.jobTitle},
        {"company", rja.company},
        {"location", rja.location},
        {"status", rja.status},
        {"totalApplicants", rja.totalApplicants},
        {"averageMatchScore", rja.averageMatchScore},
        {"highestMatchScore", rja.highestMatchScore},
        {"lowestMatchScore", rja.lowestMatchScore},
        {"averageSkillCoverage", rja.averageSkillCoverage},
        {"stageDistribution", stageObj},
        {"matchDistribution", distObj}
    };

    ApiResponse::sendSuccess(res, data, "Recruiter job analytics retrieved.", 200, corsOrigin);
}

void AnalyticsController::getRecruiterJobCandidateComparison(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    SecurityContext ctx;
    if (!ApiMiddleware::authenticate(req, res, ctx, corsOrigin)) return;
    if (!ApiMiddleware::requireRecruiter(ctx, res, corsOrigin)) return;

    std::string jobId = req.matches[1];
    bool found = false;
    bool owned = false;
    std::vector<CandidateJobComparisonRow> rows = AnalyticsService::compareJobCandidates(ctx.accountId, jobId, found, owned);

    if (!found) {
        ApiResponse::sendError(res, "JOB_NOT_FOUND", "Job '" + jobId + "' not found.", 404, corsOrigin);
        return;
    }
    if (!owned) {
        ApiResponse::sendError(res, "FORBIDDEN_NOT_OWNER", "You are not authorized to compare candidates for this job requisition.", 403, corsOrigin);
        return;
    }

    nlohmann::json rowsArr = nlohmann::json::array();
    for (const auto& r : rows) {
        nlohmann::json matchedArr = nlohmann::json::array();
        for (const auto& s : r.matchedSkills) matchedArr.push_back(s);

        nlohmann::json missingArr = nlohmann::json::array();
        for (const auto& s : r.missingSkills) missingArr.push_back(s);

        rowsArr.push_back({
            {"candidateId", r.candidateId},
            {"candidateName", r.candidateName},
            {"applicationId", r.applicationId},
            {"appliedDate", r.appliedDate},
            {"status", r.status},
            {"matchScore", r.matchScore},
            {"readinessScore", r.readinessScore},
            {"requiredSkillsMatched", r.requiredSkillsMatched},
            {"totalRequiredSkills", r.totalRequiredSkills},
            {"preferredSkillsMatched", r.preferredSkillsMatched},
            {"totalPreferredSkills", r.totalPreferredSkills},
            {"totalExperience", r.totalExperience},
            {"educationSummary", r.educationSummary},
            {"matchedSkills", matchedArr},
            {"missingSkills", missingArr}
        });
    }

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_CANDIDATE_COMP_VIEWED, "Viewed candidate comparison for job " + jobId);
    ApiResponse::sendSuccess(res, rowsArr, "Candidate comparison table generated.", 200, corsOrigin);
}
