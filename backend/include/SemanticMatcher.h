#pragma once

#include "Resume.h"
#include "Job.h"
#include "MatchExplanation.h"

// ============================================================
// SemanticMatcher — Context-aware evaluation engine for skills,
// text relevance, projects, experiences, and education.
// Produces explainable MatchExplanation breakdowns.
// ============================================================
class SemanticMatcher {
public:
    // Generate comprehensive explainable match evaluation
    static MatchExplanation evaluate(const Resume& resume, const Job& job);

    // Calculate skill score considering exact and related technologies
    static double calculateSkillScore(
        const Resume& resume,
        const Job& job,
        std::vector<std::string>& exactMatches,
        std::vector<std::string>& relatedMatches,
        std::vector<std::string>& missingRequired,
        std::vector<std::string>& missingPreferred
    );

    // Calculate full document text relevance using weighted cosine & TF-IDF
    static double calculateTextRelevance(const Resume& resume, const Job& job);

    // Calculate project relevance and detect relevant projects
    static double calculateProjectRelevance(
        const Resume& resume,
        const Job& job,
        std::vector<std::string>& relevantProjects
    );

    // Calculate experience relevance and detect relevant experiences
    static double calculateExperienceRelevance(
        const Resume& resume,
        const Job& job,
        std::vector<std::string>& relevantExperiences
    );

    // Calculate education alignment
    static double calculateEducationScore(const Resume& resume, const Job& job);
};
