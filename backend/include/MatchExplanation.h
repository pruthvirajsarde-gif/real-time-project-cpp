#pragma once

#include "Job.h"

#include <string>
#include <vector>

// ============================================================
// MatchExplanation — Explainable breakdown of candidate-to-job
// compatibility, detailing exact/related skills, missing skills,
// project relevance, experience relevance, strengths, and suggestions.
// ============================================================
struct MatchExplanation {
    double overallScore = 0.0;
    double skillScore = 0.0;
    double textScore = 0.0;
    double projectScore = 0.0;
    double experienceScore = 0.0;
    double educationScore = 0.0;
    double resumeQualityScore = 0.0;

    std::vector<std::string> exactMatches;
    std::vector<std::string> relatedMatches; // Formatted as "CandidateTech -> JobTech"
    std::vector<std::string> missingRequiredSkills;
    std::vector<std::string> missingPreferredSkills;

    std::vector<std::string> relevantKeywords;
    std::vector<std::string> relevantProjects;
    std::vector<std::string> relevantExperiences;

    std::vector<std::string> strengths;
    std::vector<std::string> improvementAreas;
    std::string matchClassification;

    // Helper methods
    static std::string getClassificationLabel(double score);
    static void displayMatchExplanation(const MatchExplanation& explanation, const Job& job, const std::string& candidateName);
    static void displayMatcherComparison(const MatchExplanation& advancedExpl, double classicScore, const Job& job);
};
