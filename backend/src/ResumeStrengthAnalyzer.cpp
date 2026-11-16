#include "ResumeStrengthAnalyzer.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>

std::string ResumeStrengthAnalyzer::normalizeTechName(const std::string& raw) {
    // Trim
    size_t start = raw.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = raw.find_last_not_of(" \t\r\n");
    std::string trimmed = raw.substr(start, end - start + 1);

    std::string lower = trimmed;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Canonical tech dictionary
    if (lower == "c++") return "C++";
    if (lower == "c") return "C";
    if (lower == "java") return "Java";
    if (lower == "python") return "Python";
    if (lower == "sql") return "SQL";
    if (lower == "html") return "HTML";
    if (lower == "css") return "CSS";
    if (lower == "git") return "Git";
    if (lower == "cmake") return "CMake";
    if (lower == "stl") return "STL";
    if (lower == "docker") return "Docker";
    if (lower == "javascript" || lower == "js") return "JavaScript";
    if (lower == "react") return "React";
    if (lower == "mysql") return "MySQL";
    if (lower == "jsp") return "JSP";
    if (lower == "linux") return "Linux";
    if (lower == "spring boot" || lower == "springboot") return "Spring Boot";
    if (lower == "rest apis" || lower == "rest api") return "REST APIs";
    if (lower == "data structures") return "Data Structures";

    // Default: capitalized first letter
    if (!trimmed.empty()) {
        trimmed[0] = static_cast<char>(std::toupper(trimmed[0]));
    }
    return trimmed;
}

ResumeQualityBreakdown ResumeStrengthAnalyzer::getQualityBreakdown(const Resume& resume) {
    ResumeQualityBreakdown bd;

    // ── 1. Profile completeness (15% max) ─────────────────────
    double prof = 0.0;
    if (!resume.getName().empty()) prof += 3.0;
    if (!resume.getEmail().empty()) prof += 2.0;
    if (!resume.getPhone().empty()) prof += 2.0;
    if (!resume.getLocation().empty()) prof += 2.0;
    if (!resume.getLinkedin().empty()) prof += 2.0;
    if (!resume.getGithub().empty()) prof += 2.0;
    if (!resume.getPortfolio().empty()) prof += 2.0;
    bd.profileScore = std::min(15.0, prof);

    // ── 2. Skills (20% max) ───────────────────────────────────
    double sk = static_cast<double>(resume.getSkills().size()) * 3.5;
    bd.skillsScore = std::min(20.0, sk);

    // ── 3. Education (15% max) ────────────────────────────────
    double edu = 0.0;
    if (!resume.getEducation().empty()) {
        const auto& e = resume.getEducation()[0];
        if (!e.getInstitution().empty()) edu += 3.0;
        if (!e.getDegree().empty()) edu += 3.0;
        if (!e.getFieldOfStudy().empty()) edu += 3.0;
        if (e.getStartYear() > 0 && e.getEndYear() >= e.getStartYear()) edu += 3.0;
        if (e.getPercentageOrCGPA() > 0.0) edu += 3.0;
    }
    bd.educationScore = std::min(15.0, edu);

    // ── 4. Projects (20% max) ─────────────────────────────────
    double proj = 0.0;
    for (size_t i = 0; i < resume.getProjects().size() && i < 2; ++i) {
        const auto& p = resume.getProjects()[i];
        double pScore = 0.0;
        if (!p.getProjectName().empty()) pScore += 2.0;
        if (!p.getDescription().empty()) pScore += 3.0;
        if (!p.getTechnologies().empty()) pScore += 3.0;
        if (!p.getRole().empty() || !p.getDuration().empty()) pScore += 2.0;
        proj += pScore;
    }
    bd.projectsScore = std::min(20.0, proj);

    // ── 5. Experience (20% max) ───────────────────────────────
    double exp = 0.0;
    if (!resume.getExperience().empty()) {
        if (resume.getExperience().size() == 1) {
            const auto& x = resume.getExperience()[0];
            if (!x.getCompany().empty()) exp += 4.0;
            if (!x.getPosition().empty()) exp += 4.0;
            if (!x.getDuration().empty()) exp += 4.0;
            if (!x.getResponsibilities().empty()) exp += 4.0;
            if (!x.getTechnologies().empty()) exp += 4.0;
        } else {
            for (size_t i = 0; i < resume.getExperience().size() && i < 2; ++i) {
                const auto& x = resume.getExperience()[i];
                double xScore = 0.0;
                if (!x.getCompany().empty()) xScore += 2.0;
                if (!x.getPosition().empty()) xScore += 2.0;
                if (!x.getDuration().empty()) xScore += 2.0;
                if (!x.getResponsibilities().empty()) xScore += 2.0;
                if (!x.getTechnologies().empty()) xScore += 2.0;
                exp += xScore;
            }
        }
    }
    bd.experienceScore = std::min(20.0, exp);

    // ── 6. Certifications (10% max) ───────────────────────────
    double cert = 0.0;
    for (size_t i = 0; i < resume.getCertifications().size() && i < 2; ++i) {
        const auto& c = resume.getCertifications()[i];
        double cScore = 0.0;
        if (!c.getCertificationName().empty()) cScore += 2.0;
        if (!c.getIssuingOrganization().empty()) cScore += 2.0;
        if (c.getYear() > 0) cScore += 1.0;
        cert += cScore;
    }
    bd.certificationsScore = std::min(10.0, cert);

    // ── Total ─────────────────────────────────────────────────
    double total = bd.profileScore + bd.skillsScore + bd.educationScore +
                   bd.projectsScore + bd.experienceScore + bd.certificationsScore;
    if (total < 0.0) total = 0.0;
    if (total > 100.0) total = 100.0;
    bd.totalQualityScore = total;

    return bd;
}

