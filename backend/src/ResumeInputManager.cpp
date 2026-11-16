#include "ResumeInputManager.h"
#include "ResumeDocumentReader.h"
#include "ResumeParser.h"

#include <iostream>
#include <algorithm>

Resume ResumeInputManager::loadResume(const std::string& filePath) {
    if (!ResumeDocumentReader::fileExists(filePath)) {
        std::cerr << "  [ERROR] Resume file does not exist: " << filePath << "\n";
        return Resume();
    }

    if (!ResumeDocumentReader::isSupportedFile(filePath)) {
        std::cerr << "  [ERROR] Unsupported resume format: " << filePath << "\n";
        std::cerr << "  Supported formats: PDF, TXT, DOCX, DOC, RTF, MD\n";
        return Resume();
    }

    std::string ext = ResumeDocumentReader::getFileExtension(filePath);
    std::string formatName = (ext == ".pdf") ? "PDF" : ((ext == ".docx" || ext == ".doc") ? "DOCX" : "TXT");

    std::string text = ResumeDocumentReader::extractText(filePath);
    if (text.empty()) {
        std::cerr << "  [ERROR] Failed to extract text from document.\n";
        return Resume();
    }

    Resume resume = ResumeParser::parseResumeText(text);

    if (resume.getName().empty()) {
        resume.setName("Candidate");
    }

    return resume;
}

bool ResumeInputManager::validateResume(const Resume& resume) {
    return (!resume.getName().empty() &&
            (!resume.getSkills().empty() ||
             !resume.getEducation().empty() ||
             !resume.getExperience().empty() ||
             !resume.getProjects().empty() ||
             !resume.getEmail().empty()));
}

void ResumeInputManager::displayLoadingInformation(
        const std::string& filePath, const Resume& resume) {
    std::string ext = ResumeDocumentReader::getFileExtension(filePath);
    std::string formatName = (ext == ".pdf") ? "PDF" : ((ext == ".docx" || ext == ".doc") ? "DOCX" : "TXT");

    std::cout << "\n========================================\n";
    std::cout << "        RESUME LOADING RESULT           \n";
    std::cout << "========================================\n\n";

    std::cout << "File:\n" << filePath << "\n\n";
    std::cout << "Format:\n" << formatName << "\n\n";
    std::cout << "Extraction:\nSUCCESS\n\n";
    std::cout << "Parsing:\nSUCCESS\n\n";

    std::cout << "Candidate:\n"
              << (resume.getName().empty() ? "(None)" : resume.getName()) << "\n\n";

    std::cout << "Skills:\n" << resume.getSkills().size() << "\n\n";
    std::cout << "Education:\n" << resume.getEducation().size() << "\n\n";
    std::cout << "Projects:\n" << resume.getProjects().size() << "\n\n";
    std::cout << "Experience:\n" << resume.getExperience().size() << "\n\n";
    std::cout << "Certifications:\n" << resume.getCertifications().size() << "\n";
    std::cout << "========================================\n";
}
