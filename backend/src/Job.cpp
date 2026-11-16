#include "Job.h"

Job::Job()
    : jobId(""), jobTitle(""), company(""), location(""),
      employmentType(""), educationRequirement(""),
      minimumExperience(0), description("") {}

Job::Job(const std::string& jobId, const std::string& jobTitle,
         const std::string& company, const std::string& location,
         const std::string& employmentType, const std::string& educationRequirement,
         int minimumExperience, const std::string& description)
    : jobId(jobId), jobTitle(jobTitle), company(company), location(location),
      employmentType(employmentType), educationRequirement(educationRequirement),
      minimumExperience(minimumExperience), description(description) {}

// Getters
const std::string& Job::getJobId() const { return jobId; }
const std::string& Job::getJobTitle() const { return jobTitle; }
const std::string& Job::getCompany() const { return company; }
const std::string& Job::getLocation() const { return location; }
const std::string& Job::getEmploymentType() const { return employmentType; }
const std::vector<std::string>& Job::getRequiredSkills() const { return requiredSkills; }
const std::vector<std::string>& Job::getPreferredSkills() const { return preferredSkills; }
const std::string& Job::getEducationRequirement() const { return educationRequirement; }
int Job::getMinimumExperience() const { return minimumExperience; }
const std::string& Job::getDescription() const { return description; }

// Setters
void Job::setJobId(const std::string& jobId) { this->jobId = jobId; }
void Job::setJobTitle(const std::string& jobTitle) { this->jobTitle = jobTitle; }
void Job::setCompany(const std::string& company) { this->company = company; }
void Job::setLocation(const std::string& location) { this->location = location; }
void Job::setEmploymentType(const std::string& employmentType) { this->employmentType = employmentType; }
void Job::setEducationRequirement(const std::string& educationRequirement) {
    this->educationRequirement = educationRequirement;
}
void Job::setMinimumExperience(int minimumExperience) { this->minimumExperience = minimumExperience; }
void Job::setDescription(const std::string& description) { this->description = description; }

void Job::addRequiredSkill(const std::string& skill) {
    requiredSkills.push_back(skill);
}

void Job::addPreferredSkill(const std::string& skill) {
    preferredSkills.push_back(skill);
}

void Job::displayJob() const {
    std::cout << "\n========================================\n";
    std::cout << "        SMARTHIRE — JOB LISTING         \n";
    std::cout << "========================================\n\n";

    std::cout << "  Job ID          : " << jobId << "\n";
    std::cout << "  Title           : " << jobTitle << "\n";
    std::cout << "  Company         : " << company << "\n";
    std::cout << "  Location        : " << location << "\n";
    std::cout << "  Employment Type : " << employmentType << "\n";
    std::cout << "  Min. Experience : " << minimumExperience << " year(s)\n";
    std::cout << "  Education Req.  : " << educationRequirement << "\n";
    std::cout << "  Description     : " << description << "\n";

    if (!requiredSkills.empty()) {
        std::cout << "\n  [Required Skills]\n";
        for (const auto& s : requiredSkills) {
            std::cout << "    - " << s << "\n";
        }
    }

    if (!preferredSkills.empty()) {
        std::cout << "\n  [Preferred Skills]\n";
        for (const auto& s : preferredSkills) {
            std::cout << "    - " << s << "\n";
        }
    }

    std::cout << "\n========================================\n";
}
