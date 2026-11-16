#pragma once

#include "Resume.h"
#include "Job.h"
#include "ResumeAnalyzer.h"

#include <string>
#include <vector>

// ============================================================
// JobMatchResult — Holds the result of matching a resume to a job
// ============================================================
struct JobMatchResult {
    std::string jobId;
    std::string jobTitle;
    std::string company;
    double score;

    JobMatchResult()
        : jobId(""), jobTitle(""), company(""), score(0.0) {}

    JobMatchResult(const std::string& jobId, const std::string& jobTitle,
                   const std::string& company, double score)
        : jobId(jobId), jobTitle(jobTitle), company(company), score(score) {}
};

// ============================================================
// JobMatcher — Compares one resume against multiple jobs
// ============================================================
// Uses ResumeAnalyzer internally to calculate match scores,
// then sorts and ranks the results.
// ============================================================

class JobMatcher {
public:
    // Calculate match scores for all jobs (returns parallel vector)
    static std::vector<double> calculateJobScores(
        const Resume& resume, const std::vector<Job>& jobs);

    // Get sorted match results (highest score first)
    static std::vector<JobMatchResult> getMatchResults(
        const Resume& resume, const std::vector<Job>& jobs);

    // Get top N matches (safely handles edge cases)
    static std::vector<JobMatchResult> getTopMatches(
        const Resume& resume, const std::vector<Job>& jobs, int count);

    // Display all job matches ranked by score
    static void displayJobMatches(
        const Resume& resume, const std::vector<Job>& jobs);

    // Display full analysis summary with statistics
    static void displayAnalysisSummary(
        const Resume& resume, const std::vector<Job>& jobs);
};
