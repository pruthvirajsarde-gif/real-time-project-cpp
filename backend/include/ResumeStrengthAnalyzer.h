#pragma once

#include "Resume.h"

#include <string>
#include <vector>
#include <map>

// ============================================================
// ResumeQualityBreakdown — Granular scores for each resume section
// ============================================================
struct ResumeQualityBreakdown {
    double profileScore;        // max 15.0
    double skillsScore;         // max 20.0
    double educationScore;      // max 15.0
    double projectsScore;       // max 20.0
    double experienceScore;     // max 20.0
    double certificationsScore; // max 10.0
    double totalQualityScore;   // max 100.0

    ResumeQualityBreakdown()
        : profileScore(0.0), skillsScore(0.0), educationScore(0.0),
          projectsScore(0.0), experienceScore(0.0),
          certificationsScore(0.0), totalQualityScore(0.0) {}
};

// ============================================================
// ResumeStrengthAnalyzer — Evaluates quality, strengths, weak areas,
// and technology frequency across the candidate's resume
// ============================================================
class ResumeStrengthAnalyzer {
public:
    // Calculates overall resume quality score (0.0 to 100.0)
    static double calculateResumeQualityScore(const Resume& resume);

    // Returns detailed points breakdown for each resume section
    static ResumeQualityBreakdown getQualityBreakdown(const Resume& resume);

    // Generates deterministic strengths supported by the underlying resume data
    static std::vector<std::string> getStrengths(const Resume& resume);

    // Generates actionable weak areas / improvement opportunities from actual data
    static std::vector<std::string> getWeakAreas(const Resume& resume);

    // Counts technology mentions across skills, projects, and experiences (case-normalized)
    static std::map<std::string, int> calculateTechnologyFrequency(const Resume& resume);

    // Displays full resume strength evaluation to console
    static void displayResumeStrengths(const Resume& resume);

    // Displays technology frequency table
    static void displayTechnologyFrequency(const Resume& resume);

private:
    static std::string normalizeTechName(const std::string& raw);
};
