#include "ResumeController.h"
#include "ApiResponse.h"
#include "ApiMiddleware.h"
#include "ResumeInputManager.h"
#include "ResumeDocumentReader.h"
#include "ResumeRepository.h"
#include "ResumeStrengthAnalyzer.h"
#include "AuditLogger.h"
#include "SecurityUtils.h"
#include <json.hpp>
#include <fstream>
#include <algorithm>
#include <chrono>

namespace {
std::string getExtensionLower(const std::string& path) {
    auto dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) return "";
    std::string ext = path.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    return ext;
}

bool isSafeFilename(const std::string& filename) {
    if (filename.find("..") != std::string::npos) return false;
    if (filename.find('/') != std::string::npos) return false;
    if (filename.find('\\') != std::string::npos) return false;
    return true;
}

nlohmann::json serializeResume(const Resume& resume, double qualityScore = 0.0) {
    nlohmann::json j;
    j["contact"] = {
        {"name", resume.getName()},
        {"email", resume.getEmail()},
        {"phone", resume.getPhone()},
        {"location", resume.getLocation()},
        {"linkedin", resume.getLinkedin()},
        {"github", resume.getGithub()},
        {"portfolio", resume.getPortfolio()}
    };

    nlohmann::json skillsArr = nlohmann::json::array();
    for (const auto& s : resume.getSkills()) {
        nlohmann::json item;
        item["name"] = s.getName();
        item["category"] = s.getCategory();
        skillsArr.push_back(item);
    }
    j["skills"] = skillsArr;

    nlohmann::json eduArr = nlohmann::json::array();
    for (const auto& e : resume.getEducation()) {
        nlohmann::json item;
        item["degree"] = e.getDegree();
        item["institution"] = e.getInstitution();
        item["fieldOfStudy"] = e.getFieldOfStudy();
        item["startYear"] = e.getStartYear();
        item["endYear"] = e.getEndYear();
        item["score"] = e.getPercentageOrCGPA();
        eduArr.push_back(item);
    }
    j["education"] = eduArr;

    nlohmann::json expArr = nlohmann::json::array();
    for (const auto& ex : resume.getExperience()) {
        nlohmann::json techs = nlohmann::json::array();
        for (const auto& t : ex.getTechnologies()) techs.push_back(t);
        nlohmann::json resps = nlohmann::json::array();
        for (const auto& r : ex.getResponsibilities()) resps.push_back(r);

        nlohmann::json item;
        item["company"] = ex.getCompany();
        item["position"] = ex.getPosition();
        item["duration"] = ex.getDuration();
        item["responsibilities"] = resps;
        item["technologies"] = techs;
        expArr.push_back(item);
    }
    j["experience"] = expArr;

    nlohmann::json projArr = nlohmann::json::array();
    for (const auto& p : resume.getProjects()) {
        nlohmann::json techs = nlohmann::json::array();
        for (const auto& t : p.getTechnologies()) techs.push_back(t);

        nlohmann::json item;
        item["name"] = p.getProjectName();
        item["description"] = p.getDescription();
        item["role"] = p.getRole();
        item["duration"] = p.getDuration();
        item["technologies"] = techs;
        projArr.push_back(item);
    }
    j["projects"] = projArr;

    nlohmann::json certArr = nlohmann::json::array();
    for (const auto& c : resume.getCertifications()) {
        nlohmann::json item;
        item["name"] = c.getCertificationName();
        item["issuer"] = c.getIssuingOrganization();
        item["year"] = c.getYear();
        certArr.push_back(item);
    }
    j["certifications"] = certArr;
    j["qualityScore"] = qualityScore;
    j["completionPercentage"] = resume.calculateCompletionPercentage();

    return j;
}
} // anonymous namespace

