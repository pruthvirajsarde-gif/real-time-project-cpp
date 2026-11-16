#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct TermWeight {
    std::string term;
    double tf = 0.0;
    double idf = 0.0;
    double tfidf = 0.0;
};

// ============================================================
// TextSimilarity — Deterministic similarity engine providing
// Jaccard, Cosine, Keyword-Weighted, and TF-IDF relevance metrics.
// ============================================================
class TextSimilarity {
public:
    // Jaccard Similarity on token sets: |A ∩ B| / |A ∪ B| (0.0 to 1.0)
    static double jaccardSimilarity(
        const std::vector<std::string>& tokensA,
        const std::vector<std::string>& tokensB
    );

    // Term-frequency Cosine Similarity: A·B / (|A| |B|) (0.0 to 1.0)
    // Returns 0.0 if either vector has zero magnitude.
    static double cosineSimilarity(
        const std::unordered_map<std::string, int>& freqA,
        const std::unordered_map<std::string, int>& freqB
    );

    // Cosine similarity on raw text (handles preprocessing and frequency extraction)
    static double cosineSimilarityText(
        const std::string& textA,
        const std::string& textB
    );

    // Keyword-weighted similarity giving higher weight to technical terms & priority skills
    static double keywordWeightedSimilarity(
        const std::string& textA,
        const std::string& textB,
        const std::vector<std::string>& priorityKeywords = {}
    );

    // TF-IDF style relevance percentage between candidate document and job query (0.0 to 100.0)
    static double computeTfIdfRelevance(
        const std::string& docText,
        const std::string& queryText
    );

    // Calculate term weights for explanation
    static std::vector<TermWeight> computeTermWeights(
        const std::string& text,
        const std::vector<std::string>& referenceCorpus = {}
    );
};
