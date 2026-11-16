#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// ============================================================
// KeywordExtractor — Extracts technical concepts, single-word
// and multi-word keywords, and calculates term frequency maps.
// ============================================================
class KeywordExtractor {
private:
    static const std::unordered_set<std::string> TECHNICAL_TERMS;
    static const std::vector<std::string> MULTI_WORD_PHRASES;

public:
    // Extract keyword frequencies (single-word, multi-word, technologies)
    static std::unordered_map<std::string, int> extractKeywordFrequencies(const std::string& text);

    // Extract unique list of technical keywords present in text
    static std::vector<std::string> extractKeywords(const std::string& text);

    // Retrieve top N keywords sorted by frequency descending
    static std::vector<std::pair<std::string, int>> getTopKeywords(const std::string& text, int topN = 10);

    // Check if a specific phrase or token is a recognized technical keyword
    static bool isTechnicalKeyword(const std::string& term);

    // Identify multi-word phrases present in text (e.g. "data structures", "machine learning", "rest api")
    static std::vector<std::string> extractMultiWordPhrases(const std::string& text);
};
