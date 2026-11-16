#include "DashboardController.h"
#include "ApiResponse.h"
#include "ApiMiddleware.h"
#include "ResumeRepository.h"
#include "SavedJobManager.h"
#include "ApplicationManager.h"
#include "JobRepository.h"
#include "AccountRepository.h"
#include "AdvancedJobMatcher.h"
#include <json.hpp>
#include <algorithm>

void DashboardController::getCandidateDashboard(
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

    double quality = 0.0;
    int skillsCount = 0;
    int projectsCount = 0;
    int expCount = 0;
    int certsCount = 0;
    std::string topJobTitle = "N/A";
    double topMatchScore = 0.0;

    if (ResumeRepository::resumeExists(ctx.candidateId)) {
        Resume resume = ResumeRepository::loadResume(ctx.candidateId);
        quality = ResumeRepository::getResumeQualityScore(ctx.candidateId);
        skillsCount = static_cast<int>(resume.getSkills().size());
        projectsCount = static_cast<int>(resume.getProjects().size());
        expCount = static_cast<int>(resume.getExperience().size());
        certsCount = static_cast<int>(resume.getCertifications().size());

        std::vector<Job> allJobs = JobRepository::getAllJobs();
        if (!allJobs.empty()) {
            auto rankings = AdvancedJobMatcher::rankJobsAdvanced(resume, allJobs);
            if (!rankings.empty()) {
                topJobTitle = rankings[0].first.getJobTitle();
                topMatchScore = rankings[0].second.overallScore;
            }
        }
    }

    int savedJobsCount = SavedJobManager::countSavedJobs(ctx.candidateId);
    std::vector<ApplicationRecord> apps = ApplicationManager::getCandidateApplications(ctx.candidateId);

    int totalApps = static_cast<int>(apps.size());
    int underReviewCount = 0;
    int shortlistedCount = 0;
    for (const auto& a : apps) {
        if (a.status == ApplicationManager::STATUS_UNDER_REVIEW) underReviewCount++;
        else if (a.status == ApplicationManager::STATUS_SHORTLISTED) shortlistedCount++;
    }

    nlohmann::json data = {
        {"resumeQuality", quality},
        {"skills", skillsCount},
        {"projects", projectsCount},
        {"experience", expCount},
        {"certifications", certsCount},
        {"applications", {
            {"total", totalApps},
            {"underReview", underReviewCount},
            {"shortlisted", shortlistedCount}
        }},
        {"savedJobs", savedJobsCount},
        {"topMatch", {
            {"job", topJobTitle},
            {"score", topMatchScore}
        }}
    };

    ApiResponse::sendSuccess(res, data, "Candidate dashboard data retrieved.", 200, corsOrigin);
}

void DashboardController::getRecruiterDashboard(
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
    int jobsPosted = static_cast<int>(ownedJobIds.size());

    std::vector<ApplicationRecord> allApps = ApplicationManager::getAllApplications();
    int appsReceived = 0;
    int shortlistedCount = 0;
    int underReviewCount = 0;

    for (const auto& a : allApps) {
        if (std::find(ownedJobIds.begin(), ownedJobIds.end(), a.jobId) != ownedJobIds.end()) {
            appsReceived++;
            if (a.status == ApplicationManager::STATUS_SHORTLISTED) shortlistedCount++;
            else if (a.status == ApplicationManager::STATUS_UNDER_REVIEW) underReviewCount++;
        }
    }

    nlohmann::json data = {
        {"jobsPosted", jobsPosted},
        {"applicationsReceived", appsReceived},
        {"shortlistedCandidates", shortlistedCount},
        {"underReview", underReviewCount}
    };

    ApiResponse::sendSuccess(res, data, "Recruiter dashboard data retrieved.", 200, corsOrigin);
}
