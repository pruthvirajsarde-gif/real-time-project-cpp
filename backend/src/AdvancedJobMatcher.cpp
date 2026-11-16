#include "AdvancedJobMatcher.h"
#include "SemanticMatcher.h"
#include "ResumeAnalyzer.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

MatchExplanation AdvancedJobMatcher::explainMatch(const Resume& resume, const Job& job) {
    return SemanticMatcher::evaluate(resume, job);
}

double AdvancedJobMatcher::calculateAdvancedMatchScore(const Resume& resume, const Job& job) {
    return explainMatch(resume, job).overallScore;
}

std::vector<std::pair<Job, MatchExplanation>> AdvancedJobMatcher::rankJobsAdvanced(
    const Resume& resume,
    const std::vector<Job>& jobs) {

    std::vector<std::pair<Job, MatchExplanation>> ranked;
    ranked.reserve(jobs.size());

    for (const auto& j : jobs) {
        MatchExplanation exp = explainMatch(resume, j);
        ranked.emplace_back(j, exp);
    }

    std::sort(ranked.begin(), ranked.end(),
              [](const auto& a, const auto& b) {
                  if (std::abs(a.second.overallScore - b.second.overallScore) > 0.0001) {
                      return a.second.overallScore > b.second.overallScore;
                  }
                  return a.first.getJobId() < b.first.getJobId();
              });

    return ranked;
}

std::vector<std::pair<Job, MatchExplanation>> AdvancedJobMatcher::getTopMatchesAdvanced(
    const Resume& resume,
    const std::vector<Job>& jobs,
    int count) {

    auto ranked = rankJobsAdvanced(resume, jobs);
    if (count > 0 && static_cast<int>(ranked.size()) > count) {
        ranked.resize(count);
    }
    return ranked;
}

void AdvancedJobMatcher::displayAdvancedJobRankings(const Resume& resume, const std::vector<Job>& jobs) {
    auto ranked = rankJobsAdvanced(resume, jobs);

    std::cout << "\n========================================================================\n";
    std::cout << "                  ADVANCED JOB MATCH RANKINGS (NLP)                    \n";
    std::cout << "========================================================================\n\n";

    if (ranked.empty()) {
        std::cout << "  No jobs available for ranking.\n\n";
        std::cout << "========================================================================\n";
        return;
    }

    std::cout << std::left
              << std::setw(6)  << "Rank"
              << std::setw(8)  << "Job ID"
              << std::setw(28) << "Title"
              << std::setw(20) << "Company"
              << std::setw(12) << "Score"
              << "Classification"
              << "\n";
    std::cout << std::string(88, '-') << "\n";

    for (size_t i = 0; i < ranked.size(); ++i) {
        const auto& item = ranked[i];
        const Job& job = item.first;
        const MatchExplanation& exp = item.second;

        std::cout << std::left
                  << std::setw(6)  << (std::to_string(i + 1) + ".")
                  << std::setw(8)  << job.getJobId()
                  << std::setw(28) << (job.getJobTitle().length() > 26 ? job.getJobTitle().substr(0, 24) + ".." : job.getJobTitle())
                  << std::setw(20) << (job.getCompany().length() > 18 ? job.getCompany().substr(0, 16) + ".." : job.getCompany())
                  << std::fixed << std::setprecision(2)
                  << std::setw(12) << (std::to_string(exp.overallScore).substr(0, 5) + "%")
                  << exp.matchClassification
                  << "\n";
    }
    std::cout << std::string(88, '-') << "\n\n";
}

void AdvancedJobMatcher::displayAdvancedMatchDetails(const Resume& resume, const Job& job) {
    MatchExplanation exp = explainMatch(resume, job);
    MatchExplanation::displayMatchExplanation(exp, job, resume.getName());
}

void AdvancedJobMatcher::displayComparisonWithClassic(const Resume& resume, const Job& job) {
    double classicScore = ResumeAnalyzer::calculateOverallMatchScore(resume, job);
    MatchExplanation advancedExpl = explainMatch(resume, job);
    MatchExplanation::displayMatcherComparison(advancedExpl, classicScore, job);
}
