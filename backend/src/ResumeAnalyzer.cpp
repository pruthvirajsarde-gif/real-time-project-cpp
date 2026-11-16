#include "ResumeAnalyzer.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cmath>

// ============================================================
//  Helper: Convert a string to lowercase
// ============================================================
std::string ResumeAnalyzer::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// ============================================================
//  Helper: Case-insensitive string equality
// ============================================================
bool ResumeAnalyzer::equalsIgnoreCase(const std::string& a, const std::string& b) {
    return toLower(a) == toLower(b);
}

// ============================================================
//  Helper: Check if the resume contains a given skill name
//  (case-insensitive comparison against Skill::getName())
// ============================================================
bool ResumeAnalyzer::resumeHasSkill(const Resume& resume, const std::string& skillName) {
    const auto& skills = resume.getSkills();
    return std::any_of(skills.begin(), skills.end(),
        [&](const Skill& s) { return equalsIgnoreCase(s.getName(), skillName); });
}

// ============================================================
//  Helper: Parse a duration string into approximate months
//
//  Supported formats:
//    "May 2025 - Jul 2025"  → month-name year - month-name year
//    "2 years"              → numeric + unit
//    "6 months"             → numeric + unit
//    "1 year"               → numeric + unit
//    "Jan 2025 - Present"   → treats "Present" as current approx
// ============================================================
int ResumeAnalyzer::parseDurationMonths(const std::string& duration) {
    std::string lower = toLower(duration);

    // --- Try numeric format first: "2 years", "6 months", "1 year" ---
    {
        std::istringstream iss(lower);
        double num = 0;
        std::string unit;
        if (iss >> num >> unit) {
            if (unit.find("year") != std::string::npos) {
                return static_cast<int>(std::round(num * 12));
            }
            if (unit.find("month") != std::string::npos) {
                return static_cast<int>(std::round(num));
            }
        }
    }

    // --- Try "Month Year - Month Year" format ---
    // Month name to number mapping
    auto monthToNum = [](const std::string& m) -> int {
        static const std::string months[] = {
            "jan", "feb", "mar", "apr", "may", "jun",
            "jul", "aug", "sep", "oct", "nov", "dec"
        };
        std::string ml = m.substr(0, 3);
        for (int i = 0; i < 12; ++i) {
            if (ml == months[i]) return i + 1;
        }
        return 0;
    };

    // Find the dash separator
    auto dashPos = lower.find('-');
    if (dashPos == std::string::npos) return 0;

    std::string startPart = lower.substr(0, dashPos);
    std::string endPart   = lower.substr(dashPos + 1);

    // Trim whitespace helper
    auto trim = [](std::string& s) {
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
    };
    trim(startPart);
    trim(endPart);

    // Parse start: "may 2025"
    std::istringstream startSS(startPart);
    std::string startMonthStr;
    int startYear = 0;
    startSS >> startMonthStr >> startYear;
    int startMonth = monthToNum(startMonthStr);

    // Parse end: "jul 2025" or "present"
    int endMonth = 0, endYear = 0;
    if (endPart.find("present") != std::string::npos) {
        // Approximate "present" as Sep 2026 (current date)
        endMonth = 9;
        endYear = 2026;
    } else {
        std::istringstream endSS(endPart);
        std::string endMonthStr;
        endSS >> endMonthStr >> endYear;
        endMonth = monthToNum(endMonthStr);
    }

    if (startMonth > 0 && startYear > 0 && endMonth > 0 && endYear > 0) {
        int totalMonths = (endYear - startYear) * 12 + (endMonth - startMonth);
        return std::max(totalMonths, 0);
    }

    return 0;  // Unable to parse
}

// ============================================================
//  Helper: Extract total experience in years from all entries
// ============================================================
double ResumeAnalyzer::extractTotalExperienceYears(const Resume& resume) {
    int totalMonths = 0;
    for (const auto& exp : resume.getExperience()) {
        totalMonths += parseDurationMonths(exp.getDuration());
    }
    return static_cast<double>(totalMonths) / 12.0;
}

