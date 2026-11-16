#pragma once

#include "Profile.h"
#include "Skill.h"
#include "Education.h"
#include "Project.h"
#include "Experience.h"
#include "Certification.h"

#include <vector>
#include <string>
#include <iostream>

// Resume inherits from Profile to reuse personal information fields
class Resume : public Profile {
private:
    std::vector<Skill> skills;
    std::vector<Education> education;
    std::vector<Project> projects;
    std::vector<Experience> experience;
    std::vector<Certification> certifications;

public:
    // Default constructor
    Resume();

    // Parameterized constructor (forwards profile info to base class)
    Resume(const std::string& name, const std::string& email,
           const std::string& phone, const std::string& location,
           const std::string& linkedin, const std::string& github,
           const std::string& portfolio);

    // Add methods
    void addSkill(const Skill& skill);
    void addEducation(const Education& edu);
    void addProject(const Project& project);
    void addExperience(const Experience& exp);
    void addCertification(const Certification& cert);

    // Getters
    const std::vector<Skill>& getSkills() const;
    const std::vector<Education>& getEducation() const;
    const std::vector<Project>& getProjects() const;
    const std::vector<Experience>& getExperience() const;
    const std::vector<Certification>& getCertifications() const;

    // Display the complete resume
    void displayResume() const;

    // Calculate how complete the resume is (based on filled sections)
    double calculateCompletionPercentage() const;
};
