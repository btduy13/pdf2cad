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

HANDLE hEventLog = NULL;

bool has_suffix(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void log(const char* format, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    
    // Format the message
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Write to stderr
    fprintf(stderr, "%s\n", buffer);
    fflush(stderr);
    
    // Write to Windows debug output
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");

    // Write to Windows Event Log
    if (hEventLog != NULL) {
        const char* strings[1] = { buffer };
        ReportEventA(hEventLog, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, strings, NULL);
    }
}

void printUsage() {
    log("Usage: pdf2cad <input.pdf> <output.dxf/dwg>");
}

int main(int argc, char* argv[]) {
    // Register event source
    hEventLog = RegisterEventSourceA(NULL, "PDF2CAD");
    if (hEventLog == NULL) {
        fprintf(stderr, "Failed to register event source: %lu\n", GetLastError());
        return 1;
    }

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
        FILE* testFile = fopen(argv[1], "rb");
        if (testFile == nullptr) {
            log("Error: Input file does not exist or cannot be opened: %s (%s)", argv[1], strerror(errno));
            goto cleanup;
        }
        fclose(testFile);
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
    // Deregister event source
    if (hEventLog != NULL) {
        DeregisterEventSource(hEventLog);
        hEventLog = NULL;
    }

    return result;
} 