// ============================================================
//  Helper: Check if candidate's education field matches job req
//
//  Strategy: tokenize the job requirement and candidate field,
//  then check if any meaningful keywords overlap.
//  Recognized keywords include degree levels and field names.
// ============================================================
bool ResumeAnalyzer::educationFieldMatches(
        const std::string& candidateField,
        const std::string& jobRequirement) {
    if (candidateField.empty() || jobRequirement.empty()) return false;

    std::string candLower = toLower(candidateField);
    std::string jobLower  = toLower(jobRequirement);

    // Field-of-study keywords to look for
    static const std::vector<std::string> fieldKeywords = {
        "computer engineering", "computer science", "information technology",
        "software engineering", "computer", "engineering", "it",
        "science", "technology"
    };

    // Check if any field keyword appears in BOTH candidate and job requirement
    for (const auto& keyword : fieldKeywords) {
        if (candLower.find(keyword) != std::string::npos &&
            jobLower.find(keyword) != std::string::npos) {
            return true;
        }
    }

    return false;
}

// ============================================================
//  Skill Match Percentage
//
//  Formula:
//    SkillScore = (RequiredMatch × 0.70) + (PreferredMatch × 0.30)
//
//  RequiredMatch  = matchingRequired / totalRequired  × 100
//  PreferredMatch = matchingPreferred / totalPreferred × 100
//
//  If a category is empty, its weight is redistributed.
// ============================================================
double ResumeAnalyzer::calculateSkillMatchPercentage(
        const Resume& resume, const Job& job) {
    const auto& required  = job.getRequiredSkills();
    const auto& preferred = job.getPreferredSkills();

    bool hasRequired  = !required.empty();
    bool hasPreferred = !preferred.empty();

    // If no skills defined in job, consider it 100% match
    if (!hasRequired && !hasPreferred) return 100.0;

    // Count matching required skills
    int reqMatched = 0;
    if (hasRequired) {
        for (const auto& skill : required) {
            if (resumeHasSkill(resume, skill)) ++reqMatched;
        }
    }

    // Count matching preferred skills
    int prefMatched = 0;
    if (hasPreferred) {
        for (const auto& skill : preferred) {
            if (resumeHasSkill(resume, skill)) ++prefMatched;
        }
    }

    // Calculate individual percentages
    double reqPercent  = hasRequired
        ? (static_cast<double>(reqMatched) / required.size()) * 100.0
        : 100.0;
    double prefPercent = hasPreferred
        ? (static_cast<double>(prefMatched) / preferred.size()) * 100.0
        : 100.0;

    // Weighted combination:
    //   Required = 70%, Preferred = 30%
    //   If only one category exists, it gets 100% weight.
    double reqWeight  = 0.70;
    double prefWeight = 0.30;

    if (!hasRequired)  { prefWeight = 1.0; reqWeight = 0.0; }
    if (!hasPreferred) { reqWeight = 1.0; prefWeight = 0.0; }

    return (reqPercent * reqWeight) + (prefPercent * prefWeight);
}

