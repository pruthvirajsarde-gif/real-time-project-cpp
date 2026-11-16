#pragma once

#include "Profile.h"
#include <string>
#include <vector>
#include <utility>

// ============================================================
// CandidateRepository — Data access operations for candidates table
// Provides secure parameterized CRUD and deduplication by email.
// ============================================================
class CandidateRepository {
public:
    // Create a new candidate record. Returns candidate ID (> 0) on success, or -1 on failure.
    static int createCandidate(const Profile& profile);

    // Retrieve candidate profile by candidate ID
    static Profile getCandidate(int candidateId);

    // Update an existing candidate profile
    static bool updateCandidate(int candidateId, const Profile& profile);

    // Delete candidate by candidate ID (cascades to resumes, applications, saved jobs)
    static bool deleteCandidate(int candidateId);

    // Find candidate ID by email address. Returns candidate ID if found, or -1 if not found.
    static int findCandidateByEmail(const std::string& email);

    // Save or update candidate. If email already exists, updates profile; otherwise creates new.
    // Returns candidate ID. Sets isNew to true if created, false if updated.
    static int saveOrUpdateCandidate(const Profile& profile, bool& isNew);

    // Retrieve all candidates as (ID, Profile) pairs
    static std::vector<std::pair<int, Profile>> getAllCandidates();

    // Check if a candidate exists by ID
    static bool candidateExists(int candidateId);
};
