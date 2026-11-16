#include "DemoDataManager.h"
#include "AuthenticationManager.h"
#include "AccountRepository.h"
#include "CandidateRepository.h"
#include "JobRepository.h"
#include "ResumeRepository.h"
#include "ApplicationManager.h"
#include "SavedJobManager.h"
#include "ResumeStrengthAnalyzer.h"
#include "AuditLogger.h"
#include "DatabaseManager.h"

#include <iostream>
#include <vector>

const std::string DemoDataManager::DEMO_PASSWORD = "DemoPassword123!";
const std::string DemoDataManager::DEMO_CANDIDATE_1_USER = "cand_alice";
const std::string DemoDataManager::DEMO_CANDIDATE_2_USER = "cand_rahul";
const std::string DemoDataManager::DEMO_CANDIDATE_3_USER = "cand_priya";
const std::string DemoDataManager::DEMO_RECRUITER_1_USER = "rec_techcorp";
const std::string DemoDataManager::DEMO_RECRUITER_2_USER = "rec_innovate";

bool DemoDataManager::isDemoDataPresent() {
    Account acc;
    return AccountRepository::findByUsername(DEMO_CANDIDATE_1_USER, acc);
}

bool DemoDataManager::clearDemoData(bool verbose) {
    if (verbose) {
        std::cout << "[SmartHire Demo] Cleaning up existing demo dataset..." << std::endl;
    }

    std::vector<std::string> demoJobIds = {"J101", "J102", "J103", "J104", "J105"};
    std::vector<std::string> demoUsernames = {
        DEMO_CANDIDATE_1_USER,
        DEMO_CANDIDATE_2_USER,
        DEMO_CANDIDATE_3_USER,
        DEMO_RECRUITER_1_USER,
        DEMO_RECRUITER_2_USER
    };

    // Remove demo jobs
    for (const auto& jid : demoJobIds) {
        JobRepository::deleteJob(jid);
    }

    // Remove demo accounts and candidate records
    for (const auto& uname : demoUsernames) {
        Account acc;
        if (AccountRepository::findByUsername(uname, acc)) {
            int candId = AccountRepository::getCandidateIdForAccount(acc.id);
            if (candId > 0) {
                ResumeRepository::deleteResume(candId);
                CandidateRepository::deleteCandidate(candId);
            }
            // Delete account
            std::string sql = "DELETE FROM accounts WHERE id = " + std::to_string(acc.id) + ";";
            DatabaseManager::execute(sql);
        }
    }

    if (verbose) {
        std::cout << "[SmartHire Demo] Demo dataset cleared cleanly." << std::endl;
    }
    return true;
}

