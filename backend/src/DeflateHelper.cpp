#include "DeflateHelper.h"

#include <cstring>
#include <algorithm>

namespace {

struct HuffmanTree {
    uint16_t counts[16];
    uint16_t offsets[16];
    uint16_t symbols[288];
};

struct BitStream {
    const uint8_t* data;
    size_t length;
    size_t bytePos;
    uint32_t bitBuffer;
    int bitsLeft;

    BitStream(const uint8_t* d, size_t len)
        : data(d), length(len), bytePos(0), bitBuffer(0), bitsLeft(0) {}

    uint32_t readBits(int count) {
        while (bitsLeft < count) {
            if (bytePos < length) {
                bitBuffer |= static_cast<uint32_t>(data[bytePos++]) << bitsLeft;
                bitsLeft += 8;
            } else {
                break;
            }
        }
        uint32_t result = bitBuffer & ((1u << count) - 1);
        bitBuffer >>= count;
        bitsLeft -= count;
        return result;
    }

    void alignToByte() {
        bitBuffer = 0;
        bitsLeft = 0;
    }
};

static void buildHuffmanTree(HuffmanTree* tree, const uint8_t* lengths, int numSymbols) {
    std::memset(tree->counts, 0, sizeof(tree->counts));
    for (int i = 0; i < numSymbols; ++i) {
        if (lengths[i] > 0 && lengths[i] < 16) {
            tree->counts[lengths[i]]++;
        }
    }

    tree->offsets[0] = 0;
    tree->offsets[1] = 0;
    for (int i = 1; i < 15; ++i) {
        tree->offsets[i + 1] = tree->offsets[i] + tree->counts[i];
    }

    uint16_t currOffset[16];
    std::memcpy(currOffset, tree->offsets, sizeof(currOffset));

    for (int i = 0; i < numSymbols; ++i) {
        if (lengths[i] > 0 && lengths[i] < 16) {
            tree->symbols[currOffset[lengths[i]]++] = static_cast<uint16_t>(i);
        }
    }
}

static int decodeSymbol(BitStream& bs, const HuffmanTree* tree) {
    int code = 0;
    int first = 0;
    for (int len = 1; len <= 15; ++len) {
        code |= static_cast<int>(bs.readBits(1));
        int count = tree->counts[len];
        if (code - count < first) {
            int idx = tree->offsets[len] + (code - first);
            return (idx >= 0 && idx < 288) ? tree->symbols[idx] : -1;
        }
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -1;
}

// Code length order for dynamic Huffman header
static const uint8_t CL_ORDER[19] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

// Length code base values and extra bits (codes 257..285)
static const uint16_t LEN_BASE[29] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};
static const uint8_t LEN_EXTRA[29] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};

// Distance code base values and extra bits (codes 0..29)
static const uint16_t DIST_BASE[30] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};
static const uint8_t DIST_EXTRA[30] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

} // namespace

bool DeflateHelper::isZlibHeader(const uint8_t* src, size_t srcLen) {
    if (!src || srcLen < 2) return false;
    uint8_t cmf = src[0];
    uint8_t flg = src[1];
    // Check compression method is 8 (deflate) and header checksum check
    if ((cmf & 0x0F) != 8) return false;
    return ((static_cast<uint32_t>(cmf) * 256 + flg) % 31 == 0);
}

