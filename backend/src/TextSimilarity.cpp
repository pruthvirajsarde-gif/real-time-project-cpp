#include "TextSimilarity.h"
#include "TextPreprocessor.h"
#include "KeywordExtractor.h"

#include <cmath>
#include <unordered_set>
#include <algorithm>

double TextSimilarity::jaccardSimilarity(const std::vector<std::string>& tokensA,
                                         const std::vector<std::string>& tokensB) {
    if (tokensA.empty() && tokensB.empty()) return 1.0;
    if (tokensA.empty() || tokensB.empty()) return 0.0;

    std::unordered_set<std::string> setA(tokensA.begin(), tokensA.end());
    std::unordered_set<std::string> setB(tokensB.begin(), tokensB.end());

    size_t intersectionCount = 0;
    for (const auto& item : setA) {
        if (setB.find(item) != setB.end()) {
            intersectionCount++;
        }
    }

    // |A ∪ B| = |A| + |B| - |A ∩ B|
    size_t unionCount = setA.size() + setB.size() - intersectionCount;
    if (unionCount == 0) return 0.0;

    return static_cast<double>(intersectionCount) / static_cast<double>(unionCount);
}

double TextSimilarity::cosineSimilarity(const std::unordered_map<std::string, int>& freqA,
                                        const std::unordered_map<std::string, int>& freqB) {
    if (freqA.empty() || freqB.empty()) return 0.0;

    double dotProduct = 0.0;
    double magA = 0.0;
    double magB = 0.0;

    for (const auto& pair : freqA) {
        double val = pair.second;
        magA += val * val;
        auto it = freqB.find(pair.first);
        if (it != freqB.end()) {
            dotProduct += val * it->second;
        }
    }

    for (const auto& pair : freqB) {
        double val = pair.second;
        magB += val * val;
    }

    if (magA <= 0.0 || magB <= 0.0) return 0.0;

    return dotProduct / (std::sqrt(magA) * std::sqrt(magB));
}

double TextSimilarity::cosineSimilarityText(const std::string& textA, const std::string& textB) {
    auto tokensA = TextPreprocessor::tokenize(textA);
    auto tokensB = TextPreprocessor::tokenize(textB);
    auto cleanA = TextPreprocessor::removeStopWords(tokensA);
    auto cleanB = TextPreprocessor::removeStopWords(tokensB);

    std::unordered_map<std::string, int> freqA;
    std::unordered_map<std::string, int> freqB;

    for (const auto& t : cleanA) freqA[t]++;
    for (const auto& t : cleanB) freqB[t]++;

    // Also blend in multi-word keyword frequencies
    auto kwA = KeywordExtractor::extractKeywordFrequencies(textA);
    auto kwB = KeywordExtractor::extractKeywordFrequencies(textB);
    for (const auto& pair : kwA) freqA[pair.first] += pair.second * 2;
    for (const auto& pair : kwB) freqB[pair.first] += pair.second * 2;

    return cosineSimilarity(freqA, freqB);
}

double TextSimilarity::keywordWeightedSimilarity(const std::string& textA,
                                                 const std::string& textB,
                                                 const std::vector<std::string>& priorityKeywords) {
    auto tokensA = TextPreprocessor::removeStopWords(TextPreprocessor::tokenize(textA));
    auto tokensB = TextPreprocessor::removeStopWords(TextPreprocessor::tokenize(textB));

    std::unordered_map<std::string, double> weightedA;
    std::unordered_map<std::string, double> weightedB;

    std::unordered_set<std::string> prioritySet;
    for (const auto& pk : priorityKeywords) {
        prioritySet.insert(TextPreprocessor::normalizeText(pk));
    }

    auto getWeight = [&](const std::string& tok) -> double {
        if (prioritySet.find(tok) != prioritySet.end()) return 3.5;
        if (KeywordExtractor::isTechnicalKeyword(tok)) return 2.5;
        return 1.0;
    };

    for (const auto& t : tokensA) weightedA[t] += getWeight(t);
    for (const auto& t : tokensB) weightedB[t] += getWeight(t);

    // Multi-word phrases
    auto phrasesA = KeywordExtractor::extractMultiWordPhrases(textA);
    auto phrasesB = KeywordExtractor::extractMultiWordPhrases(textB);
    for (const auto& p : phrasesA) weightedA[p] += 4.0;
    for (const auto& p : phrasesB) weightedB[p] += 4.0;

    double dotProduct = 0.0;
    double magA = 0.0;
    double magB = 0.0;

    for (const auto& pair : weightedA) {
        magA += pair.second * pair.second;
        auto it = weightedB.find(pair.first);
        if (it != weightedB.end()) {
            dotProduct += pair.second * it->second;
        }
    }

    for (const auto& pair : weightedB) {
        magB += pair.second * pair.second;
    }

    if (magA <= 0.0 || magB <= 0.0) return 0.0;

    double sim = dotProduct / (std::sqrt(magA) * std::sqrt(magB));
    return std::max(0.0, std::min(1.0, sim));
}

