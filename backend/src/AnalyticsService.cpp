#include "AnalyticsService.h"
#include "ResumeRepository.h"
#include "JobRepository.h"
#include "CandidateRepository.h"
#include "AccountRepository.h"
#include "SavedJobManager.h"
#include "AdvancedJobMatcher.h"
#include "SkillGapAnalyzer.h"
#include "RecommendationEngine.h"
#include "ResumeStrengthAnalyzer.h"
#include "ResumeAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <set>

namespace {
    // Helper to calculate candidate years of experience from Resume
    double calculateCandidateYears(const Resume& resume) {
        double totalYears = 0.0;
        for (const auto& exp : resume.getExperience()) {
            std::string dur = exp.getDuration();
            // Parse common patterns like "2 years", "6 months", "1.5 yrs"
            double val = 0.0;
            std::istringstream iss(dur);
            if (iss >> val) {
                std::string unit;
                iss >> unit;
                std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
                if (unit.find("month") != std::string::npos) {
                    totalYears += (val / 12.0);
                } else {
                    totalYears += val;
                }
            } else {
                // If text cannot be parsed directly, assume 1 year per experience item
                totalYears += 1.0;
            }
        }
        return totalYears;
    }

    std::string toLowerString(const std::string& str) {
        std::string lower = str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower;
    }
}

CandidateAnalytics AnalyticsService::getCandidateAnalytics(int candidateId) {
    CandidateAnalytics ca;

    // Applications statistics
    std::vector<ApplicationRecord> apps = ApplicationManager::getCandidateApplications(candidateId);
    ca.totalApplications = static_cast<int>(apps.size());
    ca.savedJobsCount = SavedJobManager::countSavedJobs(candidateId);

    double scoreSum = 0.0;
    int scoreCount = 0;
    double maxScore = 0.0;

    for (const auto& app : apps) {
        ca.statusBreakdown[app.status]++;
        if (app.status == ApplicationManager::STATUS_WITHDRAWN) {
            ca.withdrawnApplications++;
        } else {
            ca.activeApplications++;
        }

        if (app.matchScore > 0.0) {
            scoreSum += app.matchScore;
            scoreCount++;
            if (app.matchScore > maxScore) {
                maxScore = app.matchScore;
            }
        }
    }

    if (scoreCount > 0) {
        ca.avgMatchScore = std::round((scoreSum / scoreCount) * 10.0) / 10.0;
        ca.highestMatchScore = std::round(maxScore * 10.0) / 10.0;
    }

    // Resume health & readiness
    if (ResumeRepository::resumeExists(candidateId)) {
        Resume resume = ResumeRepository::loadResume(candidateId);
        ca.resumeQualityScore = ResumeRepository::getResumeQualityScore(candidateId);
        ca.profileCompleteness = resume.calculateCompletionPercentage();

        std::vector<Job> allJobs = JobRepository::getAllJobs();
        if (!allJobs.empty()) {
            double totalCoverage = 0.0;
            double totalEduScore = 0.0;
            double totalExpScore = 0.0;
            double totalReadiness = 0.0;

            for (const auto& job : allJobs) {
                totalCoverage += SkillGapAnalyzer::calculateSkillCoveragePercentage(resume, job);
                totalEduScore += ResumeAnalyzer::calculateEducationMatchPercentage(resume, job);
                totalExpScore += ResumeAnalyzer::calculateExperienceMatchPercentage(resume, job);
                totalReadiness += RecommendationEngine::calculateJobReadinessScore(resume, job);
            }

            size_t n = allJobs.size();
            ca.skillCoverage = std::round((totalCoverage / n) * 10.0) / 10.0;
            ca.educationCoverage = std::round((totalEduScore / n) * 10.0) / 10.0;
            ca.experienceCoverage = std::round((totalExpScore / n) * 10.0) / 10.0;
            ca.overallReadiness = std::round((totalReadiness / n) * 10.0) / 10.0;
        }

        ca.skillReadiness = ca.skillCoverage;
        ca.educationReadiness = ca.educationCoverage;
        ca.experienceReadiness = ca.experienceCoverage;

        // Categorize Readiness Level
        if (ca.overallReadiness >= 75.0) {
            ca.readinessLevel = "High Career Readiness";
        } else if (ca.overallReadiness >= 50.0) {
            ca.readinessLevel = "Moderate Career Readiness";
        } else {
            ca.readinessLevel = "Foundational / Growth Potential";
        }

        ca.readinessStrengths = ResumeStrengthAnalyzer::getStrengths(resume);
        ca.readinessImprovements = ResumeStrengthAnalyzer::getWeakAreas(resume);
    } else {
        ca.readinessLevel = "No Resume Uploaded";
        ca.readinessImprovements.push_back("Upload or import a resume to unlock career readiness and health metrics.");
    }

    return ca;
}

