#pragma once

#include <string>
#include <vector>
#include <iostream>

class Job {
private:
    std::string jobId;
    std::string jobTitle;
    std::string company;
    std::string location;
    std::string employmentType;

    std::vector<std::string> requiredSkills;
    std::vector<std::string> preferredSkills;

    std::string educationRequirement;
    int minimumExperience;

    std::string description;

public:
    // Default constructor
    Job();

    // Parameterized constructor
    Job(const std::string& jobId, const std::string& jobTitle,
        const std::string& company, const std::string& location,
        const std::string& employmentType, const std::string& educationRequirement,
        int minimumExperience, const std::string& description);

    // Getters
    const std::string& getJobId() const;
    const std::string& getJobTitle() const;
    const std::string& getCompany() const;
    const std::string& getLocation() const;
    const std::string& getEmploymentType() const;
    const std::vector<std::string>& getRequiredSkills() const;
    const std::vector<std::string>& getPreferredSkills() const;
    const std::string& getEducationRequirement() const;
    int getMinimumExperience() const;
    const std::string& getDescription() const;

    // Setters
    void setJobId(const std::string& jobId);
    void setJobTitle(const std::string& jobTitle);
    void setCompany(const std::string& company);
    void setLocation(const std::string& location);
    void setEmploymentType(const std::string& employmentType);
    void setEducationRequirement(const std::string& educationRequirement);
    void setMinimumExperience(int minimumExperience);
    void setDescription(const std::string& description);

    // Add skills
    void addRequiredSkill(const std::string& skill);
    void addPreferredSkill(const std::string& skill);

    // Display job listing
    void displayJob() const;
};
