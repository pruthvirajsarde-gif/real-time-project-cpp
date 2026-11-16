#include "ReportGenerator.h"
#include "ResumeAnalyzer.h"
#include "ResumeStrengthAnalyzer.h"
#include "SkillGapAnalyzer.h"
#include "RecommendationEngine.h"
#include "ResumeDocumentReader.h"
#include "ApplicationManager.h"
#include "SavedJobManager.h"
#include "AdvancedJobMatcher.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

bool ReportGenerator::saveAnalysisReport(
        const std::string& filePath,
        const Resume& resume,
        const std::vector<JobMatchResult>& results,
        const std::vector<Job>& jobs,
        const std::string& resumeSourcePath,
        int candidateId,
        const std::string& accountRole,
        const std::string& authenticatedUser) {

    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "  [ERROR] Could not create report file: "
                  << filePath << "\n";
        return false;
    }

    file << std::fixed << std::setprecision(2);

    file << "========================================\n";
    file << "           SMARTHIRE REPORT             \n";
    file << "========================================\n\n";

    // ── Phase 5 Document Metadata ───────────────────────
    if (!resumeSourcePath.empty()) {
        std::string ext = ResumeDocumentReader::getFileExtension(resumeSourcePath);
        std::string fmt = (ext == ".pdf") ? "PDF" : "TXT";
        file << "Resume Source:\n";
        file << "  " << resumeSourcePath << "\n\n";
        file << "Resume Format:\n";
        file << "  " << fmt << "\n\n";
        file << "Text Extraction:\n";
        file << "  Successful\n\n";
        file << "----------------------------------------\n";
    }

    // ── 1. Candidate Info ───────────────────────────────
    file << "Candidate:\n";
    file << "  " << resume.getName() << "\n\n";

    file << "Email:\n";
    file << "  " << (resume.getEmail().empty() ? "(not provided)" : resume.getEmail()) << "\n\n";

    file << "Phone:\n";
    file << "  " << (resume.getPhone().empty() ? "(not provided)" : resume.getPhone()) << "\n\n";

    file << "Location:\n";
    file << "  " << (resume.getLocation().empty() ? "(not provided)" : resume.getLocation()) << "\n\n";

    // ── 2. Resume Summary ───────────────────────────────
    file << "----------------------------------------\n";
    file << "RESUME SUMMARY\n";
    file << "----------------------------------------\n\n";

    file << "Skills         : " << resume.getSkills().size() << "\n";
    file << "Education      : " << resume.getEducation().size() << "\n";
    file << "Projects       : " << resume.getProjects().size() << "\n";
    file << "Experience     : " << resume.getExperience().size() << "\n";
    file << "Certifications : " << resume.getCertifications().size() << "\n";

    // List skills
    if (!resume.getSkills().empty()) {
        file << "\nSkill List:\n";
        for (const auto& s : resume.getSkills()) {
            file << "  - " << s.getName() << "\n";
        }
    }
    file << "\n";

    // ── 3. Job Matching Results ─────────────────────────
    file << "----------------------------------------\n";
    file << "JOB MATCHING RESULTS\n";
    file << "----------------------------------------\n\n";

    if (results.empty()) {
        file << "No job matches available.\n\n";
    } else {
        for (size_t i = 0; i < results.size(); ++i) {
            std::string category = ResumeAnalyzer::getMatchCategory(results[i].score);
            file << (i + 1) << ". " << results[i].jobTitle << "\n";
            file << "   Company     : " << results[i].company << "\n";
            file << "   Match Score : " << results[i].score << "%\n";
            file << "   Category    : " << category << "\n\n";
        }

        // ── Summary Statistics ──────────────────────────
        file << "----------------------------------------\n";
        file << "SUMMARY\n";
        file << "----------------------------------------\n\n";

        int strongCount = 0, goodCount = 0, partialCount = 0, lowCount = 0;
        double totalScore = 0.0;

        for (const auto& r : results) {
            totalScore += r.score;
            std::string cat = ResumeAnalyzer::getMatchCategory(r.score);
            if (cat == "Strong Match")       ++strongCount;
            else if (cat == "Good Match")    ++goodCount;
            else if (cat == "Partial Match") ++partialCount;
            else                             ++lowCount;
        }

        double avgScore = totalScore / static_cast<double>(results.size());

        file << "Total Jobs     : " << results.size() << "\n";
        file << "Strong Matches : " << strongCount << "\n";
        file << "Good Matches   : " << goodCount << "\n";
        file << "Partial Matches: " << partialCount << "\n";
        file << "Low Matches    : " << lowCount << "\n\n";
        file << "Average Score  : " << avgScore << "%\n\n";

        file << "Top Match:\n";
        file << "  " << results[0].jobTitle << "\n";
        file << "  " << results[0].company << "\n";
        file << "  " << results[0].score << "%\n\n";
    }

    // ── 4. Phase 4: Smart Resume Analysis ───────────────
    double qualityScore = ResumeStrengthAnalyzer::calculateResumeQualityScore(resume);
    auto qualityBreakdown = ResumeStrengthAnalyzer::getQualityBreakdown(resume);
    auto strengths = ResumeStrengthAnalyzer::getStrengths(resume);
    auto weakAreas = ResumeStrengthAnalyzer::getWeakAreas(resume);
    auto techFreq = ResumeStrengthAnalyzer::calculateTechnologyFrequency(resume);

    file << "----------------------------------------\n";
    file << "SMART RESUME ANALYSIS\n";
    file << "----------------------------------------\n\n";

    file << "Resume Quality Score: " << qualityScore << "%\n\n";

    file << "Quality Score Breakdown:\n";
    file << "  - Profile Completeness : " << qualityBreakdown.profileScore << " / 15.0\n";
    file << "  - Technical Skills     : " << qualityBreakdown.skillsScore << " / 20.0\n";
    file << "  - Education Details    : " << qualityBreakdown.educationScore << " / 15.0\n";
    file << "  - Project Portfolio    : " << qualityBreakdown.projectsScore << " / 20.0\n";
    file << "  - Practical Experience : " << qualityBreakdown.experienceScore << " / 20.0\n";
    file << "  - Certifications       : " << qualityBreakdown.certificationsScore << " / 10.0\n\n";

    file << "Resume Strengths:\n";
    for (const auto& s : strengths) {
        file << "  ✓ " << s << "\n";
    }
    file << "\n";

    file << "Resume Weak Areas:\n";
    if (weakAreas.empty()) {
        file << "  ✓ No significant weak areas detected.\n";
    } else {
        for (const auto& w : weakAreas) {
            file << "  • " << w << "\n";
        }
    }
    file << "\n";

    file << "Technology Frequency:\n";
    if (techFreq.empty()) {
        file << "  (None detected)\n";
    } else {
        std::vector<std::pair<std::string, int>> sortedFreq(techFreq.begin(), techFreq.end());
        std::sort(sortedFreq.begin(), sortedFreq.end(),
                  [](const auto& a, const auto& b) {
                      if (a.second != b.second) return a.second > b.second;
                      return a.first < b.first;
                  });
        for (const auto& item : sortedFreq) {
            file << "  - " << item.first << ": " << item.second << "\n";
        }
    }
    file << "\n";

    // ── 5. Phase 4: Job-Specific Gap Analysis & Recommendations ───
    if (!jobs.empty()) {
        file << "----------------------------------------\n";
        file << "JOB READINESS & GAP ANALYSIS\n";
        file << "----------------------------------------\n\n";

        for (const auto& res : results) {
            const Job* jobPtr = nullptr;
            for (const auto& j : jobs) {
                if (j.getJobId() == res.jobId || j.getJobTitle() == res.jobTitle) {
                    jobPtr = &j;
                    break;
                }
            }
            if (!jobPtr) continue;

            const Job& job = *jobPtr;
            double coverage = SkillGapAnalyzer::calculateSkillCoveragePercentage(resume, job);
            double gap = SkillGapAnalyzer::calculateSkillGapPercentage(resume, job);
            auto missingReq = SkillGapAnalyzer::findMissingRequiredSkills(resume, job);
            auto missingPref = SkillGapAnalyzer::findMissingPreferredSkills(resume, job);
            double readiness = RecommendationEngine::calculateJobReadinessScore(resume, job);
            auto recs = RecommendationEngine::generateRecommendations(resume, job);

            file << job.getJobTitle();
            if (!job.getCompany().empty()) {
                file << " (" << job.getCompany() << ")";
            }
            file << "\n";
            file << "  Match Score             : " << res.score << "%\n";
            file << "  Skill Coverage          : " << coverage << "%\n";
            file << "  Skill Gap               : " << gap << "%\n";
            file << "  Job Readiness Score     : " << readiness << "%\n";

            file << "  Missing Required Skills : ";
            if (missingReq.empty()) {
                file << "None (All matched)\n";
            } else {
                for (size_t k = 0; k < missingReq.size(); ++k) {
                    file << missingReq[k] << (k + 1 < missingReq.size() ? ", " : "\n");
                }
            }

            file << "  Missing Preferred Skills: ";
            if (missingPref.empty()) {
                file << "None\n";
            } else {
                for (size_t k = 0; k < missingPref.size(); ++k) {
                    file << missingPref[k] << (k + 1 < missingPref.size() ? ", " : "\n");
                }
            }

            file << "  Targeted Recommendations:\n";
            for (size_t k = 0; k < recs.size(); ++k) {
                file << "    " << (k + 1) << ". " << recs[k] << "\n";
            }
            file << "\n";
        }
    }

    // ── 6. Phase 7: Advanced Match Analysis ───────────────────────
    if (!jobs.empty()) {
        file << "----------------------------------------\n";
        file << "ADVANCED MATCH ANALYSIS\n";
        file << "========================================\n\n";

        const Job* topJob = &jobs[0];
        if (!results.empty()) {
            for (const auto& j : jobs) {
                if (j.getJobId() == results[0].jobId || j.getJobTitle() == results[0].jobTitle) {
                    topJob = &j;
                    break;
                }
            }
        }

        MatchExplanation exp = AdvancedJobMatcher::explainMatch(resume, *topJob);

        file << "Job:\n" << topJob->getJobTitle() << "\n\n";
        file << "Company:\n" << topJob->getCompany() << "\n\n";
        file << "Overall Match:\n" << exp.overallScore << "%\n\n";
        file << "Skill Match:\n" << exp.skillScore << "%\n\n";
        file << "Text Relevance:\n" << exp.textScore << "%\n\n";
        file << "Project Relevance:\n" << exp.projectScore << "%\n\n";
        file << "Experience Relevance:\n" << exp.experienceScore << "%\n\n";
        file << "Education Match:\n" << exp.educationScore << "%\n\n";
        file << "Resume Quality:\n" << exp.resumeQualityScore << "%\n\n";

        file << "----------------------------------------\n";
        file << "EXACT MATCHES\n";
        file << "----------------------------------------\n";
        if (exp.exactMatches.empty()) {
            file << "None\n\n";
        } else {
            for (const auto& s : exp.exactMatches) file << s << "\n";
            file << "\n";
        }

        file << "----------------------------------------\n";
        file << "RELATED MATCHES\n";
        file << "----------------------------------------\n";
        if (exp.relatedMatches.empty()) {
            file << "None\n\n";
        } else {
            for (const auto& s : exp.relatedMatches) file << s << "\n";
            file << "\n";
        }

        file << "----------------------------------------\n";
        file << "MISSING REQUIRED\n";
        file << "----------------------------------------\n";
        if (exp.missingRequiredSkills.empty()) {
            file << "None\n\n";
        } else {
            for (const auto& s : exp.missingRequiredSkills) file << s << "\n";
            file << "\n";
        }

        file << "----------------------------------------\n";
        file << "RECOMMENDATIONS\n";
        file << "----------------------------------------\n";
        if (exp.improvementAreas.empty()) {
            file << "Profile is well-aligned with this role.\n\n";
        } else {
            for (const auto& rec : exp.improvementAreas) file << rec << "\n";
            file << "\n";
        }
    }

    // ── 7. Phase 6: Database Summary ──────────────────────────────
    if (candidateId > 0) {
        int appsCount = ApplicationManager::countCandidateApplications(candidateId);
        int savedCount = SavedJobManager::countSavedJobs(candidateId);
        int selCount = ApplicationManager::countCandidateApplications(candidateId, ApplicationManager::STATUS_SELECTED);
        int intCount = ApplicationManager::countCandidateApplications(candidateId, ApplicationManager::STATUS_INTERVIEW);

        file << "----------------------------------------\n";
        file << "DATABASE SUMMARY\n";
        file << "----------------\n\n";
        file << "Candidate ID:\n" << candidateId << "\n\n";
        file << "Applications:\n" << appsCount << "\n\n";
        file << "Saved Jobs:\n" << savedCount << "\n\n";
        file << "Selected:\n" << selCount << "\n\n";
        file << "Interview:\n" << intCount << "\n\n";
    }

    // ── 8. Phase 8: Security Context ──────────────────────────────
    if (!accountRole.empty() || !authenticatedUser.empty() || candidateId > 0) {
        file << "----------------------------------------\n";
        file << "SECURITY CONTEXT\n";
        file << "========================================\n\n";
        file << "Candidate ID:\n" << (candidateId > 0 ? std::to_string(candidateId) : "N/A") << "\n\n";
        file << "Account Role:\n" << (accountRole.empty() ? "CANDIDATE" : accountRole) << "\n\n";
        file << "Authenticated:\n" << (!authenticatedUser.empty() ? "YES" : "NO") << "\n\n";
        file << "Generated By:\n" << (!authenticatedUser.empty() ? authenticatedUser : "Pruthviraj") << "\n\n";
        file << "========================================\n";
    }

    file << "----------------------------------------\n";
    file << "Generated by SmartHire\n";
    file << "========================================\n";

    file.close();
    return true;
}
