#pragma once

#include <string>
#include <vector>
#include <memory>

class PDFProcessor {
public:
    PDFProcessor();
    ~PDFProcessor();

    bool loadPDF(const std::string& filepath);
    bool extractVectors();
    bool extractText();
    bool extractImages();
    
    struct VectorElement {
        enum class Type {
            LINE,
            CURVE,
            CIRCLE,
            RECTANGLE
        };
        Type type;
        std::vector<double> points;
        double thickness;
    };

    const std::vector<VectorElement>& getVectors() const { return getVectorElements(); }
    const std::vector<std::string>& getText() const { return getTextElements(); }

    const std::vector<VectorElement>& getVectorElements() const;
    const std::vector<std::string>& getTextElements() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
}; 