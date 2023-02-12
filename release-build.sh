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

echo

benchmark_executable="$build_dir/benchmark/SeedBenchmarks_artefacts/Release/SeedBenchmarks"
if [ -n "$CI" ] ; then
  "$benchmark_executable" --benchmark_format=json | tee benchmark_result.json
else
  "$benchmark_executable" 
fi