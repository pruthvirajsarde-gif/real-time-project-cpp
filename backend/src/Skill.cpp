#include "Skill.h"

Skill::Skill() : name(""), category("Other") {}

Skill::Skill(const std::string& name, const std::string& category)
    : name(name), category(category) {}

// Getters
const std::string& Skill::getName() const { return name; }
const std::string& Skill::getCategory() const { return category; }

// Setters
void Skill::setName(const std::string& name) { this->name = name; }
void Skill::setCategory(const std::string& category) { this->category = category; }

void Skill::display() const {
    std::cout << "  " << name << " [" << category << "]\n";
}
