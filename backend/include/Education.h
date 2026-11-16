#pragma once

#include <string>
#include <iostream>

class Education {
private:
    std::string degree;
    std::string institution;
    std::string fieldOfStudy;
    int startYear;
    int endYear;
    double percentageOrCGPA;

public:
    // Default constructor
    Education();

    // Parameterized constructor
    Education(const std::string& degree, const std::string& institution,
              const std::string& fieldOfStudy, int startYear, int endYear,
              double percentageOrCGPA);

    // Getters
    const std::string& getDegree() const;
    const std::string& getInstitution() const;
    const std::string& getFieldOfStudy() const;
    int getStartYear() const;
    int getEndYear() const;
    double getPercentageOrCGPA() const;

    // Setters
    void setDegree(const std::string& degree);
    void setInstitution(const std::string& institution);
    void setFieldOfStudy(const std::string& fieldOfStudy);
    bool setStartYear(int startYear);
    bool setEndYear(int endYear);
    void setPercentageOrCGPA(double percentageOrCGPA);

    // Display education information
    void display() const;
};
