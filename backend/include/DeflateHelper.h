#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

// ============================================================
// DeflateHelper — Lightweight, self-contained DEFLATE and zlib
// decompressor (RFC 1950 / RFC 1951) with 0 external dependencies.
// ============================================================
class DeflateHelper {
public:
    // Decompresses zlib or raw DEFLATE byte buffer
    static bool decompress(const uint8_t* src, size_t srcLen, std::vector<uint8_t>& dest);

    // Convenience string-based overload
    static std::string decompressString(const std::string& compressedData);

    // Checks if the buffer starts with a standard zlib header (0x78...)
    static bool isZlibHeader(const uint8_t* src, size_t srcLen);
};
