#pragma once

#include <string>
#include <iostream>

class Profile {
private:
    std::string name;
    std::string email;
    std::string phone;
    std::string location;
    std::string linkedin;
    std::string github;
    std::string portfolio;

public:
    // Default constructor
    Profile();

    // Parameterized constructor
    Profile(const std::string& name, const std::string& email,
            const std::string& phone, const std::string& location,
            const std::string& linkedin, const std::string& github,
            const std::string& portfolio);

    // Getters
    const std::string& getName() const;
    const std::string& getEmail() const;
    const std::string& getPhone() const;
    const std::string& getLocation() const;
    const std::string& getLinkedin() const;
    const std::string& getGithub() const;
    const std::string& getPortfolio() const;

    // Setters
    void setName(const std::string& name);
    void setEmail(const std::string& email);
    void setPhone(const std::string& phone);
    void setLocation(const std::string& location);
    void setLinkedin(const std::string& linkedin);
    void setGithub(const std::string& github);
    void setPortfolio(const std::string& portfolio);

    // Display profile information
    void displayProfile() const;
};
