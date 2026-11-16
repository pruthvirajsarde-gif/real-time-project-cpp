#include "PasswordHasher.h"
#include "SecurityUtils.h"

std::string PasswordHasher::generateSalt(size_t byteCount) {
    return SecurityUtils::generateSecureRandomHex(byteCount);
}

std::string PasswordHasher::hashPassword(
    const std::string& password,
    const std::string& salt,
    int iterations) 
{
    std::vector<uint8_t> saltBytes = SecurityUtils::fromHex(salt);
    // If salt is raw text rather than hex, convert directly to bytes
    if (saltBytes.empty() && !salt.empty()) {
        saltBytes.assign(salt.begin(), salt.end());
    }

    std::vector<uint8_t> derived = SecurityUtils::pbkdf2HmacSha256(
        password,
        saltBytes,
        iterations,
        DERIVED_KEY_BYTES
    );

    return SecurityUtils::toHex(derived);
}

bool PasswordHasher::verifyPassword(
    const std::string& password,
    const std::string& salt,
    const std::string& expectedHash,
    int iterations) 
{
    if (password.empty() || salt.empty() || expectedHash.empty()) {
        return false;
    }

    std::string computedHash = hashPassword(password, salt, iterations);
    return SecurityUtils::constantTimeCompare(computedHash, expectedHash);
}