CandidateSkillGapAnalytics AnalyticsService::getCandidateSkillGapAnalytics(int candidateId) {
    CandidateSkillGapAnalytics sga;
    std::vector<Job> allJobs = JobRepository::getAllJobs();

    if (!ResumeRepository::resumeExists(candidateId)) {
        // Return catalog-level demand even if candidate hasn't uploaded a resume
        std::map<std::string, int> reqCounts;
        std::map<std::string, int> prefCounts;
        for (const auto& job : allJobs) {
            for (const auto& s : job.getRequiredSkills()) reqCounts[s]++;
            for (const auto& s : job.getPreferredSkills()) prefCounts[s]++;
        }

        for (const auto& [s, count] : reqCounts) {
            MissingSkillDemand m;
            m.skill = s;
            m.jobCount = count;
            m.demandPercentage = allJobs.empty() ? 0.0 : std::round((count * 100.0 / allJobs.size()) * 10.0) / 10.0;
            m.priority = (m.demandPercentage >= 35.0) ? "HIGH" : (m.demandPercentage >= 15.0 ? "MEDIUM" : "LOW");
            sga.prioritizedMissingRequired.push_back(m);
        }

        std::sort(sga.prioritizedMissingRequired.begin(), sga.prioritizedMissingRequired.end(),
                  [](const MissingSkillDemand& a, const MissingSkillDemand& b) {
                      return a.jobCount > b.jobCount;
                  });

        return sga;
    }

    Resume resume = ResumeRepository::loadResume(candidateId);
    for (const auto& sk : resume.getSkills()) {
        sga.candidateSkills.push_back(sk.getName());
    }

    std::map<std::string, int> missingRequiredTally;
    std::map<std::string, int> missingPreferredTally;
    std::set<std::string> exactMatchesSet;
    std::set<std::string> relatedSet;
    double sumCoverage = 0.0;

    for (const auto& job : allJobs) {
        sumCoverage += SkillGapAnalyzer::calculateSkillCoveragePercentage(resume, job);

        std::vector<std::string> matched = SkillGapAnalyzer::findMatchingSkills(resume, job);
        for (const auto& m : matched) exactMatchesSet.insert(m);

        std::vector<std::string> missingReq = SkillGapAnalyzer::findMissingRequiredSkills(resume, job);
        for (const auto& mr : missingReq) missingRequiredTally[mr]++;

        std::vector<std::string> missingPref = SkillGapAnalyzer::findMissingPreferredSkills(resume, job);
        for (const auto& mp : missingPref) missingPreferredTally[mp]++;

        MatchExplanation expl = AdvancedJobMatcher::explainMatch(resume, job);
        for (const auto& rel : expl.relatedMatches) relatedSet.insert(rel);
    }

    if (!allJobs.empty()) {
        sga.catalogSkillCoverage = std::round((sumCoverage / allJobs.size()) * 10.0) / 10.0;
    }

    for (const auto& em : exactMatchesSet) sga.exactMatchesAcrossJobs.push_back(em);
    for (const auto& r : relatedSet) sga.relatedSkills.push_back(r);

    for (const auto& [skill, count] : missingRequiredTally) {
        MissingSkillDemand item;
        item.skill = skill;
        item.jobCount = count;
        item.demandPercentage = allJobs.empty() ? 0.0 : std::round((count * 100.0 / allJobs.size()) * 10.0) / 10.0;
        item.priority = (item.demandPercentage >= 35.0) ? "HIGH" : (item.demandPercentage >= 15.0 ? "MEDIUM" : "LOW");
        sga.prioritizedMissingRequired.push_back(item);
    }

    for (const auto& [skill, count] : missingPreferredTally) {
        MissingSkillDemand item;
        item.skill = skill;
        item.jobCount = count;
        item.demandPercentage = allJobs.empty() ? 0.0 : std::round((count * 100.0 / allJobs.size()) * 10.0) / 10.0;
        item.priority = (item.demandPercentage >= 30.0) ? "HIGH" : (item.demandPercentage >= 15.0 ? "MEDIUM" : "LOW");
        sga.prioritizedMissingPreferred.push_back(item);
    }

    std::sort(sga.prioritizedMissingRequired.begin(), sga.prioritizedMissingRequired.end(),
              [](const MissingSkillDemand& a, const MissingSkillDemand& b) {
                  return a.jobCount > b.jobCount;
              });

    std::sort(sga.prioritizedMissingPreferred.begin(), sga.prioritizedMissingPreferred.end(),
              [](const MissingSkillDemand& a, const MissingSkillDemand& b) {
                  return a.jobCount > b.jobCount;
              });

    return sga;
}