bool DemoDataManager::seedDemoData(bool verbose) {
    if (verbose) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "     SMARTHIRE DEMO DATA SEEDING" << std::endl;
        std::cout << "========================================" << std::endl;
    }

    // Ensure idempotency: clear prior demo records
    clearDemoData(false);

    // ── 1. Create Recruiters ────────────────────────────────────
    std::string err;
    int techCorpAccId = -1;
    bool rec1Ok = AuthenticationManager::registerRecruiter(
        "Bob Recruiter",
        "TechCorp Solutions",
        "bob.recruiter.demo@techcorp.local",
        DEMO_RECRUITER_1_USER,
        DEMO_PASSWORD,
        DEMO_PASSWORD,
        err,
        techCorpAccId
    );
    if (!rec1Ok) {
        if (verbose) std::cerr << "Failed to register recruiter 1: " << err << std::endl;
        return false;
    }

    int innovateAccId = -1;
    bool rec2Ok = AuthenticationManager::registerRecruiter(
        "Meera Sen",
        "InnovateAI Labs",
        "meera.sen.demo@innovateai.local",
        DEMO_RECRUITER_2_USER,
        DEMO_PASSWORD,
        DEMO_PASSWORD,
        err,
        innovateAccId
    );
    if (!rec2Ok) {
        if (verbose) std::cerr << "Failed to register recruiter 2: " << err << std::endl;
        return false;
    }

    // ── 2. Create Job Postings ──────────────────────────────────
    Job j101("J101", "C++ Systems Engineer", "TechCorp Solutions", "Pune", "Full Time", "B.Tech Computer Science", 2,
             "Architect and optimize mission-critical embedded and low-latency C++ core microservices with modern C++17.");
    j101.addRequiredSkill("C++");
    j101.addRequiredSkill("Linux");
    j101.addRequiredSkill("Multithreading");
    j101.addRequiredSkill("Data Structures");
    j101.addPreferredSkill("Docker");
    j101.addPreferredSkill("Git");
    j101.addPreferredSkill("CMake");
    JobRepository::saveJob(j101);
    AccountRepository::linkJobToRecruiter("J101", techCorpAccId);

    Job j102("J102", "Java Backend Engineer", "TechCorp Solutions", "Hyderabad", "Full Time", "B.Tech / MCA", 3,
             "Build scalable enterprise REST services using Java, Spring Boot, and robust relational persistence.");
    j102.addRequiredSkill("Java");
    j102.addRequiredSkill("Spring Boot");
    j102.addRequiredSkill("SQL");
    j102.addRequiredSkill("Microservices");
    j102.addPreferredSkill("Kafka");
    j102.addPreferredSkill("Docker");
    JobRepository::saveJob(j102);
    AccountRepository::linkJobToRecruiter("J102", techCorpAccId);

    Job j105("J105", "Software QA & Automation Engineer", "TechCorp Solutions", "Pune", "Full Time", "B.Tech / B.E.", 1,
             "Develop test plans, continuous integration harnesses, and automated regression suites.");
    j105.addRequiredSkill("Testing");
    j105.addRequiredSkill("Selenium");
    j105.addRequiredSkill("Python");
    j105.addRequiredSkill("Git");
    j105.addPreferredSkill("CI/CD");
    j105.addPreferredSkill("Jenkins");
    JobRepository::saveJob(j105);
    AccountRepository::linkJobToRecruiter("J105", techCorpAccId);

    Job j103("J103", "Frontend React Developer", "InnovateAI Labs", "Bengaluru", "Full Time", "Any Degree", 1,
             "Craft elegant, accessible, responsive single page applications using modern React and styled component tokens.");
    j103.addRequiredSkill("React");
    j103.addRequiredSkill("JavaScript");
    j103.addRequiredSkill("CSS");
    j103.addRequiredSkill("HTML");
    j103.addPreferredSkill("TypeScript");
    j103.addPreferredSkill("Vite");
    JobRepository::saveJob(j103);
    AccountRepository::linkJobToRecruiter("J103", innovateAccId);

    Job j104("J104", "Data Analyst & BI Specialist", "InnovateAI Labs", "Mumbai", "Full Time", "Bachelor in Statistics or CS", 1,
             "Perform exploratory data analysis, build executive dashboards, and translate business metrics into insights.");
    j104.addRequiredSkill("Python");
    j104.addRequiredSkill("SQL");
    j104.addRequiredSkill("Excel");
    j104.addRequiredSkill("Statistics");
    j104.addPreferredSkill("Tableau");
    j104.addPreferredSkill("Power BI");
    JobRepository::saveJob(j104);
    AccountRepository::linkJobToRecruiter("J104", innovateAccId);

    // ── 3. Create Candidates & Resumes ──────────────────────────
    // Candidate 1: Alice Smith (C++ Systems specialist)
    int aliceAccId = -1, aliceCandId = -1;
    bool cand1Ok = AuthenticationManager::registerCandidate(
        "Alice Smith",
        "alice.smith.demo@smarthire.local",
        "+91 98765 43210",
        DEMO_CANDIDATE_1_USER,
        DEMO_PASSWORD,
        DEMO_PASSWORD,
        "Pune, India",
        err,
        aliceAccId,
        aliceCandId
    );
    if (!cand1Ok) {
        if (verbose) std::cerr << "Failed to register candidate 1: " << err << std::endl;
        return false;
    }

    Resume aliceResume("Alice Smith", "alice.smith.demo@smarthire.local", "+91 98765 43210",
                       "Pune, India", "linkedin.com/in/alicesmith-demo", "github.com/alicesmith-dev", "alicesmith.dev");
    aliceResume.addSkill(Skill("C++", "Programming"));
    aliceResume.addSkill(Skill("Linux", "OS"));
    aliceResume.addSkill(Skill("Multithreading", "Architecture"));
    aliceResume.addSkill(Skill("Data Structures", "Computer Science"));
    aliceResume.addSkill(Skill("SQL", "Databases"));
    aliceResume.addSkill(Skill("Git", "Tools"));
    aliceResume.addSkill(Skill("CMake", "Build"));
    aliceResume.addSkill(Skill("Algorithms", "Computer Science"));
    aliceResume.addEducation(Education("B.Tech", "COEP Technological University", "Computer Engineering", 2021, 2025, 9.1));
    Experience aliceExp("Tata Elxsi", "Systems Software Intern", "6 months");
    aliceExp.addResponsibility("Designed and implemented multithreaded network protocols and C++17 abstractions.");
    aliceExp.addTechnology("C++");
    aliceExp.addTechnology("Linux");
    aliceExp.addTechnology("Git");
    aliceResume.addExperience(aliceExp);

    Project aliceProj("SmartHire Core", "Intelligent resume parsing and semantic job matching engine in C++17.", "Lead Architecture", "4 months");
    aliceProj.addTechnology("C++");
    aliceProj.addTechnology("SQLite");
    aliceProj.addTechnology("cpp-httplib");
    aliceResume.addProject(aliceProj);

    aliceResume.addCertification(Certification("Certified C++ Professional", "CppInstitute", 2024));

    double aliceQuality = ResumeStrengthAnalyzer::calculateResumeQualityScore(aliceResume);
    ResumeRepository::saveResume(aliceCandId, aliceResume, "data/resumes/alice_demo_resume.txt", "TXT", aliceQuality);

    // Candidate 2: Rahul Sharma (Frontend Developer)
    int rahulAccId = -1, rahulCandId = -1;
    AuthenticationManager::registerCandidate(
        "Rahul Sharma",
        "rahul.sharma.demo@smarthire.local",
        "+91 91234 56789",
        DEMO_CANDIDATE_2_USER,
        DEMO_PASSWORD,
        DEMO_PASSWORD,
        "Bengaluru, India",
        err,
        rahulAccId,
        rahulCandId
    );

    Resume rahulResume("Rahul Sharma", "rahul.sharma.demo@smarthire.local", "+91 91234 56789",
                       "Bengaluru, India", "linkedin.com/in/rahulsharma-demo", "github.com/rahulsharma-ui", "");
    rahulResume.addSkill(Skill("React", "Frontend"));
    rahulResume.addSkill(Skill("JavaScript", "Languages"));
    rahulResume.addSkill(Skill("CSS", "Styling"));
    rahulResume.addSkill(Skill("HTML", "Web"));
    rahulResume.addSkill(Skill("TypeScript", "Languages"));
    rahulResume.addSkill(Skill("Vite", "Build"));
    rahulResume.addEducation(Education("B.E.", "RV College of Engineering", "Information Science", 2020, 2024, 8.4));

    Experience rahulExp("InnoTech Solutions", "Junior Frontend Developer", "1 year");
    rahulExp.addResponsibility("Engineered interactive dashboards with React and vanilla CSS.");
    rahulExp.addTechnology("React");
    rahulExp.addTechnology("JavaScript");
    rahulResume.addExperience(rahulExp);

    Project rahulProj("SmartHire UI", "Glassmorphic responsive web portal for candidate and recruiter workflows.", "Frontend Developer", "3 months");
    rahulProj.addTechnology("React");
    rahulProj.addTechnology("Vite");
    rahulProj.addTechnology("CSS");
    rahulResume.addProject(rahulProj);

    double rahulQuality = ResumeStrengthAnalyzer::calculateResumeQualityScore(rahulResume);
    ResumeRepository::saveResume(rahulCandId, rahulResume, "data/resumes/rahul_demo_resume.txt", "TXT", rahulQuality);

    // Candidate 3: Priya Patel (Data Analyst)
    int priyaAccId = -1, priyaCandId = -1;
    AuthenticationManager::registerCandidate(
        "Priya Patel",
        "priya.patel.demo@smarthire.local",
        "+91 98220 11223",
        DEMO_CANDIDATE_3_USER,
        DEMO_PASSWORD,
        DEMO_PASSWORD,
        "Mumbai, India",
        err,
        priyaAccId,
        priyaCandId
    );

    Resume priyaResume("Priya Patel", "priya.patel.demo@smarthire.local", "+91 98220 11223",
                       "Mumbai, India", "linkedin.com/in/priyapatel-demo", "github.com/priyapatel-data", "");
    priyaResume.addSkill(Skill("Python", "Languages"));
    priyaResume.addSkill(Skill("SQL", "Databases"));
    priyaResume.addSkill(Skill("Excel", "Analytics"));
    priyaResume.addSkill(Skill("Statistics", "Mathematics"));
    priyaResume.addSkill(Skill("Tableau", "BI"));
    priyaResume.addSkill(Skill("Power BI", "BI"));
    priyaResume.addEducation(Education("B.Sc.", "St. Xavier's College", "Statistics", 2021, 2024, 9.0));

    Project priyaProj("Recruitment Predictive Analytics", "Statistical modeling of candidate match distributions.", "Data Analyst", "3 months");
    priyaProj.addTechnology("Python");
    priyaProj.addTechnology("Pandas");
    priyaProj.addTechnology("SQL");
    priyaResume.addProject(priyaProj);

    double priyaQuality = ResumeStrengthAnalyzer::calculateResumeQualityScore(priyaResume);
    ResumeRepository::saveResume(priyaCandId, priyaResume, "data/resumes/priya_demo_resume.txt", "TXT", priyaQuality);

    // ── 4. Submit Applications & Transition Stages ──────────────
    // Alice applies to J101 (C++ Systems Engineer) -> SHORTLISTED
    int app1Id = ApplicationManager::applyForJob(aliceCandId, j101, 88.5, 86.0);
    if (app1Id > 0) {
        ApplicationManager::updateStatus(app1Id, "SHORTLISTED");
    }

    // Alice applies to J105 (QA Engineer) -> APPLIED
    ApplicationManager::applyForJob(aliceCandId, j105, 65.0, 60.0);

    // Rahul applies to J103 (React Developer) -> UNDER_REVIEW
    int app3Id = ApplicationManager::applyForJob(rahulCandId, j103, 85.0, 80.0);
    if (app3Id > 0) {
        ApplicationManager::updateStatus(app3Id, "UNDER_REVIEW");
    }

    // Priya applies to J104 (Data Analyst) -> INTERVIEW
    int app4Id = ApplicationManager::applyForJob(priyaCandId, j104, 92.0, 90.0);
    if (app4Id > 0) {
        ApplicationManager::updateStatus(app4Id, "INTERVIEW");
    }

    // ── 5. Saved Jobs ───────────────────────────────────────────
    SavedJobManager::saveJob(aliceCandId, "J102");
    SavedJobManager::saveJob(rahulCandId, "J101");

    // ── 6. Operational Audit Trail ──────────────────────────────
    AuditLogger::log(aliceAccId, "LOGIN_SUCCESS", "Demo candidate Alice logged in.");
    AuditLogger::log(aliceAccId, "RESUME_UPLOADED", "Alice uploaded resume: alice_demo_resume.txt (Score: 92.5%)");
    AuditLogger::log(aliceAccId, "APPLICATION_SUBMITTED", "Alice applied for job J101 (C++ Systems Engineer).");
    AuditLogger::log(techCorpAccId, "JOB_CREATED", "Recruiter created job requisition J101.");
    AuditLogger::log(techCorpAccId, "APPLICATION_STATUS_UPDATED", "Application for J101 advanced to SHORTLISTED.");

    if (verbose) {
        std::cout << "[SUCCESS] SmartHire Demo Dataset successfully initialized!\n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << "DEMO CANDIDATE CREDENTIALS:\n";
        std::cout << "  Username: " << DEMO_CANDIDATE_1_USER << "\n";
        std::cout << "  Password: " << DEMO_PASSWORD << "\n";
        std::cout << "  Profile:  Alice Smith (C++ Systems Specialist)\n\n";
        std::cout << "DEMO RECRUITER CREDENTIALS:\n";
        std::cout << "  Username: " << DEMO_RECRUITER_1_USER << "\n";
        std::cout << "  Password: " << DEMO_PASSWORD << "\n";
        std::cout << "  Company:  TechCorp Solutions\n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << "SEEDED JOBS:\n";
        std::cout << "  J101 - C++ Systems Engineer (TechCorp Solutions)\n";
        std::cout << "  J102 - Java Backend Engineer (TechCorp Solutions)\n";
        std::cout << "  J103 - Frontend React Developer (InnovateAI Labs)\n";
        std::cout << "  J104 - Data Analyst & BI Specialist (InnovateAI Labs)\n";
        std::cout << "  J105 - Software QA & Automation Engineer (TechCorp Solutions)\n";
        std::cout << "========================================\n" << std::endl;
    }

    return true;
}
