#include "Education.h"

Education::Education()
    : degree(""), institution(""), fieldOfStudy(""),
      startYear(0), endYear(0), percentageOrCGPA(0.0) {}

Education::Education(const std::string& degree, const std::string& institution,
                     const std::string& fieldOfStudy, int startYear, int endYear,
                     double percentageOrCGPA)
    : degree(degree), institution(institution), fieldOfStudy(fieldOfStudy),
      startYear(0), endYear(0), percentageOrCGPA(percentageOrCGPA) {
    // Validate years through setters
    setStartYear(startYear);
    setEndYear(endYear);
}

// Getters
const std::string& Education::getDegree() const { return degree; }
const std::string& Education::getInstitution() const { return institution; }
const std::string& Education::getFieldOfStudy() const { return fieldOfStudy; }
int Education::getStartYear() const { return startYear; }
int Education::getEndYear() const { return endYear; }
double Education::getPercentageOrCGPA() const { return percentageOrCGPA; }

// Setters
void Education::setDegree(const std::string& degree) { this->degree = degree; }
void Education::setInstitution(const std::string& institution) { this->institution = institution; }
void Education::setFieldOfStudy(const std::string& fieldOfStudy) { this->fieldOfStudy = fieldOfStudy; }

bool Education::setStartYear(int startYear) {
    if (startYear < 1900 || startYear > 2100) {
        std::cerr << "  [Warning] Invalid start year: " << startYear << "\n";
        return false;
    }
    this->startYear = startYear;
    return true;
}

bool Education::setEndYear(int endYear) {
    if (endYear < 1900 || endYear > 2100) {
        std::cerr << "  [Warning] Invalid end year: " << endYear << "\n";
        return false;
    }
    if (startYear > 0 && endYear < startYear) {
        std::cerr << "  [Warning] End year (" << endYear
                  << ") is before start year (" << startYear << ")\n";
        return false;
    }
    this->endYear = endYear;
    return true;
}

void Education::setPercentageOrCGPA(double percentageOrCGPA) {
    this->percentageOrCGPA = percentageOrCGPA;
}

void Education::display() const {
    std::cout << "  " << degree << " in " << fieldOfStudy << "\n";
    std::cout << "    Institution : " << institution << "\n";
    std::cout << "    Duration    : " << startYear << " - " << endYear << "\n";
    if (percentageOrCGPA > 0.0) {
        std::cout << "    CGPA / %    : " << percentageOrCGPA << "\n";
    }
}
