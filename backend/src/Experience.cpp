#include "Experience.h"

Experience::Experience()
    : company(""), position(""), duration("") {}

Experience::Experience(const std::string& company, const std::string& position,
                       const std::string& duration)
    : company(company), position(position), duration(duration) {}

// Getters
const std::string& Experience::getCompany() const { return company; }
const std::string& Experience::getPosition() const { return position; }
const std::string& Experience::getDuration() const { return duration; }
const std::vector<std::string>& Experience::getResponsibilities() const { return responsibilities; }
const std::vector<std::string>& Experience::getTechnologies() const { return technologies; }

// Setters
void Experience::setCompany(const std::string& company) { this->company = company; }
void Experience::setPosition(const std::string& position) { this->position = position; }
void Experience::setDuration(const std::string& duration) { this->duration = duration; }

void Experience::addResponsibility(const std::string& responsibility) {
    responsibilities.push_back(responsibility);
}

void Experience::addTechnology(const std::string& tech) {
    technologies.push_back(tech);
}

void Experience::display() const {
    std::cout << "  " << position << " at " << company << "\n";
    std::cout << "    Duration: " << duration << "\n";
    if (!responsibilities.empty()) {
        std::cout << "    Responsibilities:\n";
        for (const auto& r : responsibilities) {
            std::cout << "      - " << r << "\n";
        }
    }
    if (!technologies.empty()) {
        std::cout << "    Technologies: ";
        for (size_t i = 0; i < technologies.size(); ++i) {
            std::cout << technologies[i];
            if (i < technologies.size() - 1) std::cout << ", ";
        }
        std::cout << "\n";
    }
}
