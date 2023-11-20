#!/bin/sh

cmake -DCMAKE_BUILD_TYPE=Debug build &&
cmake --build ./build --target all