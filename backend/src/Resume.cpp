#include "Resume.h"
#include <iomanip>

Resume::Resume() : Profile() {}

Resume::Resume(const std::string& name, const std::string& email,
               const std::string& phone, const std::string& location,
               const std::string& linkedin, const std::string& github,
               const std::string& portfolio)
    : Profile(name, email, phone, location, linkedin, github, portfolio) {}

// Add methods
void Resume::addSkill(const Skill& skill) { skills.push_back(skill); }
void Resume::addEducation(const Education& edu) { education.push_back(edu); }
void Resume::addProject(const Project& project) { projects.push_back(project); }
void Resume::addExperience(const Experience& exp) { experience.push_back(exp); }
void Resume::addCertification(const Certification& cert) { certifications.push_back(cert); }

// Getters
const std::vector<Skill>& Resume::getSkills() const { return skills; }
const std::vector<Education>& Resume::getEducation() const { return education; }
const std::vector<Project>& Resume::getProjects() const { return projects; }
const std::vector<Experience>& Resume::getExperience() const { return experience; }
const std::vector<Certification>& Resume::getCertifications() const { return certifications; }

void Resume::displayResume() const {
    std::cout << "\n========================================\n";
    std::cout << "          SMARTHIRE — RESUME            \n";
    std::cout << "========================================\n\n";

    // Personal information (inherited from Profile)
    std::cout << "[Personal Information]\n";
    displayProfile();

    // Skills
    if (!skills.empty()) {
        std::cout << "\n[Skills]\n";
        for (const auto& s : skills) {
            s.display();
        }
    }

    // Education
    if (!education.empty()) {
        std::cout << "\n[Education]\n";
        for (const auto& e : education) {
            e.display();
        }
    }

    // Projects
    if (!projects.empty()) {
        std::cout << "\n[Projects]\n";
        for (const auto& p : projects) {
            p.display();
            std::cout << "\n";
        }
    }

    // Experience
    if (!experience.empty()) {
        std::cout << "\n[Experience]\n";
        for (const auto& e : experience) {
            e.display();
            std::cout << "\n";
        }
    }

    // Certifications
    if (!certifications.empty()) {
        std::cout << "\n[Certifications]\n";
        for (const auto& c : certifications) {
            c.display();
        }
    }

    std::cout << "\n========================================\n";
}

double Resume::calculateCompletionPercentage() const {
    // Each major section contributes equally to completion.
    // Sections checked:
    //   1. Name (from Profile)
    //   2. Email
    //   3. Phone
    //   4. Location
    //   5. Skills
    //   6. Education
    //   7. Projects
    //   8. Experience
    //   9. Certifications
    //  10. LinkedIn / GitHub / Portfolio (at least one link)

    const int totalSections = 10;
    int filledSections = 0;

    if (!getName().empty())     ++filledSections;
    if (!getEmail().empty())    ++filledSections;
    if (!getPhone().empty())    ++filledSections;
    if (!getLocation().empty()) ++filledSections;
    if (!skills.empty())        ++filledSections;
    if (!education.empty())     ++filledSections;
    if (!projects.empty())      ++filledSections;
    if (!experience.empty())    ++filledSections;
    if (!certifications.empty())++filledSections;
    if (!getLinkedin().empty() || !getGithub().empty() || !getPortfolio().empty())
        ++filledSections;

    return (static_cast<double>(filledSections) / totalSections) * 100.0;
}
