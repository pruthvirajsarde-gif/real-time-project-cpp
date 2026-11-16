#include "SemanticMatcher.h"
#include "SkillNormalizer.h"
#include "TextPreprocessor.h"
#include "KeywordExtractor.h"
#include "TextSimilarity.h"
#include "ResumeStrengthAnalyzer.h"
#include "ResumeAnalyzer.h"

#include <algorithm>
#include <sstream>
#include <cmath>

static std::string buildResumeFullText(const Resume& resume) {
    std::stringstream ss;
    ss << resume.getName() << " " << resume.getLocation() << " ";

    for (const auto& s : resume.getSkills()) {
        ss << s.getName() << " ";
    }
    for (const auto& e : resume.getEducation()) {
        ss << e.getDegree() << " " << e.getInstitution() << " " << e.getFieldOfStudy() << " ";
    }
    for (const auto& p : resume.getProjects()) {
        ss << p.getProjectName() << " " << p.getDescription() << " " << p.getRole() << " ";
        for (const auto& t : p.getTechnologies()) {
            ss << t << " ";
        }
    }
    for (const auto& exp : resume.getExperience()) {
        ss << exp.getCompany() << " " << exp.getPosition() << " ";
        for (const auto& r : exp.getResponsibilities()) {
            ss << r << " ";
        }
        for (const auto& t : exp.getTechnologies()) {
            ss << t << " ";
        }
    }
    for (const auto& c : resume.getCertifications()) {
        ss << c.getCertificationName() << " " << c.getIssuingOrganization() << " ";
    }
    return ss.str();
}

static bool projectsCount(const Resume& resume) {
    return !resume.getProjects().empty();
}

static std::string buildJobFullText(const Job& job) {
    std::stringstream ss;
    ss << job.getJobTitle() << " " << job.getCompany() << " " << job.getLocation() << " "
       << job.getEmploymentType() << " " << job.getEducationRequirement() << " "
       << job.getDescription() << " ";

    for (const auto& s : job.getRequiredSkills()) {
        ss << s << " ";
    }
    for (const auto& s : job.getPreferredSkills()) {
        ss << s << " ";
    }
    return ss.str();
}

double SemanticMatcher::calculateSkillScore(
    const Resume& resume,
    const Job& job,
    std::vector<std::string>& exactMatches,
    std::vector<std::string>& relatedMatches,
    std::vector<std::string>& missingRequired,
    std::vector<std::string>& missingPreferred) {

    exactMatches.clear();
    relatedMatches.clear();
    missingRequired.clear();
    missingPreferred.clear();

    // Collect all candidate skills (from skills list, projects, and experiences)
    std::vector<std::string> candidateSkills;
    for (const auto& s : resume.getSkills()) {
        candidateSkills.push_back(s.getName());
    }
    for (const auto& p : resume.getProjects()) {
        for (const auto& t : p.getTechnologies()) {
            candidateSkills.push_back(t);
        }
    }
    for (const auto& exp : resume.getExperience()) {
        for (const auto& t : exp.getTechnologies()) {
            candidateSkills.push_back(t);
        }
    }

    // Helper: evaluate single target skill
    auto evaluateSkill = [&](const std::string& targetSkill, std::string& matchedCandidateSkill) -> int {
        // 2: Exact match, 1: Related match, 0: Missing
        for (const auto& cSkill : candidateSkills) {
            if (SkillNormalizer::areSkillsEquivalent(cSkill, targetSkill)) {
                matchedCandidateSkill = SkillNormalizer::normalizeSkillName(cSkill);
                return 2;
            }
        }
        for (const auto& cSkill : candidateSkills) {
            if (SkillNormalizer::areSkillsRelated(cSkill, targetSkill)) {
                matchedCandidateSkill = SkillNormalizer::normalizeSkillName(cSkill);
                return 1;
            }
        }
        return 0;
    };

    // 1. Evaluate Required Skills
    double reqPoints = 0.0;
    const auto& reqSkills = job.getRequiredSkills();
    for (const auto& rSkill : reqSkills) {
        std::string candMatch;
        int status = evaluateSkill(rSkill, candMatch);
        std::string normReq = SkillNormalizer::normalizeSkillName(rSkill);

        if (status == 2) {
            reqPoints += 1.0;
            if (std::find(exactMatches.begin(), exactMatches.end(), normReq) == exactMatches.end()) {
                exactMatches.push_back(normReq);
            }
        } else if (status == 1) {
            reqPoints += 0.60; // 60% credit for related technology
            std::string relationStr = candMatch + " -> " + normReq;
            if (std::find(relatedMatches.begin(), relatedMatches.end(), relationStr) == relatedMatches.end()) {
                relatedMatches.push_back(relationStr);
            }
        } else {
            missingRequired.push_back(normReq);
        }
    }

    // 2. Evaluate Preferred Skills
    double prefPoints = 0.0;
    const auto& prefSkills = job.getPreferredSkills();
    for (const auto& pSkill : prefSkills) {
        std::string candMatch;
        int status = evaluateSkill(pSkill, candMatch);
        std::string normPref = SkillNormalizer::normalizeSkillName(pSkill);

        if (status == 2) {
            prefPoints += 1.0;
            if (std::find(exactMatches.begin(), exactMatches.end(), normPref) == exactMatches.end()) {
                exactMatches.push_back(normPref);
            }
        } else if (status == 1) {
            prefPoints += 0.60;
            std::string relationStr = candMatch + " -> " + normPref;
            if (std::find(relatedMatches.begin(), relatedMatches.end(), relationStr) == relatedMatches.end()) {
                relatedMatches.push_back(relationStr);
            }
        } else {
            missingPreferred.push_back(normPref);
        }
    }

    double reqRatio = reqSkills.empty() ? 1.0 : (reqPoints / static_cast<double>(reqSkills.size()));
    double prefRatio = prefSkills.empty() ? 1.0 : (prefPoints / static_cast<double>(prefSkills.size()));

    double finalSkillScore = 0.0;
    if (prefSkills.empty()) {
        finalSkillScore = reqRatio * 100.0;
    } else if (reqSkills.empty()) {
        finalSkillScore = prefRatio * 100.0;
    } else {
        finalSkillScore = (reqRatio * 0.75 + prefRatio * 0.25) * 100.0;
    }

    return std::max(0.0, std::min(100.0, finalSkillScore));
}

