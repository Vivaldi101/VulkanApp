#!/bin/sh

# Create a build directory if it doesn't exist
mkdir -p build

# Navigate into the build directory
cd build

# Run CMake to configure the project and generate compile_commands.json
CC=clang CXX=clang++ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S ..

# Build the project using make
cmake --build .

# Copy the compile_commands.json to the source directory
cp compile_commands.json ..