MarketInsights AnalyticsService::getMarketInsights() {
    MarketInsights mi;
    std::vector<Job> allJobs = JobRepository::getAllJobs();
    mi.totalJobs = static_cast<int>(allJobs.size());

    std::map<std::string, int> totalFreq;
    std::map<std::string, int> reqFreq;
    std::map<std::string, int> prefFreq;
    std::map<std::pair<std::string, std::string>, int> coPairs;

    for (const auto& job : allJobs) {
        std::string loc = job.getLocation().empty() ? "Unspecified" : job.getLocation();
        mi.jobsByLocation[loc]++;

        std::string emp = job.getEmploymentType().empty() ? "Full-time" : job.getEmploymentType();
        mi.jobsByEmploymentType[emp]++;

        // Tally required
        std::vector<std::string> reqs = job.getRequiredSkills();
        for (const auto& s : reqs) {
            totalFreq[s]++;
            reqFreq[s]++;
        }

        // Tally preferred
        for (const auto& s : job.getPreferredSkills()) {
            totalFreq[s]++;
            prefFreq[s]++;
        }

        // Co-occurrences in required skills
        for (size_t i = 0; i < reqs.size(); ++i) {
            for (size_t j = i + 1; j < reqs.size(); ++j) {
                std::string a = reqs[i];
                std::string b = reqs[j];
                if (a > b) std::swap(a, b);
                coPairs[{a, b}]++;
            }
        }
    }

    // Top skills
    for (const auto& [skill, count] : totalFreq) {
        SkillFrequency sf;
        sf.skill = skill;
        sf.totalCount = count;
        sf.requiredCount = reqFreq[skill];
        sf.preferredCount = prefFreq[skill];
        sf.percentage = mi.totalJobs > 0 ? std::round((count * 100.0 / mi.totalJobs) * 10.0) / 10.0 : 0.0;
        mi.topSkills.push_back(sf);
    }

    std::sort(mi.topSkills.begin(), mi.topSkills.end(),
              [](const SkillFrequency& a, const SkillFrequency& b) {
                  return a.totalCount > b.totalCount;
              });

    // Top co-occurrences
    std::vector<std::pair<std::pair<std::string, std::string>, int>> sortedPairs(coPairs.begin(), coPairs.end());
    std::sort(sortedPairs.begin(), sortedPairs.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });

    for (size_t i = 0; i < sortedPairs.size() && i < 15; ++i) {
        SkillCoOccurrence sc;
        sc.skillA = sortedPairs[i].first.first;
        sc.skillB = sortedPairs[i].first.second;
        sc.count = sortedPairs[i].second;
        mi.skillCoOccurrences.push_back(sc);
    }

    return mi;
}

