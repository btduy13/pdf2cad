#include "pdf_processor.hpp"
#include "poppler-document.h"
#include "poppler-page.h"
#include "poppler-page-renderer.h"

// Forward declaration of the log function
extern void log(const char* format, ...);

class PDFProcessor::Impl {
public:
    std::unique_ptr<poppler::document> doc;
    std::vector<VectorElement> vectorElements;
    std::vector<std::string> textElements;
};

PDFProcessor::PDFProcessor() : pimpl(std::make_unique<Impl>()) {
    log("PDFProcessor instance created");
}

PDFProcessor::~PDFProcessor() = default;

bool PDFProcessor::loadPDF(const std::string& filepath) {
    try {
        log("Attempting to load PDF: %s", filepath.c_str());

        // Check if file exists
        FILE* file = fopen(filepath.c_str(), "rb");
        if (!file) {
            log("Error: Cannot open file '%s': %s", filepath.c_str(), strerror(errno));
            return false;
        }
        fclose(file);

        // Try to load the PDF
        log("File exists, attempting to load with Poppler...");
        pimpl->doc.reset(poppler::document::load_from_file(filepath));
        
        if (!pimpl->doc) {
            log("Failed to load PDF document: Poppler returned null document");
            return false;
        }

        int pageCount = pimpl->doc->pages();
        log("Successfully loaded PDF with %d pages", pageCount);

        if (pageCount == 0) {
            log("Warning: PDF has no pages");
        }

        return true;
    } catch (const std::exception& e) {
        log("Exception while loading PDF: %s", e.what());
        return false;
    } catch (...) {
        log("Unknown exception while loading PDF");
        return false;
    }
}

bool PDFProcessor::extractVectors() {
    if (!pimpl->doc) {
        log("Cannot extract vectors: No PDF loaded");
        return false;
    }

    try {
        log("Starting vector extraction...");
        int pageCount = pimpl->doc->pages();
        log("Processing %d pages for vector elements", pageCount);

        for (int i = 0; i < pageCount; ++i) {
            log("Processing page %d for vectors...", i + 1);
            std::unique_ptr<poppler::page> page(pimpl->doc->create_page(i));
            
            if (!page) {
                log("Warning: Failed to create page %d", i + 1);
                continue;
            }

            // TODO: Implement actual vector extraction
            // For now, just adding a sample vector element
            VectorElement line;
            line.type = VectorElement::Type::LINE;
            line.points = {0.0, 0.0, 100.0, 100.0}; // Sample line from (0,0) to (100,100)
            line.thickness = 1.0;
            pimpl->vectorElements.push_back(line);
            
            log("Added sample vector element for page %d", i + 1);
        }
        
        log("Vector extraction complete. Found %zu vector elements", pimpl->vectorElements.size());
        return true;
    } catch (const std::exception& e) {
        log("Exception while extracting vectors: %s", e.what());
        return false;
    } catch (...) {
        log("Unknown exception while extracting vectors");
        return false;
    }
}

bool PDFProcessor::extractText() {
    if (!pimpl->doc) {
        log("Cannot extract text: No PDF loaded");
        return false;
    }

    try {
        log("Starting text extraction...");
        int pageCount = pimpl->doc->pages();
        log("Processing %d pages for text", pageCount);

        for (int i = 0; i < pageCount; ++i) {
            log("Processing page %d for text...", i + 1);
            std::unique_ptr<poppler::page> page(pimpl->doc->create_page(i));
            
            if (!page) {
                log("Warning: Failed to create page %d", i + 1);
                continue;
            }

            // Get text as a byte array
            log("Extracting text from page %d...", i + 1);
            poppler::byte_array text_data = page->text().to_utf8();
            
            // Convert byte array to string
            if (text_data.size() > 0) {
                std::string text(reinterpret_cast<const char*>(text_data.data()), text_data.size());
                if (!text.empty()) {
                    log("Found text on page %d (%zu bytes): %s...", 
                        i + 1, text.length(), 
                        text.substr(0, std::min(size_t(50), text.length())).c_str());
                    pimpl->textElements.push_back(text);
                } else {
                    log("No text found on page %d (empty string)", i + 1);
                }
            } else {
                log("No text data on page %d (zero bytes)", i + 1);
            }
        }

        log("Text extraction complete. Found %zu text blocks", pimpl->textElements.size());
        return true;
    } catch (const std::exception& e) {
        log("Exception while extracting text: %s", e.what());
        return false;
    } catch (...) {
        log("Unknown exception while extracting text");
        return false;
    }
}

bool PDFProcessor::extractImages() {
    if (!pimpl->doc) {
        log("Cannot extract images: No PDF loaded");
        return false;
    }

    log("Image extraction not implemented yet");
    return true;
}

const std::vector<PDFProcessor::VectorElement>& PDFProcessor::getVectorElements() const {
    return pimpl->vectorElements;
}

const std::vector<std::string>& PDFProcessor::getTextElements() const {
    return pimpl->textElements;
} 