#include "ResumeRepository.h"
#include "DatabaseManager.h"
#include "CandidateRepository.h"
#include "JobRepository.h"

#include <iostream>
#include <sstream>

static std::string joinStrings(const std::vector<std::string>& vec, char delimiter = ';') {
    std::string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        result += vec[i];
        if (i + 1 < vec.size()) {
            result += delimiter;
        }
    }
    return result;
}

static std::vector<std::string> splitStrings(const std::string& str, char delimiter = ';') {
    std::vector<std::string> result;
    if (str.empty()) return result;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    return result;
}

bool ResumeRepository::saveResume(int candidateId, const Resume& resume,
                                  const std::string& sourcePath,
                                  const std::string& fileFormat,
                                  double qualityScore) {
    if (candidateId <= 0) return false;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return false;

    if (!DatabaseManager::beginTransaction()) {
        return false;
    }

    // 1. Update Candidate profile table
    Profile prof(resume.getName(), resume.getEmail(), resume.getPhone(),
                 resume.getLocation(), resume.getLinkedin(),
                 resume.getGithub(), resume.getPortfolio());
    CandidateRepository::updateCandidate(candidateId, prof);

    // 2. Insert or update resumes metadata
    sqlite3_stmt* stmt = nullptr;
    bool exists = resumeExists(candidateId);
    if (exists) {
        const char* sql = "UPDATE resumes SET source_path=?, file_format=?, resume_quality_score=?, "
                          "updated_at=CURRENT_TIMESTAMP WHERE candidate_id=?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            DatabaseManager::rollbackTransaction();
            return false;
        }
        sqlite3_bind_text(stmt, 1, sourcePath.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, fileFormat.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 3, qualityScore);
        sqlite3_bind_int(stmt, 4, candidateId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        const char* sql = "INSERT INTO resumes (candidate_id, source_path, file_format, resume_quality_score) "
                          "VALUES (?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            DatabaseManager::rollbackTransaction();
            return false;
        }
        sqlite3_bind_int(stmt, 1, candidateId);
        sqlite3_bind_text(stmt, 2, sourcePath.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, fileFormat.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, qualityScore);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    // 3. Clear and save candidate_skills
    const char* delSkills = "DELETE FROM candidate_skills WHERE candidate_id = ?;";
    if (sqlite3_prepare_v2(db, delSkills, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, candidateId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    const char* insSkill = "INSERT OR IGNORE INTO candidate_skills (candidate_id, skill_id) VALUES (?, ?);";
    for (const auto& skill : resume.getSkills()) {
        int sid = JobRepository::getOrCreateSkillId(skill.getName(), skill.getCategory());
        if (sid > 0) {
            if (sqlite3_prepare_v2(db, insSkill, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, candidateId);
                sqlite3_bind_int(stmt, 2, sid);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
    }

    // 4. Clear and save candidate_education
    const char* delEdu = "DELETE FROM candidate_education WHERE candidate_id = ?;";
    if (sqlite3_prepare_v2(db, delEdu, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, candidateId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    const char* insEdu = "INSERT INTO candidate_education (candidate_id, degree, institution, field_of_study, "
                         "start_year, end_year, score) VALUES (?, ?, ?, ?, ?, ?, ?);";
    for (const auto& edu : resume.getEducation()) {
        if (sqlite3_prepare_v2(db, insEdu, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, candidateId);
            sqlite3_bind_text(stmt, 2, edu.getDegree().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, edu.getInstitution().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, edu.getFieldOfStudy().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 5, edu.getStartYear());
            sqlite3_bind_int(stmt, 6, edu.getEndYear());
            sqlite3_bind_double(stmt, 7, edu.getPercentageOrCGPA());
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    // 5. Clear and save candidate_experience
    const char* delExp = "DELETE FROM candidate_experience WHERE candidate_id = ?;";
    if (sqlite3_prepare_v2(db, delExp, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, candidateId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    const char* insExp = "INSERT INTO candidate_experience (candidate_id, company, position, duration, "
                         "responsibilities, technologies) VALUES (?, ?, ?, ?, ?, ?);";
    for (const auto& exp : resume.getExperience()) {
        if (sqlite3_prepare_v2(db, insExp, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, candidateId);
            sqlite3_bind_text(stmt, 2, exp.getCompany().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, exp.getPosition().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, exp.getDuration().c_str(), -1, SQLITE_TRANSIENT);
            std::string respStr = joinStrings(exp.getResponsibilities());
            std::string techStr = joinStrings(exp.getTechnologies());
            sqlite3_bind_text(stmt, 5, respStr.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 6, techStr.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    // 6. Clear and save candidate_projects
    const char* delProj = "DELETE FROM candidate_projects WHERE candidate_id = ?;";
    if (sqlite3_prepare_v2(db, delProj, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, candidateId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    const char* insProj = "INSERT INTO candidate_projects (candidate_id, name, description, role, duration, technologies) "
                          "VALUES (?, ?, ?, ?, ?, ?);";
    for (const auto& proj : resume.getProjects()) {
        if (sqlite3_prepare_v2(db, insProj, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, candidateId);
            sqlite3_bind_text(stmt, 2, proj.getProjectName().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, proj.getDescription().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, proj.getRole().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, proj.getDuration().c_str(), -1, SQLITE_TRANSIENT);
            std::string techStr = joinStrings(proj.getTechnologies());
            sqlite3_bind_text(stmt, 6, techStr.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    // 7. Clear and save candidate_certifications
    const char* delCert = "DELETE FROM candidate_certifications WHERE candidate_id = ?;";
    if (sqlite3_prepare_v2(db, delCert, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, candidateId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    const char* insCert = "INSERT INTO candidate_certifications (candidate_id, name, issuer, year) "
                          "VALUES (?, ?, ?, ?);";
    for (const auto& cert : resume.getCertifications()) {
        if (sqlite3_prepare_v2(db, insCert, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, candidateId);
            sqlite3_bind_text(stmt, 2, cert.getCertificationName().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, cert.getIssuingOrganization().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 4, cert.getYear());
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    return DatabaseManager::commitTransaction();
}

Resume ResumeRepository::loadResume(int candidateId) {
    Resume r;
    if (candidateId <= 0) return r;
    sqlite3* db = DatabaseManager::getDb();
    if (!db) return r;

    // Load Profile
    Profile p = CandidateRepository::getCandidate(candidateId);
    r.setName(p.getName());
    r.setEmail(p.getEmail());
    r.setPhone(p.getPhone());
    r.setLocation(p.getLocation());
    r.setLinkedin(p.getLinkedin());
    r.setGithub(p.getGithub());
    r.setPortfolio(p.getPortfolio());

    // Load Skills
    const char* skillSql = "SELECT s.name, s.category FROM candidate_skills cs "
                           "JOIN skills s ON cs.skill_id = s.id WHERE cs.candidate_id = ? ORDER BY s.id ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, skillSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, candidateId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* sName = sqlite3_column_text(stmt, 0);
            const unsigned char* sCat = sqlite3_column_text(stmt, 1);
            Skill sk(
                sName ? reinterpret_cast<const char*>(sName) : "",
                sCat ? reinterpret_cast<const char*>(sCat) : "Other"
            );
            r.addSkill(sk);
        }
        sqlite3_finalize(stmt);
    }

    // Load Education
    const char* eduSql = "SELECT degree, institution, field_of_study, start_year, end_year, score "
                         "FROM candidate_education WHERE candidate_id = ? ORDER BY id ASC;";
    if (sqlite3_prepare_v2(db, eduSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, candidateId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* deg = sqlite3_column_text(stmt, 0);
            const unsigned char* inst = sqlite3_column_text(stmt, 1);
            const unsigned char* fos = sqlite3_column_text(stmt, 2);
            int sy = sqlite3_column_int(stmt, 3);
            int ey = sqlite3_column_int(stmt, 4);
            double sc = sqlite3_column_double(stmt, 5);
            Education edu(
                deg ? reinterpret_cast<const char*>(deg) : "",
                inst ? reinterpret_cast<const char*>(inst) : "",
                fos ? reinterpret_cast<const char*>(fos) : "",
                sy, ey, sc
            );
            r.addEducation(edu);
        }
        sqlite3_finalize(stmt);
    }

    // Load Experience
    const char* expSql = "SELECT company, position, duration, responsibilities, technologies "
                         "FROM candidate_experience WHERE candidate_id = ? ORDER BY id ASC;";
    if (sqlite3_prepare_v2(db, expSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, candidateId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* comp = sqlite3_column_text(stmt, 0);
            const unsigned char* pos = sqlite3_column_text(stmt, 1);
            const unsigned char* dur = sqlite3_column_text(stmt, 2);
            const unsigned char* resp = sqlite3_column_text(stmt, 3);
            const unsigned char* tech = sqlite3_column_text(stmt, 4);

            Experience exp(
                comp ? reinterpret_cast<const char*>(comp) : "",
                pos ? reinterpret_cast<const char*>(pos) : "",
                dur ? reinterpret_cast<const char*>(dur) : ""
            );
            if (resp) {
                for (const auto& rItem : splitStrings(reinterpret_cast<const char*>(resp))) {
                    exp.addResponsibility(rItem);
                }
            }
            if (tech) {
                for (const auto& tItem : splitStrings(reinterpret_cast<const char*>(tech))) {
                    exp.addTechnology(tItem);
                }
            }
            r.addExperience(exp);
        }
        sqlite3_finalize(stmt);
    }

    // Load Projects
    const char* projSql = "SELECT name, description, role, duration, technologies "
                          "FROM candidate_projects WHERE candidate_id = ? ORDER BY id ASC;";
    if (sqlite3_prepare_v2(db, projSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, candidateId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* pName = sqlite3_column_text(stmt, 0);
            const unsigned char* pDesc = sqlite3_column_text(stmt, 1);
            const unsigned char* pRole = sqlite3_column_text(stmt, 2);
            const unsigned char* pDur = sqlite3_column_text(stmt, 3);
            const unsigned char* pTech = sqlite3_column_text(stmt, 4);

            Project proj(
                pName ? reinterpret_cast<const char*>(pName) : "",
                pDesc ? reinterpret_cast<const char*>(pDesc) : "",
                pRole ? reinterpret_cast<const char*>(pRole) : "",
                pDur ? reinterpret_cast<const char*>(pDur) : ""
            );
            if (pTech) {
                for (const auto& tItem : splitStrings(reinterpret_cast<const char*>(pTech))) {
                    proj.addTechnology(tItem);
                }
            }
            r.addProject(proj);
        }
        sqlite3_finalize(stmt);
    }

    // Load Certifications
    const char* certSql = "SELECT name, issuer, year FROM candidate_certifications WHERE candidate_id = ? ORDER BY id ASC;";
    if (sqlite3_prepare_v2(db, certSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, candidateId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* cName = sqlite3_column_text(stmt, 0);
            const unsigned char* cIss = sqlite3_column_text(stmt, 1);
            int yr = sqlite3_column_int(stmt, 2);
            Certification cert(
                cName ? reinterpret_cast<const char*>(cName) : "",
                cIss ? reinterpret_cast<const char*>(cIss) : "",
                yr
            );
            r.addCertification(cert);
        }
        sqlite3_finalize(stmt);
    }

    return r;
}

bool ResumeRepository::resumeExists(int candidateId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db || candidateId <= 0) return false;

    const char* sql = "SELECT 1 FROM resumes WHERE candidate_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

bool ResumeRepository::deleteResume(int candidateId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db || candidateId <= 0) return false;

    const char* sql = "DELETE FROM resumes WHERE candidate_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

double ResumeRepository::getResumeQualityScore(int candidateId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db || candidateId <= 0) return 0.0;

    const char* sql = "SELECT resume_quality_score FROM resumes WHERE candidate_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0.0;
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    double score = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        score = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return score;
}

std::string ResumeRepository::getResumeSourcePath(int candidateId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db || candidateId <= 0) return "";

    const char* sql = "SELECT source_path FROM resumes WHERE candidate_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return "";
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    std::string path = "";
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text) path = reinterpret_cast<const char*>(text);
    }
    sqlite3_finalize(stmt);
    return path;
}

std::string ResumeRepository::getResumeFileFormat(int candidateId) {
    sqlite3* db = DatabaseManager::getDb();
    if (!db || candidateId <= 0) return "TXT";

    const char* sql = "SELECT file_format FROM resumes WHERE candidate_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return "TXT";
    }

    sqlite3_bind_int(stmt, 1, candidateId);
    std::string fmt = "TXT";
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text) fmt = reinterpret_cast<const char*>(text);
    }
    sqlite3_finalize(stmt);
    return fmt;
}