// ============================================================
//  Education Match Percentage
//
//  Checks if ANY of the candidate's education entries match
//  the job's education requirement by field of study.
//  Also checks for degree-level keywords (diploma, b.tech, etc.)
//
//  Returns 100% if match found, 0% otherwise.
// ============================================================
double ResumeAnalyzer::calculateEducationMatchPercentage(
        const Resume& resume, const Job& job) {
    const std::string& jobReq = job.getEducationRequirement();

    // If job has no education requirement, it's a full match
    if (jobReq.empty()) return 100.0;

    // If resume has no education, it's 0%
    if (resume.getEducation().empty()) return 0.0;

    std::string jobLower = toLower(jobReq);

    // Degree-level keywords and their hierarchy (higher index = higher level)
    // We check both field match and degree level
    static const std::vector<std::string> degreeKeywords = {
        "diploma", "bachelor", "b.e.", "b.tech", "b.sc",
        "master", "m.e.", "m.tech", "m.sc", "phd", "doctorate"
    };

    for (const auto& edu : resume.getEducation()) {
        // Check field-of-study match
        bool fieldMatch = educationFieldMatches(edu.getFieldOfStudy(), jobReq);

        // Also check degree + field combined against job requirement
        std::string degreeLower = toLower(edu.getDegree());
        bool degreeMatch = false;
        for (const auto& dk : degreeKeywords) {
            if (degreeLower.find(dk) != std::string::npos &&
                jobLower.find(dk) != std::string::npos) {
                degreeMatch = true;
                break;
            }
        }

        // Check if the degree name itself appears in the requirement
        if (!degreeMatch) {
            degreeMatch = (jobLower.find(degreeLower) != std::string::npos);
        }

        if (fieldMatch || degreeMatch) {
            return 100.0;
        }
    }

    // Partial credit: check if any education field keyword at least partially
    // overlaps with the job requirement (broader matching)
    for (const auto& edu : resume.getEducation()) {
        std::string combined = toLower(edu.getDegree() + " " + edu.getFieldOfStudy());
        // Check for common substrings of 4+ characters
        for (size_t len = 4; len <= combined.size(); ++len) {
            for (size_t i = 0; i + len <= combined.size(); ++i) {
                std::string sub = combined.substr(i, len);
                if (sub.find(' ') == std::string::npos && len >= 5 &&
                    jobLower.find(sub) != std::string::npos) {
                    return 75.0;  // Partial match
                }
            }
        }
    }

    return 0.0;
}

// ============================================================
//  Experience Match Percentage
//
//  Formula:
//    If job requires 0 years → 100%
//    Otherwise → min(candidateYears / requiredYears, 1.0) × 100
//
//  Candidate's total experience is summed from all Experience
//  entries using duration parsing.
// ============================================================
double ResumeAnalyzer::calculateExperienceMatchPercentage(
        const Resume& resume, const Job& job) {
    int required = job.getMinimumExperience();

    // If no experience required, full match
    if (required <= 0) return 100.0;

    double candidateYears = extractTotalExperienceYears(resume);

    // If candidate has no experience data, 0%
    if (candidateYears <= 0.0 && resume.getExperience().empty()) return 0.0;

    // Score: capped at 100% (exceeding is fine, but doesn't go over 100)
    double ratio = candidateYears / static_cast<double>(required);
    return std::min(ratio, 1.0) * 100.0;
}

// ============================================================
//  Overall Match Score
//
//  Formula:
//    Overall = (SkillScore × 0.60)
//            + (EducationScore × 0.20)
//            + (ExperienceScore × 0.20)
//
//  Result is clamped to [0, 100]
// ============================================================
double ResumeAnalyzer::calculateOverallMatchScore(
        const Resume& resume, const Job& job) {
    double skillScore = calculateSkillMatchPercentage(resume, job);
    double eduScore   = calculateEducationMatchPercentage(resume, job);
    double expScore   = calculateExperienceMatchPercentage(resume, job);

    double overall = (skillScore * 0.60) + (eduScore * 0.20) + (expScore * 0.20);

    // Clamp to [0, 100]
    return std::max(0.0, std::min(100.0, overall));
}

// ============================================================
//  Matching Skills — skills present in both resume and job
// ============================================================
std::vector<std::string> ResumeAnalyzer::getMatchingSkills(
        const Resume& resume, const Job& job) {
    std::vector<std::string> matching;

    // Check against required skills
    for (const auto& skill : job.getRequiredSkills()) {
        if (resumeHasSkill(resume, skill)) {
            matching.push_back(skill);
        }
    }

    // Check against preferred skills (avoid duplicates)
    for (const auto& skill : job.getPreferredSkills()) {
        if (resumeHasSkill(resume, skill)) {
            // Only add if not already in matching list
            bool alreadyAdded = std::any_of(matching.begin(), matching.end(),
                [&](const std::string& s) { return equalsIgnoreCase(s, skill); });
            if (!alreadyAdded) {
                matching.push_back(skill);
            }
        }
    }

    return matching;
}

