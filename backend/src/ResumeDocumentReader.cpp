#include "ResumeDocumentReader.h"
#include "DeflateHelper.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <regex>
#include <iomanip>

bool ResumeDocumentReader::fileExists(const std::string& filePath) {
    try {
        return std::filesystem::exists(filePath) && !std::filesystem::is_directory(filePath);
    } catch (...) {
        return false;
    }
}

std::string ResumeDocumentReader::getFileExtension(const std::string& filePath) {
    size_t lastDot = filePath.find_last_of('.');
    size_t lastSlash = filePath.find_last_of("/\\");

    if (lastDot == std::string::npos || (lastSlash != std::string::npos && lastDot < lastSlash)) {
        return "";
    }

    std::string ext = filePath.substr(lastDot);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return ext;
}

bool ResumeDocumentReader::isSupportedFile(const std::string& filePath) {
    std::string ext = getFileExtension(filePath);
    return (ext == ".txt" || ext == ".pdf");
}

std::string ResumeDocumentReader::normalizeText(const std::string& rawText) {
    std::string result;
    result.reserve(rawText.size());

    for (size_t i = 0; i < rawText.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(rawText[i]);
        if (c == '\r') {
            if (i + 1 < rawText.size() && rawText[i + 1] == '\n') {
                continue;
            }
            result.push_back('\n');
        } else if (c == '\t') {
            result.push_back(' ');
        } else if (c == '\n') {
            result.push_back('\n');
        } else if (c >= 32 && c <= 126) {
            result.push_back(static_cast<char>(c));
        } else if (c >= 128) {
            // Keep valid multi-byte UTF-8 or convert bullets
            if (c == 0xE2 && i + 2 < rawText.size() &&
                static_cast<unsigned char>(rawText[i + 1]) == 0x80 &&
                static_cast<unsigned char>(rawText[i + 2]) == 0xA2) { // UTF-8 bullet
                result.push_back('*');
                result.push_back(' ');
                i += 2;
            } else if (c == 0xC2 && i + 1 < rawText.size() &&
                       static_cast<unsigned char>(rawText[i + 1]) == 0xB7) { // middle dot
                result.push_back('*');
                result.push_back(' ');
                i += 1;
            } else {
                result.push_back(static_cast<char>(c));
            }
        }
    }

    // Collapse multiple consecutive blank lines (> 2 newlines -> 2 newlines)
    std::string cleaned;
    cleaned.reserve(result.size());
    int consecutiveNewlines = 0;

    for (char c : result) {
        if (c == '\n') {
            ++consecutiveNewlines;
            if (consecutiveNewlines <= 2) {
                cleaned.push_back(c);
            }
        } else {
            consecutiveNewlines = 0;
            cleaned.push_back(c);
        }
    }

    return cleaned;
}

std::string ResumeDocumentReader::extractTextFromTXT(const std::string& filePath) {
    if (!fileExists(filePath)) {
        return "";
    }

    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string raw = buffer.str();
    if (raw.empty()) {
        return "";
    }

    return normalizeText(raw);
}

std::string ResumeDocumentReader::decodePdfLiteralString(const std::string& str) {
    std::string out;
    out.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\\' && i + 1 < str.size()) {
            char next = str[i + 1];
            switch (next) {
                case 'n': out.push_back('\n'); ++i; break;
                case 'r': out.push_back('\r'); ++i; break;
                case 't': out.push_back('\t'); ++i; break;
                case 'b': out.push_back('\b'); ++i; break;
                case 'f': out.push_back('\f'); ++i; break;
                case '(': out.push_back('('); ++i; break;
                case ')': out.push_back(')'); ++i; break;
                case '\\': out.push_back('\\'); ++i; break;
                default:
                    if (next >= '0' && next <= '7') {
                        int oct = next - '0';
                        size_t j = i + 2;
                        while (j < str.size() && j <= i + 3 && str[j] >= '0' && str[j] <= '7') {
                            oct = oct * 8 + (str[j] - '0');
                            ++j;
                        }
                        out.push_back(static_cast<char>(oct));
                        i = j - 1;
                    } else {
                        out.push_back(next);
                        ++i;
                    }
                    break;
            }
        } else {
            out.push_back(str[i]);
        }
    }
    return out;
}

