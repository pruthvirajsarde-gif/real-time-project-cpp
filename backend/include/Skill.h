#pragma once

#include <string>
#include <iostream>

class Skill {
private:
    std::string name;
    std::string category;  // Programming Language, Framework, Database, Tool, Soft Skill, Other

public:
    // Default constructor
    Skill();

    // Parameterized constructor
    Skill(const std::string& name, const std::string& category);

    // Getters
    const std::string& getName() const;
    const std::string& getCategory() const;

    // Setters
    void setName(const std::string& name);
    void setCategory(const std::string& category);

    // Display skill information
    void display() const;
};