// ============================================================
//  Missing Required Skills — required skills candidate lacks
// ============================================================
std::vector<std::string> ResumeAnalyzer::getMissingRequiredSkills(
        const Resume& resume, const Job& job) {
    std::vector<std::string> missing;
    for (const auto& skill : job.getRequiredSkills()) {
        if (!resumeHasSkill(resume, skill)) {
            missing.push_back(skill);
        }
    }
    return missing;
}

// ============================================================
//  Missing Preferred Skills — preferred skills candidate lacks
// ============================================================
std::vector<std::string> ResumeAnalyzer::getMissingPreferredSkills(
        const Resume& resume, const Job& job) {
    std::vector<std::string> missing;
    for (const auto& skill : job.getPreferredSkills()) {
        if (!resumeHasSkill(resume, skill)) {
            missing.push_back(skill);
        }
    }
    return missing;
}

// ============================================================
//  Match Category based on overall score
// ============================================================
std::string ResumeAnalyzer::getMatchCategory(double score) {
    if (score >= 80.0) return "Strong Match";
    if (score >= 60.0) return "Good Match";
    if (score >= 40.0) return "Partial Match";
    return "Low Match";
}

// ============================================================
//  Display Analysis — prints full analysis report
// ============================================================
void ResumeAnalyzer::displayAnalysis(
        const Resume& resume, const Job& job) {
    std::cout << "\n========================================\n";
    std::cout << "          RESUME ANALYSIS               \n";
    std::cout << "========================================\n\n";

    std::cout << "  Candidate : " << resume.getName() << "\n";
    std::cout << "  Job       : " << job.getJobTitle() << "\n";
    std::cout << "  Company   : " << job.getCompany() << "\n\n";

    // --- Skill Match ---
    double skillScore = calculateSkillMatchPercentage(resume, job);
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Skill Match     : " << skillScore << "%\n\n";

    // Matching skills
    auto matching = getMatchingSkills(resume, job);
    if (!matching.empty()) {
        std::cout << "  Matching Skills:\n";
        for (const auto& s : matching) {
            std::cout << "    " << "\xe2\x9c\x93" << " " << s << "\n";
        }
    }

    // Missing required skills
    auto missingReq = getMissingRequiredSkills(resume, job);
    if (!missingReq.empty()) {
        std::cout << "\n  Missing Required Skills:\n";
        for (const auto& s : missingReq) {
            std::cout << "    " << "\xe2\x9c\x97" << " " << s << "\n";
        }
    }

    // Missing preferred skills
    auto missingPref = getMissingPreferredSkills(resume, job);
    if (!missingPref.empty()) {
        std::cout << "\n  Missing Preferred Skills:\n";
        for (const auto& s : missingPref) {
            std::cout << "    - " << s << "\n";
        }
    }

    // --- Education Match ---
    double eduScore = calculateEducationMatchPercentage(resume, job);
    std::cout << "\n  Education Match : " << eduScore << "%\n";

    // --- Experience Match ---
    double expScore = calculateExperienceMatchPercentage(resume, job);
    std::cout << "  Experience Match: " << expScore << "%\n";

    // --- Overall Score ---
    double overall = calculateOverallMatchScore(resume, job);
    std::string category = getMatchCategory(overall);

    std::cout << "\n  ----------------------------------------\n";
    std::cout << "  Overall Match Score : " << overall << "%\n";
    std::cout << "  Category            : " << category << "\n";
    std::cout << "  ----------------------------------------\n";
}
