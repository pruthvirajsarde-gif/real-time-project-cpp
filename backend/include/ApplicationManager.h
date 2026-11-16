#pragma once

#include "Job.h"
#include <string>
#include <vector>

struct ApplicationRecord {
    int id = 0;
    int candidateId = 0;
    std::string candidateName;
    std::string jobId;
    std::string jobTitle;
    std::string company;
    double matchScore = 0.0;
    double readinessScore = 0.0;
    std::string status;
    std::string appliedAt;
    std::string updatedAt;
};

// ============================================================
// ApplicationManager — Manages candidate job applications,
// status transitions, history tracking, and dashboards.
// ============================================================
class ApplicationManager {
public:
    // Allowed status constants
    static const std::string STATUS_APPLIED;
    static const std::string STATUS_UNDER_REVIEW;
    static const std::string STATUS_SHORTLISTED;
    static const std::string STATUS_INTERVIEW;
    static const std::string STATUS_REJECTED;
    static const std::string STATUS_SELECTED;
    static const std::string STATUS_WITHDRAWN;

    // Validate status string against allowed values
    static bool isValidStatus(const std::string& status);

    // Apply for a job. Returns application ID (> 0) on success, or -1 on duplicate/failure.
    static int applyForJob(
        int candidateId,
        const Job& job,
        double matchScore,
        double readinessScore
    );

    // Update status of an application with validation
    static bool updateStatus(
        int applicationId,
        const std::string& status
    );

    // Candidate withdraws an active application (status becomes WITHDRAWN)
    static bool withdrawApplication(
        int applicationId
    );

    // Display formatted application history for candidate
    static void displayApplicationHistory(
        int candidateId
    );

    // Retrieve all applications for a specific candidate
    static std::vector<ApplicationRecord> getCandidateApplications(int candidateId);

    // Retrieve all applications in the system (for recruiter)
    static std::vector<ApplicationRecord> getAllApplications();

    // Check if candidate already has an active application for a job
    static bool hasActiveApplication(int candidateId, const std::string& jobId);

    // Count applications for candidate, optionally filtered by status
    static int countCandidateApplications(int candidateId, const std::string& status = "");

    // Count all applications in the system, optionally filtered by status
    static int countAllApplications(const std::string& status = "");

    // Display Recruiter applications view
    static void displayRecruiterApplications();

    // Display Recruiter dashboard
    static void displayRecruiterDashboard();

    // Display Candidate dashboard
    static void displayCandidateDashboard(
        int candidateId,
        const std::string& candidateName,
        double resumeQuality,
        int totalSkills,
        int projectsCount,
        int experienceCount,
        int certsCount,
        int savedJobsCount,
        const std::string& topJobTitle = "N/A",
        double topMatchScore = 0.0
    );
};
