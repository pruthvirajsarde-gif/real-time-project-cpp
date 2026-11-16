#include "Profile.h"

Profile::Profile()
    : name(""), email(""), phone(""), location(""),
      linkedin(""), github(""), portfolio("") {}

Profile::Profile(const std::string& name, const std::string& email,
                 const std::string& phone, const std::string& location,
                 const std::string& linkedin, const std::string& github,
                 const std::string& portfolio)
    : name(name), email(email), phone(phone), location(location),
      linkedin(linkedin), github(github), portfolio(portfolio) {}

// Getters
const std::string& Profile::getName() const { return name; }
const std::string& Profile::getEmail() const { return email; }
const std::string& Profile::getPhone() const { return phone; }
const std::string& Profile::getLocation() const { return location; }
const std::string& Profile::getLinkedin() const { return linkedin; }
const std::string& Profile::getGithub() const { return github; }
const std::string& Profile::getPortfolio() const { return portfolio; }

// Setters
void Profile::setName(const std::string& name) { this->name = name; }
void Profile::setEmail(const std::string& email) { this->email = email; }
void Profile::setPhone(const std::string& phone) { this->phone = phone; }
void Profile::setLocation(const std::string& location) { this->location = location; }
void Profile::setLinkedin(const std::string& linkedin) { this->linkedin = linkedin; }
void Profile::setGithub(const std::string& github) { this->github = github; }
void Profile::setPortfolio(const std::string& portfolio) { this->portfolio = portfolio; }

void Profile::displayProfile() const {
    std::cout << "  Name      : " << name << "\n";
    std::cout << "  Email     : " << email << "\n";
    std::cout << "  Phone     : " << phone << "\n";
    std::cout << "  Location  : " << location << "\n";
    if (!linkedin.empty())
        std::cout << "  LinkedIn  : " << linkedin << "\n";
    if (!github.empty())
        std::cout << "  GitHub    : " << github << "\n";
    if (!portfolio.empty())
        std::cout << "  Portfolio : " << portfolio << "\n";
}
