#!/bin/sh

$PWD=$(pwd)

echo "" && echo "~~~ Boost library 1.83 installer ~~~" &&
echo "WARN: execute this script from the root directory of the repository" &&

wget --https-only https://boostorg.jfrog.io/artifactory/main/release/1.83.0/source/boost_1_83_0.tar.bz2 &&
tar --directory=./src/libs/ --bzip2 -xfv boost_1_83_0.tar.bz2 &&
rm -f boost_1_83_0.tar.bz2 &&

echo "" && echo "~~~ Boost 1.83 installed in src/libs/ ~~~"