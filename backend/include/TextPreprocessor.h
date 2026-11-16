#pragma once

#include <string>
#include <vector>
#include <unordered_set>

// ============================================================
// TextPreprocessor — Natural Language Preprocessing for Resumes
// and Job Descriptions. Handles case-folding, whitespace normalization,
// tokenization with tech-term preservation, stop-word filtering,
// and safe token normalization.
// ============================================================
class TextPreprocessor {
private:
    static const std::unordered_set<std::string> STOP_WORDS;

public:
    // Convert to lowercase and normalize whitespace
    static std::string normalizeText(const std::string& text);

    // Tokenize text into words while preserving technology names like C++, C#, .NET, Node.js
    static std::vector<std::string> tokenize(const std::string& text);

    // Remove common English stop-words from token stream without dropping tech terms
    static std::vector<std::string> removeStopWords(const std::vector<std::string>& tokens);

    // Normalize simple word variations (e.g. plurals: developers -> developer, databases -> database)
    static std::vector<std::string> normalizeTokens(const std::vector<std::string>& tokens);

    // Complete preprocessing pipeline: normalize -> tokenize -> removeStopWords -> normalizeTokens -> joined string
    static std::string cleanText(const std::string& text);

    // Check if a single token is recognized as an English stop-word
    static bool isStopWord(const std::string& token);

    // Check if token represents a protected technology term (e.g. c++, c#, .net, node.js, r)
    static bool isProtectedTechToken(const std::string& token);
};
