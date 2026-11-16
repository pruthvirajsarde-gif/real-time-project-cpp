#pragma once

#include "Resume.h"

#include <string>
#include <vector>

// ============================================================
// ResumeParser — Parses both structured (bracketed sections) and
// semi-structured (free-form headings) resume text into Resume objects.
// ============================================================
class ResumeParser {
public:
    // Parse resume from a file (.txt or .pdf via ResumeDocumentReader)
    static Resume parseResumeFile(const std::string& filePath);

    // Parse resume directly from a normalized text string
    static Resume parseResumeText(const std::string& resumeText);

    // Validate that a resume file exists and contains parseable content
    static bool validateResumeFile(const std::string& filePath);

    // Display summary of parsed entities
    static void displayParsingResult(const Resume& resume);

    // Utility helpers
    static std::string trim(const std::string& str);
    static std::vector<std::string> splitCSV(const std::string& csv);
    static std::string extractValue(const std::string& line, const std::string& key);

private:
    enum class SectionType {
        NONE, PROFILE, SKILLS, EDUCATION, PROJECT, EXPERIENCE, CERTIFICATION
    };

    // Parsing pipelines
    static Resume parseStructuredResume(const std::string& text);
    static Resume parseSemiStructuredResume(const std::string& text);

    // Format detection and heading matching
    static bool isStructuredFormat(const std::string& text);
    static SectionType matchSectionHeading(const std::string& line);
};
