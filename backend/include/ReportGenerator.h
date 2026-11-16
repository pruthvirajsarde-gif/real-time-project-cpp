#pragma once

#include "Resume.h"
#include "Job.h"
#include "JobMatcher.h"

#include <string>
#include <vector>

// ============================================================
// ReportGenerator — Saves comprehensive candidate analysis and
// matching reports to a text file.
// ============================================================
class ReportGenerator {
public:
    // Save analysis report to the given file path.
    // If jobs vector is provided, appends Phase 4/5 Smart Resume Analysis,
    // Technology Frequency, and Job-Specific Readiness/Recommendations.
    // If resumeSourcePath is provided, includes document source & format metadata.
    // If candidateId > 0, includes Phase 6 DATABASE SUMMARY section.
    static bool saveAnalysisReport(
        const std::string& filePath,
        const Resume& resume,
        const std::vector<JobMatchResult>& results,
        const std::vector<Job>& jobs = {},
        const std::string& resumeSourcePath = "",
        int candidateId = -1,
        const std::string& accountRole = "",
        const std::string& authenticatedUser = "");
};
