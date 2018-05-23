# WIP

# Psychic UI

[![Build Status](https://travis-ci.org/ubald/psychic-ui.svg?branch=master)](https://travis-ci.org/ubald/psychic-ui)

![Screenshot](/docs/screenshot.png)

## Roadmap

https://trello.com/b/XYz6BS9v

## What is it?

This is a cross-platform (Mac/Linux initially) C++/OpenGL UI library.

### Why make it?

As I was looking around for options I grew tired of seeing countless projects with no attention to detail and very little customization possibilities.
Most of the libraries seemed to focus on creating windows inside of windows and didn't include much widget/component options beside the bare minimum. 
Being able to create complex layouts with constraints was also a requirement. So this is why I started working on this project.
    
## What's with the name?

This is intended to be part of a future larger project so it is named according to that projet's name. I intend to make
sources available as much as possible for all of the project's parts in the form of libraries like this.

## Architecture

The library uses:

* either [GLFW](http://www.glfw.org/) or [SLD2](https://www.libsdl.org/) for the window and context creation
* [Skia](https://skia.org/) for drawing shapes and text
* [Yoga](https://facebook.github.io/yoga/) for the layout

## Sample App

A sample app that is used for development and demonstration is available in the `example` directory.
It can be built with the rest of the library with the CMake option `PSYCHIC_UI_BUILD_EXAMPLE` (`ON` by default).
    
## Building

### Dependencies

#### Linux

    sudo apt install xorg-dev libjpeg-dev libturbojpeg libpng12-dev libgl1-mesa-dev libfreetype6-dev fontconfig
    
#### Mac

    brew install cmake sdl2 libpng jpeg jpeg-turbo icu4c freetype
    
### Building

    mkdir build
    cd build
    cmake ..
    make -j"$(nproc)"

