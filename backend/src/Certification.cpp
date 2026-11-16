#include "Certification.h"

Certification::Certification()
    : certificationName(""), issuingOrganization(""), year(0) {}

Certification::Certification(const std::string& certificationName,
                             const std::string& issuingOrganization, int year)
    : certificationName(certificationName),
      issuingOrganization(issuingOrganization), year(year) {}

// Getters
const std::string& Certification::getCertificationName() const { return certificationName; }
const std::string& Certification::getIssuingOrganization() const { return issuingOrganization; }
int Certification::getYear() const { return year; }

// Setters
void Certification::setCertificationName(const std::string& certificationName) {
    this->certificationName = certificationName;
}
void Certification::setIssuingOrganization(const std::string& issuingOrganization) {
    this->issuingOrganization = issuingOrganization;
}
void Certification::setYear(int year) { this->year = year; }

void Certification::display() const {
    std::cout << "  " << certificationName
              << " — " << issuingOrganization
              << " (" << year << ")\n";
}
