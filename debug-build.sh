#!/bin/bash

set -e

build_dir=build

cmake \
  -B $build_dir \
  -G 'Unix Makefiles' \
  -D CMAKE_BUILD_TYPE=Debug \
  -D JUCE_COPY_PLUGIN_AFTER_BUILD=ON \
  -D JUCE_BUILD_EXTRAS=ON
  
cmake --build $build_dir