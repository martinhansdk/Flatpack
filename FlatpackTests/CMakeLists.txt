cmake_minimum_required(VERSION 3.14)
project(FlatpackTests LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glm Catch2)

add_executable(FlatpackTests
    transformer_test.cpp
    nester_test.cpp
    ../Nester/Nester.cpp
)

target_include_directories(FlatpackTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Nester
)

target_compile_features(FlatpackTests PRIVATE cxx_std_14)
target_link_libraries(FlatpackTests PRIVATE glm::glm Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(FlatpackTests)
