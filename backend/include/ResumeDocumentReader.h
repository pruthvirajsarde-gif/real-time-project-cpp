#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

// ============================================================
// ResumeDocumentReader — Extracts raw plain text from supported
// document formats (.pdf, .txt, .docx, .doc, .rtf, .md) with
// advanced PDF FlateDecode decompression, ToUnicode CMap decoding,
// DOCX XML extraction, and text normalization.
// ============================================================
class ResumeDocumentReader {
public:
    // Check whether the file has a supported format (.pdf, .txt, .docx, .doc, .rtf, .md)
    static bool isSupportedFile(const std::string& filePath);

    // Returns the file extension in lowercase (e.g., ".txt", ".pdf", ".docx")
    static std::string getFileExtension(const std::string& filePath);

    // Checks whether the file exists on the filesystem
    static bool fileExists(const std::string& filePath);

    // Universal text extractor from any supported resume file
    static std::string extractText(const std::string& filePath);

    // Normalizes extracted text: fixes line endings, cleans control chars,
    // normalizes unicode homoglyphs, trims excessive whitespace
    static std::string normalizeText(const std::string& rawText);

    // Dedicated reader for plain text, markdown, or RTF files
    static std::string extractTextFromTXT(const std::string& filePath);

    // Advanced extractor for PDF documents (handles FlateDecode, ToUnicode CMaps, metadata)
    static std::string extractTextFromPDF(const std::string& filePath);

    // Dedicated extractor for Microsoft Word .docx files (ZIP archive + word/document.xml)
    static std::string extractTextFromDOCX(const std::string& filePath);

    // Fallback scanner: extracts printable ASCII/UTF-8 character clusters from arbitrary files
    static std::string extractPrintableStrings(const std::string& filePath);

private:
    // Decodes PDF literal string with escape sequences
    static std::string decodePdfLiteralString(const std::string& str);

    // Decodes UTF-16BE hex string (e.g., "0041" -> "A")
    static std::string hexToUtf8(const std::string& hex);

    // Parses a ToUnicode CMap stream into a character lookup map (srcHex -> utf8String)
    static std::map<std::string, std::string> parseToUnicodeCMap(const std::string& cmapText);
};
