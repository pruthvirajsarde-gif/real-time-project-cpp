#include "RecommendationEngine.h"
#include "ResumeAnalyzer.h"
#include "ResumeStrengthAnalyzer.h"
#include "SkillGapAnalyzer.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

JobReadinessBreakdown RecommendationEngine::getJobReadinessBreakdown(
        const Resume& resume, const Job& job) {
    JobReadinessBreakdown bd;

    bd.skillScore = ResumeAnalyzer::calculateSkillMatchPercentage(resume, job);
    bd.educationScore = ResumeAnalyzer::calculateEducationMatchPercentage(resume, job);
    bd.experienceScore = ResumeAnalyzer::calculateExperienceMatchPercentage(resume, job);
    bd.resumeQuality = ResumeStrengthAnalyzer::calculateResumeQualityScore(resume);

    double total = (bd.skillScore * 0.50) +
                   (bd.educationScore * 0.20) +
                   (bd.experienceScore * 0.20) +
                   (bd.resumeQuality * 0.10);

    if (total < 0.0) total = 0.0;
    if (total > 100.0) total = 100.0;
    bd.overallReadiness = total;

    return bd;
}

double RecommendationEngine::calculateJobReadinessScore(
        const Resume& resume, const Job& job) {
    return getJobReadinessBreakdown(resume, job).overallReadiness;
}

std::vector<std::string> RecommendationEngine::generateRecommendations(
        const Resume& resume, const Job& job) {
    std::vector<std::string> recommendations;

    // 1. Missing Required Skills (Highest Priority)
    auto missingReq = SkillGapAnalyzer::findMissingRequiredSkills(resume, job);
    for (const auto& s : missingReq) {
        recommendations.push_back("Add " + s + " to your technical skills (Required for this role).");
    }

    // 2. Missing Preferred Skills
    auto missingPref = SkillGapAnalyzer::findMissingPreferredSkills(resume, job);
    for (const auto& s : missingPref) {
        recommendations.push_back("Consider learning " + s + " (Preferred skill for " + job.getJobTitle() + ").");
    }

    // 3. Profile Enhancements
    if (resume.getLinkedin().empty()) {
        recommendations.push_back("Add your LinkedIn professional profile link to increase recruiter visibility.");
    }
    if (resume.getPortfolio().empty()) {
        recommendations.push_back("Add a personal portfolio website to showcase deployed projects.");
    }

    // 4. Project Portfolio
    if (resume.getProjects().size() < 2) {
        std::string tech = !job.getRequiredSkills().empty() ? job.getRequiredSkills()[0] : "core technologies";
        recommendations.push_back("Build and document an additional project highlighting " + tech + ".");
    }

    bool hasShortDesc = false;
    for (const auto& p : resume.getProjects()) {
        if (p.getDescription().size() < 30) {
            hasShortDesc = true;
            break;
        }
    }
    if (hasShortDesc) {
        recommendations.push_back("Add more technical details and measurable outcomes to your project descriptions.");
    }

    // 5. Experience
    double expScore = ResumeAnalyzer::calculateExperienceMatchPercentage(resume, job);
    if (expScore < 100.0 && job.getMinimumExperience() > 0) {
        recommendations.push_back("Gain additional hands-on experience to meet the " +
                                 std::to_string(job.getMinimumExperience()) + " year(s) minimum requirement.");
    }

    bool hasSparseExp = false;
    for (const auto& x : resume.getExperience()) {
        if (x.getResponsibilities().size() < 2) {
            hasSparseExp = true;
            break;
        }
    }
    if (hasSparseExp) {
        recommendations.push_back("Add measurable responsibilities and key achievements to your experience section.");
    }

    // 6. Certifications
    if (resume.getCertifications().empty()) {
        std::string domain = !job.getRequiredSkills().empty() ? job.getRequiredSkills()[0] : "relevant";
        recommendations.push_back("Pursue an industry-recognized certification in " + domain + ".");
    }

    if (recommendations.empty()) {
        recommendations.push_back("Your profile is exceptionally aligned with this role! Customize your resume summary for this application.");
    }

    return recommendations;
}

void RecommendationEngine::displayJobReadiness(
        const Resume& resume, const Job& job) {
    auto bd = getJobReadinessBreakdown(resume, job);

    std::cout << "\n========================================\n";
    std::cout << "          JOB READINESS                 \n";
    std::cout << "========================================\n\n";

    std::cout << "Job:\n" << job.getJobTitle();
    if (!job.getCompany().empty()) {
        std::cout << " (" << job.getCompany() << ")";
    }
    std::cout << "\n\n";

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Skill Match       : " << bd.skillScore << "%\n";
    std::cout << "Education Match   : " << bd.educationScore << "%\n";
    std::cout << "Experience Match  : " << bd.experienceScore << "%\n";
    std::cout << "Resume Quality    : " << bd.resumeQuality << "%\n\n";

    std::cout << "----------------------------------------\n";
    std::cout << "Job Readiness     : " << bd.overallReadiness << "%\n";
    std::cout << "----------------------------------------\n";
    std::cout << "========================================\n";
}

void RecommendationEngine::displayRecommendations(
        const Resume& resume, const Job& job) {
    auto recs = generateRecommendations(resume, job);

    std::cout << "\n========================================\n";
    std::cout << "       SMART RECOMMENDATIONS            \n";
    std::cout << "========================================\n\n";

    std::cout << "Target Job: " << job.getJobTitle();
    if (!job.getCompany().empty()) {
        std::cout << " (" << job.getCompany() << ")";
    }
    std::cout << "\n\n";

    for (size_t i = 0; i < recs.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << recs[i] << "\n";
    }
    std::cout << "========================================\n";
}
