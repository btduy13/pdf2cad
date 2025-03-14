#pragma once

#include "pdf_processor.hpp"
#include <string>

class CADGenerator {
public:
    CADGenerator();
    ~CADGenerator();

    enum class Format {
        DXF,
        DWG
    };

    // New method that combines setting elements and generating CAD file
    bool generateCAD(const std::vector<PDFProcessor::VectorElement>& vectors,
                    const std::vector<std::string>& texts,
                    const std::string& outputPath);

    // Original methods kept for backward compatibility
    bool setVectorElements(const std::vector<PDFProcessor::VectorElement>& elements);
    bool setTextElements(const std::vector<std::string>& texts);
    bool generateCAD(const std::string& outputPath, Format format);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
}; 