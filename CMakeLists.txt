cmake_minimum_required(VERSION 3.24)
project(Flatpack LANGUAGES CXX)

##----------------
# Find Fusion 360 API directories in various places where the ordinary installer, chocolatey or homebrew might install it.
# This is too complex to do nicely in CMake, therefore a Python script does it
find_package(Python)

set(POSSIBLE_FUSION_360_API_DIRS
    "${CMAKE_SOURCE_DIR}/fusion_360_api"  # The API files are copied here when running in CI. See copy_fusion360_api_dir.py
    "$ENV{APPDATA}/Autodesk/Autodesk Fusion 360/API/CPP"
    "$ENV{HOME}/Library/Application Support/Autodesk/Autodesk Fusion 360/API/CPP"
)

find_path(FUSION_360_CPP_INCLUDE_DIR
    NAMES Fusion/FusionAll.h Core/CoreAll.h
    REQUIRED
    HINTS ${POSSIBLE_FUSION_360_API_DIRS}
    PATH_SUFFIXES include
)

find_library(CORE_LIBRARY 
    core.lib core.dylib
    REQUIRED
    HINTS ${POSSIBLE_FUSION_360_API_DIRS}
    PATH_SUFFIXES lib)

find_library(FUSION_LIBRARY 
    fusion.lib fusion.dylib
    REQUIRED
    HINTS ${POSSIBLE_FUSION_360_API_DIRS}
    PATH_SUFFIXES lib)
    
##----------------
# Fetch dependencies
include(FetchContent)

# GLM - OpenGL Mathematics for transformations
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3  # Use latest stable release
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# libnest2d - 2D nesting library (Ultimaker fork - actively maintained)
FetchContent_Declare(
    libnest2d
    GIT_REPOSITORY https://github.com/Ultimaker/libnest2d.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(libnest2d)

##----------------
# Compile the addin


# Compiler flags
IF(MSVC)
    SET(CMAKE_CXX_FLAGS "/EHsc") # Enable exception unwind semantics in the compiler
ENDIF(MSVC)

add_library(Flatpack SHARED 
    Flatpack.cpp
    Nester/DXFWriter.cpp
    Nester/Nester.cpp
    Nester/SVGWriter.cpp
    Nester/Units.cpp)

target_include_directories(Flatpack PRIVATE
    Nester
    XDxfGen/include
    ${FUSION_360_CPP_INCLUDE_DIR}
    )

target_link_libraries(Flatpack ${CORE_LIBRARY} ${FUSION_LIBRARY} glm::glm libnest2d)
target_compile_features(Flatpack PRIVATE cxx_std_14)

##----------------
# Build the zip file

install(DIRECTORY Flatpack.bundle DESTINATION .)
if(APPLE)
    install(TARGETS Flatpack LIBRARY DESTINATION Flatpack.bundle/Contents)
else()
    install(TARGETS Flatpack RUNTIME DESTINATION Flatpack.bundle/Contents)
endif()

# Include documentation in the package
install(FILES 
    README.md 
    LICENSE 
    MIT.txt
    DESTINATION .)
install(FILES 
    doc/USER_GUIDE.md
    doc/privacy_policy.md
    DESTINATION doc)

set(CPACK_BINARY_NSIS OFF)
set(CPACK_BINARY_ZIP ON)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)