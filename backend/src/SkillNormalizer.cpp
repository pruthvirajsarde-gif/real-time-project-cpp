#include "SkillNormalizer.h"
#include "TextPreprocessor.h"

#include <fstream>
#include <sstream>
#include <algorithm>

std::unordered_map<std::string, SkillKnowledge> SkillNormalizer::knowledgeBase;
std::unordered_map<std::string, std::string> SkillNormalizer::aliasToCanonical;
bool SkillNormalizer::initialized = false;

static std::string trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void SkillNormalizer::registerSkill(const SkillKnowledge& knowledge) {
    knowledgeBase[knowledge.canonicalName] = knowledge;

    // Register canonical name in lowercase
    std::string lowerCanon = TextPreprocessor::normalizeText(knowledge.canonicalName);
    aliasToCanonical[lowerCanon] = knowledge.canonicalName;

    // Register all aliases
    for (const auto& alias : knowledge.aliases) {
        std::string lowerAlias = TextPreprocessor::normalizeText(alias);
        if (!lowerAlias.empty()) {
            aliasToCanonical[lowerAlias] = knowledge.canonicalName;
        }
    }
}

void SkillNormalizer::ensureInitialized() {
    if (initialized) return;
    initialized = true;

    // 1. Initialize Built-In Taxonomy
    registerSkill({"C++", "Programming Languages", {"cpp", "c plus plus", "cplusplus", "c/c++"}, {"STL", "CMake", "C"}});
    registerSkill({"Java", "Programming Languages", {"java", "core java", "java 8", "java 11", "java 17"}, {"Spring Boot", "JSP", "JUnit"}});
    registerSkill({"Python", "Programming Languages", {"python", "python3", "py"}, {"Django", "Flask"}});
    registerSkill({"JavaScript", "Programming Languages", {"js", "javascript", "java script", "es6"}, {"TypeScript", "Node.js", "React", "HTML", "CSS", "Web Development"}});
    registerSkill({"C#", "Programming Languages", {"csharp", "c sharp", "c#"}, {".NET", "ASP.NET"}});
    registerSkill({"TypeScript", "Programming Languages", {"ts", "typescript"}, {"JavaScript", "React"}});
    registerSkill({"Go", "Programming Languages", {"golang", "go"}, {"Docker", "Kubernetes", "Microservices"}});
    registerSkill({"PHP", "Programming Languages", {"php", "php7", "php8"}, {"MySQL"}});
    registerSkill({"Ruby", "Programming Languages", {"ruby"}, {"Ruby on Rails"}});
    registerSkill({"Kotlin", "Programming Languages", {"kotlin"}, {"Java", "Android"}});
    registerSkill({"Swift", "Programming Languages", {"swift"}, {"iOS"}});

    registerSkill({"SQL", "Databases", {"sql", "structured query language", "relational database"}, {"MySQL", "PostgreSQL", "SQLite", "Oracle"}});
    registerSkill({"MySQL", "Databases", {"mysql", "my sql", "mysql database"}, {"SQL", "Relational Database"}});
    registerSkill({"PostgreSQL", "Databases", {"postgres", "postgresql", "postgres sql"}, {"SQL", "Relational Database"}});
    registerSkill({"MongoDB", "Databases", {"mongo", "mongodb", "mongo database"}, {"NoSQL", "Document Database"}});
    registerSkill({"SQLite", "Databases", {"sqlite", "sqlite3"}, {"SQL", "Embedded Database"}});
    registerSkill({"Redis", "Databases", {"redis", "redis cache"}, {"NoSQL", "Caching"}});
    registerSkill({"Oracle", "Databases", {"oracle", "oracle db", "pl/sql"}, {"SQL"}});

    registerSkill({"HTML", "Web", {"html", "html5"}, {"CSS", "JavaScript", "Web Development"}});
    registerSkill({"CSS", "Web", {"css", "css3"}, {"HTML", "Web Development"}});
    registerSkill({"React", "Web", {"react", "reactjs", "react.js"}, {"JavaScript", "TypeScript", "Frontend"}});
    registerSkill({"Node.js", "Web", {"node", "nodejs", "node.js", "node js"}, {"JavaScript", "Express", "REST API", "Backend"}});
    registerSkill({"Express", "Web", {"express", "expressjs", "express.js"}, {"Node.js", "REST API"}});
    registerSkill({"Spring Boot", "Web", {"spring boot", "springboot", "spring framework"}, {"Java", "REST API", "Microservices"}});
    registerSkill({"JSP", "Web", {"jsp", "java server pages"}, {"Java", "Web Development"}});
    registerSkill({"REST API", "Web", {"rest api", "rest apis", "restful api", "restful apis", "web api"}, {"HTTP", "Microservices", "Backend"}});
    registerSkill({"Web Development", "Web", {"web development", "web dev", "web design"}, {"HTML", "CSS", "JavaScript"}});

    registerSkill({"AWS", "Cloud", {"aws", "amazon web services"}, {"Cloud Computing"}});
    registerSkill({"Azure", "Cloud", {"azure", "microsoft azure"}, {"Cloud Computing"}});
    registerSkill({"Google Cloud", "Cloud", {"gcp", "google cloud", "google cloud platform"}, {"Cloud Computing"}});
    registerSkill({"Firebase", "Cloud", {"firebase", "google firebase"}, {"Cloud", "NoSQL"}});
    registerSkill({"Docker", "Cloud", {"docker", "containerization", "containers"}, {"Kubernetes", "DevOps", "Linux"}});
    registerSkill({"Kubernetes", "Cloud", {"k8s", "kubernetes"}, {"Docker", "DevOps"}});

    registerSkill({"Git", "Tools", {"git", "version control", "source control"}, {"GitHub", "GitLab"}});
    registerSkill({"GitHub", "Tools", {"github", "git hub"}, {"Git", "CI/CD"}});
    registerSkill({"GitLab", "Tools", {"gitlab", "git lab"}, {"Git", "CI/CD"}});
    registerSkill({"CMake", "Tools", {"cmake", "c-make"}, {"C++", "Build Tools"}});
    registerSkill({"Linux", "Tools", {"linux", "unix", "bash", "shell"}, {"Ubuntu", "Operating System"}});
    registerSkill({"JUnit", "Tools", {"junit", "junit5"}, {"Java", "Software Testing"}});
    registerSkill({"STL", "Tools", {"stl", "standard template library"}, {"C++", "Data Structures"}});

    registerSkill({"OOP", "Concepts", {"oop", "oops", "object oriented programming", "object-oriented programming"}, {"Software Design"}});
    registerSkill({"Data Structures", "Concepts", {"data structures", "dsa", "data structure"}, {"Algorithms"}});
    registerSkill({"Algorithms", "Concepts", {"algorithms", "algorithm", "algo"}, {"Data Structures"}});
    registerSkill({"Software Testing", "Concepts", {"software testing", "testing", "qa", "quality assurance"}, {"JUnit", "Unit Testing"}});
    registerSkill({"Microservices", "Concepts", {"microservices", "microservices architecture", "micro-services"}, {"REST API", "Docker"}});
    registerSkill({"CI/CD", "Concepts", {"ci/cd", "continuous integration", "continuous delivery"}, {"Git", "GitHub", "DevOps"}});

    // 2. Try loading custom skill dictionary file if present
    loadSkillDictionary("data/skill_dictionary.txt");
}

