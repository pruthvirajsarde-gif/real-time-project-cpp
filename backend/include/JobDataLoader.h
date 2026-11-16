#pragma once

#include "Job.h"

#include <string>
#include <vector>

// ============================================================
// JobDataLoader — Reads multiple job listings from a text file
//
// File format: multiple [JOB] sections, each with key:value pairs.
// Supported fields:
//   JobId, Title, Company, Location, EmploymentType,
//   RequiredSkills, PreferredSkills, Education,
//   MinimumExperience, Description
// ============================================================

class JobDataLoader {
public:
    // Load all jobs from the given file path
    static std::vector<Job> loadJobsFromFile(const std::string& filePath);

    // Validate that the file exists and contains at least one [JOB]
    static bool validateJobFile(const std::string& filePath);

    // Display summary of loaded jobs
    static void displayLoadingSummary(const std::vector<Job>& jobs);

private:
    // Trim leading/trailing whitespace
    static std::string trim(const std::string& str);

    // Split comma-separated values into trimmed tokens
    static std::vector<std::string> splitCSV(const std::string& csv);

    // Extract value from "Key: Value" line
    static std::string extractValue(const std::string& line,
                                    const std::string& key);
};
