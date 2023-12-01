#!/bin/sh

sudo echo "~~~ OpenCV latest release + extra modules installer ~~~" && echo "" &&

sudo apt update &&
sudo apt install git cmake make g++-12 &&
git clone https://github.com/opencv/opencv.git &&
git clone https://github.com/opencv/opencv_contrib.git && 
cd opencv && mkdir -p build && cd build &&
cmake -DWITH_QT=ON -DWITH_OPENGL=ON -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules .. &&
make -j4 &&
sudo make install &&

echo "" && echo "~~~ OpenCV installed :) ~~~"