#include "SkillGapAnalyzer.h"
#include "ResumeAnalyzer.h"
#include "ResumeStrengthAnalyzer.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <set>
#include <cctype>

// Helper to convert string to lower-case for comparison
static std::string toLowerString(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return lower;
}

std::vector<std::string> SkillGapAnalyzer::findMissingRequiredSkills(
        const Resume& resume, const Job& job) {
    return ResumeAnalyzer::getMissingRequiredSkills(resume, job);
}

std::vector<std::string> SkillGapAnalyzer::findMissingPreferredSkills(
        const Resume& resume, const Job& job) {
    return ResumeAnalyzer::getMissingPreferredSkills(resume, job);
}

std::vector<std::string> SkillGapAnalyzer::findMatchingSkills(
        const Resume& resume, const Job& job) {
    return ResumeAnalyzer::getMatchingSkills(resume, job);
}

double SkillGapAnalyzer::calculateSkillCoveragePercentage(
        const Resume& resume, const Job& job) {
    // Gather all unique job skills (case-insensitive)
    std::set<std::string> uniqueJobSkillsLower;
    for (const auto& s : job.getRequiredSkills()) {
        if (!s.empty()) uniqueJobSkillsLower.insert(toLowerString(s));
    }
    for (const auto& s : job.getPreferredSkills()) {
        if (!s.empty()) uniqueJobSkillsLower.insert(toLowerString(s));
    }

    if (uniqueJobSkillsLower.empty()) {
        return 100.0;
    }

    // Count how many unique job skills the candidate has
    int matchedCount = 0;
    for (const auto& jobSkillLower : uniqueJobSkillsLower) {
        for (const auto& candidateSkill : resume.getSkills()) {
            if (toLowerString(candidateSkill.getName()) == jobSkillLower) {
                ++matchedCount;
                break;
            }
        }
    }

    double coverage = (static_cast<double>(matchedCount) / static_cast<double>(uniqueJobSkillsLower.size())) * 100.0;
    if (coverage < 0.0) coverage = 0.0;
    if (coverage > 100.0) coverage = 100.0;
    return coverage;
}

double SkillGapAnalyzer::calculateSkillGapPercentage(
        const Resume& resume, const Job& job) {
    double coverage = calculateSkillCoveragePercentage(resume, job);
    double gap = 100.0 - coverage;
    if (gap < 0.0) gap = 0.0;
    if (gap > 100.0) gap = 100.0;
    return gap;
}

void SkillGapAnalyzer::displaySkillGapAnalysis(
        const Resume& resume, const Job& job) {
    auto matching = findMatchingSkills(resume, job);
    auto missingReq = findMissingRequiredSkills(resume, job);
    auto missingPref = findMissingPreferredSkills(resume, job);
    double coverage = calculateSkillCoveragePercentage(resume, job);
    double gap = calculateSkillGapPercentage(resume, job);

    std::cout << "\n========================================\n";
    std::cout << "          SKILL GAP ANALYSIS            \n";
    std::cout << "========================================\n\n";

    std::cout << "Job:\n" << job.getJobTitle();
    if (!job.getCompany().empty()) {
        std::cout << " (" << job.getCompany() << ")";
    }
    std::cout << "\n\n";

    std::cout << "Matching Skills:\n";
    if (matching.empty()) {
        std::cout << "  (None)\n";
    } else {
        for (const auto& s : matching) {
            std::cout << "  ✓ " << s << "\n";
        }
    }
    std::cout << "\n";

    std::cout << "Missing Required Skills:\n";
    if (missingReq.empty()) {
        std::cout << "  ✓ All required skills matched!\n";
    } else {
        for (const auto& s : missingReq) {
            std::cout << "  ✗ " << s << "\n";
        }
    }
    std::cout << "\n";

    std::cout << "Missing Preferred Skills:\n";
    if (missingPref.empty()) {
        std::cout << "  ✓ All preferred skills matched!\n";
    } else {
        for (const auto& s : missingPref) {
            std::cout << "  - " << s << "\n";
        }
    }
    std::cout << "\n";

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Skill Coverage:\n" << coverage << "%\n\n";
    std::cout << "Skill Gap:\n" << gap << "%\n";
    std::cout << "========================================\n";
}

std::vector<JobSkillGapSummary> SkillGapAnalyzer::generateMultiJobGapSummary(
        const Resume& resume, const std::vector<Job>& jobs) {
    std::vector<JobSkillGapSummary> summaries;
    double resumeQuality = ResumeStrengthAnalyzer::calculateResumeQualityScore(resume);

    for (const auto& job : jobs) {
        JobSkillGapSummary summary;
        summary.jobId = job.getJobId();
        summary.jobTitle = job.getJobTitle();
        summary.company = job.getCompany();

        summary.matchScore = ResumeAnalyzer::calculateOverallMatchScore(resume, job);
        summary.skillCoverage = calculateSkillCoveragePercentage(resume, job);
        summary.skillGap = calculateSkillGapPercentage(resume, job);

        summary.missingRequiredCount = static_cast<int>(findMissingRequiredSkills(resume, job).size());
        summary.missingPreferredCount = static_cast<int>(findMissingPreferredSkills(resume, job).size());

        double skillScore = ResumeAnalyzer::calculateSkillMatchPercentage(resume, job);
        double eduScore = ResumeAnalyzer::calculateEducationMatchPercentage(resume, job);
        double expScore = ResumeAnalyzer::calculateExperienceMatchPercentage(resume, job);

        double readiness = (skillScore * 0.50) + (eduScore * 0.20) + (expScore * 0.20) + (resumeQuality * 0.10);
        if (readiness < 0.0) readiness = 0.0;
        if (readiness > 100.0) readiness = 100.0;
        summary.jobReadiness = readiness;

        summaries.push_back(summary);
    }

    // Sort descending by matchScore to match Phase 2 ranking
    std::sort(summaries.begin(), summaries.end(),
              [](const JobSkillGapSummary& a, const JobSkillGapSummary& b) {
                  return a.matchScore > b.matchScore;
              });

    return summaries;
}

void SkillGapAnalyzer::displayMultiJobGapComparison(
        const Resume& resume, const std::vector<Job>& jobs) {
    auto summaries = generateMultiJobGapSummary(resume, jobs);

    std::cout << "\n========================================\n";
    std::cout << "      JOB GAP COMPARISON                \n";
    std::cout << "========================================\n";

    std::cout << std::fixed << std::setprecision(2);
    for (const auto& s : summaries) {
        std::cout << "\n" << s.jobTitle;
        if (!s.company.empty()) {
            std::cout << " (" << s.company << ")";
        }
        std::cout << "\n";
        std::cout << "  Match: " << s.matchScore << "%\n";
        std::cout << "  Skill Coverage: " << s.skillCoverage << "%\n";
        std::cout << "  Skill Gap: " << s.skillGap << "%\n";
        std::cout << "  Missing Required: " << s.missingRequiredCount << "\n";
        std::cout << "  Missing Preferred: " << s.missingPreferredCount << "\n";
        std::cout << "  Job Readiness: " << s.jobReadiness << "%\n";
    }
    std::cout << "========================================\n";
}
