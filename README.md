# PDF to CAD Converter

This program converts PDF drawings to CAD formats (DXF/DWG). It extracts vector graphics, text, and images from PDF files and generates corresponding CAD files.

## Dependencies

- CMake (>= 3.10)
- C++17 compatible compiler
- OpenCV (for image processing)
- Poppler (for PDF processing)
- LibreDWG (for DWG support)

## Building

1. Install dependencies:

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install cmake build-essential
sudo apt-get install libopencv-dev
sudo apt-get install libpoppler-cpp-dev
sudo apt-get install libredwg-dev
```

### Windows (using vcpkg)
```powershell
vcpkg install opencv:x64-windows
vcpkg install poppler:x64-windows
vcpkg install libredwg:x64-windows
```

2. Build the project:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage

```bash
./pdf2cad input.pdf output.dxf
```
or
```bash
./pdf2cad input.pdf output.dwg
```

## Features

- Vector graphics extraction from PDF
- Text extraction and positioning
- Image extraction and processing
- Support for DXF and DWG output formats
- Maintains scale and dimensions from original PDF

## Implementation Details

The program uses:
- Poppler for PDF parsing and content extraction
- OpenCV for image processing and vector detection
- LibreDWG for DWG file format support
- Custom vector processing for CAD conversion

## License

MIT License #   p d f 2 c a d  
 