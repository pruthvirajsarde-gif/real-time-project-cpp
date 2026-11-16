#include "KeywordExtractor.h"
#include "TextPreprocessor.h"

#include <algorithm>

const std::vector<std::string> KeywordExtractor::MULTI_WORD_PHRASES = {
    "data structures",
    "machine learning",
    "object oriented programming",
    "database management",
    "web development",
    "software testing",
    "artificial intelligence",
    "computer networks",
    "rest api",
    "rest apis",
    "restful api",
    "full stack",
    "spring boot",
    "cloud computing",
    "unit testing",
    "version control",
    "relational database",
    "operating system",
    "deep learning",
    "microservices architecture",
    "system design",
    "embedded systems",
    "continuous integration",
    "continuous delivery",
    "agile development"
};

const std::unordered_set<std::string> KeywordExtractor::TECHNICAL_TERMS = {
    "c++", "java", "python", "javascript", "c#", "typescript", "php", "ruby",
    "go", "golang", "kotlin", "swift", "rust", "sql", "mysql", "postgresql",
    "mongodb", "sqlite", "redis", "oracle", "nosql", "html", "css", "react",
    "node.js", "nodejs", "express", "angular", "vue", "django", "flask", "jsp",
    "servlet", "aws", "azure", "firebase", "docker", "kubernetes", "linux",
    "unix", "bash", "git", "github", "gitlab", "cmake", "ci/cd", "jenkins",
    "oop", "oops", "dsa", "api", "apis", "algorithm", "algorithms", "developer",
    "backend", "frontend", "architecture", "security", "database", "testing",
    "software", "cloud", "microservice", "microservices", "framework", "agile",
    "junit", "stl", "bootstrap", "tailwind", "rest"
};

bool KeywordExtractor::isTechnicalKeyword(const std::string& term) {
    std::string norm = TextPreprocessor::normalizeText(term);
    if (TECHNICAL_TERMS.find(norm) != TECHNICAL_TERMS.end()) {
        return true;
    }
    for (const auto& phrase : MULTI_WORD_PHRASES) {
        if (phrase == norm) return true;
    }
    return false;
}

std::vector<std::string> KeywordExtractor::extractMultiWordPhrases(const std::string& text) {
    std::string normText = TextPreprocessor::normalizeText(text);
    std::vector<std::string> found;

    for (const auto& phrase : MULTI_WORD_PHRASES) {
        size_t pos = normText.find(phrase);
        while (pos != std::string::npos) {
            // Verify word boundary before and after
            bool startOk = (pos == 0 || !std::isalnum(static_cast<unsigned char>(normText[pos - 1])));
            size_t endPos = pos + phrase.size();
            bool endOk = (endPos >= normText.size() || !std::isalnum(static_cast<unsigned char>(normText[endPos])));

            if (startOk && endOk) {
                found.push_back(phrase);
                break;
            }
            pos = normText.find(phrase, pos + 1);
        }
    }
    return found;
}

std::unordered_map<std::string, int> KeywordExtractor::extractKeywordFrequencies(const std::string& text) {
    std::unordered_map<std::string, int> freqs;
    if (text.empty()) return freqs;

    std::string normText = TextPreprocessor::normalizeText(text);

    // 1. Scan for multi-word phrases
    for (const auto& phrase : MULTI_WORD_PHRASES) {
        size_t pos = normText.find(phrase);
        while (pos != std::string::npos) {
            bool startOk = (pos == 0 || !std::isalnum(static_cast<unsigned char>(normText[pos - 1])));
            size_t endPos = pos + phrase.size();
            bool endOk = (endPos >= normText.size() || !std::isalnum(static_cast<unsigned char>(normText[endPos])));

            if (startOk && endOk) {
                freqs[phrase]++;
            }
            pos = normText.find(phrase, pos + 1);
        }
    }

    // 2. Tokenize and count individual technical terms
    auto tokens = TextPreprocessor::tokenize(text);
    auto filtered = TextPreprocessor::removeStopWords(tokens);

    for (const auto& tok : filtered) {
        if (TECHNICAL_TERMS.find(tok) != TECHNICAL_TERMS.end()) {
            freqs[tok]++;
        } else if (TextPreprocessor::isProtectedTechToken(tok)) {
            freqs[tok]++;
        }
    }

    return freqs;
}

std::vector<std::string> KeywordExtractor::extractKeywords(const std::string& text) {
    auto freqs = extractKeywordFrequencies(text);
    std::vector<std::string> keywords;
    keywords.reserve(freqs.size());

    for (const auto& pair : freqs) {
        keywords.push_back(pair.first);
    }
    std::sort(keywords.begin(), keywords.end());
    return keywords;
}

std::vector<std::pair<std::string, int>> KeywordExtractor::getTopKeywords(const std::string& text, int topN) {
    auto freqs = extractKeywordFrequencies(text);
    std::vector<std::pair<std::string, int>> sortedList(freqs.begin(), freqs.end());

    std::sort(sortedList.begin(), sortedList.end(),
              [](const auto& a, const auto& b) {
                  if (a.second != b.second) return a.second > b.second;
                  return a.first < b.first;
              });

    if (topN > 0 && static_cast<int>(sortedList.size()) > topN) {
        sortedList.resize(topN);
    }
    return sortedList;
}
