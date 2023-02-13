#!/bin/bash

set -e

build_dir=release-build

cmake \
  -B $build_dir \
  -G 'Unix Makefiles' \
  -D CMAKE_BUILD_TYPE=Release \
  -D JUCE_COPY_PLUGIN_AFTER_BUILD=ON \
  -D DONT_SET_USING_JUCE_NAMESPACE=ON
  
cmake --build $build_dir