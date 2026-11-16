#pragma once

#include "Resume.h"
#include "Job.h"

#include <string>
#include <vector>

// ============================================================
// JobReadinessBreakdown — Holds factors contributing to job readiness
// ============================================================
struct JobReadinessBreakdown {
    double skillScore;       // 50%
    double educationScore;   // 20%
    double experienceScore;  // 20%
    double resumeQuality;    // 10%
    double overallReadiness; // 0-100%

    JobReadinessBreakdown()
        : skillScore(0.0), educationScore(0.0), experienceScore(0.0),
          resumeQuality(0.0), overallReadiness(0.0) {}
};

// ============================================================
// RecommendationEngine — Generates data-driven career improvement
// recommendations and calculates job readiness scores
// ============================================================
class RecommendationEngine {
public:
    // Calculates job readiness score based on 50/20/20/10 weighted formula
    static double calculateJobReadinessScore(
        const Resume& resume, const Job& job);

    // Returns full breakdown of the job readiness factors
    static JobReadinessBreakdown getJobReadinessBreakdown(
        const Resume& resume, const Job& job);

    // Generates prioritized recommendations tailored to the resume and job
    static std::vector<std::string> generateRecommendations(
        const Resume& resume, const Job& job);

    // Displays job readiness score breakdown
    static void displayJobReadiness(
        const Resume& resume, const Job& job);

    // Displays prioritized recommendations
    static void displayRecommendations(
        const Resume& resume, const Job& job);
};
