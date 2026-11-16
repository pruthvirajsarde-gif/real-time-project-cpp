#include "Project.h"

Project::Project()
    : projectName(""), description(""), role(""), duration("") {}

Project::Project(const std::string& projectName, const std::string& description,
                 const std::string& role, const std::string& duration)
    : projectName(projectName), description(description),
      role(role), duration(duration) {}

// Getters
const std::string& Project::getProjectName() const { return projectName; }
const std::string& Project::getDescription() const { return description; }
const std::vector<std::string>& Project::getTechnologies() const { return technologies; }
const std::string& Project::getRole() const { return role; }
const std::string& Project::getDuration() const { return duration; }

// Setters
void Project::setProjectName(const std::string& projectName) { this->projectName = projectName; }
void Project::setDescription(const std::string& description) { this->description = description; }
void Project::setRole(const std::string& role) { this->role = role; }
void Project::setDuration(const std::string& duration) { this->duration = duration; }

void Project::addTechnology(const std::string& tech) {
    technologies.push_back(tech);
}

void Project::display() const {
    std::cout << "  Project   : " << projectName << "\n";
    std::cout << "    Role    : " << role << "\n";
    std::cout << "    Duration: " << duration << "\n";
    std::cout << "    Description: " << description << "\n";
    if (!technologies.empty()) {
        std::cout << "    Technologies: ";
        for (size_t i = 0; i < technologies.size(); ++i) {
            std::cout << technologies[i];
            if (i < technologies.size() - 1) std::cout << ", ";
        }
        std::cout << "\n";
    }
}
