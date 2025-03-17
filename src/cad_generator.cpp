#include "cad_generator.hpp"
#include <fstream>
#include <cstring>  // For strcmp

// Forward declaration of the log function
extern void log(const char* format, ...);

class CADGenerator::Impl {
public:
    std::vector<PDFProcessor::VectorElement> vectors;
    std::vector<std::string> texts;
    int nextHandle = 100;  // Start with a higher handle number

    std::string getNextHandle() {
        return std::to_string(nextHandle++);
    }

    bool writeDXF(const std::string& outputPath) {
        log("Attempting to write DXF file: %s", outputPath.c_str());
        std::ofstream file(outputPath);
        if (!file) {
            log("Failed to open output file for writing");
            return false;
        }

        // Write each group code and value on separate lines
        auto writeGroup = [&file](int code, const std::string& value) {
            file << code << "\n" << value << "\n";
        };

        log("Writing DXF header...");
        // Write DXF header
        writeGroup(0, "SECTION");
        writeGroup(2, "HEADER");
        
        // Required header variables with proper formatting
        writeGroup(9, "$ACADVER");
        writeGroup(1, "AC1032");  // AutoCAD 2018 version
        writeGroup(9, "$DWGCODEPAGE");
        writeGroup(3, "ANSI_1252");
        writeGroup(9, "$INSBASE");
        writeGroup(10, "0.0");
        writeGroup(20, "0.0");
        writeGroup(30, "0.0");
        writeGroup(9, "$EXTMIN");
        writeGroup(10, "-100.0");  // Expanded drawing limits
        writeGroup(20, "-100.0");
        writeGroup(30, "-100.0");
        writeGroup(9, "$EXTMAX");
        writeGroup(10, "3000.0");  // Expanded drawing limits
        writeGroup(20, "2000.0");
        writeGroup(30, "100.0");
        writeGroup(9, "$LIMMIN");
        writeGroup(10, "0.0");
        writeGroup(20, "0.0");
        writeGroup(9, "$LIMMAX");
        writeGroup(10, "420.0");
        writeGroup(20, "297.0");
        writeGroup(9, "$HANDSEED");
        writeGroup(5, "FF");
        writeGroup(9, "$MEASUREMENT");
        writeGroup(70, "1");
        writeGroup(9, "$LUNITS");
        writeGroup(70, "2");
        writeGroup(9, "$AUNITS");
        writeGroup(70, "0");
        writeGroup(0, "ENDSEC");

        // Write CLASSES section (required for AC1032)
        writeGroup(0, "SECTION");
        writeGroup(2, "CLASSES");
        writeGroup(0, "ENDSEC");

        // Write TABLES section
        writeGroup(0, "SECTION");
        writeGroup(2, "TABLES");
        
        // VPORT table
        writeGroup(0, "TABLE");
        writeGroup(2, "VPORT");
        writeGroup(5, getNextHandle());
        writeGroup(330, "0");
        writeGroup(100, "AcDbSymbolTable");
        writeGroup(70, "1");
        writeGroup(0, "VPORT");
        writeGroup(5, getNextHandle());
        writeGroup(330, "2");
        writeGroup(100, "AcDbSymbolTableRecord");
        writeGroup(100, "AcDbViewportTableRecord");
        writeGroup(2, "*ACTIVE");
        writeGroup(70, "0");
        writeGroup(10, "0.0");
        writeGroup(20, "0.0");
        writeGroup(11, "1.0");
        writeGroup(21, "1.0");
        writeGroup(12, "0.0");
        writeGroup(22, "0.0");
        writeGroup(13, "0.0");
        writeGroup(23, "0.0");
        writeGroup(14, "10.0");
        writeGroup(24, "10.0");
        writeGroup(15, "10.0");
        writeGroup(25, "10.0");
        writeGroup(16, "0.0");
        writeGroup(26, "0.0");
        writeGroup(36, "1.0");
        writeGroup(17, "0.0");
        writeGroup(27, "0.0");
        writeGroup(37, "0.0");
        writeGroup(40, "297.0");
        writeGroup(41, "1.24");
        writeGroup(42, "50.0");
        writeGroup(43, "0.0");
        writeGroup(44, "0.0");
        writeGroup(50, "0.0");
        writeGroup(51, "0.0");
        writeGroup(71, "0");
        writeGroup(72, "100");
        writeGroup(73, "1");
        writeGroup(74, "3");
        writeGroup(75, "0");
        writeGroup(76, "1");
        writeGroup(77, "0");
        writeGroup(78, "0");
        writeGroup(0, "ENDTAB");

        // LTYPE table
        writeGroup(0, "TABLE");
        writeGroup(2, "LTYPE");
        writeGroup(5, getNextHandle());
        writeGroup(330, "0");
        writeGroup(100, "AcDbSymbolTable");
        writeGroup(70, "1");
        writeGroup(0, "LTYPE");
        writeGroup(5, getNextHandle());
        writeGroup(330, "5");
        writeGroup(100, "AcDbSymbolTableRecord");
        writeGroup(100, "AcDbLinetypeTableRecord");
        writeGroup(2, "CONTINUOUS");
        writeGroup(70, "0");
        writeGroup(3, "Solid line");
        writeGroup(72, "65");
        writeGroup(73, "0");
        writeGroup(40, "0.0");
        writeGroup(0, "ENDTAB");

        // LAYER table
        writeGroup(0, "TABLE");
        writeGroup(2, "LAYER");
        writeGroup(5, getNextHandle());
        writeGroup(330, "0");
        writeGroup(100, "AcDbSymbolTable");
        writeGroup(70, "1");
        writeGroup(0, "LAYER");
        writeGroup(5, getNextHandle());
        writeGroup(330, "2");
        writeGroup(100, "AcDbSymbolTableRecord");
        writeGroup(100, "AcDbLayerTableRecord");
        writeGroup(2, "0");
        writeGroup(70, "0");
        writeGroup(62, "7");
        writeGroup(6, "CONTINUOUS");
        writeGroup(370, "25");
        writeGroup(390, "F");
        writeGroup(0, "ENDTAB");

        // STYLE table
        writeGroup(0, "TABLE");
        writeGroup(2, "STYLE");
        writeGroup(5, getNextHandle());
        writeGroup(330, "0");
        writeGroup(100, "AcDbSymbolTable");
        writeGroup(70, "1");
        writeGroup(0, "STYLE");
        writeGroup(5, getNextHandle());
        writeGroup(330, "2");
        writeGroup(100, "AcDbSymbolTableRecord");
        writeGroup(100, "AcDbTextStyleTableRecord");
        writeGroup(2, "STANDARD");
        writeGroup(70, "0");
        writeGroup(40, "0.0");
        writeGroup(41, "1.0");
        writeGroup(50, "0.0");
        writeGroup(71, "0");
        writeGroup(42, "2.5");
        writeGroup(3, "txt");
        writeGroup(4, "");
        writeGroup(0, "ENDTAB");

        // Other required tables
        const char* tables[] = {"VIEW", "UCS", "APPID", "DIMSTYLE"};
        for (const auto& table : tables) {
            writeGroup(0, "TABLE");
            writeGroup(2, table);
            writeGroup(5, getNextHandle());
            writeGroup(330, "0");
            writeGroup(100, "AcDbSymbolTable");
            writeGroup(70, "0");
            if (strcmp(table, "APPID") == 0) {
                writeGroup(0, "APPID");
                writeGroup(5, getNextHandle());
                writeGroup(330, "9");
                writeGroup(100, "AcDbSymbolTableRecord");
                writeGroup(100, "AcDbRegAppTableRecord");
                writeGroup(2, "ACAD");
                writeGroup(70, "0");
            }
            writeGroup(0, "ENDTAB");
        }

        writeGroup(0, "ENDSEC");

        // Write BLOCKS section
        writeGroup(0, "SECTION");
        writeGroup(2, "BLOCKS");
        
        // MODEL_SPACE block definition
        writeGroup(0, "BLOCK");
        writeGroup(5, getNextHandle());
        writeGroup(330, "1F");
        writeGroup(100, "AcDbEntity");
        writeGroup(8, "0");
        writeGroup(100, "AcDbBlockBegin");
        writeGroup(2, "*MODEL_SPACE");
        writeGroup(70, "0");
        writeGroup(10, "0.0");
        writeGroup(20, "0.0");
        writeGroup(30, "0.0");
        writeGroup(3, "*MODEL_SPACE");
        writeGroup(1, "");
        writeGroup(0, "ENDBLK");
        writeGroup(5, getNextHandle());
        writeGroup(330, "1F");
        writeGroup(100, "AcDbEntity");
        writeGroup(8, "0");
        writeGroup(100, "AcDbBlockEnd");

        // PAPER_SPACE block definition
        writeGroup(0, "BLOCK");
        writeGroup(5, getNextHandle());
        writeGroup(330, "1B");
        writeGroup(100, "AcDbEntity");
        writeGroup(8, "0");
        writeGroup(100, "AcDbBlockBegin");
        writeGroup(2, "*PAPER_SPACE");
        writeGroup(70, "0");
        writeGroup(10, "0.0");
        writeGroup(20, "0.0");
        writeGroup(30, "0.0");
        writeGroup(3, "*PAPER_SPACE");
        writeGroup(1, "");
        writeGroup(0, "ENDBLK");
        writeGroup(5, getNextHandle());
        writeGroup(330, "1B");
        writeGroup(100, "AcDbEntity");
        writeGroup(8, "0");
        writeGroup(100, "AcDbBlockEnd");
        
        writeGroup(0, "ENDSEC");

        log("Writing entities section...");
        // Write ENTITIES section
        writeGroup(0, "SECTION");
        writeGroup(2, "ENTITIES");

        // Write vector elements
        log("Writing %zu vector elements...", vectors.size());
        for (const auto& vec : vectors) {
            if (vec.type == PDFProcessor::VectorElement::Type::LINE) {
                writeGroup(0, "LINE");
                writeGroup(5, getNextHandle());
                writeGroup(330, "1F");
                writeGroup(100, "AcDbEntity");
                writeGroup(8, "0");
                writeGroup(100, "AcDbLine");
                writeGroup(10, std::to_string(vec.points[0]));
                writeGroup(20, std::to_string(vec.points[1]));
                writeGroup(30, "0.0");
                writeGroup(11, std::to_string(vec.points[2]));
                writeGroup(21, std::to_string(vec.points[3]));
                writeGroup(31, "0.0");
                log("  Added line from (%.2f,%.2f) to (%.2f,%.2f)", 
                    vec.points[0], vec.points[1], vec.points[2], vec.points[3]);
            }
        }

        // Write text elements with proper positioning
        log("Writing %zu text elements...", texts.size());
        double textY = 0.0;
        for (const auto& text : texts) {
            writeGroup(0, "TEXT");
            writeGroup(5, getNextHandle());
            writeGroup(330, "1F");
            writeGroup(100, "AcDbEntity");
            writeGroup(8, "0");
            writeGroup(100, "AcDbText");
            writeGroup(10, "0.0");
            writeGroup(20, std::to_string(textY));
            writeGroup(30, "0.0");
            writeGroup(40, "2.5");
            writeGroup(1, text);
            writeGroup(50, "0.0");
            writeGroup(41, "1.0");
            writeGroup(7, "STANDARD");
            writeGroup(71, "0");
            writeGroup(72, "0");
            writeGroup(73, "0");
            writeGroup(100, "AcDbText");
            log("  Added text at (0.0,%.2f): %s", textY, text.c_str());
            textY += 3.0;
        }

        writeGroup(0, "ENDSEC");

        // Write OBJECTS section (required for AC1032)
        writeGroup(0, "SECTION");
        writeGroup(2, "OBJECTS");
        
        // Dictionary
        writeGroup(0, "DICTIONARY");
        writeGroup(5, "C");
        writeGroup(330, "0");
        writeGroup(100, "AcDbDictionary");
        writeGroup(281, "1");
        writeGroup(3, "ACAD_GROUP");
        writeGroup(350, "D");
        writeGroup(3, "ACAD_MLINESTYLE");
        writeGroup(350, "17");

        // Empty group dictionary
        writeGroup(0, "DICTIONARY");
        writeGroup(5, "D");
        writeGroup(330, "C");
        writeGroup(100, "AcDbDictionary");
        writeGroup(281, "1");

        // MLStyle dictionary
        writeGroup(0, "DICTIONARY");
        writeGroup(5, "17");
        writeGroup(330, "C");
        writeGroup(100, "AcDbDictionary");
        writeGroup(281, "1");
        writeGroup(3, "Standard");
        writeGroup(350, "18");

        // Standard MLStyle
        writeGroup(0, "MLINESTYLE");
        writeGroup(5, "18");
        writeGroup(330, "17");
        writeGroup(100, "AcDbMlineStyle");
        writeGroup(2, "STANDARD");
        writeGroup(70, "0");
        writeGroup(3, "");
        writeGroup(62, "256");
        writeGroup(51, "90.0");
        writeGroup(52, "90.0");
        writeGroup(71, "2");
        writeGroup(49, "0.5");
        writeGroup(62, "256");
        writeGroup(6, "BYLAYER");
        writeGroup(49, "-0.5");
        writeGroup(62, "256");
        writeGroup(6, "BYLAYER");

        writeGroup(0, "ENDSEC");

        // Write EOF
        writeGroup(0, "EOF");

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