#pragma once

#include <string>
#include <iostream>

class Certification {
private:
    std::string certificationName;
    std::string issuingOrganization;
    int year;

public:
    // Default constructor
    Certification();

    // Parameterized constructor
    Certification(const std::string& certificationName,
                  const std::string& issuingOrganization, int year);

    // Getters
    const std::string& getCertificationName() const;
    const std::string& getIssuingOrganization() const;
    int getYear() const;

    // Setters
    void setCertificationName(const std::string& certificationName);
    void setIssuingOrganization(const std::string& issuingOrganization);
    void setYear(int year);

    // Display certification information
    void display() const;
};
