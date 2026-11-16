#pragma once

#include <string>
#include <vector>
#include <iostream>

class Experience {
private:
    std::string company;
    std::string position;
    std::string duration;
    std::vector<std::string> responsibilities;
    std::vector<std::string> technologies;

public:
    // Default constructor
    Experience();

    // Parameterized constructor
    Experience(const std::string& company, const std::string& position,
               const std::string& duration);

    // Getters
    const std::string& getCompany() const;
    const std::string& getPosition() const;
    const std::string& getDuration() const;
    const std::vector<std::string>& getResponsibilities() const;
    const std::vector<std::string>& getTechnologies() const;

    // Setters
    void setCompany(const std::string& company);
    void setPosition(const std::string& position);
    void setDuration(const std::string& duration);

    // Add a responsibility
    void addResponsibility(const std::string& responsibility);

    // Add a technology
    void addTechnology(const std::string& tech);

    // Display experience information
    void display() const;
};
