#!/bin/sh -x

# Default installation directory
export PATH="/opt/rocm/hcc/bin/:$PATH"

hcc $(hcc-config --cxxflags) main.cpp -c -o main.cpp.o
hcc $(hcc-config --ldflags) main.cpp.o -o saxpy-hcc
