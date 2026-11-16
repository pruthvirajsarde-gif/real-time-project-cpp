#include "SecurityUtils.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <cctype>

#include <random>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#endif

// ============================================================
// Internal SHA-256 Implementation (FIPS 180-4)
// ============================================================
namespace {

inline uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

inline uint32_t choose(uint32_t e, uint32_t f, uint32_t g) {
    return (e & f) ^ (~e & g);
}

inline uint32_t majority(uint32_t a, uint32_t b, uint32_t c) {
    return (a & b) ^ (a & c) ^ (b & c);
}

inline uint32_t sig0(uint32_t x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

inline uint32_t sig1(uint32_t x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

inline uint32_t theta0(uint32_t x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

inline uint32_t theta1(uint32_t x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

static const uint32_t K256[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

struct Sha256Context {
    uint32_t state[8];
    uint64_t bitCount;
    uint8_t buffer[64];
    size_t bufferLen;
};

void sha256Init(Sha256Context& ctx) {
    ctx.state[0] = 0x6a09e667;
    ctx.state[1] = 0xbb67ae85;
    ctx.state[2] = 0x3c6ef372;
    ctx.state[3] = 0xa54ff53a;
    ctx.state[4] = 0x510e527f;
    ctx.state[5] = 0x9b05688c;
    ctx.state[6] = 0x1f83d9ab;
    ctx.state[7] = 0x5be0cd19;
    ctx.bitCount = 0;
    ctx.bufferLen = 0;
}

void sha256Transform(Sha256Context& ctx, const uint8_t* block) {
    uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = (static_cast<uint32_t>(block[i * 4 + 0]) << 24) |
               (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(block[i * 4 + 2]) << 8)  |
               (static_cast<uint32_t>(block[i * 4 + 3]));
    }
    for (int i = 16; i < 64; ++i) {
        w[i] = theta1(w[i - 2]) + w[i - 7] + theta0(w[i - 15]) + w[i - 16];
    }

    uint32_t a = ctx.state[0];
    uint32_t b = ctx.state[1];
    uint32_t c = ctx.state[2];
    uint32_t d = ctx.state[3];
    uint32_t e = ctx.state[4];
    uint32_t f = ctx.state[5];
    uint32_t g = ctx.state[6];
    uint32_t h = ctx.state[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + sig1(e) + choose(e, f, g) + K256[i] + w[i];
        uint32_t t2 = sig0(a) + majority(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx.state[0] += a;
    ctx.state[1] += b;
    ctx.state[2] += c;
    ctx.state[3] += d;
    ctx.state[4] += e;
    ctx.state[5] += f;
    ctx.state[6] += g;
    ctx.state[7] += h;
}

void sha256Update(Sha256Context& ctx, const uint8_t* data, size_t len) {
    ctx.bitCount += static_cast<uint64_t>(len) * 8;
    while (len > 0) {
        size_t toCopy = std::min(len, 64 - ctx.bufferLen);
        std::memcpy(ctx.buffer + ctx.bufferLen, data, toCopy);
        ctx.bufferLen += toCopy;
        data += toCopy;
        len -= toCopy;
        if (ctx.bufferLen == 64) {
            sha256Transform(ctx, ctx.buffer);
            ctx.bufferLen = 0;
        }
    }
}

void sha256Final(Sha256Context& ctx, uint8_t hash[32]) {
    // Append 0x80
    ctx.buffer[ctx.bufferLen++] = 0x80;
    if (ctx.bufferLen > 56) {
        std::memset(ctx.buffer + ctx.bufferLen, 0, 64 - ctx.bufferLen);
        sha256Transform(ctx, ctx.buffer);
        ctx.bufferLen = 0;
    }
    std::memset(ctx.buffer + ctx.bufferLen, 0, 56 - ctx.bufferLen);
    // Append length in bits big-endian
    for (int i = 7; i >= 0; --i) {
        ctx.buffer[56 + (7 - i)] = static_cast<uint8_t>((ctx.bitCount >> (i * 8)) & 0xFF);
    }
    sha256Transform(ctx, ctx.buffer);

    for (int i = 0; i < 8; ++i) {
        hash[i * 4 + 0] = static_cast<uint8_t>((ctx.state[i] >> 24) & 0xFF);
        hash[i * 4 + 1] = static_cast<uint8_t>((ctx.state[i] >> 16) & 0xFF);
        hash[i * 4 + 2] = static_cast<uint8_t>((ctx.state[i] >> 8)  & 0xFF);
        hash[i * 4 + 3] = static_cast<uint8_t>((ctx.state[i])       & 0xFF);
    }
}

} // anonymous namespace

// ── SHA-256 Hashing ─────────────────────────────────────────

std::vector<uint8_t> SecurityUtils::sha256(const std::vector<uint8_t>& data) {
    Sha256Context ctx;
    sha256Init(ctx);
    if (!data.empty()) {
        sha256Update(ctx, data.data(), data.size());
    }
    uint8_t hash[32];
    sha256Final(ctx, hash);
    return std::vector<uint8_t>(hash, hash + 32);
}

std::vector<uint8_t> SecurityUtils::sha256(const std::string& input) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(input.data());
    Sha256Context ctx;
    sha256Init(ctx);
    if (!input.empty()) {
        sha256Update(ctx, p, input.size());
    }
    uint8_t hash[32];
    sha256Final(ctx, hash);
    return std::vector<uint8_t>(hash, hash + 32);
}

std::string SecurityUtils::sha256Hex(const std::string& input) {
    return toHex(sha256(input));
}

// ── HMAC-SHA256 ─────────────────────────────────────────────

std::vector<uint8_t> SecurityUtils::hmacSha256(
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& message) 
{
    uint8_t k0[64] = {0};
    if (key.size() > 64) {
        std::vector<uint8_t> keyHash = sha256(key);
        std::memcpy(k0, keyHash.data(), 32);
    } else {
        std::memcpy(k0, key.data(), key.size());
    }

    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    for (int i = 0; i < 64; ++i) {
        k_ipad[i] = k0[i] ^ 0x36;
        k_opad[i] = k0[i] ^ 0x5c;
    }

    // Inner hash = SHA256(k_ipad || message)
    Sha256Context innerCtx;
    sha256Init(innerCtx);
    sha256Update(innerCtx, k_ipad, 64);
    if (!message.empty()) {
        sha256Update(innerCtx, message.data(), message.size());
    }
    uint8_t innerHash[32];
    sha256Final(innerCtx, innerHash);

    // Outer hash = SHA256(k_opad || innerHash)
    Sha256Context outerCtx;
    sha256Init(outerCtx);
    sha256Update(outerCtx, k_opad, 64);
    sha256Update(outerCtx, innerHash, 32);
    uint8_t outerHash[32];
    sha256Final(outerCtx, outerHash);

    return std::vector<uint8_t>(outerHash, outerHash + 32);
}

std::vector<uint8_t> SecurityUtils::hmacSha256(
    const std::string& key,
    const std::string& message) 
{
    std::vector<uint8_t> k(key.begin(), key.end());
    std::vector<uint8_t> m(message.begin(), message.end());
    return hmacSha256(k, m);
}

std::string SecurityUtils::hmacSha256Hex(
    const std::string& key,
    const std::string& message) 
{
    return toHex(hmacSha256(key, message));
}

// ── PBKDF2-HMAC-SHA256 Key Derivation ───────────────────────

std::vector<uint8_t> SecurityUtils::pbkdf2HmacSha256(
    const std::string& password,
    const std::vector<uint8_t>& salt,
    int iterations,
    size_t keyLength) 
{
    std::vector<uint8_t> derivedKey(keyLength, 0);

    // Prepare key block for HMAC
    uint8_t k0[64] = {0};
    if (password.size() > 64) {
        std::vector<uint8_t> keyHash = sha256(password);
        std::memcpy(k0, keyHash.data(), 32);
    } else {
        std::memcpy(k0, password.data(), password.size());
    }

    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    for (int i = 0; i < 64; ++i) {
        k_ipad[i] = k0[i] ^ 0x36;
        k_opad[i] = k0[i] ^ 0x5c;
    }

    // Pre-initialize contexts for inner and outer padding to maximize throughput
    Sha256Context baseInnerCtx;
    sha256Init(baseInnerCtx);
    sha256Update(baseInnerCtx, k_ipad, 64);

    Sha256Context baseOuterCtx;
    sha256Init(baseOuterCtx);
    sha256Update(baseOuterCtx, k_opad, 64);

    auto hmacFast = [&](const uint8_t* msg, size_t msgLen, uint8_t out[32]) {
        Sha256Context inner = baseInnerCtx;
        if (msgLen > 0) {
            sha256Update(inner, msg, msgLen);
        }
        uint8_t innerHash[32];
        sha256Final(inner, innerHash);

        Sha256Context outer = baseOuterCtx;
        sha256Update(outer, innerHash, 32);
        sha256Final(outer, out);
    };

    size_t blocks = (keyLength + 31) / 32;
    size_t written = 0;

    for (size_t block = 1; block <= blocks; ++block) {
        // Salt || INT_32_BE(block)
        std::vector<uint8_t> saltAndBlock = salt;
        saltAndBlock.push_back(static_cast<uint8_t>((block >> 24) & 0xFF));
        saltAndBlock.push_back(static_cast<uint8_t>((block >> 16) & 0xFF));
        saltAndBlock.push_back(static_cast<uint8_t>((block >> 8) & 0xFF));
        saltAndBlock.push_back(static_cast<uint8_t>(block & 0xFF));

        uint8_t u[32];
        hmacFast(saltAndBlock.data(), saltAndBlock.size(), u);

        uint8_t f[32];
        std::memcpy(f, u, 32);

        for (int iter = 1; iter < iterations; ++iter) {
            hmacFast(u, 32, u);
            for (int j = 0; j < 32; ++j) {
                f[j] ^= u[j];
            }
        }

        size_t toCopy = std::min(static_cast<size_t>(32), keyLength - written);
        std::memcpy(derivedKey.data() + written, f, toCopy);
        written += toCopy;
    }

    return derivedKey;
}

// ── Constant-Time Comparison ────────────────────────────────

bool SecurityUtils::constantTimeCompare(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    uint8_t diff = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        diff |= (static_cast<uint8_t>(a[i]) ^ static_cast<uint8_t>(b[i]));
    }
    return diff == 0;
}

bool SecurityUtils::constantTimeCompareBytes(
    const std::vector<uint8_t>& a,
    const std::vector<uint8_t>& b) 
{
    if (a.size() != b.size()) {
        return false;
    }
    uint8_t diff = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        diff |= (a[i] ^ b[i]);
    }
    return diff == 0;
}

// ── Cryptographically Secure Randomness ─────────────────────

std::vector<uint8_t> SecurityUtils::generateSecureRandomBytes(size_t length) {
    std::vector<uint8_t> bytes(length, 0);
    if (length == 0) return bytes;

#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    if (CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        CryptGenRandom(hProv, static_cast<DWORD>(length), bytes.data());
        CryptReleaseContext(hProv, 0);
        return bytes;
    }
#endif

    // Fallback: standard random_device
    std::random_device rd;
    for (size_t i = 0; i < length; ++i) {
        bytes[i] = static_cast<uint8_t>(rd() & 0xFF);
    }
    return bytes;
}

std::string SecurityUtils::generateSecureRandomHex(size_t byteCount) {
    return toHex(generateSecureRandomBytes(byteCount));
}

std::string SecurityUtils::generateNumericCode(int digits) {
    if (digits <= 0) digits = 6;
    std::vector<uint8_t> randBytes = generateSecureRandomBytes(digits * 2);
    std::string code;
    for (int i = 0; i < digits; ++i) {
        uint32_t val = (static_cast<uint32_t>(randBytes[i * 2]) << 8) |
                       (static_cast<uint32_t>(randBytes[i * 2 + 1]));
        code += std::to_string(val % 10);
    }
    return code;
}

// ── Hex Encoding / Decoding ─────────────────────────────────

std::string SecurityUtils::toHex(const std::vector<uint8_t>& bytes) {
    return toHex(bytes.data(), bytes.size());
}

std::string SecurityUtils::toHex(const unsigned char* data, size_t len) {
    static const char hexChars[] = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out.push_back(hexChars[(data[i] >> 4) & 0x0F]);
        out.push_back(hexChars[data[i] & 0x0F]);
    }
    return out;
}

std::vector<uint8_t> SecurityUtils::fromHex(const std::string& hex) {
    std::vector<uint8_t> bytes;
    if (hex.size() % 2 != 0) return bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        char high = hex[i];
        char low = hex[i + 1];
        auto hexVal = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        int h = hexVal(high);
        int l = hexVal(low);
        if (h == -1 || l == -1) {
            bytes.clear();
            return bytes;
        }
        bytes.push_back(static_cast<uint8_t>((h << 4) | l));
    }
    return bytes;
}

