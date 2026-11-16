#include "JobController.h"
#include "ApiResponse.h"
#include "JobRepository.h"
#include <json.hpp>
#include <algorithm>

namespace {
std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool containsCaseInsensitive(const std::string& source, const std::string& target) {
    if (target.empty()) return true;
    return toLower(source).find(toLower(target)) != std::string::npos;
}

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
} // anonymous namespace

void JobController::getJobs(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    std::string title = req.get_param_value("title");
    std::string company = req.get_param_value("company");
    std::string location = req.get_param_value("location");
    std::string skill = req.get_param_value("skill");
    std::string employmentType = req.get_param_value("employmentType");

    int page = 1;
    int limit = 10;
    if (req.has_param("page")) {
        try { page = std::stoi(req.get_param_value("page")); } catch (...) {}
    }
    if (req.has_param("limit")) {
        try { limit = std::stoi(req.get_param_value("limit")); } catch (...) {}
    }

    if (page < 1) page = 1;
    if (limit < 1) limit = 10;
    if (limit > 100) limit = 100; // Enforce maximum limit of 100

    std::vector<Job> allJobs = JobRepository::getAllJobs();
    std::vector<Job> filtered;

    for (const auto& job : allJobs) {
        if (!title.empty() && !containsCaseInsensitive(job.getJobTitle(), title)) continue;
        if (!company.empty() && !containsCaseInsensitive(job.getCompany(), company)) continue;
        if (!location.empty() && !containsCaseInsensitive(job.getLocation(), location)) continue;
        if (!employmentType.empty() && !containsCaseInsensitive(job.getEmploymentType(), employmentType)) continue;

        if (!skill.empty()) {
            bool hasSkill = false;
            for (const auto& s : job.getRequiredSkills()) {
                if (containsCaseInsensitive(s, skill)) { hasSkill = true; break; }
            }
            if (!hasSkill) {
                for (const auto& s : job.getPreferredSkills()) {
                    if (containsCaseInsensitive(s, skill)) { hasSkill = true; break; }
                }
            }
            if (!hasSkill) continue;
        }

        filtered.push_back(job);
    }

    int total = static_cast<int>(filtered.size());
    int totalPages = (total == 0) ? 0 : ((total + limit - 1) / limit);

    int startIdx = (page - 1) * limit;
    int endIdx = std::min(startIdx + limit, total);

    nlohmann::json items = nlohmann::json::array();
    if (startIdx < total) {
        for (int i = startIdx; i < endIdx; ++i) {
            items.push_back(serializeJob(filtered[i]));
        }
    }

    nlohmann::json data = {
        {"items", items},
        {"page", page},
        {"limit", limit},
        {"total", total},
        {"totalPages", totalPages}
    };

    ApiResponse::sendSuccess(res, data, "Jobs retrieved successfully.", 200, corsOrigin);
}

void JobController::getJobById(
    const httplib::Request& req,
    httplib::Response& res,
    const std::string& corsOrigin
) {
    std::string jobId = req.matches[1];
    if (jobId.empty()) {
        ApiResponse::sendError(res, "JOB_INVALID_ID", "Job ID is required in path.", 400, corsOrigin);
        return;
    }

    if (!JobRepository::jobExists(jobId)) {
        ApiResponse::sendError(res, "JOB_NOT_FOUND", "Job '" + jobId + "' not found.", 404, corsOrigin);
        return;
    }

    Job job = JobRepository::getJob(jobId);
    ApiResponse::sendSuccess(res, serializeJob(job), "Job retrieved.", 200, corsOrigin);
}
