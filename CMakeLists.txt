cmake_minimum_required(VERSION 3.15)

PROJECT(SEED_PLUGIN
LANGUAGES CXX
VERSION 0.0.1
)

add_subdirectory(libs/JUCE)

set(GOOGLETEST_PATH ${CMAKE_SOURCE_DIR}/libs/googletest CACHE PATH "")
# set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
add_subdirectory(libs/benchmark)

enable_testing()

add_subdirectory(src)
add_subdirectory(benchmark)