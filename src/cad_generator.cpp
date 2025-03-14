#include "cad_generator.hpp"
#include <fstream>

// Forward declaration of the log function
extern void log(const char* format, ...);

class CADGenerator::Impl {
public:
    std::vector<PDFProcessor::VectorElement> vectors;
    std::vector<std::string> texts;

    bool writeDXF(const std::string& outputPath) {
        log("Attempting to write DXF file: %s", outputPath.c_str());
        std::ofstream file(outputPath);
        if (!file) {
            log("Failed to open output file for writing");
            return false;
        }

        log("Writing DXF header...");
        // Write DXF header
        file << "0\nSECTION\n";
        file << "2\nHEADER\n";
        file << "0\nENDSEC\n";
        
        log("Writing entities section...");
        // Write entities section
        file << "0\nSECTION\n";
        file << "2\nENTITIES\n";

        // Write vector elements
        log("Writing %d vector elements...", vectors.size());
        for (const auto& vec : vectors) {
            if (vec.type == PDFProcessor::VectorElement::Type::LINE) {
                file << "0\nLINE\n";
                file << "8\n0\n"; // Layer
                file << "10\n" << vec.points[0] << "\n"; // Start X
                file << "20\n" << vec.points[1] << "\n"; // Start Y
                file << "11\n" << vec.points[2] << "\n"; // End X
                file << "21\n" << vec.points[3] << "\n"; // End Y
                log("  Added line from (%.2f,%.2f) to (%.2f,%.2f)", 
                    vec.points[0], vec.points[1], vec.points[2], vec.points[3]);
            }
            // Add other vector types here
        }

        // Write text elements
        log("Writing %d text elements...", texts.size());
        for (const auto& text : texts) {
            file << "0\nTEXT\n";
            file << "8\n0\n"; // Layer
            file << "10\n0\n"; // Position X
            file << "20\n0\n"; // Position Y
            file << "1\n" << text << "\n"; // Text content
            log("  Added text: %s", text.c_str());
        }

        // Write footer
        file << "0\nENDSEC\n";
        file << "0\nEOF\n";

        log("DXF file written successfully");
        return true;
    }

    bool writeDWG(const std::string& outputPath) {
        log("DWG format not yet implemented");
        return false;
    }
};

CADGenerator::CADGenerator() : pimpl(std::make_unique<Impl>()) {}
CADGenerator::~CADGenerator() = default;

bool CADGenerator::generateCAD(const std::vector<PDFProcessor::VectorElement>& vectors,
                             const std::vector<std::string>& texts,
                             const std::string& outputPath) {
    log("Generating CAD file with %d vectors and %d text elements", vectors.size(), texts.size());
    
    // Store the elements
    pimpl->vectors = vectors;
    pimpl->texts = texts;

    // For now, we only support DXF format
    return pimpl->writeDXF(outputPath);
}

bool CADGenerator::setVectorElements(const std::vector<PDFProcessor::VectorElement>& elements) {
    log("Setting %d vector elements", elements.size());
    pimpl->vectors = elements;
    return true;
}

bool CADGenerator::setTextElements(const std::vector<std::string>& texts) {
    log("Setting %d text elements", texts.size());
    pimpl->texts = texts;
    return true;
}

bool CADGenerator::generateCAD(const std::string& outputPath, Format format) {
    log("Generating CAD file in %s format", format == Format::DXF ? "DXF" : "DWG");
    switch (format) {
        case Format::DXF:
            return pimpl->writeDXF(outputPath);
        case Format::DWG:
            return pimpl->writeDWG(outputPath);
        default:
            log("Unknown CAD format");
            return false;
    }
} 