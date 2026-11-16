#pragma once

#include "Resume.h"
#include "Job.h"

#include <string>
#include <vector>

// ============================================================
// ResumeAnalyzer — Analyzes a candidate's resume against a job
// ============================================================
// Scoring Formula:
//   Overall = (SkillScore × 0.60) + (EducationScore × 0.20) + (ExperienceScore × 0.20)
//
//   SkillScore = (RequiredMatch × 0.70) + (PreferredMatch × 0.30)
//     RequiredMatch  = matchingRequired / totalRequired   (0–100%)
//     PreferredMatch = matchingPreferred / totalPreferred  (0–100%)
//
//   EducationScore = 0% or 100% based on field-of-study keyword match
//   ExperienceScore = min(candidateYears / requiredYears, 1.0) × 100%
// ============================================================

class ResumeAnalyzer {
public:
    // ── Skill Matching ─────────────────────────────────────

    // Returns percentage of required + preferred skills matched (weighted)
    static double calculateSkillMatchPercentage(
        const Resume& resume, const Job& job);

    // Returns percentage of education relevance (0 or 100)
    static double calculateEducationMatchPercentage(
        const Resume& resume, const Job& job);

    // Returns percentage based on candidate vs required experience
    static double calculateExperienceMatchPercentage(
        const Resume& resume, const Job& job);

    // Combines skill, education, experience into one score (0–100)
    static double calculateOverallMatchScore(
        const Resume& resume, const Job& job);

    // ── Skill Detail Queries ───────────────────────────────

    // Skills present in both resume and job (required + preferred)
    static std::vector<std::string> getMatchingSkills(
        const Resume& resume, const Job& job);

    // Required skills the candidate does NOT have
    static std::vector<std::string> getMissingRequiredSkills(
        const Resume& resume, const Job& job);

    // Preferred skills the candidate does NOT have
    static std::vector<std::string> getMissingPreferredSkills(
        const Resume& resume, const Job& job);

    // ── Match Category ─────────────────────────────────────

    // Returns "Strong Match", "Good Match", "Partial Match", or "Low Match"
    static std::string getMatchCategory(double score);

    // ── Display ────────────────────────────────────────────

    // Prints a full analysis report to the console
    static void displayAnalysis(
        const Resume& resume, const Job& job);

private:
    // ── Internal Helpers ───────────────────────────────────

    // Case-insensitive string comparison
    static bool equalsIgnoreCase(const std::string& a, const std::string& b);

    // Convert string to lowercase (returns a copy)
    static std::string toLower(const std::string& str);

    // Check if candidate has a skill (case-insensitive)
    static bool resumeHasSkill(const Resume& resume, const std::string& skillName);

    // Extract approximate total years of experience from all Experience entries
    static double extractTotalExperienceYears(const Resume& resume);

    // Parse a duration string like "May 2025 - Jul 2025" or "2 years" into months
    static int parseDurationMonths(const std::string& duration);

    // Check if a candidate's education field matches a job's requirement
    static bool educationFieldMatches(
        const std::string& candidateField,
        const std::string& jobRequirement);
};
