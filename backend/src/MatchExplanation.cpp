#include "MatchExplanation.h"

#include <iostream>
#include <iomanip>
#include <cmath>

std::string MatchExplanation::getClassificationLabel(double score) {
    if (score >= 90.0) return "Exceptional Match";
    if (score >= 80.0) return "Strong Match";
    if (score >= 70.0) return "Good Match";
    if (score >= 60.0) return "Moderate Match";
    return "Low Match";
}

void MatchExplanation::displayMatchExplanation(const MatchExplanation& exp,
                                              const Job& job,
                                              const std::string& candidateName) {
    std::cout << "\n========================================\n";
    std::cout << "         MATCH EXPLANATION\n";
    std::cout << "========================================\n\n";

    if (!candidateName.empty()) {
        std::cout << "Candidate:\n" << candidateName << "\n\n";
    }
    std::cout << "Job:\n" << job.getJobTitle() << "\n\n";
    std::cout << "Company:\n" << job.getCompany() << "\n\n";

    std::cout << "----------------------------------------\n";
    std::cout << "OVERALL SCORE\n";
    std::cout << "----------------------------------------\n\n";
    std::cout << std::fixed << std::setprecision(2) << exp.overallScore << "% ("
              << exp.matchClassification << ")\n\n";

    std::cout << "----------------------------------------\n";
    std::cout << "SCORE BREAKDOWN\n";
    std::cout << "----------------------------------------\n\n";
    std::cout << "Skill Match:          " << std::setw(6) << exp.skillScore << "%\n";
    std::cout << "Text Relevance:       " << std::setw(6) << exp.textScore << "%\n";
    std::cout << "Project Relevance:    " << std::setw(6) << exp.projectScore << "%\n";
    std::cout << "Experience Relevance: " << std::setw(6) << exp.experienceScore << "%\n";
    std::cout << "Education Match:      " << std::setw(6) << exp.educationScore << "%\n";
    std::cout << "Resume Quality:       " << std::setw(6) << exp.resumeQualityScore << "%\n\n";

    std::cout << "----------------------------------------\n";
    std::cout << "EXACT SKILLS\n";
    std::cout << "----------------------------------------\n\n";
    if (exp.exactMatches.empty()) {
        std::cout << "None\n\n";
    } else {
        for (const auto& s : exp.exactMatches) {
            std::cout << s << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "RELATED SKILLS\n";
    std::cout << "----------------------------------------\n\n";
    if (exp.relatedMatches.empty()) {
        std::cout << "None\n\n";
    } else {
        for (const auto& s : exp.relatedMatches) {
            std::cout << s << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "MISSING REQUIRED\n";
    std::cout << "----------------------------------------\n\n";
    if (exp.missingRequiredSkills.empty()) {
        std::cout << "None (All required skills satisfied)\n\n";
    } else {
        for (const auto& s : exp.missingRequiredSkills) {
            std::cout << s << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "MISSING PREFERRED\n";
    std::cout << "----------------------------------------\n\n";
    if (exp.missingPreferredSkills.empty()) {
        std::cout << "None\n\n";
    } else {
        for (const auto& s : exp.missingPreferredSkills) {
            std::cout << s << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "RELEVANT PROJECTS\n";
    std::cout << "----------------------------------------\n\n";
    if (exp.relevantProjects.empty()) {
        std::cout << "None detected\n\n";
    } else {
        for (const auto& p : exp.relevantProjects) {
            std::cout << p << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "RELEVANT EXPERIENCE\n";
    std::cout << "----------------------------------------\n\n";
    if (exp.relevantExperiences.empty()) {
        std::cout << "None detected\n\n";
    } else {
        for (const auto& e : exp.relevantExperiences) {
            std::cout << e << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "STRENGTHS\n";
    std::cout << "----------------------------------------\n\n";
    if (exp.strengths.empty()) {
        std::cout << "Profile meets minimum requirements.\n\n";
    } else {
        for (const auto& s : exp.strengths) {
            std::cout << "✓ " << s << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "IMPROVEMENT AREAS\n";
    std::cout << "----------------------------------------\n\n";
    if (exp.improvementAreas.empty()) {
        std::cout << "✓ No critical gaps detected for this role!\n\n";
    } else {
        for (const auto& a : exp.improvementAreas) {
            std::cout << "• " << a << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "========================================\n";
}

void MatchExplanation::displayMatcherComparison(const MatchExplanation& advancedExpl,
                                               double classicScore,
                                               const Job& job) {
    double diff = advancedExpl.overallScore - classicScore;

    std::cout << "\n========================================\n";
    std::cout << "       MATCHER COMPARISON\n";
    std::cout << "========================================\n\n";
    std::cout << "Job: " << job.getJobTitle() << " (" << job.getCompany() << ")\n\n";

    std::cout << "Existing Matcher:\n";
    std::cout << std::fixed << std::setprecision(2) << classicScore << "%\n\n";

    std::cout << "Advanced Matcher:\n";
    std::cout << std::fixed << std::setprecision(2) << advancedExpl.overallScore << "%\n\n";

    std::cout << "Difference:\n";
    if (diff >= 0.0) {
        std::cout << "+" << diff << " percentage points\n\n";
    } else {
        std::cout << diff << " percentage points\n\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "WHY DIFFERENT?\n";
    std::cout << "----------------------------------------\n\n";
    std::cout << "Advanced matcher considers:\n\n";
    std::cout << "✓ normalized skills\n";
    std::cout << "✓ related technologies\n";
    std::cout << "✓ job description relevance\n";
    std::cout << "✓ project relevance\n";
    std::cout << "✓ experience relevance\n";
    std::cout << "✓ keyword frequency\n";
    std::cout << "✓ text similarity\n\n";
    std::cout << "========================================\n";
}