std::vector<RecommendedJob> AnalyticsService::getCandidateRecommendations(int candidateId, size_t limit) {
    std::vector<RecommendedJob> recs;
    std::vector<Job> allJobs = JobRepository::getAllJobs();
    if (allJobs.empty()) return recs;

    if (!ResumeRepository::resumeExists(candidateId)) {
        // Return latest jobs with neutral reason
        for (size_t i = 0; i < allJobs.size() && i < limit; ++i) {
            const auto& job = allJobs[i];
            RecommendedJob rj;
            rj.jobId = job.getJobId();
            rj.jobTitle = job.getJobTitle();
            rj.company = job.getCompany();
            rj.location = job.getLocation();
            rj.matchScore = 0.0;
            rj.readinessScore = 0.0;
            rj.explainableReasons.push_back("Active position in " + job.getLocation());
            rj.explainableReasons.push_back("Upload resume to generate tailored match ranking");
            recs.push_back(rj);
        }
        return recs;
    }

    Resume resume = ResumeRepository::loadResume(candidateId);
    Profile profile = CandidateRepository::getCandidate(candidateId);
    std::string candidateLoc = profile.getLocation();
    double candidateYears = calculateCandidateYears(resume);

    auto ranked = AdvancedJobMatcher::rankJobsAdvanced(resume, allJobs);

    for (size_t i = 0; i < ranked.size() && i < limit; ++i) {
        const auto& [job, expl] = ranked[i];
        RecommendedJob rj;
        rj.jobId = job.getJobId();
        rj.jobTitle = job.getJobTitle();
        rj.company = job.getCompany();
        rj.location = job.getLocation();
        rj.matchScore = expl.overallScore;
        rj.readinessScore = RecommendationEngine::calculateJobReadinessScore(resume, job);

        for (const auto& s : expl.exactMatches) rj.topMatchedSkills.push_back(s);
        for (const auto& s : expl.missingRequiredSkills) rj.missingRequiredSkills.push_back(s);

        // Build explainable reasons
        size_t totalReq = job.getRequiredSkills().size();
        size_t matchedReq = totalReq - expl.missingRequiredSkills.size();
        if (totalReq > 0) {
            rj.explainableReasons.push_back(
                std::to_string(matchedReq) + "/" + std::to_string(totalReq) + " required technical skills matched"
            );
        }

        if (expl.textScore >= 60.0) {
            rj.explainableReasons.push_back("High semantic synergy with candidate project experience");
        }

        if (candidateYears >= job.getMinimumExperience()) {
            rj.explainableReasons.push_back(
                "Experience requirement satisfied (" + std::to_string(static_cast<int>(candidateYears)) +
                " yrs vs " + std::to_string(job.getMinimumExperience()) + " yrs required)"
            );
        }

        if (!candidateLoc.empty() && !job.getLocation().empty() &&
            toLowerString(candidateLoc).find(toLowerString(job.getLocation())) != std::string::npos) {
            rj.explainableReasons.push_back("Preferred location matches (" + job.getLocation() + ")");
        }

        if (rj.explainableReasons.empty()) {
            rj.explainableReasons.push_back("Compatible role based on profile attributes");
        }

        recs.push_back(rj);
    }

    return recs;
}

ResumeJobComparison AnalyticsService::compareResumeToJob(
    int candidateId, const std::string& jobId, bool& outFound
) {
    ResumeJobComparison cmp;
    if (!JobRepository::jobExists(jobId)) {
        outFound = false;
        return cmp;
    }

    outFound = true;
    Job job = JobRepository::getJob(jobId);
    cmp.jobId = job.getJobId();
    cmp.jobTitle = job.getJobTitle();
    cmp.company = job.getCompany();
    cmp.location = job.getLocation();
    cmp.requiredEducation = job.getEducationRequirement().empty() ? "Degree in Engineering / CS or equivalent" : job.getEducationRequirement();
    cmp.requiredExperience = std::to_string(job.getMinimumExperience()) + " years minimum";

    if (!ResumeRepository::resumeExists(candidateId)) {
        cmp.candidateEducation = "No resume uploaded";
        cmp.candidateExperience = "No resume uploaded";
        cmp.educationCompatibility = "Mismatch / Missing";
        cmp.experienceCompatibility = "Not Specified";
        for (const auto& s : job.getRequiredSkills()) cmp.missingRequiredSkills.push_back(s);
        for (const auto& s : job.getPreferredSkills()) cmp.missingPreferredSkills.push_back(s);
        return cmp;
    }

    Resume resume = ResumeRepository::loadResume(candidateId);
    MatchExplanation expl = AdvancedJobMatcher::explainMatch(resume, job);

    cmp.overallScore = expl.overallScore;
    cmp.classification = expl.matchClassification;
    cmp.skillScore = expl.skillScore;
    cmp.textScore = expl.textScore;
    cmp.projectScore = expl.projectScore;
    cmp.experienceScore = expl.experienceScore;
    cmp.educationScore = expl.educationScore;
    cmp.resumeQualityScore = expl.resumeQualityScore;

    for (const auto& s : expl.exactMatches) cmp.matchedSkills.push_back(s);
    for (const auto& s : expl.relatedMatches) cmp.relatedSkills.push_back(s);
    for (const auto& s : expl.missingRequiredSkills) cmp.missingRequiredSkills.push_back(s);
    for (const auto& s : expl.missingPreferredSkills) cmp.missingPreferredSkills.push_back(s);

    // Education comparison
    if (!resume.getEducation().empty()) {
        const auto& edu = resume.getEducation()[0];
        cmp.candidateEducation = edu.getDegree() + " in " + edu.getFieldOfStudy() + " (" + edu.getInstitution() + ")";
        double eduMatch = ResumeAnalyzer::calculateEducationMatchPercentage(resume, job);
        if (eduMatch >= 80.0) {
            cmp.educationCompatibility = "Compatible";
        } else if (eduMatch > 0.0) {
            cmp.educationCompatibility = "Partially Compatible";
        } else {
            cmp.educationCompatibility = "Different Field";
        }
    } else {
        cmp.candidateEducation = "Not specified in resume";
        cmp.educationCompatibility = "Mismatch / Missing";
    }

    // Experience comparison
    double candYears = calculateCandidateYears(resume);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << candYears << " years total experience";
    cmp.candidateExperience = oss.str();

    if (candYears >= job.getMinimumExperience()) {
        cmp.experienceCompatibility = "Meets / Exceeds Requirement";
    } else {
        cmp.experienceCompatibility = "Below Requirement";
    }

    return cmp;
}

