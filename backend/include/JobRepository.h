#pragma once

#include "Job.h"
#include <string>
#include <vector>

// ============================================================
// JobRepository — Data access operations for jobs, job_required_skills,
// and job_preferred_skills.
// Uses transactions, prepared statements, and deduplication.
// ============================================================
class JobRepository {
public:
    // Save a new or updated job with required & preferred skills. Uses transactions.
    static bool saveJob(const Job& job);

    // Retrieve all jobs from SQLite with associated skills
    static std::vector<Job> getAllJobs();

    // Retrieve a single job by its unique jobId (e.g. "J001")
    static Job getJob(const std::string& jobId);

    // Check if a job with the given jobId exists
    static bool jobExists(const std::string& jobId);

    // Delete a job and its associated skill links
    static bool deleteJob(const std::string& jobId);

    // Count the total number of jobs in the database
    static int countJobs();

    // Import jobs from file (using JobDataLoader) into SQLite if not already present.
    // Returns the count of newly inserted jobs.
    static int syncJobsFromFile(const std::string& filePath);

    // Helper: get or create skill ID in skills table
    static int getOrCreateSkillId(const std::string& skillName, const std::string& category = "Other");
};
