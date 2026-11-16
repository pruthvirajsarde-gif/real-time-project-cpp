#include "CandidateController.h"
#include "ApiResponse.h"
#include "ApiMiddleware.h"
#include "CandidateRepository.h"
#include "AuditLogger.h"
#include <json.hpp>

void CandidateController::getMyProfile(
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

    if (ctx.candidateId <= 0) {
        ApiResponse::sendError(res, "CANDIDATE_NOT_LINKED", "No candidate profile linked to this account.", 404, corsOrigin);
        return;
    }

    Profile profile = CandidateRepository::getCandidate(ctx.candidateId);
    nlohmann::json data = {
        {"id", ctx.candidateId},
        {"name", profile.getName()},
        {"email", profile.getEmail()},
        {"phone", profile.getPhone()},
        {"location", profile.getLocation()},
        {"linkedin", profile.getLinkedin()},
        {"github", profile.getGithub()},
        {"portfolio", profile.getPortfolio()}
    };

    ApiResponse::sendSuccess(res, data, "Candidate profile retrieved.", 200, corsOrigin);
}

void CandidateController::updateMyProfile(
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

    if (ctx.candidateId <= 0) {
        ApiResponse::sendError(res, "CANDIDATE_NOT_LINKED", "No candidate profile linked to this account.", 404, corsOrigin);
        return;
    }

    nlohmann::json body;
    try {
        body = nlohmann::json::parse(req.body);
    } catch (...) {
        ApiResponse::sendError(res, "BAD_REQUEST_MALFORMED_JSON", "Malformed JSON body.", 400, corsOrigin);
        return;
    }

    Profile profile = CandidateRepository::getCandidate(ctx.candidateId);
    if (body.contains("name")) profile.setName(body["name"].get<std::string>());
    if (body.contains("email")) profile.setEmail(body["email"].get<std::string>());
    if (body.contains("phone")) profile.setPhone(body["phone"].get<std::string>());
    if (body.contains("location")) profile.setLocation(body["location"].get<std::string>());
    if (body.contains("linkedin")) profile.setLinkedin(body["linkedin"].get<std::string>());
    if (body.contains("github")) profile.setGithub(body["github"].get<std::string>());
    if (body.contains("portfolio")) profile.setPortfolio(body["portfolio"].get<std::string>());

    bool ok = CandidateRepository::updateCandidate(ctx.candidateId, profile);
    if (!ok) {
        ApiResponse::sendError(res, "PROFILE_UPDATE_FAILED", "Failed to update profile.", 500, corsOrigin);
        return;
    }

    AuditLogger::log(ctx.accountId, AuditLogger::ACTION_PROFILE_UPDATED, "Profile updated via REST API");

    nlohmann::json data = {
        {"id", ctx.candidateId},
        {"name", profile.getName()},
        {"email", profile.getEmail()},
        {"phone", profile.getPhone()},
        {"location", profile.getLocation()},
        {"linkedin", profile.getLinkedin()},
        {"github", profile.getGithub()},
        {"portfolio", profile.getPortfolio()}
    };

    ApiResponse::sendSuccess(res, data, "Profile updated successfully.", 200, corsOrigin);
}
