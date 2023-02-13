cmake_minimum_required(VERSION 3.15)

PROJECT(SEED_PLUGIN
LANGUAGES CXX
VERSION 0.0.1
)

add_subdirectory(libs/JUCE)

enable_testing()

add_subdirectory(src)