RecruiterAnalytics AnalyticsService::getRecruiterAnalytics(int recruiterAccountId) {
    RecruiterAnalytics ra;
    std::vector<std::string> ownedJobs = AccountRepository::getJobIdsForRecruiter(recruiterAccountId);
    ra.activeJobs = static_cast<int>(ownedJobs.size());

    // Initialize buckets
    ra.matchScoreDistribution["90-100%"] = 0;
    ra.matchScoreDistribution["80-89%"] = 0;
    ra.matchScoreDistribution["70-79%"] = 0;
    ra.matchScoreDistribution["60-69%"] = 0;
    ra.matchScoreDistribution["<60%"] = 0;

    std::vector<ApplicationRecord> allApps = ApplicationManager::getAllApplications();
    double matchSum = 0.0;
    int matchCount = 0;

    for (const auto& a : allApps) {
        if (std::find(ownedJobs.begin(), ownedJobs.end(), a.jobId) != ownedJobs.end()) {
            ra.totalApplications++;
            ra.pipelineFunnel[a.status]++;

            if (a.matchScore > 0.0) {
                matchSum += a.matchScore;
                matchCount++;

                if (a.matchScore >= 90.0) ra.matchScoreDistribution["90-100%"]++;
                else if (a.matchScore >= 80.0) ra.matchScoreDistribution["80-89%"]++;
                else if (a.matchScore >= 70.0) ra.matchScoreDistribution["70-79%"]++;
                else if (a.matchScore >= 60.0) ra.matchScoreDistribution["60-69%"]++;
                else ra.matchScoreDistribution["<60%"]++;
            }
        }
    }

    if (matchCount > 0) {
        ra.averageMatchScore = std::round((matchSum / matchCount) * 10.0) / 10.0;
    }

    return ra;
}

