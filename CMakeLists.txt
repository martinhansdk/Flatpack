cmake_minimum_required(VERSION 3.24)
project(Flatpack LANGUAGES CXX)

##----------------
# Find Fusion 360 API directories in various places where the ordinary installer, chocolatey or homebrew might install it
if(APPLE)
    # on MacOS in Github workflow, Autodesk Fusion 360 is installed by homebrew in a location that looks like it might change
    # in the future. Use "find" to find one of the include files and then manipulate the path until we have the location
    # of the API directory
    execute_process(
        COMMAND find "$ENV{HOME}/Library/Application Support/Autodesk/" -name FusionAll.h
        OUTPUT_VARIABLE FUSION_360_H_FILE
        ECHO_OUTPUT_VARIABLE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REPLACE "\n" ";" FUSION_360_H_FILE ${FUSION_360_H_FILE})
    list(GET FUSION_360_H_FILE 0 FUSION_360_H_FILE)
    cmake_path(GET FUSION_360_H_FILE PARENT_PATH FUSION_360_HOMEBREW_API_DIR)
    cmake_path(GET FUSION_360_HOMEBREW_API_DIR PARENT_PATH FUSION_360_HOMEBREW_API_DIR)
    cmake_path(GET FUSION_360_HOMEBREW_API_DIR PARENT_PATH FUSION_360_HOMEBREW_API_DIR)
    cmake_path(GET FUSION_360_HOMEBREW_API_DIR PARENT_PATH FUSION_360_HOMEBREW_API_DIR)
endif()

message(INFO " HOMEBREW_DIRS: ${FUSION_360_HOMEBREW_API_DIR}")

set(POSSIBLE_FUSION_360_INSTALL_DIRS
    "$ENV{APPDATA}/Autodesk/Autodesk Fusion 360/API"
    "$ENV{HOME}/Library/Application Support/Autodesk/Autodesk Fusion 360/API"
    "${FUSION_360_HOMEBREW_API_DIR}"
)

message(INFO "ID: ${POSSIBLE_FUSION_360_INSTALL_DIRS}")

find_path(FUSION_360_CPP_INCLUDE_DIR
    NAMES Fusion/FusionAll.h Core/CoreAll.h
    REQUIRED
    HINTS ${POSSIBLE_FUSION_360_INSTALL_DIRS}
    PATH_SUFFIXES CPP/include
)

find_library(CORE_LIBRARY 
    core.lib core.dylib
    REQUIRED
    HINTS ${POSSIBLE_FUSION_360_INSTALL_DIRS}
    PATH_SUFFIXES CPP/lib)

find_library(FUSION_LIBRARY 
    fusion.lib fusion.dylib
    REQUIRED
    HINTS ${POSSIBLE_FUSION_360_INSTALL_DIRS}
    PATH_SUFFIXES CPP/lib)

# add_library(core_lib SHARED IMPORTED)
# set_target_properties(core_lib PROPERTIES
#             IMPORTED_LOCATION ${FUSION_360_CPP_LIB_DIR}/core${CMAKE_SHARED_LIBRARY_SUFFIX}
#             IMPORTED_IMPLIB ${FUSION_360_CPP_LIB_DIR}/core${CMAKE_SHARED_LIBRARY_SUFFIX}
#             INTERFACE_INCLUDE_DIRECTORIES ${FUSION_360_CPP_INCLUDE_DIR}
#             )
# add_library(fusion_lib SHARED IMPORTED)
# set_target_properties(fusion_lib PROPERTIES
#             IMPORTED_LOCATION ${FUSION_360_CPP_LIB_DIR}/fusion${CMAKE_SHARED_LIBRARY_SUFFIX}
#             IMPORTED_IMPLIB ${FUSION_360_CPP_LIB_DIR}/fusion${CMAKE_SHARED_LIBRARY_SUFFIX}
#             INTERFACE_INCLUDE_DIRECTORIES ${FUSION_360_CPP_INCLUDE_DIR}
#             )


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