bool DeflateHelper::decompress(const uint8_t* src, size_t srcLen, std::vector<uint8_t>& dest) {
    if (!src || srcLen == 0) return false;

    size_t offset = 0;
    // Check for zlib header
    if (isZlibHeader(src, srcLen)) {
        offset = 2; // skip CMF and FLG
        if (src[1] & 0x20) {
            // Preset dictionary flag is set: skip 4-byte dictionary ID
            if (srcLen < 6) return false;
            offset += 4;
        }
    }

    BitStream bs(src + offset, srcLen - offset);
    int bfinal = 0;

    // Safety limit on uncompressed output: 32MB
    const size_t MAX_UNCOMPRESSED_SIZE = 32 * 1024 * 1024;

    while (!bfinal) {
        bfinal = static_cast<int>(bs.readBits(1));
        int btype = static_cast<int>(bs.readBits(2));

        if (btype == 0) {
            // Uncompressed block (stored)
            bs.alignToByte();
            if (bs.bytePos + 4 > bs.length) return false;
            uint16_t len = bs.data[bs.bytePos] | (static_cast<uint16_t>(bs.data[bs.bytePos + 1]) << 8);
            uint16_t nlen = bs.data[bs.bytePos + 2] | (static_cast<uint16_t>(bs.data[bs.bytePos + 3]) << 8);
            bs.bytePos += 4;
            if (static_cast<uint16_t>(~len) != nlen) return false;
            if (bs.bytePos + len > bs.length) return false;
            if (dest.size() + len > MAX_UNCOMPRESSED_SIZE) return false;

            dest.insert(dest.end(), bs.data + bs.bytePos, bs.data + bs.bytePos + len);
            bs.bytePos += len;
        } else if (btype == 1 || btype == 2) {
            HuffmanTree litTree;
            HuffmanTree distTree;

            if (btype == 1) {
                // Fixed Huffman codes
                uint8_t ll[288];
                for (int i = 0; i <= 143; ++i) ll[i] = 8;
                for (int i = 144; i <= 255; ++i) ll[i] = 9;
                for (int i = 256; i <= 279; ++i) ll[i] = 7;
                for (int i = 280; i <= 287; ++i) ll[i] = 8;
                buildHuffmanTree(&litTree, ll, 288);

                uint8_t dl[32];
                for (int i = 0; i < 32; ++i) dl[i] = 5;
                buildHuffmanTree(&distTree, dl, 32);
            } else {
                // Dynamic Huffman codes
                int hlit = static_cast<int>(bs.readBits(5)) + 257;
                int hdist = static_cast<int>(bs.readBits(5)) + 1;
                int hclen = static_cast<int>(bs.readBits(4)) + 4;

                if (hlit > 286 || hdist > 30) return false;

                uint8_t clLengths[19] = {0};
                for (int i = 0; i < hclen; ++i) {
                    clLengths[CL_ORDER[i]] = static_cast<uint8_t>(bs.readBits(3));
                }

                HuffmanTree codeTree;
                buildHuffmanTree(&codeTree, clLengths, 19);

                uint8_t codeLengths[320] = {0};
                int totalCodes = hlit + hdist;
                int idx = 0;

                while (idx < totalCodes) {
                    int sym = decodeSymbol(bs, &codeTree);
                    if (sym < 0) return false;

                    if (sym < 16) {
                        codeLengths[idx++] = static_cast<uint8_t>(sym);
                    } else if (sym == 16) {
                        int rep = static_cast<int>(bs.readBits(2)) + 3;
                        uint8_t prev = (idx > 0) ? codeLengths[idx - 1] : 0;
                        while (rep-- > 0 && idx < totalCodes) codeLengths[idx++] = prev;
                    } else if (sym == 17) {
                        int rep = static_cast<int>(bs.readBits(3)) + 3;
                        while (rep-- > 0 && idx < totalCodes) codeLengths[idx++] = 0;
                    } else if (sym == 18) {
                        int rep = static_cast<int>(bs.readBits(7)) + 11;
                        while (rep-- > 0 && idx < totalCodes) codeLengths[idx++] = 0;
                    } else {
                        return false;
                    }
                }

                buildHuffmanTree(&litTree, codeLengths, hlit);
                buildHuffmanTree(&distTree, codeLengths + hlit, hdist);
            }

            // Decode symbols in block
            while (true) {
                int sym = decodeSymbol(bs, &litTree);
                if (sym < 0 || sym > 285) return false;

                if (sym < 256) {
                    if (dest.size() >= MAX_UNCOMPRESSED_SIZE) return false;
                    dest.push_back(static_cast<uint8_t>(sym));
                } else if (sym == 256) {
                    break; // End of block marker
                } else {
                    int lenIdx = sym - 257;
                    int length = LEN_BASE[lenIdx] + static_cast<int>(bs.readBits(LEN_EXTRA[lenIdx]));

                    int distSym = decodeSymbol(bs, &distTree);
                    if (distSym < 0 || distSym > 29) return false;

                    int distance = DIST_BASE[distSym] + static_cast<int>(bs.readBits(DIST_EXTRA[distSym]));
                    if (distance <= 0 || static_cast<size_t>(distance) > dest.size()) {
                        return false;
                    }

                    if (dest.size() + length > MAX_UNCOMPRESSED_SIZE) return false;

                    size_t startPos = dest.size() - distance;
                    for (int k = 0; k < length; ++k) {
                        dest.push_back(dest[startPos + k]);
                    }
                }
            }
        } else {
            // btype == 3: reserved error
            return false;
        }
    }

    return true;
}

std::string DeflateHelper::decompressString(const std::string& compressedData) {
    if (compressedData.empty()) return "";
    std::vector<uint8_t> out;
    if (decompress(reinterpret_cast<const uint8_t*>(compressedData.data()),
                   compressedData.size(), out)) {
        return std::string(reinterpret_cast<const char*>(out.data()), out.size());
    }
    return "";
}