void ResumeController::uploadResume(
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

    if (!req.form.has_file("resume")) {
        ApiResponse::sendError(res, "VALIDATION_MISSING_FILE", "Missing 'resume' multipart file.", 400, corsOrigin);
        return;
    }

    auto file = req.form.get_file("resume");

    // Check file size (10 MB limit)
    constexpr size_t MAX_SIZE = 10 * 1024 * 1024;
    if (file.content.size() > MAX_SIZE) {
        ApiResponse::sendError(res, "PAYLOAD_TOO_LARGE", "File size exceeds 10MB limit.", 413, corsOrigin);
        return;
    }

    if (file.content.empty()) {
        ApiResponse::sendError(res, "VALIDATION_EMPTY_FILE", "Uploaded file is empty.", 400, corsOrigin);
        return;
    }

    // Path traversal check
    if (!isSafeFilename(file.filename)) {
        ApiResponse::sendError(res, "VALIDATION_MALICIOUS_PATH", "Path traversal detected in filename.", 400, corsOrigin);
        return;
    }

    std::string ext = getExtensionLower(file.filename);
    if (!ResumeDocumentReader::isSupportedFile(file.filename)) {
        ApiResponse::sendError(res, "UNSUPPORTED_FILE_TYPE", "Supported formats: .pdf, .txt, .docx, .doc, .rtf, .md.", 400, corsOrigin);
        return;
    }

    // Generate safe unique filename
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    std::string safeFilename = "resume_cand_" + std::to_string(ctx.candidateId) + "_" +
                               std::to_string(timestamp) + ext;
    std::string uploadDir = "data/uploads/";
    std::string fullUploadPath = uploadDir + safeFilename;

    std::ofstream out(fullUploadPath, std::ios::binary);
    if (!out.is_open()) {
        ApiResponse::sendError(res, "UPLOAD_STORAGE_FAILED", "Failed to store uploaded file on server.", 500, corsOrigin);
        return;
    }
    out.write(file.content.data(), file.content.size());
    out.close();

    Resume resume = ResumeInputManager::loadResume(fullUploadPath);
    if (!ResumeInputManager::validateResume(resume)) {
        ApiResponse::sendError(res, "RESUME_PARSE_FAILED", "File uploaded but resume text could not be parsed into valid structure.", 422, corsOrigin);
        return;
    }

    double qualityScore = ResumeStrengthAnalyzer::calculateResumeQualityScore(resume);
    std::string format = (ext == ".pdf" ? "PDF" : ((ext == ".docx" || ext == ".doc") ? "DOCX" : "TXT"));

    bool saved = ResumeRepository::saveResume(ctx.candidateId, resume, fullUploadPath, format, qualityScore);
    if (!saved) {
        ApiResponse::sendError(res, "DATABASE_ERROR", "Failed to persist resume record to database.", 500, corsOrigin);
        return;
    }

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_RESUME_UPLOADED, "Uploaded: " + safeFilename);

    auto breakdown = ResumeStrengthAnalyzer::getQualityBreakdown(resume);
    auto strengths = ResumeStrengthAnalyzer::getStrengths(resume);

    nlohmann::json data = {
        {"candidateId", ctx.candidateId},
        {"candidateName", resume.getName()},
        {"email", resume.getEmail()},
        {"phone", resume.getPhone()},
        {"location", resume.getLocation()},
        {"filename", safeFilename},
        {"format", format},
        {"qualityScore", qualityScore},
        {"skillsCount", resume.getSkills().size()},
        {"projectsCount", resume.getProjects().size()},
        {"experienceCount", resume.getExperience().size()},
        {"educationCount", resume.getEducation().size()},
        {"certificationsCount", resume.getCertifications().size()},
        {"breakdown", {
            {"profileScore", breakdown.profileScore},
            {"skillsScore", breakdown.skillsScore},
            {"educationScore", breakdown.educationScore},
            {"projectsScore", breakdown.projectsScore},
            {"experienceScore", breakdown.experienceScore},
            {"certificationsScore", breakdown.certificationsScore},
            {"totalQualityScore", breakdown.totalQualityScore}
        }},
        {"strengths", strengths}
    };

    ApiResponse::sendSuccess(res, data, "Resume uploaded, deeply parsed, and analyzed successfully.", 201, corsOrigin);
}

