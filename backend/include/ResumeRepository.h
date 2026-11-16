#pragma once

#include "Resume.h"
#include <string>

// ============================================================
// ResumeRepository — Data access operations for resumes, skills,
// education, experience, projects, and certifications.
// Ensures complete persistence of parsed resume objects.
// ============================================================
class ResumeRepository {
public:
    // Persist full resume for a candidate (profile, skills, edu, exp, proj, certs).
    // Uses transactions and prepared statements.
    static bool saveResume(
        int candidateId,
        const Resume& resume,
        const std::string& sourcePath,
        const std::string& fileFormat = "TXT",
        double qualityScore = 0.0
    );

    // Fully reconstruct a Resume object from SQLite by candidate ID
    static Resume loadResume(int candidateId);

    // Check if resume record exists for candidate
    static bool resumeExists(int candidateId);

    // Delete resume record and associated education, experience, projects, certifications
    static bool deleteResume(int candidateId);

    // Retrieve saved resume quality score for candidate
    static double getResumeQualityScore(int candidateId);

    // Retrieve saved resume source path
    static std::string getResumeSourcePath(int candidateId);

    // Retrieve saved resume file format (TXT, PDF)
    static std::string getResumeFileFormat(int candidateId);
};