std::string ResumeDocumentReader::hexToUtf8(const std::string& hex) {
    std::string s;
    for (size_t i = 0; i < hex.size(); i += 4) {
        if (i + 4 <= hex.size()) {
            try {
                int code = std::stoi(hex.substr(i, 4), nullptr, 16);
                if (code > 0 && code < 128) {
                    s.push_back(static_cast<char>(code));
                } else if (code >= 128 && code < 0x800) {
                    s.push_back(static_cast<char>(0xC0 | (code >> 6)));
                    s.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                } else if (code >= 0x800) {
                    s.push_back(static_cast<char>(0xE0 | (code >> 12)));
                    s.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
                    s.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                }
            } catch (...) {}
        }
    }
    return s;
}

std::map<std::string, std::string> ResumeDocumentReader::parseToUnicodeCMap(const std::string& cmapText) {
    std::map<std::string, std::string> cMap;

    auto toLowerHex = [](std::string h) {
        std::transform(h.begin(), h.end(), h.begin(), [](unsigned char c) { return std::tolower(c); });
        while (h.size() < 4) h = "0" + h;
        return h;
    };

    // 1. Parse beginbfchar ... endbfchar blocks
    size_t pos = 0;
    while ((pos = cmapText.find("beginbfchar", pos)) != std::string::npos) {
        size_t endBlock = cmapText.find("endbfchar", pos);
        if (endBlock == std::string::npos) break;
        std::string block = cmapText.substr(pos, endBlock - pos);
        pos = endBlock + 9;

        std::regex pairRegex("<([0-9a-fA-F]+)>\\s+<([0-9a-fA-F]+)>");
        std::sregex_iterator it(block.begin(), block.end(), pairRegex);
        std::sregex_iterator endIt;
        for (; it != endIt; ++it) {
            std::string src = toLowerHex((*it)[1].str());
            std::string dst = (*it)[2].str();
            cMap[src] = hexToUtf8(dst);
        }
    }

    // 2. Parse beginbfrange ... endbfrange blocks
    pos = 0;
    while ((pos = cmapText.find("beginbfrange", pos)) != std::string::npos) {
        size_t endBlock = cmapText.find("endbfrange", pos);
        if (endBlock == std::string::npos) break;
        std::string block = cmapText.substr(pos, endBlock - pos);
        pos = endBlock + 10;

        // Array form: <s1> <s2> [ <dst1> <dst2> ... ]
        std::regex arrRegex("<([0-9a-fA-F]+)>\\s+<([0-9a-fA-F]+)>\\s*\\[([^\\]]+)\\]");
        std::sregex_iterator it(block.begin(), block.end(), arrRegex);
        std::sregex_iterator endIt;
        for (; it != endIt; ++it) {
            try {
                int s1 = std::stoi((*it)[1].str(), nullptr, 16);
                int s2 = std::stoi((*it)[2].str(), nullptr, 16);
                std::string targetBlock = (*it)[3].str();
                std::regex tRegex("<([0-9a-fA-F]+)>");
                std::sregex_iterator tit(targetBlock.begin(), targetBlock.end(), tRegex);
                std::vector<std::string> targets;
                for (; tit != endIt; ++tit) {
                    targets.push_back((*tit)[1].str());
                }
                for (int c = s1; c <= s2; ++c) {
                    size_t idx = static_cast<size_t>(c - s1);
                    if (idx < targets.size()) {
                        std::stringstream ss;
                        ss << std::hex << std::setw(4) << std::setfill('0') << c;
                        cMap[toLowerHex(ss.str())] = hexToUtf8(targets[idx]);
                    }
                }
            } catch (...) {}
        }

        // Linear form: <s1> <s2> <dstStart>
        std::string cleanBlock = std::regex_replace(block, std::regex("\\[[^\\]]+\\]"), " ");
        std::regex linRegex("<([0-9a-fA-F]+)>\\s+<([0-9a-fA-F]+)>\\s+<([0-9a-fA-F]+)>");
        std::sregex_iterator lit(cleanBlock.begin(), cleanBlock.end(), linRegex);
        for (; lit != endIt; ++lit) {
            try {
                int s1 = std::stoi((*lit)[1].str(), nullptr, 16);
                int s2 = std::stoi((*lit)[2].str(), nullptr, 16);
                int dstBase = std::stoi((*lit)[3].str(), nullptr, 16);
                for (int c = s1; c <= s2; ++c) {
                    int target = dstBase + (c - s1);
                    std::stringstream ssSrc, ssDst;
                    ssSrc << std::hex << std::setw(4) << std::setfill('0') << c;
                    ssDst << std::hex << std::setw(4) << std::setfill('0') << target;
                    cMap[toLowerHex(ssSrc.str())] = hexToUtf8(ssDst.str());
                }
            } catch (...) {}
        }
    }

    return cMap;
}

