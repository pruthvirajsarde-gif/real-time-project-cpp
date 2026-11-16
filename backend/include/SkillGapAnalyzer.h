#pragma once

#include "Resume.h"
#include "Job.h"

#include <string>
#include <vector>

// ============================================================
// JobSkillGapSummary — Holds multi-job skill gap comparison metrics
// ============================================================
struct JobSkillGapSummary {
    std::string jobId;
    std::string jobTitle;
    std::string company;
    double matchScore;
    double skillCoverage;
    double skillGap;
    int missingRequiredCount;
    int missingPreferredCount;
    double jobReadiness;

    JobSkillGapSummary()
        : jobId(""), jobTitle(""), company(""), matchScore(0.0),
          skillCoverage(0.0), skillGap(0.0), missingRequiredCount(0),
          missingPreferredCount(0), jobReadiness(0.0) {}
};

// ============================================================
// SkillGapAnalyzer — Identifies skill coverage and missing skills
// ============================================================
class SkillGapAnalyzer {
public:
    // Required skills in the job not present in the candidate's resume
    static std::vector<std::string> findMissingRequiredSkills(
        const Resume& resume, const Job& job);

    // Preferred skills in the job not present in the candidate's resume
    static std::vector<std::string> findMissingPreferredSkills(
        const Resume& resume, const Job& job);

    // Skills present in both the resume and the job (required + preferred)
    static std::vector<std::string> findMatchingSkills(
        const Resume& resume, const Job& job);

    // Skill coverage: (matched unique skills / total unique job skills) * 100%
    // Returns 100.0% if job has no required or preferred skills. Clamped to [0, 100].
    static double calculateSkillCoveragePercentage(
        const Resume& resume, const Job& job);

    // Skill gap: 100% - Skill Coverage (clamped to [0, 100])
    static double calculateSkillGapPercentage(
        const Resume& resume, const Job& job);

    // Display formatted skill gap report for a single job
    static void displaySkillGapAnalysis(
        const Resume& resume, const Job& job);

    // Generate multi-job skill gap summary sorted by match score descending
    static std::vector<JobSkillGapSummary> generateMultiJobGapSummary(
        const Resume& resume, const std::vector<Job>& jobs);

    // Display comparison table of skill gaps across all loaded jobs
    static void displayMultiJobGapComparison(
        const Resume& resume, const std::vector<Job>& jobs);
};