double SemanticMatcher::calculateTextRelevance(const Resume& resume, const Job& job) {
    std::string resText = buildResumeFullText(resume);
    std::string jobText = buildJobFullText(job);

    if (resText.empty() || jobText.empty()) return 0.0;

    double cosineSim = TextSimilarity::keywordWeightedSimilarity(resText, jobText, job.getRequiredSkills()) * 100.0;
    double tfidfSim = TextSimilarity::computeTfIdfRelevance(resText, jobText);

    double combined = (cosineSim * 0.50) + (tfidfSim * 0.50);
    return std::max(0.0, std::min(100.0, combined));
}

double SemanticMatcher::calculateProjectRelevance(
    const Resume& resume,
    const Job& job,
    std::vector<std::string>& relevantProjects) {

    relevantProjects.clear();
    const auto& projects = resume.getProjects();
    if (projects.empty()) return 0.0;

    double totalRelevance = 0.0;
    std::string jobNormTitle = TextPreprocessor::normalizeText(job.getJobTitle());
    std::string jobDesc = TextPreprocessor::normalizeText(job.getDescription());

    for (const auto& proj : projects) {
        double projScore = 0.0;
        bool isRelevant = false;

        // Check project technologies against job required and preferred skills
        for (const auto& pTech : proj.getTechnologies()) {
            for (const auto& rSkill : job.getRequiredSkills()) {
                if (SkillNormalizer::areSkillsEquivalent(pTech, rSkill)) {
                    projScore += 35.0;
                    isRelevant = true;
                } else if (SkillNormalizer::areSkillsRelated(pTech, rSkill)) {
                    projScore += 20.0;
                    isRelevant = true;
                }
            }
            for (const auto& pSkill : job.getPreferredSkills()) {
                if (SkillNormalizer::areSkillsEquivalent(pTech, pSkill)) {
                    projScore += 20.0;
                    isRelevant = true;
                }
            }
        }

        // Check project description keyword overlap with job
        std::string projDesc = TextPreprocessor::normalizeText(proj.getDescription() + " " + proj.getProjectName());
        auto kwOverlap = TextSimilarity::jaccardSimilarity(
            TextPreprocessor::removeStopWords(TextPreprocessor::tokenize(projDesc)),
            TextPreprocessor::removeStopWords(TextPreprocessor::tokenize(jobDesc + " " + jobNormTitle))
        );

        if (kwOverlap > 0.08) {
            projScore += kwOverlap * 40.0;
            isRelevant = true;
        }

        if (isRelevant) {
            relevantProjects.push_back(proj.getProjectName());
            totalRelevance += std::min(100.0, projScore);
        }
    }

    if (relevantProjects.empty()) return 0.0;

    double avgScore = totalRelevance / static_cast<double>(projects.size());
    // Boost slightly if multiple relevant projects exist
    if (relevantProjects.size() >= 2) {
        avgScore = std::min(100.0, avgScore * 1.20);
    }
    return std::max(0.0, std::min(100.0, avgScore));
}

