cmake_minimum_required(VERSION 3.15)
project(pdf2cad)

# Enable debug information
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")

# Find required packages
find_package(OpenCV REQUIRED)
find_package(protobuf CONFIG REQUIRED)

# Find Poppler
set(POPPLER_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows")
find_library(POPPLER_CPP_LIBRARY
    NAMES poppler-cpp
    PATHS "${POPPLER_DIR}/lib"
    REQUIRED
)
find_library(POPPLER_LIBRARY
    NAMES poppler
    PATHS "${POPPLER_DIR}/lib"
    REQUIRED
)
find_path(POPPLER_INCLUDE_DIR
    NAMES poppler-document.h
    PATHS "${POPPLER_DIR}/include"
    PATH_SUFFIXES "poppler/cpp"
    REQUIRED
)

# Add include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${POPPLER_INCLUDE_DIR}
    ${POPPLER_DIR}/include
)

# Add executable
add_executable(pdf2cad
    src/main.cpp
    src/pdf_processor.cpp
    src/cad_generator.cpp
)

# Link libraries
target_link_libraries(pdf2cad PRIVATE
    ${OpenCV_LIBS}
    ${POPPLER_CPP_LIBRARY}
    ${POPPLER_LIBRARY}
    protobuf::libprotobuf
)

# Copy DLLs to output directory
add_custom_command(TARGET pdf2cad POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:pdf2cad>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${POPPLER_DIR}/bin/poppler-cpp.dll"
        "${POPPLER_DIR}/bin/poppler.dll"
        $<TARGET_FILE_DIR:pdf2cad>
) 