void ResumeController::importResume(
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

    std::string filePath = body.value("filePath", "");
    if (filePath.empty()) {
        ApiResponse::sendError(res, "VALIDATION_FAILED", "Field 'filePath' is required.", 400, corsOrigin);
        return;
    }

    // Path traversal check
    if (filePath.find("..") != std::string::npos) {
        ApiResponse::sendError(res, "VALIDATION_MALICIOUS_PATH", "Path traversal detected in filePath.", 400, corsOrigin);
        return;
    }

    std::string ext = getExtensionLower(filePath);
    if (!ResumeDocumentReader::isSupportedFile(filePath)) {
        ApiResponse::sendError(res, "UNSUPPORTED_FILE_TYPE", "Supported formats: .pdf, .txt, .docx, .doc, .rtf, .md.", 400, corsOrigin);
        return;
    }

    Resume resume = ResumeInputManager::loadResume(filePath);
    if (!ResumeInputManager::validateResume(resume)) {
        ApiResponse::sendError(res, "RESUME_PARSE_FAILED", "Resume file could not be parsed into valid structure.", 422, corsOrigin);
        return;
    }

    double qualityScore = ResumeStrengthAnalyzer::calculateResumeQualityScore(resume);
    std::string format = (ext == ".pdf" ? "PDF" : ((ext == ".docx" || ext == ".doc") ? "DOCX" : "TXT"));

    bool saved = ResumeRepository::saveResume(ctx.candidateId, resume, filePath, format, qualityScore);
    if (!saved) {
        ApiResponse::sendError(res, "DATABASE_ERROR", "Failed to persist resume record to database.", 500, corsOrigin);
        return;
    }

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_RESUME_UPLOADED, "Imported: " + filePath);

    auto breakdown = ResumeStrengthAnalyzer::getQualityBreakdown(resume);
    auto strengths = ResumeStrengthAnalyzer::getStrengths(resume);

    nlohmann::json data = {
        {"candidateId", ctx.candidateId},
        {"candidateName", resume.getName()},
        {"email", resume.getEmail()},
        {"phone", resume.getPhone()},
        {"location", resume.getLocation()},
        {"sourcePath", filePath},
        {"format", format},
        {"qualityScore", qualityScore},
        {"skillsCount", resume.getSkills().size()},
        {"projectsCount", resume.getProjects().size()},
        {"experienceCount", resume.getExperience().size()},
        {"educationCount", resume.getEducation().size()},
        {"certificationsCount", resume.getCertifications().size()},
        {"breakdown", {
            {"profileScore", breakdown.profileScore},
            {"skillsScore", breakdown.skillsScore},
            {"educationScore", breakdown.educationScore},
            {"projectsScore", breakdown.projectsScore},
            {"experienceScore", breakdown.experienceScore},
            {"certificationsScore", breakdown.certificationsScore},
            {"totalQualityScore", breakdown.totalQualityScore}
        }},
        {"strengths", strengths}
    };

    ApiResponse::sendSuccess(res, data, "Resume imported, deeply parsed, and analyzed successfully.", 200, corsOrigin);
}

void ResumeController::getResume(
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
        ApiResponse::sendError(res, "RESUME_NOT_FOUND", "No resume found for candidate.", 404, corsOrigin);
        return;
    }

    Resume resume = ResumeRepository::loadResume(ctx.candidateId);
    double quality = ResumeRepository::getResumeQualityScore(ctx.candidateId);
    nlohmann::json data = serializeResume(resume, quality);

    ApiResponse::sendSuccess(res, data, "Resume retrieved.", 200, corsOrigin);
}

