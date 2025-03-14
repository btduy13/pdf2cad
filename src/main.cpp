#define _CRT_SECURE_NO_WARNINGS
#include "pdf_processor.hpp"
#include "cad_generator.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <cstdio>
#include <cstdarg>
#include <cerrno>
#include <cstring>
#include <direct.h>
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

void log(const char* format, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    
    // Format the message with timestamp
    SYSTEMTIME st;
    GetLocalTime(&st);
    int prefixLen = snprintf(buffer, sizeof(buffer), 
        "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    
    vsnprintf(buffer + prefixLen, sizeof(buffer) - prefixLen, format, args);
    va_end(args);
    
    // Add newline
    strcat(buffer, "\n");
    
    // Write to stdout
    printf("%s", buffer);
    fflush(stdout);
    
    // Write to Windows debug output
    OutputDebugStringA(buffer);
}

bool has_suffix(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void printUsage() {
    log("Usage: pdf2cad <input.pdf> <output.dxf/dwg>");
}

int main(int argc, char* argv[]) {
    int result = 1;  // Default to error
    try {
        log("pdf2cad starting...");
        
        // Check arguments
        if (argc != 3) {
            log("Error: Invalid number of arguments");
            printUsage();
            goto cleanup;
        }
        
        // Get current directory
        char currentDir[MAX_PATH];
        if (_getcwd(currentDir, sizeof(currentDir)) == nullptr) {
            log("Error: Failed to get current directory: %s", strerror(errno));
            goto cleanup;
        }
        
        // Log current directory and files
        log("Current directory: %s", currentDir);
        log("Input file: %s", argv[1]);
        log("Output file: %s", argv[2]);

        // Check if input file exists
        DWORD fileAttrs = GetFileAttributesA(argv[1]);
        if (fileAttrs == INVALID_FILE_ATTRIBUTES) {
            log("Error: Input file does not exist or cannot be opened: %s (Error: %lu)", argv[1], GetLastError());
            goto cleanup;
        }
        log("Input file exists and is readable");

        std::string inputPath = argv[1];
        std::string outputPath = argv[2];

        // Create processor and generator instances
        log("Creating processor and generator instances...");
        PDFProcessor pdfProcessor;
        CADGenerator cadGenerator;

        // Load and process PDF
        log("Loading PDF file: %s", inputPath.c_str());
        if (!pdfProcessor.loadPDF(inputPath)) {
            log("Failed to load PDF file: %s", inputPath.c_str());
            goto cleanup;
        }
        log("PDF loaded successfully");

        // Extract elements
        log("Extracting vectors from PDF...");
        if (!pdfProcessor.extractVectors()) {
            log("Failed to extract vector elements");
            goto cleanup;
        }
        log("Vector extraction completed");

        log("Extracting text from PDF...");
        if (!pdfProcessor.extractText()) {
            log("Failed to extract text elements");
            goto cleanup;
        }
        log("Text extraction completed");

        // Check output format
        log("Checking output format...");
        bool isDXF = has_suffix(outputPath, ".dxf");
        bool isDWG = has_suffix(outputPath, ".dwg");
        
        if (!isDXF && !isDWG) {
            log("Error: Unsupported output format. Only .dxf and .dwg are supported");
            goto cleanup;
        }
        log("Output format is valid: %s", isDXF ? "DXF" : "DWG");

        // Generate CAD file
        log("Generating CAD file: %s", outputPath.c_str());
        if (!cadGenerator.generateCAD(pdfProcessor.getVectors(), pdfProcessor.getText(), outputPath)) {
            log("Failed to generate CAD file");
            goto cleanup;
        }
        log("CAD file generated successfully");

        log("Conversion completed successfully");
        result = 0;  // Success
    }
    catch (const std::exception& e) {
        log("Error: Unhandled exception: %s", e.what());
    }
    catch (...) {
        log("Error: Unknown exception occurred");
    }

cleanup:
    return result;
} 