#pragma once

#include <string>
#include <cstddef>

// ============================================================
// PasswordHasher — Secure password hashing using PBKDF2-HMAC-SHA256
// with cryptographically random salts and constant-time verification.
// ============================================================
class PasswordHasher {
public:
    static constexpr int DEFAULT_ITERATIONS = 100000;
    static constexpr size_t DEFAULT_SALT_BYTES = 16;
    static constexpr size_t DERIVED_KEY_BYTES = 32;

    // Generates a cryptographically secure random salt (hex-encoded)
    static std::string generateSalt(size_t byteCount = DEFAULT_SALT_BYTES);

    // Derives a 32-byte key from password and salt using PBKDF2-HMAC-SHA256 (hex-encoded)
    static std::string hashPassword(
        const std::string& password,
        const std::string& salt,
        int iterations = DEFAULT_ITERATIONS);

    // Verifies password in constant-time against expected PBKDF2-HMAC-SHA256 hash
    static bool verifyPassword(
        const std::string& password,
        const std::string& salt,
        const std::string& expectedHash,
        int iterations = DEFAULT_ITERATIONS);
};
