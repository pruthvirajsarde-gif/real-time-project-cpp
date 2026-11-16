#pragma once

#include "Job.h"
#include <string>
#include <vector>

// ============================================================
// SavedJobManager — Manages candidate saved/bookmarked jobs
// in the saved_jobs table.
// ============================================================
class SavedJobManager {
public:
    // Save a job for candidate
    static bool saveJob(int candidateId, const std::string& jobId);

    // Remove a saved job for candidate
    static bool removeSavedJob(int candidateId, const std::string& jobId);

    // Check if a job is saved by candidate
    static bool isJobSaved(int candidateId, const std::string& jobId);

    // Retrieve all saved jobs for a candidate
    static std::vector<Job> getSavedJobs(int candidateId);

    // Count how many jobs are saved by candidate
    static int countSavedJobs(int candidateId);

    // Display formatted saved jobs list for candidate (with optional match scores)
    static void displaySavedJobs(int candidateId);
};
