cmake_minimum_required(VERSION 3.24)
project(Flatpack LANGUAGES CXX)

##----------------
# Option to build the Fusion 360 plugin (requires Fusion 360 SDK, not available on Linux)
option(BUILD_FUSION_PLUGIN "Build the Fusion 360 plugin (requires Fusion 360 SDK)" ON)

if(BUILD_FUSION_PLUGIN)
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
endif()
    
##----------------
# Fetch GLM
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3  # Use latest stable release
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

##----------------
# Fetch libdeepnest
include(FetchContent)
FetchContent_Declare(
    libdeepnest
    GIT_REPOSITORY https://github.com/martinhansdk/libdeepnest.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(libdeepnest)

##----------------
# Fetch Boost (header-only)
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.90.0/boost-1.90.0-cmake.tar.gz
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(Boost)

##----------------
# Compile the Nester library (no Fusion 360 dependency)
add_library(Nester STATIC
    Nester/DXFWriter.cpp
    Nester/Nester.cpp
    Nester/SVGWriter.cpp
    Nester/Units.cpp)

target_include_directories(Nester PUBLIC
    Nester
    XDxfGen/include)

target_link_libraries(Nester PUBLIC glm::glm)
target_compile_features(Nester PRIVATE cxx_std_14)

##----------------
# Compile the Fusion 360 plugin
if(BUILD_FUSION_PLUGIN)
    # Compiler flags
    IF(MSVC)
        SET(CMAKE_CXX_FLAGS "/EHsc") # Enable exception unwind semantics in the compiler
    ENDIF(MSVC)

    add_library(Flatpack SHARED Flatpack.cpp)

    target_include_directories(Flatpack PRIVATE
        ${FUSION_360_CPP_INCLUDE_DIR}
        )

    target_link_libraries(Flatpack Nester ${CORE_LIBRARY} ${FUSION_LIBRARY})
    target_compile_features(Flatpack PRIVATE cxx_std_14)
endif()

##----------------
# Build the zip file (only when building the Fusion 360 plugin)
if(BUILD_FUSION_PLUGIN)
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
endif()