std::string ResumeDocumentReader::extractTextFromPDF(const std::string& filePath) {
    if (!fileExists(filePath)) {
        return "";
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
    file.close();

    if (buffer.size() < 10) {
        return "";
    }

    std::string pdfData(reinterpret_cast<const char*>(buffer.data()), buffer.size());

    // Validate PDF header
    if (pdfData.rfind("%PDF-", 0) != 0 && pdfData.find("%PDF-") == std::string::npos) {
        return "";
    }

    // 1. Scan and parse all ToUnicode CMaps and Content Streams
    std::map<int, std::map<std::string, std::string>> cmapObjToMap;
    std::map<std::string, std::map<std::string, std::string>> fontNameToCMap;
    std::map<int, std::string> objectStreams; // objNum -> decompressed content

    // Locate all objects: "<num> 0 obj"
    std::regex objRegex("(\\d+)\\s+0\\s+obj");
    auto it = std::sregex_iterator(pdfData.begin(), pdfData.end(), objRegex);
    auto endIt = std::sregex_iterator();

    for (; it != endIt; ++it) {
        int objNum = 0;
        try {
            objNum = std::stoi((*it)[1].str());
        } catch (...) {
            continue;
        }

        size_t objStart = it->position();
        size_t endObj = pdfData.find("endobj", objStart);
        if (endObj == std::string::npos) endObj = pdfData.size();

        size_t streamPos = pdfData.find("stream", objStart);
        if (streamPos != std::string::npos && streamPos < endObj) {
            size_t dataStart = streamPos + 6;
            if (dataStart < buffer.size() && buffer[dataStart] == '\r') ++dataStart;
            if (dataStart < buffer.size() && buffer[dataStart] == '\n') ++dataStart;

            size_t endStream = pdfData.find("endstream", dataStart);
            if (endStream != std::string::npos && endStream >= dataStart) {
                const uint8_t* rawData = buffer.data() + dataStart;
                size_t rawLen = endStream - dataStart;

                std::vector<uint8_t> decomp;
                bool inflated = DeflateHelper::decompress(rawData, rawLen, decomp);
                std::string streamContent;
                if (inflated && !decomp.empty()) {
                    streamContent.assign(reinterpret_cast<const char*>(decomp.data()), decomp.size());
                } else {
                    streamContent.assign(reinterpret_cast<const char*>(rawData), rawLen);
                }

                objectStreams[objNum] = streamContent;

                // Check if this stream is a CMap
                if (streamContent.find("beginbfchar") != std::string::npos ||
                    streamContent.find("beginbfrange") != std::string::npos) {
                    cmapObjToMap[objNum] = parseToUnicodeCMap(streamContent);
                }
            }
        }
    }

    // 2. Map Font Objects to their ToUnicode CMap
    std::regex fontToUnicodeRegex("(\\d+)\\s+0\\s+obj[\\s\\S]*?/ToUnicode\\s+(\\d+)\\s+0\\s+R");
    auto ftIt = std::sregex_iterator(pdfData.begin(), pdfData.end(), fontToUnicodeRegex);
    std::map<int, int> fontObjToCmapObj;
    for (; ftIt != endIt; ++ftIt) {
        try {
            int fObj = std::stoi((*ftIt)[1].str());
            int cObj = std::stoi((*ftIt)[2].str());
            fontObjToCmapObj[fObj] = cObj;
        } catch (...) {}
    }

    // Map resource names (/F1, /F2, etc.) to font objects
    std::regex resRegex("/Font\\s*<<([^>]*)>>");
    auto resIt = std::sregex_iterator(pdfData.begin(), pdfData.end(), resRegex);
    for (; resIt != endIt; ++resIt) {
        std::string fontDict = (*resIt)[1].str();
        std::regex feRegex("/([A-Za-z0-9]+)\\s+(\\d+)\\s+0\\s+R");
        auto feIt = std::sregex_iterator(fontDict.begin(), fontDict.end(), feRegex);
        for (; feIt != endIt; ++feIt) {
            std::string fontName = (*feIt)[1].str();
            try {
                int fObj = std::stoi((*feIt)[2].str());
                if (fontObjToCmapObj.count(fObj)) {
                    int cObj = fontObjToCmapObj[fObj];
                    if (cmapObjToMap.count(cObj)) {
                        fontNameToCMap[fontName] = cmapObjToMap[cObj];
                    }
                }
            } catch (...) {}
        }
    }

    // 3. Extract Text from Content Streams
    std::string extractedText;

    // Helper lambda to decode text tokens in a content stream
    auto decodeContentStream = [&](const std::string& streamText) {
        std::string currentFont = "F1";
        size_t pos = 0;

        while ((pos = streamText.find("BT", pos)) != std::string::npos) {
            size_t blockStart = pos + 2;
            size_t blockEnd = streamText.find("ET", blockStart);
            if (blockEnd == std::string::npos) break;

            std::string block = streamText.substr(blockStart, blockEnd - blockStart);
            pos = blockEnd + 2;

            // Scan through operators in text object
            size_t i = 0;
            while (i < block.size()) {
                // Font change: /F... Tf
                if (block[i] == '/' && i + 2 < block.size()) {
                    size_t fEnd = block.find_first_of(" \t\r\n", i);
                    if (fEnd != std::string::npos) {
                        std::string potentialFont = block.substr(i + 1, fEnd - (i + 1));
                        size_t tfPos = block.find("Tf", fEnd);
                        if (tfPos != std::string::npos && tfPos - fEnd < 15) {
                            currentFont = potentialFont;
                            i = tfPos + 2;
                            continue;
                        }
                    }
                }

                // Check for TJ array: [ ... ] TJ
                if (block[i] == '[') {
                    size_t arrEnd = block.find(']', i);
                    if (arrEnd != std::string::npos) {
                        std::string arrContent = block.substr(i + 1, arrEnd - (i + 1));
                        const auto& curMap = fontNameToCMap[currentFont];

                        size_t k = 0;
                        while (k < arrContent.size()) {
                            if (arrContent[k] == '<') {
                                size_t hexEnd = arrContent.find('>', k);
                                if (hexEnd != std::string::npos) {
                                    std::string hexStr = arrContent.substr(k + 1, hexEnd - (k + 1));
                                    for (size_t h = 0; h < hexStr.size(); h += 4) {
                                        if (h + 4 <= hexStr.size()) {
                                            std::string code = hexStr.substr(h, 4);
                                            std::transform(code.begin(), code.end(), code.begin(),
                                                           [](unsigned char c) { return std::tolower(c); });
                                            if (curMap.count(code)) {
                                                extractedText += curMap.at(code);
                                            } else {
                                                // Fallback: check across all CMaps or hexToUtf8
                                                bool foundAny = false;
                                                for (const auto& pair : cmapObjToMap) {
                                                    if (pair.second.count(code)) {
                                                        extractedText += pair.second.at(code);
                                                        foundAny = true;
                                                        break;
                                                    }
                                                }
                                                if (!foundAny) {
                                                    extractedText += hexToUtf8(code);
                                                }
                                            }
                                        }
                                    }
                                    k = hexEnd + 1;
                                    continue;
                                }
                            } else if (arrContent[k] == '(') {
                                size_t strEnd = k + 1;
                                int depth = 1;
                                while (strEnd < arrContent.size() && depth > 0) {
                                    if (arrContent[strEnd] == '\\') { strEnd += 2; continue; }
                                    if (arrContent[strEnd] == '(') ++depth;
                                    else if (arrContent[strEnd] == ')') --depth;
                                    if (depth > 0) ++strEnd;
                                }
                                if (depth == 0) {
                                    std::string lit = arrContent.substr(k + 1, strEnd - (k + 1));
                                    extractedText += decodePdfLiteralString(lit);
                                    k = strEnd + 1;
                                    continue;
                                }
                            }
                            ++k;
                        }
                        extractedText += " ";
                        i = arrEnd + 1;
                        continue;
                    }
                }

                // Check for literal string: (string) Tj or (string) '
                if (block[i] == '(') {
                    size_t strEnd = i + 1;
                    int depth = 1;
                    while (strEnd < block.size() && depth > 0) {
                        if (block[strEnd] == '\\') { strEnd += 2; continue; }
                        if (block[strEnd] == '(') ++depth;
                        else if (block[strEnd] == ')') --depth;
                        if (depth > 0) ++strEnd;
                    }

                    if (depth == 0) {
                        std::string lit = block.substr(i + 1, strEnd - (i + 1));
                        extractedText += decodePdfLiteralString(lit);
                        extractedText += " ";
                        i = strEnd + 1;
                        continue;
                    }
                }

                // Check for hex string: <hex> Tj
                if (block[i] == '<') {
                    size_t hexEnd = block.find('>', i);
                    if (hexEnd != std::string::npos) {
                        std::string hexStr = block.substr(i + 1, hexEnd - (i + 1));
                        const auto& curMap = fontNameToCMap[currentFont];
                        for (size_t h = 0; h < hexStr.size(); h += 4) {
                            if (h + 4 <= hexStr.size()) {
                                std::string code = hexStr.substr(h, 4);
                                std::transform(code.begin(), code.end(), code.begin(),
                                               [](unsigned char c) { return std::tolower(c); });
                                if (curMap.count(code)) {
                                    extractedText += curMap.at(code);
                                } else {
                                    extractedText += hexToUtf8(code);
                                }
                            }
                        }
                        extractedText += " ";
                        i = hexEnd + 1;
                        continue;
                    }
                }

                // Check line break operators: T*, ', "
                if (block.substr(i, 2) == "T*" || block[i] == '\'' || block[i] == '"') {
                    extractedText += "\n";
                }

                ++i;
            }
            extractedText += "\n";
        }
    };

    // First process content streams referenced by /Contents
    std::regex contentsRegex("/Contents\\s+(\\d+)\\s+0\\s+R");
    auto cIt = std::sregex_iterator(pdfData.begin(), pdfData.end(), contentsRegex);
    for (; cIt != endIt; ++cIt) {
        try {
            int cObj = std::stoi((*cIt)[1].str());
            if (objectStreams.count(cObj)) {
                decodeContentStream(objectStreams[cObj]);
            }
        } catch (...) {}
    }

    // Also check contents array: /Contents [ 4 0 R 29 0 R ... ]
    std::regex contentsArrRegex("/Contents\\s*\\[([^\\]]+)\\]");
    auto caIt = std::sregex_iterator(pdfData.begin(), pdfData.end(), contentsArrRegex);
    for (; caIt != endIt; ++caIt) {
        std::string inner = (*caIt)[1].str();
        std::regex idRegex("(\\d+)\\s+0\\s+R");
        auto idIt = std::sregex_iterator(inner.begin(), inner.end(), idRegex);
        for (; idIt != endIt; ++idIt) {
            try {
                int cObj = std::stoi((*idIt)[1].str());
                if (objectStreams.count(cObj)) {
                    decodeContentStream(objectStreams[cObj]);
                }
            } catch (...) {}
        }
    }

    // If still empty or very short, decode ALL streams containing "BT"
    if (extractedText.size() < 50) {
        for (const auto& pair : objectStreams) {
            if (pair.second.find("BT") != std::string::npos) {
                decodeContentStream(pair.second);
            }
        }
    }

    // Also check for uncompressed text in raw PDF data (older PDFs)
    if (extractedText.size() < 50 && pdfData.find("BT") != std::string::npos) {
        decodeContentStream(pdfData);
    }

    // Extract PDF Author / Metadata if found (useful for candidate name fallback)
    std::regex creatorRegex("<dc:creator><rdf:Seq><rdf:li>([^<]+)</rdf:li></rdf:Seq></dc:creator>");
    std::smatch creatorMatch;
    if (std::regex_search(pdfData, creatorMatch, creatorRegex)) {
        std::string creator = creatorMatch[1].str();
        if (!creator.empty() && extractedText.find(creator) == std::string::npos) {
            extractedText = "Name: " + creator + "\n" + extractedText;
        }
    }

    // If text is still completely empty, fallback to printable strings
    if (extractedText.empty()) {
        extractedText = extractPrintableStrings(filePath);
    }

    return normalizeText(extractedText);
}

std::string ResumeDocumentReader::extractTextFromDOCX(const std::string& filePath) {
    if (!fileExists(filePath)) {
        return "";
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
    file.close();

    if (buffer.size() < 30) {
        return "";
    }

    // Check ZIP magic: PK\x03\x04
    if (buffer[0] != 'P' || buffer[1] != 'K' || buffer[2] != 0x03 || buffer[3] != 0x04) {
        return extractPrintableStrings(filePath);
    }

    // Scan ZIP local file headers for "word/document.xml"
    size_t pos = 0;
    while (pos + 30 < buffer.size()) {
        if (buffer[pos] == 'P' && buffer[pos + 1] == 'K' &&
            buffer[pos + 2] == 0x03 && buffer[pos + 3] == 0x04) {

            uint16_t method = buffer[pos + 8] | (buffer[pos + 9] << 8);
            uint32_t compSize = buffer[pos + 18] | (buffer[pos + 19] << 8) |
                                (buffer[pos + 20] << 16) | (buffer[pos + 21] << 24);
            uint16_t nameLen = buffer[pos + 26] | (buffer[pos + 27] << 8);
            uint16_t extraLen = buffer[pos + 28] | (buffer[pos + 29] << 8);

            size_t namePos = pos + 30;
            if (namePos + nameLen <= buffer.size()) {
                std::string entryName(reinterpret_cast<const char*>(buffer.data() + namePos), nameLen);
                size_t dataPos = namePos + nameLen + extraLen;

                if (entryName == "word/document.xml" && dataPos + compSize <= buffer.size()) {
                    std::string xmlContent;
                    if (method == 8) { // DEFLATE
                        std::vector<uint8_t> decomp;
                        if (DeflateHelper::decompress(buffer.data() + dataPos, compSize, decomp)) {
                            xmlContent.assign(reinterpret_cast<const char*>(decomp.data()), decomp.size());
                        }
                    } else if (method == 0) { // Stored
                        xmlContent.assign(reinterpret_cast<const char*>(buffer.data() + dataPos), compSize);
                    }

                    if (!xmlContent.empty()) {
                        // Extract text from XML
                        std::string out;
                        out.reserve(xmlContent.size());
                        bool inTag = false;
                        std::string tag;

                        for (size_t k = 0; k < xmlContent.size(); ++k) {
                            char ch = xmlContent[k];
                            if (ch == '<') {
                                inTag = true;
                                tag.clear();
                            } else if (ch == '>') {
                                inTag = false;
                                if (tag == "/w:p" || tag == "w:p") {
                                    out.push_back('\n');
                                } else if (tag == "w:tab/") {
                                    out.push_back(' ');
                                }
                            } else {
                                if (inTag) {
                                    tag.push_back(ch);
                                } else {
                                    out.push_back(ch);
                                }
                            }
                        }
                        return normalizeText(out);
                    }
                }
            }
            pos += 30 + nameLen + extraLen + compSize;
        } else {
            ++pos;
        }
    }

    return extractPrintableStrings(filePath);
}

std::string ResumeDocumentReader::extractPrintableStrings(const std::string& filePath) {
    if (!fileExists(filePath)) {
        return "";
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::string result;
    std::string currentRun;
    char ch;

    while (file.get(ch)) {
        unsigned char uc = static_cast<unsigned char>(ch);
        if ((uc >= 32 && uc <= 126) || uc == '\t' || uc == '\n' || uc == '\r') {
            currentRun.push_back(ch);
        } else {
            if (currentRun.size() >= 4) {
                result += currentRun + "\n";
            }
            currentRun.clear();
        }
    }
    file.close();

    if (currentRun.size() >= 4) {
        result += currentRun + "\n";
    }

    return normalizeText(result);
}

std::string ResumeDocumentReader::extractText(const std::string& filePath) {
    if (!fileExists(filePath) || !isSupportedFile(filePath)) {
        return "";
    }

    std::string ext = getFileExtension(filePath);
    if (ext == ".txt") {
        return extractTextFromTXT(filePath);
    } else if (ext == ".pdf") {
        return extractTextFromPDF(filePath);
    }

    return "";
}
