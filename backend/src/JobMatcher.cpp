#include "JobMatcher.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>

// ============================================================
//  Calculate match scores for all jobs
//  Returns a vector of scores parallel to the input jobs vector.
// ============================================================
std::vector<double> JobMatcher::calculateJobScores(
        const Resume& resume, const std::vector<Job>& jobs) {
    std::vector<double> scores;
    scores.reserve(jobs.size());

    for (const auto& job : jobs) {
        scores.push_back(ResumeAnalyzer::calculateOverallMatchScore(resume, job));
    }

    return scores;
}

// ============================================================
//  Get sorted match results (highest score first)
//  Uses STL sort with a lambda comparator.
// ============================================================
std::vector<JobMatchResult> JobMatcher::getMatchResults(
        const Resume& resume, const std::vector<Job>& jobs) {
    std::vector<JobMatchResult> results;
    results.reserve(jobs.size());

    // Build result objects
    for (const auto& job : jobs) {
        double score = ResumeAnalyzer::calculateOverallMatchScore(resume, job);
        results.emplace_back(job.getJobId(), job.getJobTitle(),
                             job.getCompany(), score);
    }

    // Sort by score descending using STL sort + lambda
    std::sort(results.begin(), results.end(),
        [](const JobMatchResult& a, const JobMatchResult& b) {
            return a.score > b.score;
        });

    return results;
}

// ============================================================
//  Get top N matches
//  Safely handles: zero jobs, fewer jobs than requested,
//  invalid count (<=0).
// ============================================================
std::vector<JobMatchResult> JobMatcher::getTopMatches(
        const Resume& resume, const std::vector<Job>& jobs, int count) {
    // Handle invalid count
    if (count <= 0) return {};

    auto results = getMatchResults(resume, jobs);

    // Handle fewer results than requested
    if (static_cast<int>(results.size()) <= count) {
        return results;
    }

    // Return only the top 'count' results
    return std::vector<JobMatchResult>(results.begin(), results.begin() + count);
}

// ============================================================
//  Display all job matches ranked by score
// ============================================================
void JobMatcher::displayJobMatches(
        const Resume& resume, const std::vector<Job>& jobs) {
    if (jobs.empty()) {
        std::cout << "\n  No jobs available for matching.\n";
        return;
    }

    auto results = getMatchResults(resume, jobs);

    std::cout << "\n========================================\n";
    std::cout << "        JOB MATCHING RESULTS            \n";
    std::cout << "========================================\n\n";

    std::cout << std::fixed << std::setprecision(2);

    int rank = 1;
    for (const auto& r : results) {
        std::string category = ResumeAnalyzer::getMatchCategory(r.score);
        std::cout << "  " << rank << ". " << r.jobTitle << "\n";
        std::cout << "     Company     : " << r.company << "\n";
        std::cout << "     Match Score : " << r.score << "%\n";
        std::cout << "     Category    : " << category << "\n\n";
        ++rank;
    }

    std::cout << "========================================\n";
}

// ============================================================
//  Display full analysis summary with statistics
// ============================================================
void JobMatcher::displayAnalysisSummary(
        const Resume& resume, const std::vector<Job>& jobs) {
    if (jobs.empty()) {
        std::cout << "\n  No jobs to analyze.\n";
        return;
    }

    auto results = getMatchResults(resume, jobs);

    // Count categories
    int strongCount  = 0;
    int goodCount    = 0;
    int partialCount = 0;
    int lowCount     = 0;
    double totalScore = 0.0;

    for (const auto& r : results) {
        totalScore += r.score;
        std::string cat = ResumeAnalyzer::getMatchCategory(r.score);
        if (cat == "Strong Match")  ++strongCount;
        else if (cat == "Good Match")    ++goodCount;
        else if (cat == "Partial Match") ++partialCount;
        else                             ++lowCount;
    }

    double avgScore = totalScore / static_cast<double>(results.size());

    std::cout << "\n========================================\n";
    std::cout << "          ANALYSIS SUMMARY              \n";
    std::cout << "========================================\n\n";

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Total Jobs Analyzed : " << results.size() << "\n\n";

    std::cout << "  Strong Matches  : " << strongCount << "\n";
    std::cout << "  Good Matches    : " << goodCount << "\n";
    std::cout << "  Partial Matches : " << partialCount << "\n";
    std::cout << "  Low Matches     : " << lowCount << "\n\n";

    std::cout << "  Average Match Score : " << avgScore << "%\n\n";

    // Top matching job (results are already sorted)
    if (!results.empty()) {
        std::cout << "  Top Matching Job:\n";
        std::cout << "    " << results[0].jobTitle << "\n";
        std::cout << "    Company     : " << results[0].company << "\n";
        std::cout << "    Match Score : " << results[0].score << "%\n";
        std::cout << "    Category    : "
                  << ResumeAnalyzer::getMatchCategory(results[0].score) << "\n";
    }

    std::cout << "\n========================================\n";
}
