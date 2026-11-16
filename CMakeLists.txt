cmake_minimum_required(VERSION 3.16)
project(SmartHire VERSION 1.0 LANGUAGES C CXX)

# ── C++17 Standard ──────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Include directories ────────────────────────────────────
include_directories(
    ${CMAKE_SOURCE_DIR}/backend/include
    ${CMAKE_SOURCE_DIR}/backend/third_party/sqlite
    ${CMAKE_SOURCE_DIR}/backend/third_party/httplib
    ${CMAKE_SOURCE_DIR}/backend/third_party/json
)

# ── SQLite3 Library ─────────────────────────────────────────
add_library(sqlite3 STATIC backend/third_party/sqlite/sqlite3.c)

# ── Core Sources ───────────────────────────────────────────
set(CORE_SOURCES
    backend/src/Profile.cpp
    backend/src/Skill.cpp
    backend/src/Education.cpp
    backend/src/Project.cpp
    backend/src/Experience.cpp
    backend/src/Certification.cpp
    backend/src/Resume.cpp
    backend/src/Job.cpp
    backend/src/ResumeAnalyzer.cpp
    backend/src/JobMatcher.cpp
    backend/src/ResumeParser.cpp
    backend/src/JobDataLoader.cpp
    backend/src/ReportGenerator.cpp
    backend/src/SkillGapAnalyzer.cpp
    backend/src/ResumeStrengthAnalyzer.cpp
    backend/src/RecommendationEngine.cpp
    backend/src/ResumeDocumentReader.cpp
    backend/src/DeflateHelper.cpp
    backend/src/ResumeInputManager.cpp
    backend/src/DatabaseManager.cpp
    backend/src/CandidateRepository.cpp
    backend/src/JobRepository.cpp
    backend/src/ResumeRepository.cpp
    backend/src/ApplicationManager.cpp
    backend/src/SavedJobManager.cpp
    backend/src/TextPreprocessor.cpp
    backend/src/KeywordExtractor.cpp
    backend/src/SkillNormalizer.cpp
    backend/src/TextSimilarity.cpp
    backend/src/SemanticMatcher.cpp
    backend/src/MatchExplanation.cpp
    backend/src/AdvancedJobMatcher.cpp
    backend/src/SecurityUtils.cpp
    backend/src/PasswordHasher.cpp
    backend/src/AccountRepository.cpp
    backend/src/AuditLogger.cpp
    backend/src/SessionManager.cpp
    backend/src/AuthorizationManager.cpp
    backend/src/AuthenticationManager.cpp
    backend/src/ApiResponse.cpp
    backend/src/ApiMiddleware.cpp
    backend/src/AuthController.cpp
    backend/src/CandidateController.cpp
    backend/src/ResumeController.cpp
    backend/src/JobController.cpp
    backend/src/MatchingController.cpp
    backend/src/SavedJobController.cpp
    backend/src/ApplicationController.cpp
    backend/src/RecruiterController.cpp
    backend/src/DashboardController.cpp
    backend/src/SystemController.cpp
    backend/src/ApiRouter.cpp
    backend/src/HttpServer.cpp
    backend/src/DemoDataManager.cpp
    backend/src/AnalyticsService.cpp
    backend/src/AnalyticsController.cpp
)

# ── Platform Cryptography & Networking Libraries ─────────────
set(PLATFORM_LIBS "")
if (WIN32)
    set(PLATFORM_LIBS advapi32 bcrypt ws2_32 crypt32)
else()
    set(PLATFORM_LIBS pthread dl)
endif()

# ── Main Executable ─────────────────────────────────────────
add_executable(SmartHire backend/main.cpp ${CORE_SOURCES})
target_link_libraries(SmartHire PRIVATE sqlite3 ${PLATFORM_LIBS})

# ── Test Suite Executable ───────────────────────────────────
add_executable(test_runner tests/test_runner.cpp ${CORE_SOURCES})
target_link_libraries(test_runner PRIVATE sqlite3 ${PLATFORM_LIBS})


