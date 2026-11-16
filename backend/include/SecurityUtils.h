#pragma once

#include <string>
#include <vector>
#include <cstdint>

// ============================================================
// SecurityUtils — Cryptographic utilities, input validation,
// password policy, and time helpers.
// ============================================================
class SecurityUtils {
public:
    struct ValidationResult {
        bool valid = false;
        std::string message;
    };

    // ── SHA-256 Hashing ─────────────────────────────────────────
    static std::vector<uint8_t> sha256(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> sha256(const std::string& input);
    static std::string sha256Hex(const std::string& input);

    // ── HMAC-SHA256 ─────────────────────────────────────────────
    static std::vector<uint8_t> hmacSha256(
        const std::vector<uint8_t>& key,
        const std::vector<uint8_t>& message);
    static std::vector<uint8_t> hmacSha256(
        const std::string& key,
        const std::string& message);
    static std::string hmacSha256Hex(
        const std::string& key,
        const std::string& message);

    // ── PBKDF2-HMAC-SHA256 Key Derivation ───────────────────────
    static std::vector<uint8_t> pbkdf2HmacSha256(
        const std::string& password,
        const std::vector<uint8_t>& salt,
        int iterations = 100000,
        size_t keyLength = 32);

    // ── Constant-Time Comparison ────────────────────────────────
    static bool constantTimeCompare(const std::string& a, const std::string& b);
    static bool constantTimeCompareBytes(
        const std::vector<uint8_t>& a,
        const std::vector<uint8_t>& b);

    // ── Cryptographically Secure Randomness ─────────────────────
    static std::vector<uint8_t> generateSecureRandomBytes(size_t length);
    static std::string generateSecureRandomHex(size_t byteCount);
    static std::string generateNumericCode(int digits = 6);

    // ── Hex Encoding / Decoding ─────────────────────────────────
    static std::string toHex(const std::vector<uint8_t>& bytes);
    static std::string toHex(const unsigned char* data, size_t len);
    static std::vector<uint8_t> fromHex(const std::string& hex);

    // ── Input & Policy Validation ───────────────────────────────
    static ValidationResult validatePassword(const std::string& password);
    static ValidationResult validateUsername(const std::string& username);
    static ValidationResult validateEmail(const std::string& email);
    static ValidationResult validateNonEmpty(
        const std::string& str,
        const std::string& fieldName,
        size_t maxLen = 100);

    // ── Timestamp Utilities ─────────────────────────────────────
    static std::string getCurrentTimestamp();
    static std::string getExpirationTimestamp(int minutesFromNow);
    static bool isTimestampExpired(const std::string& timestamp);
};
