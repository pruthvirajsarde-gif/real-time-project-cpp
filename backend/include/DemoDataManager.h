#pragma once

#include <string>
#include <vector>

// ============================================================
// DemoDataManager — Production & Academic Demonstration Tooling
// Provides deterministic sample datasets (candidates, recruiters,
// job requisitions, resumes, and multi-stage applications)
// to make presentations and testing completely reproducible.
// ============================================================
class DemoDataManager {
public:
    // Seeds sample candidates, recruiters, jobs, resumes, and applications
    static bool seedDemoData(bool verbose = true);

    // Cleans up all demo data without affecting real user records
    static bool clearDemoData(bool verbose = true);

    // Checks whether demo data is currently seeded
    static bool isDemoDataPresent();

    // Constant demo account credentials for presentation reference
    static const std::string DEMO_PASSWORD;
    static const std::string DEMO_CANDIDATE_1_USER;
    static const std::string DEMO_CANDIDATE_2_USER;
    static const std::string DEMO_CANDIDATE_3_USER;
    static const std::string DEMO_RECRUITER_1_USER;
    static const std::string DEMO_RECRUITER_2_USER;
};