RecruiterJobAnalytics AnalyticsService::getRecruiterJobAnalytics(
    int recruiterAccountId, const std::string& jobId, bool& outFound, bool& outOwned
) {
    RecruiterJobAnalytics rja;
    if (!JobRepository::jobExists(jobId)) {
        outFound = false;
        outOwned = false;
        return rja;
    }
    outFound = true;

    int ownerId = AccountRepository::getRecruiterAccountIdForJob(jobId);
    if (ownerId != recruiterAccountId) {
        outOwned = false;
        return rja;
    }
    outOwned = true;

    Job job = JobRepository::getJob(jobId);
    rja.jobId = job.getJobId();
    rja.jobTitle = job.getJobTitle();
    rja.company = job.getCompany();
    rja.location = job.getLocation();
    rja.status = "ACTIVE";

    rja.matchDistribution["90-100%"] = 0;
    rja.matchDistribution["80-89%"] = 0;
    rja.matchDistribution["70-79%"] = 0;
    rja.matchDistribution["60-69%"] = 0;
    rja.matchDistribution["<60%"] = 0;

    std::vector<ApplicationRecord> allApps = ApplicationManager::getAllApplications();
    double scoreSum = 0.0;
    double maxScore = 0.0;
    double minScore = 100.0;
    double skillCoverageSum = 0.0;

    for (const auto& a : allApps) {
        if (a.jobId == jobId) {
            rja.totalApplicants++;
            rja.stageDistribution[a.status]++;

            double score = a.matchScore;
            scoreSum += score;
            if (score > maxScore) maxScore = score;
            if (score < minScore) minScore = score;

            if (score >= 90.0) rja.matchDistribution["90-100%"]++;
            else if (score >= 80.0) rja.matchDistribution["80-89%"]++;
            else if (score >= 70.0) rja.matchDistribution["70-79%"]++;
            else if (score >= 60.0) rja.matchDistribution["60-69%"]++;
            else rja.matchDistribution["<60%"]++;

            if (ResumeRepository::resumeExists(a.candidateId)) {
                Resume r = ResumeRepository::loadResume(a.candidateId);
                skillCoverageSum += SkillGapAnalyzer::calculateSkillCoveragePercentage(r, job);
            }
        }
    }

    if (rja.totalApplicants > 0) {
        rja.averageMatchScore = std::round((scoreSum / rja.totalApplicants) * 10.0) / 10.0;
        rja.highestMatchScore = std::round(maxScore * 10.0) / 10.0;
        rja.lowestMatchScore = std::round(minScore * 10.0) / 10.0;
        rja.averageSkillCoverage = std::round((skillCoverageSum / rja.totalApplicants) * 10.0) / 10.0;
    } else {
        rja.lowestMatchScore = 0.0;
    }

    return rja;
}

std::vector<CandidateJobComparisonRow> AnalyticsService::compareJobCandidates(
    int recruiterAccountId, const std::string& jobId, bool& outFound, bool& outOwned
) {
    std::vector<CandidateJobComparisonRow> rows;
    if (!JobRepository::jobExists(jobId)) {
        outFound = false;
        outOwned = false;
        return rows;
    }
    outFound = true;

    int ownerId = AccountRepository::getRecruiterAccountIdForJob(jobId);
    if (ownerId != recruiterAccountId) {
        outOwned = false;
        return rows;
    }
    outOwned = true;

    Job job = JobRepository::getJob(jobId);
    std::vector<ApplicationRecord> allApps = ApplicationManager::getAllApplications();

    for (const auto& a : allApps) {
        if (a.jobId == jobId) {
            CandidateJobComparisonRow row;
            row.candidateId = a.candidateId;
            row.candidateName = a.candidateName;
            row.applicationId = a.id;
            row.appliedDate = a.appliedAt;
            row.status = a.status;
            row.matchScore = a.matchScore;
            row.readinessScore = a.readinessScore;
            row.totalRequiredSkills = static_cast<int>(job.getRequiredSkills().size());
            row.totalPreferredSkills = static_cast<int>(job.getPreferredSkills().size());

            if (ResumeRepository::resumeExists(a.candidateId)) {
                Resume r = ResumeRepository::loadResume(a.candidateId);
                auto missingReq = SkillGapAnalyzer::findMissingRequiredSkills(r, job);
                row.requiredSkillsMatched = row.totalRequiredSkills - static_cast<int>(missingReq.size());

                auto missingPref = SkillGapAnalyzer::findMissingPreferredSkills(r, job);
                row.preferredSkillsMatched = row.totalPreferredSkills - static_cast<int>(missingPref.size());

                row.matchedSkills = SkillGapAnalyzer::findMatchingSkills(r, job);
                for (const auto& mr : missingReq) row.missingSkills.push_back(mr);

                double years = calculateCandidateYears(r);
                std::ostringstream yss;
                yss << std::fixed << std::setprecision(1) << years << " yrs";
                row.totalExperience = yss.str();

                if (!r.getEducation().empty()) {
                    row.educationSummary = r.getEducation()[0].getDegree() + " (" + r.getEducation()[0].getFieldOfStudy() + ")";
                } else {
                    row.educationSummary = "Not specified";
                }
            } else {
                row.requiredSkillsMatched = 0;
                row.preferredSkillsMatched = 0;
                row.totalExperience = "N/A";
                row.educationSummary = "N/A";
                for (const auto& s : job.getRequiredSkills()) row.missingSkills.push_back(s);
            }

            rows.push_back(row);
        }
    }

    std::sort(rows.begin(), rows.end(),
              [](const CandidateJobComparisonRow& a, const CandidateJobComparisonRow& b) {
                  return a.matchScore > b.matchScore;
              });

    return rows;
}
