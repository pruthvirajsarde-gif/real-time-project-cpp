#pragma once

#include <string>
#include <vector>
#include <iostream>

class Project {
private:
    std::string projectName;
    std::string description;
    std::vector<std::string> technologies;
    std::string role;
    std::string duration;

public:
    // Default constructor
    Project();

    // Parameterized constructor
    Project(const std::string& projectName, const std::string& description,
            const std::string& role, const std::string& duration);

    // Getters
    const std::string& getProjectName() const;
    const std::string& getDescription() const;
    const std::vector<std::string>& getTechnologies() const;
    const std::string& getRole() const;
    const std::string& getDuration() const;

    // Setters
    void setProjectName(const std::string& projectName);
    void setDescription(const std::string& description);
    void setRole(const std::string& role);
    void setDuration(const std::string& duration);

    // Add a technology to the project
    void addTechnology(const std::string& tech);

    // Display project information
    void display() const;
};