// ── Input & Policy Validation ───────────────────────────────

SecurityUtils::ValidationResult SecurityUtils::validatePassword(const std::string& password) {
    if (password.length() < 8) {
        return {false, "Password must be at least 8 characters long."};
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) hasUpper = true;
        else if (std::islower(static_cast<unsigned char>(c))) hasLower = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
    }

    if (!hasUpper) {
        return {false, "Password must contain at least one uppercase letter."};
    }
    if (!hasLower) {
        return {false, "Password must contain at least one lowercase letter."};
    }
    if (!hasDigit) {
        return {false, "Password must contain at least one digit."};
    }

    // Common weak password rejection
    std::string lowerPass = password;
    std::transform(lowerPass.begin(), lowerPass.end(), lowerPass.begin(), ::tolower);
    const std::vector<std::string> weakList = {
        "password", "12345678", "password1", "qwerty123", "admin123",
        "letmein1", "welcome1", "pass1234", "password123"
    };

    for (const auto& weak : weakList) {
        if (lowerPass == weak) {
            return {false, "Password is too common and easily guessed."};
        }
    }

    return {true, "Password meets security policy requirements."};
}

SecurityUtils::ValidationResult SecurityUtils::validateUsername(const std::string& username) {
    if (username.empty()) {
        return {false, "Username cannot be empty."};
    }
    if (username.length() < 3 || username.length() > 30) {
        return {false, "Username must be between 3 and 30 characters."};
    }
    for (char c : username) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
            return {false, "Username may only contain letters, numbers, and underscores."};
        }
    }
    return {true, "Username is valid."};
}