double TextSimilarity::computeTfIdfRelevance(const std::string& docText, const std::string& queryText) {
    if (docText.empty() || queryText.empty()) return 0.0;

    auto docTokens = TextPreprocessor::removeStopWords(TextPreprocessor::tokenize(docText));
    auto queryTokens = TextPreprocessor::removeStopWords(TextPreprocessor::tokenize(queryText));

    if (docTokens.empty() || queryTokens.empty()) return 0.0;

    // Term frequencies in document
    std::unordered_map<std::string, int> docCounts;
    for (const auto& t : docTokens) docCounts[t]++;

    // Term frequencies in query
    std::unordered_map<std::string, int> queryCounts;
    for (const auto& t : queryTokens) queryCounts[t]++;

    // Add multi-word terms
    auto docPhrases = KeywordExtractor::extractMultiWordPhrases(docText);
    for (const auto& p : docPhrases) docCounts[p] += 2;
    auto queryPhrases = KeywordExtractor::extractMultiWordPhrases(queryText);
    for (const auto& p : queryPhrases) queryCounts[p] += 2;

    double docTotalTerms = static_cast<double>(docTokens.size() + docPhrases.size() * 2);
    if (docTotalTerms <= 0.0) docTotalTerms = 1.0;

    // Generic words to downweight via pseudo-IDF
    static const std::unordered_set<std::string> GENERIC_WORDS = {
        "software", "development", "developer", "system", "role", "work",
        "working", "experience", "experienced", "project", "projects",
        "company", "team", "engineer", "engineering", "application", "applications"
    };

    double matchedWeightSum = 0.0;
    double queryWeightSum = 0.0;

    for (const auto& qPair : queryCounts) {
        const std::string& term = qPair.first;
        double queryFreq = qPair.second;

        // Calculate IDF weight
        double idf = 2.0; // default for domain terms
        if (GENERIC_WORDS.find(term) != GENERIC_WORDS.end()) {
            idf = 0.5; // low IDF for generic terms
        } else if (KeywordExtractor::isTechnicalKeyword(term)) {
            idf = 3.0; // high IDF for specific technical keywords
        }

        queryWeightSum += queryFreq * idf;

        auto it = docCounts.find(term);
        if (it != docCounts.end()) {
            double tf = static_cast<double>(it->second) / docTotalTerms;
            matchedWeightSum += (1.0 + std::log(1.0 + tf * 10.0)) * idf * queryFreq;
        }
    }

    if (queryWeightSum <= 0.0) return 0.0;

    double score = (matchedWeightSum / queryWeightSum) * 100.0;
    return std::max(0.0, std::min(100.0, score));
}

std::vector<TermWeight> TextSimilarity::computeTermWeights(const std::string& text,
                                                          const std::vector<std::string>& referenceCorpus) {
    std::vector<TermWeight> weights;
    auto tokens = TextPreprocessor::removeStopWords(TextPreprocessor::tokenize(text));
    if (tokens.empty()) return weights;

    std::unordered_map<std::string, int> termCounts;
    for (const auto& t : tokens) termCounts[t]++;

    double totalTerms = static_cast<double>(tokens.size());

    static const std::unordered_set<std::string> GENERIC = {
        "software", "development", "developer", "system", "work", "role"
    };

    for (const auto& pair : termCounts) {
        TermWeight tw;
        tw.term = pair.first;
        tw.tf = static_cast<double>(pair.second) / totalTerms;

        if (GENERIC.find(tw.term) != GENERIC.end()) {
            tw.idf = 0.5;
        } else if (KeywordExtractor::isTechnicalKeyword(tw.term)) {
            tw.idf = 3.0;
        } else {
            tw.idf = 1.5;
        }

        tw.tfidf = tw.tf * tw.idf;
        weights.push_back(tw);
    }

    std::sort(weights.begin(), weights.end(), [](const auto& a, const auto& b) {
        return a.tfidf > b.tfidf;
    });

    return weights;
}