void ResumeController::getAnalysis(
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
        ApiResponse::sendError(res, "RESUME_NOT_FOUND", "No resume found for candidate.", 404, corsOrigin);
        return;
    }

    Resume resume = ResumeRepository::loadResume(ctx.candidateId);
    double totalQuality = ResumeStrengthAnalyzer::calculateResumeQualityScore(resume);
    auto breakdown = ResumeStrengthAnalyzer::getQualityBreakdown(resume);

    nlohmann::json data = {
        {"totalQualityScore", totalQuality},
        {"overallScore", totalQuality},
        {"classification", (totalQuality >= 80.0 ? "Strong Resume" : (totalQuality >= 60.0 ? "Moderate Resume" : "Needs Optimization"))},
        {"completionPercentage", resume.calculateCompletionPercentage()},
        {"breakdown", {
            {"profileScore", breakdown.profileScore},
            {"skillsScore", breakdown.skillsScore},
            {"educationScore", breakdown.educationScore},
            {"projectsScore", breakdown.projectsScore},
            {"experienceScore", breakdown.experienceScore},
            {"certificationsScore", breakdown.certificationsScore},
            {"skillsFactor", std::round((breakdown.skillsScore / 20.0) * 100.0)},
            {"experienceFactor", std::round((breakdown.experienceScore / 20.0) * 100.0)},
            {"educationFactor", std::round((breakdown.educationScore / 15.0) * 100.0)},
            {"projectsFactor", std::round((breakdown.projectsScore / 20.0) * 100.0)},
            {"certificationsFactor", std::round((breakdown.certificationsScore / 10.0) * 100.0)},
            {"profileFactor", std::round((breakdown.profileScore / 15.0) * 100.0)}
        }}
    };

    ApiResponse::sendSuccess(res, data, "Resume quality analysis retrieved.", 200, corsOrigin);
}

void ResumeController::getStrengths(
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
        ApiResponse::sendError(res, "RESUME_NOT_FOUND", "No resume found for candidate.", 404, corsOrigin);
        return;
    }

    Resume resume = ResumeRepository::loadResume(ctx.candidateId);
    std::vector<std::string> strengths = ResumeStrengthAnalyzer::getStrengths(resume);
    std::vector<std::string> weakAreas = ResumeStrengthAnalyzer::getWeakAreas(resume);
    auto techFreq = ResumeStrengthAnalyzer::calculateTechnologyFrequency(resume);

    nlohmann::json freqObj = nlohmann::json::object();
    for (const auto& [tech, count] : techFreq) {
        freqObj[tech] = count;
    }

    nlohmann::json data = {
        {"strengths", strengths},
        {"weakAreas", weakAreas},
        {"technologyFrequency", freqObj},
        {"techFrequencies", freqObj}
    };

    ApiResponse::sendSuccess(res, data, "Resume strengths retrieved.", 200, corsOrigin);
}

void ResumeController::getRecommendations(
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
        ApiResponse::sendError(res, "RESUME_NOT_FOUND", "No resume found for candidate.", 404, corsOrigin);
        return;
    }

    Resume resume = ResumeRepository::loadResume(ctx.candidateId);
    std::vector<std::string> weakAreas = ResumeStrengthAnalyzer::getWeakAreas(resume);

    nlohmann::json recommendations = nlohmann::json::array();
    for (const auto& w : weakAreas) {
        recommendations.push_back("Opportunity: " + w);
    }
    if (resume.getCertifications().empty()) {
        recommendations.push_back("Add industry-recognized certifications to increase credibility.");
    }
    if (resume.getProjects().size() < 2) {
        recommendations.push_back("Add more showcase projects demonstrating hands-on technical abilities.");
    }

    nlohmann::json data = {
        {"recommendations", recommendations}
    };

    ApiResponse::sendSuccess(res, data, "Resume recommendations retrieved.", 200, corsOrigin);
}
