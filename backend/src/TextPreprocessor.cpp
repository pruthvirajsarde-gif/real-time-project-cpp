#include "TextPreprocessor.h"

#include <algorithm>
#include <cctype>
#include <sstream>

const std::unordered_set<std::string> TextPreprocessor::STOP_WORDS = {
    "a", "an", "the", "and", "or", "of", "to", "in", "on", "for", "with", "from",
    "is", "are", "was", "were", "be", "been", "this", "that", "these", "those",
    "as", "at", "by", "into", "about", "using", "have", "has", "had", "it", "its",
    "we", "you", "they", "their", "our", "my", "your", "me", "us", "him", "her",
    "them", "which", "who", "whom", "what", "when", "where", "why", "how", "all",
    "any", "both", "each", "few", "more", "most", "other", "some", "such", "no",
    "nor", "not", "only", "own", "same", "so", "than", "too", "very", "can",
    "will", "just", "should", "now", "also", "do", "does", "did", "doing", "via"
};

bool TextPreprocessor::isProtectedTechToken(const std::string& token) {
    static const std::unordered_set<std::string> PROTECTED = {
        "c++", "c#", ".net", "node.js", "react.js", "vue.js", "next.js",
        "angular.js", "asp.net", "ci/cd", "pl/sql", "tcp/ip", "c/c++",
        "css", "html", "sql", "git", "aws", "r", "go", "c", "js", "ts",
        "dsa", "oop", "oops", "redis", "express", "pandas", "jenkins"
    };
    return (PROTECTED.find(token) != PROTECTED.end());
}

std::string TextPreprocessor::normalizeText(const std::string& text) {
    if (text.empty()) return "";
    std::string result;
    result.reserve(text.size());

    bool lastWasSpace = true;
    for (char c : text) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!lastWasSpace) {
                result.push_back(' ');
                lastWasSpace = true;
            }
        } else {
            result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            lastWasSpace = false;
        }
    }

    if (!result.empty() && result.back() == ' ') {
        result.pop_back();
    }
    return result;
}

std::vector<std::string> TextPreprocessor::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    if (text.empty()) return tokens;

    size_t i = 0;
    const size_t n = text.size();

    while (i < n) {
        // Skip whitespace and standard punctuation delimiters
        char c = text[i];
        if (std::isspace(static_cast<unsigned char>(c)) || 
            c == ',' || c == ';' || c == '(' || c == ')' || 
            c == '[' || c == ']' || c == '{' || c == '}' || 
            c == '"' || c == '\'' || c == '`' || c == '!' || 
            c == '?' || c == ':' || c == '|' || c == '\\' || c == '=') {
            ++i;
            continue;
        }

        // Check for .net specifically
        if (c == '.' && i + 3 < n && 
            (text[i+1] == 'n' || text[i+1] == 'N') &&
            (text[i+2] == 'e' || text[i+2] == 'E') &&
            (text[i+3] == 't' || text[i+3] == 'T') &&
            (i + 4 == n || !std::isalnum(static_cast<unsigned char>(text[i+4])))) {
            tokens.push_back(".net");
            i += 4;
            continue;
        }

        // Check for c++ or c#
        if ((c == 'c' || c == 'C') && i + 1 < n) {
            if (text[i+1] == '+' && i + 2 < n && text[i+2] == '+') {
                tokens.push_back("c++");
                i += 3;
                continue;
            } else if (text[i+1] == '#') {
                tokens.push_back("c#");
                i += 2;
                continue;
            }
        }

        // Parse word token
        size_t start = i;
        while (i < n) {
            char curr = text[i];

            // Handle special dot inside word (e.g. node.js)
            if (curr == '.' && i + 2 < n && 
                (text[i+1] == 'j' || text[i+1] == 'J') &&
                (text[i+2] == 's' || text[i+2] == 'S') &&
                (i + 3 == n || !std::isalnum(static_cast<unsigned char>(text[i+3])))) {
                i += 3; // include .js
                break;
            }

            // Handle slash inside tech token (e.g. ci/cd, pl/sql)
            if (curr == '/' && i + 1 < n && std::isalnum(static_cast<unsigned char>(text[i+1]))) {
                ++i;
                continue;
            }

            // Check if alphanumeric or tech character
            if (std::isalnum(static_cast<unsigned char>(curr)) || curr == '+' || curr == '#' || curr == '-') {
                // If it's a hyphen, ensure it joins two words (e.g. front-end)
                if (curr == '-' && (i + 1 == n || !std::isalnum(static_cast<unsigned char>(text[i+1])))) {
                    break;
                }
                ++i;
            } else {
                break;
            }
        }

        if (i > start) {
            std::string tok = text.substr(start, i - start);
            tok = normalizeText(tok);

            // Trim leading/trailing hyphens or dots if not part of protected term
            while (tok.size() > 1 && (tok.front() == '-' || tok.front() == '.')) {
                if (tok == ".net") break;
                tok.erase(tok.begin());
            }
            while (tok.size() > 1 && (tok.back() == '-' || tok.back() == '.')) {
                tok.pop_back();
            }

            if (!tok.empty()) {
                tokens.push_back(tok);
            }
        } else {
            ++i;
        }
    }

    return tokens;
}

bool TextPreprocessor::isStopWord(const std::string& token) {
    if (isProtectedTechToken(token)) {
        return false;
    }
    return (STOP_WORDS.find(token) != STOP_WORDS.end());
}

std::vector<std::string> TextPreprocessor::removeStopWords(const std::vector<std::string>& tokens) {
    std::vector<std::string> filtered;
    filtered.reserve(tokens.size());

    for (const auto& tok : tokens) {
        if (!isStopWord(tok)) {
            filtered.push_back(tok);
        }
    }
    return filtered;
}

std::vector<std::string> TextPreprocessor::normalizeTokens(const std::vector<std::string>& tokens) {
    std::vector<std::string> normalized;
    normalized.reserve(tokens.size());

    for (const auto& tok : tokens) {
        if (isProtectedTechToken(tok)) {
            normalized.push_back(tok);
            continue;
        }

        std::string s = tok;
        const size_t len = s.length();

        // Safe stemmer/normalizer for technical domain
        if (len > 4 && s.substr(len - 3) == "ies") {
            // technologies -> technology
            s = s.substr(0, len - 3) + "y";
        } else if (len > 4 && s.substr(len - 3) == "ing") {
            // working -> work, developing -> develop, testing -> test
            if (len > 6 && s.substr(len - 6) == "lopment") {
                // development -> develop
                s = s.substr(0, len - 4);
            } else {
                s = s.substr(0, len - 3);
            }
        } else if (len > 3 && s.back() == 's' && s[len - 2] != 's') {
            // developers -> developer, databases -> database, systems -> system
            s.pop_back();
        }

        normalized.push_back(s);
    }

    return normalized;
}

std::string TextPreprocessor::cleanText(const std::string& text) {
    auto tokens = tokenize(text);
    auto noStops = removeStopWords(tokens);
    auto normalized = normalizeTokens(noStops);

    std::string result;
    for (size_t i = 0; i < normalized.size(); ++i) {
        result += normalized[i];
        if (i + 1 < normalized.size()) {
            result += " ";
        }
    }
    return result;
}