double ResumeStrengthAnalyzer::calculateResumeQualityScore(const Resume& resume) {
    return getQualityBreakdown(resume).totalQualityScore;
}

std::vector<std::string> ResumeStrengthAnalyzer::getStrengths(const Resume& resume) {
    std::vector<std::string> strengths;

    if (resume.getSkills().size() >= 5) {
        strengths.push_back("Strong technical skill coverage (" +
                            std::to_string(resume.getSkills().size()) + " skills listed)");
    }
    if (resume.getProjects().size() >= 2) {
        strengths.push_back("Multiple projects demonstrated (" +
                            std::to_string(resume.getProjects().size()) + " projects documented)");
    }
    if (!resume.getExperience().empty()) {
        strengths.push_back("Practical development / internship experience documented");
    }

    // Git / version control
    bool hasGit = false;
    for (const auto& s : resume.getSkills()) {
        std::string lower = s.getName();
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (lower == "git") { hasGit = true; break; }
    }
    if (hasGit || !resume.getGithub().empty()) {
        strengths.push_back("Git and modern version control knowledge demonstrated");
    }

    if (!resume.getCertifications().empty()) {
        strengths.push_back("Relevant technical certification(s) earned (" +
                            std::to_string(resume.getCertifications().size()) + " active)");
    }

    if (!resume.getName().empty() && !resume.getEmail().empty() &&
        !resume.getPhone().empty() && !resume.getLocation().empty()) {
        strengths.push_back("Comprehensive contact and personal profile information provided");
    }

    if (!resume.getEducation().empty() && resume.getEducation()[0].getPercentageOrCGPA() >= 8.0) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << resume.getEducation()[0].getPercentageOrCGPA();
        strengths.push_back("Strong academic performance (CGPA / Score: " + ss.str() + ")");
    }

    if (strengths.empty()) {
        strengths.push_back("Basic candidate profile initialized");
    }

    return strengths;
}

std::vector<std::string> ResumeStrengthAnalyzer::getWeakAreas(const Resume& resume) {
    std::vector<std::string> weakAreas;

    if (resume.getProjects().size() < 2) {
        weakAreas.push_back("Limited number of projects (adding at least 2 practical projects is recommended)");
    }
    if (resume.getLinkedin().empty()) {
        weakAreas.push_back("Missing LinkedIn professional profile link");
    }
    if (resume.getPortfolio().empty()) {
        weakAreas.push_back("Missing personal portfolio or website URL");
    }
    if (resume.getCertifications().empty()) {
        weakAreas.push_back("No technical certifications listed to validate domain knowledge");
    }
    if (resume.getExperience().empty()) {
        weakAreas.push_back("No professional work experience or internships listed");
    }
    if (resume.getSkills().size() < 5) {
        weakAreas.push_back("Low technical skill count (expand skill repertoire with core frameworks)");
    }

    return weakAreas;
}