bool SkillNormalizer::loadSkillDictionary(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        line = trimWhitespace(line);
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string canon, cat, aliasesStr, relatedStr;

        if (std::getline(ss, canon, '|') &&
            std::getline(ss, cat, '|') &&
            std::getline(ss, aliasesStr, '|')) {

            std::getline(ss, relatedStr, '|'); // optional 4th column

            SkillKnowledge sk;
            sk.canonicalName = trimWhitespace(canon);
            sk.category = trimWhitespace(cat);

            std::stringstream aliasSs(aliasesStr);
            std::string aliasItem;
            while (std::getline(aliasSs, aliasItem, ',')) {
                std::string cleaned = trimWhitespace(aliasItem);
                if (!cleaned.empty()) {
                    sk.aliases.push_back(cleaned);
                }
            }

            std::stringstream relSs(relatedStr);
            std::string relItem;
            while (std::getline(relSs, relItem, ',')) {
                std::string cleaned = trimWhitespace(relItem);
                if (!cleaned.empty()) {
                    sk.relatedSkills.push_back(cleaned);
                }
            }

            registerSkill(sk);
        }
    }
    return true;
}

std::string SkillNormalizer::normalizeSkillName(const std::string& skillName) {
    ensureInitialized();
    std::string cleaned = trimWhitespace(skillName);
    if (cleaned.empty()) return "";

    std::string lower = TextPreprocessor::normalizeText(cleaned);

    auto it = aliasToCanonical.find(lower);
    if (it != aliasToCanonical.end()) {
        return it->second;
    }

    // Direct match against canonical name
    for (const auto& pair : knowledgeBase) {
        if (TextPreprocessor::normalizeText(pair.first) == lower) {
            return pair.first;
        }
    }

    return cleaned;
}

std::string SkillNormalizer::getSkillCategory(const std::string& skillName) {
    ensureInitialized();
    std::string norm = normalizeSkillName(skillName);
    auto it = knowledgeBase.find(norm);
    if (it != knowledgeBase.end()) {
        return it->second.category;
    }
    return "Other";
}

bool SkillNormalizer::areSkillsEquivalent(const std::string& skillA, const std::string& skillB) {
    ensureInitialized();
    std::string normA = normalizeSkillName(skillA);
    std::string normB = normalizeSkillName(skillB);
    if (normA.empty() || normB.empty()) return false;
    return (normA == normB);
}

bool SkillNormalizer::areSkillsRelated(const std::string& skillA, const std::string& skillB) {
    ensureInitialized();
    std::string normA = normalizeSkillName(skillA);
    std::string normB = normalizeSkillName(skillB);

    if (normA.empty() || normB.empty()) return false;
    if (normA == normB) return false; // Identical, not just related

    // Check knowledgeBase of normA
    auto itA = knowledgeBase.find(normA);
    if (itA != knowledgeBase.end()) {
        for (const auto& rel : itA->second.relatedSkills) {
            if (normalizeSkillName(rel) == normB) return true;
        }
    }

    // Check knowledgeBase of normB
    auto itB = knowledgeBase.find(normB);
    if (itB != knowledgeBase.end()) {
        for (const auto& rel : itB->second.relatedSkills) {
            if (normalizeSkillName(rel) == normA) return true;
        }
    }

    return false;
}

std::vector<std::string> SkillNormalizer::getRelatedSkills(const std::string& skillName) {
    ensureInitialized();
    std::string norm = normalizeSkillName(skillName);
    auto it = knowledgeBase.find(norm);
    if (it != knowledgeBase.end()) {
        return it->second.relatedSkills;
    }
    return {};
}
