#pragma once

#include "Resume.h"
#include "Job.h"
#include "MatchExplanation.h"

#include <vector>
#include <utility>

// ============================================================
// AdvancedJobMatcher — Multi-faceted deterministic matching engine.
// Incorporates Skill Normalization, Semantic-Style Text Similarity,
// Project Alignment, Experience Keyword Overlap, Education, and Quality.
// Generates explainable MatchExplanation records and rankings.
// ============================================================
class AdvancedJobMatcher {
public:
    // Generate explainable evaluation for a single resume against a job
    static MatchExplanation explainMatch(const Resume& resume, const Job& job);

    // Calculate advanced multi-factor match score (0.0 to 100.0)
    static double calculateAdvancedMatchScore(const Resume& resume, const Job& job);

    // Rank multiple jobs descending by advanced score
    static std::vector<std::pair<Job, MatchExplanation>> rankJobsAdvanced(
        const Resume& resume,
        const std::vector<Job>& jobs
    );

    // Get top N advanced matches
    static std::vector<std::pair<Job, MatchExplanation>> getTopMatchesAdvanced(
        const Resume& resume,
        const std::vector<Job>& jobs,
        int count
    );

    // Display formatted table of advanced job rankings
    static void displayAdvancedJobRankings(const Resume& resume, const std::vector<Job>& jobs);

    // Display full match explanation for a chosen job
    static void displayAdvancedMatchDetails(const Resume& resume, const Job& job);

    // Side-by-side diagnostic comparison between Phase 2 classic matcher and Phase 7 advanced matcher
    static void displayComparisonWithClassic(const Resume& resume, const Job& job);
};
