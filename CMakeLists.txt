cmake_minimum_required(VERSION 3.24)
project(Flatpack LANGUAGES CXX)

##----------------
# Find Fusion 360 API directories in various places where the ordinary installer, chocolatey or homebrew might install it.
# This is too complex to do nicely in CMake, therefore a Python script does it
find_package(Python)

execute_process(
    COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/find_fusion360_api_dir.py
    OUTPUT_VARIABLE FUSION_360_API_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(INFO "Fusion 360 API at: ${FUSION_360_API_DIR}")

find_path(FUSION_360_CPP_INCLUDE_DIR
    NAMES Fusion/FusionAll.h Core/CoreAll.h
    REQUIRED
    HINTS ${FUSION_360_API_DIR}
    PATH_SUFFIXES CPP/include
)

find_library(CORE_LIBRARY 
    core.lib core.dylib
    REQUIRED
    HINTS ${FUSION_360_API_DIR}
    PATH_SUFFIXES CPP/lib)

find_library(FUSION_LIBRARY 
    fusion.lib fusion.dylib
    REQUIRED
    HINTS ${FUSION_360_API_DIR}
    PATH_SUFFIXES CPP/lib)

##----------------
# Find boost
if(NOT DEFINED BOOST_ROOT)
    set(BOOST_ROOT ${FUSION_360_CPP_LIB_DIR}/../../boost)
endif()

find_package(Boost REQUIRED)

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
    ${Boost_INCLUDE_DIRS}
    ${FUSION_360_CPP_INCLUDE_DIR}
    )

target_link_libraries(Flatpack ${CORE_LIBRARY} ${FUSION_LIBRARY})
target_compile_features(Flatpack PRIVATE cxx_std_14)

##----------------
# Build the zip file

install(DIRECTORY Flatpack.bundle DESTINATION .)
install(TARGETS Flatpack RUNTIME DESTINATION Flatpack.bundle/Contents)
set(CPACK_BINARY_NSIS OFF)
set(CPACK_BINARY_ZIP ON)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)