#include "pdf_processor.hpp"
#include "poppler-document.h"
#include "poppler-page.h"
#include "poppler-page-renderer.h"
#include <locale>
#include <codecvt>
#include <opencv2/imgproc.hpp>

// Forward declaration of the log function
extern void log(const char* format, ...);

class PDFProcessor::Impl {
public:
    std::unique_ptr<poppler::document> doc;
    std::vector<VectorElement> vectorElements;
    std::vector<std::string> textElements;

    void processPath(const std::vector<cv::Point>& contour, double scale) {
        if (contour.size() < 2) return;

        // Convert contour to line segments
        for (size_t i = 1; i < contour.size(); ++i) {
            VectorElement line;
            line.type = VectorElement::Type::LINE;
            line.points = {
                static_cast<double>(contour[i-1].x) / scale,
                static_cast<double>(contour[i-1].y) / scale,
                static_cast<double>(contour[i].x) / scale,
                static_cast<double>(contour[i].y) / scale
            };
            line.thickness = 1.0;
            vectorElements.push_back(line);
        }

        // Close the path if it's a closed contour
        if (cv::norm(contour.front() - contour.back()) < 2.0) {
            VectorElement line;
            line.type = VectorElement::Type::LINE;
            line.points = {
                static_cast<double>(contour.back().x) / scale,
                static_cast<double>(contour.back().y) / scale,
                static_cast<double>(contour.front().x) / scale,
                static_cast<double>(contour.front().y) / scale
            };
            line.thickness = 1.0;
            vectorElements.push_back(line);
        }
    }
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
        
        // Get file size
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        log("File size: %ld bytes", fileSize);

        // Read first few bytes to verify PDF signature
        char signature[5] = {0};
        size_t bytesRead = fread(signature, 1, 4, file);
        fclose(file);

        if (bytesRead < 4) {
            log("Error: Failed to read file signature");
            return false;
        }

        if (strncmp(signature, "%PDF", 4) != 0) {
            log("Error: Invalid PDF signature: '%s'", signature);
            return false;
        }
        log("Valid PDF signature detected");

        // Try to load the PDF
        log("File exists and is valid PDF, attempting to load with Poppler...");
        pimpl->doc.reset(poppler::document::load_from_file(filepath));
        
        if (!pimpl->doc) {
            log("Failed to load PDF document: Poppler returned null document");
            return false;
        }

        int pageCount = pimpl->doc->pages();
        log("Successfully loaded PDF with %d pages", pageCount);

        // Log PDF metadata if available
        log("PDF is %s", pimpl->doc->is_encrypted() ? "encrypted" : "not encrypted");
        log("PDF is %s", pimpl->doc->is_linearized() ? "linearized" : "not linearized");

        if (pageCount == 0) {
            log("Warning: PDF has no pages");
        } else {
            // Log information about the first page
            std::unique_ptr<poppler::page> firstPage(pimpl->doc->create_page(0));
            if (firstPage) {
                poppler::rectf pageSize = firstPage->page_rect();
                log("First page size: %.2f x %.2f points", pageSize.width(), pageSize.height());
            }
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

        poppler::page_renderer renderer;
        renderer.set_render_hints(
            poppler::page_renderer::antialiasing |
            poppler::page_renderer::text_antialiasing |
            poppler::page_renderer::text_hinting
        );

        for (int i = 0; i < pageCount; ++i) {
            log("Processing page %d for vectors...", i + 1);
            std::unique_ptr<poppler::page> page(pimpl->doc->create_page(i));
            
            if (!page) {
                log("Warning: Failed to create page %d", i + 1);
                continue;
            }

            poppler::rectf pageSize = page->page_rect();
            log("Page %d size: %.2f x %.2f points", i + 1, pageSize.width(), pageSize.height());

            // Render page at high resolution for vector detection
            double scale = 4.0;  // Render at 4x resolution for better edge detection
            poppler::image img = renderer.render_page(page.get(), 
                72.0 * scale, 72.0 * scale);  // 72 DPI * scale

            if (!img.is_valid()) {
                log("Failed to render page %d", i + 1);
                continue;
            }

            // Convert to OpenCV format for processing
            cv::Mat image(img.height(), img.width(), CV_8UC4, 
                const_cast<char*>(img.const_data()));
            
            // Convert to grayscale
            cv::Mat gray;
            cv::cvtColor(image, gray, cv::COLOR_BGRA2GRAY);

            // Edge detection
            cv::Mat edges;
            cv::Canny(gray, edges, 50, 150);

            // Find contours
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(edges, contours, cv::RETR_LIST, 
                cv::CHAIN_APPROX_TC89_KCOS);

            log("Found %zu potential vector paths", contours.size());

            // Process each contour
            for (const auto& contour : contours) {
                if (contour.size() >= 2) {  // Only process paths with at least 2 points
                    pimpl->processPath(contour, scale);
                }
            }

            log("Processed %zu vector paths on page %d", contours.size(), i + 1);
        }
        
        log("Vector extraction complete. Found %zu vector elements", 
            pimpl->vectorElements.size());
        return true;
    } catch (const std::exception& e) {
        log("Exception while extracting vectors: %s", e.what());
        return false;
    } catch (...) {
        log("Unknown exception while extracting vectors");
        return false;
    }
}

// Helper function to convert UTF-8 to UTF-16
std::wstring utf8_to_utf16(const std::string& utf8) {
    try {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(utf8);
    } catch (...) {
        return L"[Invalid UTF-8 text]";
    }
}

// Helper function to convert UTF-16 to UTF-8
std::string utf16_to_utf8(const std::wstring& utf16) {
    try {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(utf16);
    } catch (...) {
        return "[Invalid UTF-16 text]";
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
                    // Clean up text - replace control characters with spaces
                    for (char& c : text) {
                        if (c < 32 && c != '\n' && c != '\r' && c != '\t') {
                            c = ' ';
                        }
                    }
                    
                    // Try to handle UTF-8 text properly
                    std::wstring wtext = utf8_to_utf16(text);
                    std::string cleaned_text = utf16_to_utf8(wtext);
                    
                    log("Found text on page %d (%zu bytes)", i + 1, text.length());
                    log("Text preview (first 100 chars): %s", 
                        cleaned_text.substr(0, std::min(size_t(100), cleaned_text.length())).c_str());
                    
                    pimpl->textElements.push_back(cleaned_text);
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