SecurityUtils::ValidationResult SecurityUtils::validateEmail(const std::string& email) {
    if (email.empty()) {
        return {false, "Email cannot be empty."};
    }
    size_t atPos = email.find('@');
    if (atPos == std::string::npos || atPos == 0 || atPos == email.length() - 1) {
        return {false, "Invalid email address format (missing or mispositioned '@')."};
    }
    if (email.find('@', atPos + 1) != std::string::npos) {
        return {false, "Email cannot contain multiple '@' characters."};
    }
    size_t dotPos = email.find('.', atPos + 1);
    if (dotPos == std::string::npos || dotPos == atPos + 1 || dotPos == email.length() - 1) {
        return {false, "Invalid email domain (missing domain extension)."};
    }
    if (email.find(' ') != std::string::npos) {
        return {false, "Email cannot contain spaces."};
    }
    return {true, "Email is valid."};
}

SecurityUtils::ValidationResult SecurityUtils::validateNonEmpty(
    const std::string& str,
    const std::string& fieldName,
    size_t maxLen) 
{
    if (str.empty() || str.find_first_not_of(" \t\n\r") == std::string::npos) {
        return {false, fieldName + " cannot be empty."};
    }
    if (str.length() > maxLen) {
        return {false, fieldName + " exceeds maximum allowed length (" + std::to_string(maxLen) + ")."};
    }
    return {true, "Valid."};
}

// ── Timestamp Utilities ─────────────────────────────────────

std::string SecurityUtils::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow;
#ifdef _WIN32
    localtime_s(&tmNow, &nowTime);
#else
    localtime_r(&nowTime, &tmNow);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmNow);
    return std::string(buf);
}

std::string SecurityUtils::getExpirationTimestamp(int minutesFromNow) {
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::minutes(minutesFromNow);
    std::time_t expTime = std::chrono::system_clock::to_time_t(exp);
    std::tm tmExp;
#ifdef _WIN32
    localtime_s(&tmExp, &expTime);
#else
    localtime_r(&expTime, &tmExp);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmExp);
    return std::string(buf);
}

bool SecurityUtils::isTimestampExpired(const std::string& timestamp) {
    if (timestamp.empty()) return true;
    std::tm tmExp = {};
    std::istringstream ss(timestamp);
    ss >> std::get_time(&tmExp, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        return true;
    }
    std::time_t expTime = std::mktime(&tmExp);
    if (expTime == -1) return true;

    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    return (nowTime >= expTime);
}
