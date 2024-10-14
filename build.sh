#!/bin/bash

# Set the output binary names
DEBUG_OUTPUT="debug-x64"
RELEASE_OUTPUT="release-x64"

# Debug build
gcc -o "x64/$DEBUG_OUTPUT/debug-x64" src/main_glfw.c -g -D_DEBUG -W -Wall -Wextra \
    -Wno-unused-variable -Wno-unreachable-code -Wno-unused-parameter \
     -Wno-unused-function -Wno-sign-compare -Wno-missing-braces -lglfw -lGL -lm

if [ $? -eq 0 ]; then
    echo "Debug build succeeded. Running..."
    ."/x64/$DEBUG_OUTPUT/debug-x64"
else
    echo "Debug build failed."
fi


# Release build
#gcc -o "x64/$RELEASE_OUTPUT/release-x64" src/main_glfw.c -O2 -D_RELEASE \
 #   -W -Wall -Wextra -Wno-unused-variable -Wno-unreachable-code -Wno-unused-parameter \
#    -Wno-unused-function -Wno-sign-compare -lglfw -lGL -lm

#if [ $? -eq 0 ]; then
 #   echo "Release build succeeded. Running..."
#else
 #   echo "Release build failed."
#fi
