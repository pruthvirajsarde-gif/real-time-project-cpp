#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct SkillKnowledge {
    std::string canonicalName;
    std::string category;
    std::vector<std::string> aliases;
    std::vector<std::string> relatedSkills;
};

// ============================================================
// SkillNormalizer — Standardizes technology aliases, manages
// skill taxonomy, and evaluates technology equivalence and
// relationships (e.g. SQL <-> MySQL, Git <-> GitHub).
// ============================================================
class SkillNormalizer {
private:
    static std::unordered_map<std::string, SkillKnowledge> knowledgeBase;
    static std::unordered_map<std::string, std::string> aliasToCanonical;
    static bool initialized;

    static void ensureInitialized();

public:
    // Load taxonomy from file (or initialize built-in knowledge)
    static bool loadSkillDictionary(const std::string& filePath = "data/skill_dictionary.txt");

    // Convert any raw skill alias into its canonical form (e.g. "c plus plus" -> "C++")
    static std::string normalizeSkillName(const std::string& skillName);

    // Retrieve skill category (e.g. "Programming Languages", "Databases", "Web", "Cloud")
    static std::string getSkillCategory(const std::string& skillName);

    // Check if two skill strings resolve to the exact same canonical technology
    static bool areSkillsEquivalent(const std::string& skillA, const std::string& skillB);

    // Check if two skills are distinct but related (e.g. SQL and MySQL, Git and GitHub)
    static bool areSkillsRelated(const std::string& skillA, const std::string& skillB);

    // Get list of related technologies for a given skill
    static std::vector<std::string> getRelatedSkills(const std::string& skillName);

    // Add or update knowledge entry
    static void registerSkill(const SkillKnowledge& knowledge);
};
