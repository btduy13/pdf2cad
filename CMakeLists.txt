cmake_minimum_required(VERSION 3.15)
project(pdf2cad)

# Enable debug information
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")

# Find required packages
find_package(OpenCV REQUIRED)
find_package(protobuf CONFIG REQUIRED)

# Set vcpkg root
set(VCPKG_ROOT $ENV{VCPKG_ROOT})
if(NOT VCPKG_ROOT)
    set(VCPKG_ROOT "C:/vcpkg")
endif()

# Set vcpkg installed directory
set(VCPKG_INSTALLED_DIR "${VCPKG_ROOT}/installed")

# Add include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${VCPKG_INSTALLED_DIR}/x64-windows/include
    ${VCPKG_ROOT}/buildtrees/poppler/src/er-24.03.0-8a01d026a0.clean/cpp
    ${VCPKG_ROOT}/buildtrees/poppler/src/er-24.03.0-8a01d026a0.clean/poppler
    ${VCPKG_ROOT}/buildtrees/poppler/x64-windows-rel/cpp
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
    ${VCPKG_ROOT}/buildtrees/poppler/x64-windows-rel/cpp/poppler-cpp.lib
    ${VCPKG_ROOT}/buildtrees/poppler/x64-windows-rel/poppler.lib
    protobuf::libprotobuf
)

# Copy DLLs to output directory
add_custom_command(TARGET pdf2cad POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VCPKG_ROOT}/buildtrees/poppler/x64-windows-rel/cpp/poppler-cpp.dll"
        "${VCPKG_ROOT}/buildtrees/poppler/x64-windows-rel/poppler.dll"
        $<TARGET_FILE_DIR:pdf2cad>
) 