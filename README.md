# How to build and run

**1. Verify requirements and dependecies**

Tips on the ending sections

**2. Build: Run *Release_build.sh***

```console
sh Release_build.sh
```

**3. Run: execute FF-SH on *build* folder**

*Optional: copy scripts/imgui.ini on build to enable our options*

*Mandatory: sudo required for correct nginx functioning*

```console
cd build
sudo ./FF-CH
```


## Requirements

### g++-12 and CMake
```console
sudo apt install build-essential
sudo apt install cmake
```

### ffmpeg
```console
sudo apt install ffmpeg
```

### nginx
```console
sudo add-apt-repository universe
sudo apt install libnginx-mod-rtmp
```
## Dependencies

### OpenCV 4.8

[opencv-github](https://github.com/opencv/opencv)

### Dear ImGui

[imgui-github](https://github.com/ocornut/imgui)

### GLFW and OPENGL
```console
sudo apt install libglfw3 libglfw3-dev xorg-dev freeglut3-dev
```