double SemanticMatcher::calculateExperienceRelevance(
    const Resume& resume,
    const Job& job,
    std::vector<std::string>& relevantExperiences) {

    relevantExperiences.clear();
    const auto& exps = resume.getExperience();

    // If job requires 0 experience and candidate has none, 100% relevant
    if (job.getMinimumExperience() == 0 && exps.empty()) {
        return 100.0;
    }
    if (exps.empty()) {
        return 0.0;
    }

    double totalScore = 0.0;
    for (const auto& exp : exps) {
        double expScore = 0.0;
        bool isRelevant = false;

        // Check technologies used in experience
        for (const auto& tech : exp.getTechnologies()) {
            for (const auto& rSkill : job.getRequiredSkills()) {
                if (SkillNormalizer::areSkillsEquivalent(tech, rSkill)) {
                    expScore += 40.0;
                    isRelevant = true;
                } else if (SkillNormalizer::areSkillsRelated(tech, rSkill)) {
                    expScore += 25.0;
                    isRelevant = true;
                }
            }
        }

        // Check responsibilities overlap
        std::string respStr;
        for (const auto& r : exp.getResponsibilities()) respStr += r + " ";
        double sim = TextSimilarity::cosineSimilarityText(respStr, job.getDescription());
        if (sim > 0.10) {
            expScore += sim * 40.0;
            isRelevant = true;
        }

        if (isRelevant) {
            std::string label = exp.getCompany() + " (" + exp.getPosition() + ")";
            relevantExperiences.push_back(label);
            totalScore += std::min(100.0, expScore + 30.0); // Base experience baseline
        }
    }

    if (relevantExperiences.empty()) return 15.0; // Baseline for having experience

    double finalScore = totalScore / static_cast<double>(exps.size());
    return std::max(0.0, std::min(100.0, finalScore));
}

double SemanticMatcher::calculateEducationScore(const Resume& resume, const Job& job) {
    return ResumeAnalyzer::calculateEducationMatchPercentage(resume, job);
}

MatchExplanation SemanticMatcher::evaluate(const Resume& resume, const Job& job) {
    MatchExplanation exp;

    exp.skillScore = calculateSkillScore(
        resume, job,
        exp.exactMatches,
        exp.relatedMatches,
        exp.missingRequiredSkills,
        exp.missingPreferredSkills
    );

    exp.textScore = calculateTextRelevance(resume, job);
    exp.projectScore = calculateProjectRelevance(resume, job, exp.relevantProjects);
    exp.experienceScore = calculateExperienceRelevance(resume, job, exp.relevantExperiences);
    exp.educationScore = calculateEducationScore(resume, job);
    exp.resumeQualityScore = ResumeStrengthAnalyzer::calculateResumeQualityScore(resume);

    // Default Phase 7 Formula:
    // Skill: 35%, Text: 20%, Project: 15%, Experience: 10%, Education: 10%, Quality: 10%
    exp.overallScore =
        (exp.skillScore * 0.35) +
        (exp.textScore * 0.20) +
        (exp.projectScore * 0.15) +
        (exp.experienceScore * 0.10) +
        (exp.educationScore * 0.10) +
        (exp.resumeQualityScore * 0.10);

    exp.overallScore = std::max(0.0, std::min(100.0, exp.overallScore));
    exp.matchClassification = MatchExplanation::getClassificationLabel(exp.overallScore);

    // Identify relevant keywords
    std::string resText = buildResumeFullText(resume);
    std::string jobText = buildJobFullText(job);
    std::string normResText = TextPreprocessor::normalizeText(resText);
    auto jobKws = KeywordExtractor::extractKeywords(jobText);
    for (const auto& kw : jobKws) {
        if (normResText.find(kw) != std::string::npos) {
            exp.relevantKeywords.push_back(kw);
        }
    }

    // Strengths identification
    if (exp.skillScore >= 80.0) {
        exp.strengths.push_back("Strong technical skill coverage for required job stack");
    } else if (exp.skillScore >= 60.0) {
        exp.strengths.push_back("Good foundational technical skill alignment");
    }

    if (!exp.relatedMatches.empty()) {
        exp.strengths.push_back("Demonstrated experience with related technologies in candidate background");
    }

    if (exp.projectScore >= 70.0) {
        exp.strengths.push_back("High project alignment directly targeting core job domain");
    }

    if (exp.experienceScore >= 70.0) {
        exp.strengths.push_back("Relevant practical experience matching role responsibilities");
    }

    if (exp.educationScore >= 90.0) {
        exp.strengths.push_back("Direct educational qualification alignment in Computer Engineering / Science");
    }

    if (exp.resumeQualityScore >= 85.0) {
        exp.strengths.push_back("Comprehensive and well-structured resume profile");
    }

    // Improvement areas
    for (const auto& mr : exp.missingRequiredSkills) {
        exp.improvementAreas.push_back("Focus on gaining proficiency in required technology: " + mr);
    }
    for (const auto& mp : exp.missingPreferredSkills) {
        exp.improvementAreas.push_back("Consider developing skills in preferred technology: " + mp);
    }
    if (exp.projectScore < 50.0 && !projectsCount(resume)) {
        exp.improvementAreas.push_back("Build domain-specific projects demonstrating core position technologies");
    }

    return exp;
}
