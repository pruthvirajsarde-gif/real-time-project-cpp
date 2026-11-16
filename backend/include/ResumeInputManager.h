#pragma once

#include "Resume.h"

#include <string>

// ============================================================
// ResumeInputManager — Unified interface for detecting document format,
// extracting text, parsing, and validating resumes (.txt and .pdf)
// ============================================================
class ResumeInputManager {
public:
    // Loads resume from a file path (.txt or .pdf), handling format
    // detection, text extraction, normalization, and parsing
    static Resume loadResume(const std::string& filePath);

    // Checks whether a Resume object has minimum valid candidate data
    static bool validateResume(const Resume& resume);

    // Displays the formal Resume Loading Result summary box
    static void displayLoadingInformation(
        const std::string& filePath, const Resume& resume);
};