std::map<std::string, int> ResumeStrengthAnalyzer::calculateTechnologyFrequency(const Resume& resume) {
    std::map<std::string, int> freq;

    // Skills
    for (const auto& s : resume.getSkills()) {
        std::string norm = normalizeTechName(s.getName());
        if (!norm.empty()) freq[norm]++;
    }

    // Projects
    for (const auto& p : resume.getProjects()) {
        for (const auto& t : p.getTechnologies()) {
            std::string norm = normalizeTechName(t);
            if (!norm.empty()) freq[norm]++;
        }
    }

    // Experience
    for (const auto& x : resume.getExperience()) {
        for (const auto& t : x.getTechnologies()) {
            std::string norm = normalizeTechName(t);
            if (!norm.empty()) freq[norm]++;
        }
    }

    return freq;
}

void ResumeStrengthAnalyzer::displayResumeStrengths(const Resume& resume) {
    auto bd = getQualityBreakdown(resume);
    auto strengths = getStrengths(resume);
    auto weakAreas = getWeakAreas(resume);

    std::cout << "\n========================================\n";
    std::cout << "       RESUME STRENGTH ANALYSIS         \n";
    std::cout << "========================================\n\n";

    std::cout << "Candidate: " << resume.getName() << "\n\n";

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Resume Quality Score: " << bd.totalQualityScore << "%\n\n";

    std::cout << "Section Breakdown:\n";
    std::cout << "  - Profile Completeness : " << bd.profileScore << " / 15.0\n";
    std::cout << "  - Technical Skills     : " << bd.skillsScore << " / 20.0\n";
    std::cout << "  - Education Details    : " << bd.educationScore << " / 15.0\n";
    std::cout << "  - Project Portfolio    : " << bd.projectsScore << " / 20.0\n";
    std::cout << "  - Practical Experience : " << bd.experienceScore << " / 20.0\n";
    std::cout << "  - Certifications       : " << bd.certificationsScore << " / 10.0\n\n";

    std::cout << "----------------------------------------\n";
    std::cout << "Resume Strengths:\n";
    std::cout << "----------------------------------------\n";
    for (const auto& str : strengths) {
        std::cout << "  ✓ " << str << "\n";
    }
    std::cout << "\n";

    std::cout << "----------------------------------------\n";
    std::cout << "Areas to Improve:\n";
    std::cout << "----------------------------------------\n";
    if (weakAreas.empty()) {
        std::cout << "  ✓ Excellent profile! No significant weak areas detected.\n";
    } else {
        for (const auto& w : weakAreas) {
            std::cout << "  • " << w << "\n";
        }
    }
    std::cout << "========================================\n";
}

void ResumeStrengthAnalyzer::displayTechnologyFrequency(const Resume& resume) {
    auto freq = calculateTechnologyFrequency(resume);

    std::cout << "\n========================================\n";
    std::cout << "       TECHNOLOGY FREQUENCY             \n";
    std::cout << "========================================\n\n";

    if (freq.empty()) {
        std::cout << "  No technologies detected.\n";
    } else {
        // Sort by frequency descending
        std::vector<std::pair<std::string, int>> sorted(freq.begin(), freq.end());
        std::sort(sorted.begin(), sorted.end(),
                  [](const auto& a, const auto& b) {
                      if (a.second != b.second) return a.second > b.second;
                      return a.first < b.first;
                  });

        std::cout << "  " << std::left << std::setw(20) << "Technology" << "Count\n";
        std::cout << "  " << std::string(28, '-') << "\n";
        for (const auto& item : sorted) {
            std::cout << "  " << std::left << std::setw(20) << item.first << item.second << "\n";
        }
    }
    std::cout << "========================================\n";
}
