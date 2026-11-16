#include "JobDataLoader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

// ============================================================
//  Helper: Trim leading and trailing whitespace
// ============================================================
std::string JobDataLoader::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// ============================================================
//  Helper: Split comma-separated string into trimmed tokens
// ============================================================
std::vector<std::string> JobDataLoader::splitCSV(const std::string& csv) {
    std::vector<std::string> tokens;
    std::istringstream stream(csv);
    std::string token;
    while (std::getline(stream, token, ',')) {
        std::string trimmed = trim(token);
        if (!trimmed.empty()) {
            tokens.push_back(trimmed);
        }
    }
    return tokens;
}

// ============================================================
//  Helper: Extract value from "Key: Value" line
// ============================================================
std::string JobDataLoader::extractValue(const std::string& line,
                                        const std::string& key) {
    std::string prefix = key + ":";
    if (line.size() < prefix.size()) return "";

    std::string lineLower = line.substr(0, prefix.size());
    std::string prefixLower = prefix;
    std::transform(lineLower.begin(), lineLower.end(), lineLower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    std::transform(prefixLower.begin(), prefixLower.end(), prefixLower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lineLower == prefixLower) {
        return trim(line.substr(prefix.size()));
    }
    return "";
}

// ============================================================
//  Validate Job File
// ============================================================
bool JobDataLoader::validateJobFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "  [ERROR] Job file could not be opened: "
                  << filePath << "\n";
        return false;
    }

    std::string line;
    bool hasJob = false;
    while (std::getline(file, line)) {
        if (trim(line) == "[JOB]") {
            hasJob = true;
            break;
        }
    }
    file.close();

    if (!hasJob) {
        std::cerr << "  [ERROR] Job file contains no [JOB] sections.\n";
        return false;
    }

    return true;
}

// ============================================================
//  Load Jobs From File
//
//  Parses multiple [JOB] sections. Each section creates one
//  Job object. Unknown fields produce a warning but parsing
//  continues.
// ============================================================
std::vector<Job> JobDataLoader::loadJobsFromFile(const std::string& filePath) {
    std::vector<Job> jobs;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "  [ERROR] Could not open job file: "
                  << filePath << "\n";
        return jobs;
    }

    Job currentJob;
    bool inJob = false;
    int lineNum = 0;

    // Lambda to flush the current job into the vector
    auto flushJob = [&]() {
        if (inJob) {
            // Only add if the job has at least a title
            if (!currentJob.getJobTitle().empty()) {
                jobs.push_back(currentJob);
            } else {
                std::cerr << "  [WARNING] Skipping job with no title.\n";
            }
            currentJob = Job();
            inJob = false;
        }
    };

    std::string line;
    while (std::getline(file, line)) {
        ++lineNum;
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;

        // Detect [JOB] section header
        if (trimmed == "[JOB]") {
            flushJob();
            inJob = true;
            continue;
        }

        if (!inJob) continue;

        // Parse fields
        std::string val;

        val = extractValue(trimmed, "JobId");
        if (!val.empty()) { currentJob.setJobId(val); continue; }

        val = extractValue(trimmed, "Title");
        if (!val.empty()) { currentJob.setJobTitle(val); continue; }

        val = extractValue(trimmed, "Company");
        if (!val.empty()) { currentJob.setCompany(val); continue; }

        val = extractValue(trimmed, "Location");
        if (!val.empty()) { currentJob.setLocation(val); continue; }

        val = extractValue(trimmed, "EmploymentType");
        if (!val.empty()) { currentJob.setEmploymentType(val); continue; }

        val = extractValue(trimmed, "RequiredSkills");
        if (!val.empty()) {
            auto skills = splitCSV(val);
            for (const auto& s : skills) {
                currentJob.addRequiredSkill(s);
            }
            continue;
        }

        val = extractValue(trimmed, "PreferredSkills");
        if (!val.empty()) {
            auto skills = splitCSV(val);
            for (const auto& s : skills) {
                currentJob.addPreferredSkill(s);
            }
            continue;
        }

        val = extractValue(trimmed, "Education");
        if (!val.empty()) { currentJob.setEducationRequirement(val); continue; }

        val = extractValue(trimmed, "MinimumExperience");
        if (!val.empty()) {
            try {
                int exp = std::stoi(val);
                currentJob.setMinimumExperience(exp);
            } catch (...) {
                std::cerr << "  [WARNING] Line " << lineNum
                          << ": Invalid experience value: " << val << "\n";
            }
            continue;
        }

        val = extractValue(trimmed, "Description");
        if (!val.empty()) { currentJob.setDescription(val); continue; }

        // Unknown field
        std::cerr << "  [WARNING] Line " << lineNum
                  << ": Unknown job field: " << trimmed << "\n";
    }

    // Flush the last job
    flushJob();

    file.close();
    return jobs;
}

// ============================================================
//  Display Loading Summary
// ============================================================
void JobDataLoader::displayLoadingSummary(const std::vector<Job>& jobs) {
    std::cout << "\n  ----------------------------------------\n";
    std::cout << "  Job Loading Summary\n";
    std::cout << "  ----------------------------------------\n";
    std::cout << "  Total jobs loaded: " << jobs.size() << "\n";
    for (size_t i = 0; i < jobs.size(); ++i) {
        std::cout << "    " << (i + 1) << ". " << jobs[i].getJobTitle()
                  << " — " << jobs[i].getCompany() << "\n";
    }
    std::cout << "  ----------------------------------------\n";
}
