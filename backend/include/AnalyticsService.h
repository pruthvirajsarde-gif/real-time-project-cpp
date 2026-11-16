#pragma once

#include "Resume.h"
#include "Job.h"
#include "ApplicationManager.h"

#include <string>
#include <vector>
#include <map>

// ============================================================
// Data Transfer Structures for Advanced Analytics & Intelligence
// ============================================================

struct CandidateAnalytics {
    int totalApplications = 0;
    int activeApplications = 0;
    int withdrawnApplications = 0;
    std::map<std::string, int> statusBreakdown;
    int savedJobsCount = 0;
    double avgMatchScore = 0.0;
    double highestMatchScore = 0.0;

    // Resume Health Metrics
    double resumeQualityScore = 0.0;
    double skillCoverage = 0.0;
    double experienceCoverage = 0.0;
    double educationCoverage = 0.0;
    double profileCompleteness = 0.0;

    // Career Readiness Indicators
    double overallReadiness = 0.0;
    double skillReadiness = 0.0;
    double experienceReadiness = 0.0;
    double educationReadiness = 0.0;
    std::string readinessLevel; // e.g. "High Readiness", "Moderate Readiness", "Foundational"
    std::vector<std::string> readinessStrengths;
    std::vector<std::string> readinessImprovements;
};

struct MissingSkillDemand {
    std::string skill;
    int jobCount = 0;
    double demandPercentage = 0.0;
    std::string priority; // "HIGH", "MEDIUM", "LOW"
};

struct CandidateSkillGapAnalytics {
    std::vector<std::string> candidateSkills;
    std::vector<std::string> exactMatchesAcrossJobs;
    std::vector<std::string> relatedSkills;
    std::vector<MissingSkillDemand> prioritizedMissingRequired;
    std::vector<MissingSkillDemand> prioritizedMissingPreferred;
    double catalogSkillCoverage = 0.0;
};

struct SkillFrequency {
    std::string skill;
    int totalCount = 0;
    int requiredCount = 0;
    int preferredCount = 0;
    double percentage = 0.0;
};

struct SkillCoOccurrence {
    std::string skillA;
    std::string skillB;
    int count = 0;
};

struct MarketInsights {
    int totalJobs = 0;
    std::vector<SkillFrequency> topSkills;
    std::map<std::string, int> jobsByLocation;
    std::map<std::string, int> jobsByEmploymentType;
    std::vector<SkillCoOccurrence> skillCoOccurrences;
};

struct RecommendedJob {
    std::string jobId;
    std::string jobTitle;
    std::string company;
    std::string location;
    std::string salary;
    double matchScore = 0.0;
    double readinessScore = 0.0;
    std::vector<std::string> explainableReasons;
    std::vector<std::string> topMatchedSkills;
    std::vector<std::string> missingRequiredSkills;
};

struct ResumeJobComparison {
    std::string jobId;
    std::string jobTitle;
    std::string company;
    std::string location;
    double overallScore = 0.0;
    std::string classification;

    // Factor scores
    double skillScore = 0.0;
    double textScore = 0.0;
    double projectScore = 0.0;
    double experienceScore = 0.0;
    double educationScore = 0.0;
    double resumeQualityScore = 0.0;

    // Skills breakdown
    std::vector<std::string> matchedSkills;
    std::vector<std::string> relatedSkills;
    std::vector<std::string> missingRequiredSkills;
    std::vector<std::string> missingPreferredSkills;

    // Education comparison
    std::string requiredEducation;
    std::string candidateEducation;
    std::string educationCompatibility; // "Compatible", "Partially Compatible", "Mismatch / Missing"

    // Experience comparison
    std::string requiredExperience;
    std::string candidateExperience;
    std::string experienceCompatibility; // "Meets/Exceeds Requirement", "Below Requirement", "Not Specified"
};

struct RecruiterAnalytics {
    int activeJobs = 0;
    int totalApplications = 0;
    std::map<std::string, int> pipelineFunnel;
    std::map<std::string, int> matchScoreDistribution;
    double averageMatchScore = 0.0;
};

struct RecruiterJobAnalytics {
    std::string jobId;
    std::string jobTitle;
    std::string company;
    std::string location;
    std::string postedDate;
    std::string status;

    int totalApplicants = 0;
    double averageMatchScore = 0.0;
    double highestMatchScore = 0.0;
    double lowestMatchScore = 0.0;
    double averageSkillCoverage = 0.0;

    std::map<std::string, int> stageDistribution;
    std::map<std::string, int> matchDistribution;
};

struct CandidateJobComparisonRow {
    int candidateId = 0;
    std::string candidateName;
    int applicationId = 0;
    std::string appliedDate;
    std::string status;
    double matchScore = 0.0;
    double readinessScore = 0.0;
    int requiredSkillsMatched = 0;
    int totalRequiredSkills = 0;
    int preferredSkillsMatched = 0;
    int totalPreferredSkills = 0;
    std::string totalExperience;
    std::string educationSummary;
    std::vector<std::string> matchedSkills;
    std::vector<std::string> missingSkills;
};

// ============================================================
// AnalyticsService — Deterministic domain analytics, career readiness,
// market insights, and multi-candidate comparison engine.
// ============================================================
class AnalyticsService {
public:
    // Candidate analytics
    static CandidateAnalytics getCandidateAnalytics(int candidateId);
    static CandidateSkillGapAnalytics getCandidateSkillGapAnalytics(int candidateId);
    static MarketInsights getMarketInsights();
    static std::vector<RecommendedJob> getCandidateRecommendations(int candidateId, size_t limit = 10);
    static ResumeJobComparison compareResumeToJob(int candidateId, const std::string& jobId, bool& outFound);

    // Recruiter analytics
    static RecruiterAnalytics getRecruiterAnalytics(int recruiterAccountId);
    static RecruiterJobAnalytics getRecruiterJobAnalytics(
        int recruiterAccountId, const std::string& jobId, bool& outFound, bool& outOwned);
    static std::vector<CandidateJobComparisonRow> compareJobCandidates(
        int recruiterAccountId, const std::string& jobId, bool& outFound, bool& outOwned);
};
