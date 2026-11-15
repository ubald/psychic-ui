> [!WARNING]  
> While I would love to continue this project because I believe it still has potential, I have no plan to restart development in the near future. Thanks to everyone who have starred, used or contributed to it.

# Psychic UI

[![CI](https://github.com/ubald/psychic-ui/actions/workflows/ci.yml/badge.svg)](https://github.com/ubald/psychic-ui/actions/workflows/ci.yml)

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

* [SDL3](https://www.libsdl.org/) for windows, input and the OpenGL context
* [Skia](https://skia.org/) for drawing shapes and text
* [Yoga](https://facebook.github.io/yoga/) for the layout

### Running it, or embedding it

`psychic_ui::SDL3Application` runs Psychic UI on its own: it opens an SDL window for each `Window` and runs
the loop.

```cpp
psychic_ui::SDL3Application app;
app.open(std::make_shared<MyWindow>());
app.run();
```

An application that already has a window, a GL context and a loop embeds Psychic UI with an adapter instead,
the way Dear ImGui's backends work: `SDL3SystemWindow` for an SDL3 window, `GLFWSystemWindow` for a GLFW
window. The host passes events (SDL) or keeps its callbacks, which the adapter chains to (GLFW), and calls
`render()` each frame. Their headers in `psychic-ui/applications/` show how; `example/glfw-host` is a
complete GLFW host.

## Using Psychic UI in your project

Psychic UI builds from source inside your CMake project (3.24 or newer, a C++23 compiler: GCC 14+, Clang 18+ or
Apple Clang 16+). There it builds as a static library, without its tests, demo or playground, and leaves your
`BUILD_SHARED_LIBS` alone. The checkout needs its submodules (Yoga, Skia, psychic-color):

    git submodule update --init --recursive

Add the checkout as a subdirectory:

```cmake
add_subdirectory(../psychic-ui psychic-ui)
```

or fetch it:

```cmake
include(FetchContent)
FetchContent_Declare(psychic_ui
    GIT_REPOSITORY https://github.com/ubald/psychic-ui.git
    GIT_TAG <commit>
    GIT_SUBMODULES_RECURSE TRUE)
FetchContent_MakeAvailable(psychic_ui)
```

and configure with `-DFETCHCONTENT_SOURCE_DIR_PSYCHIC_UI=<checkout>` to build a local checkout instead, for edits.
Then link it and copy its resources (the bundled fonts, `res/`) next to your executable, where it loads them from:

```cmake
add_executable(my-app main.cpp)
target_link_libraries(my-app PRIVATE psychic-ui::psychic-ui)
psychic_ui_copy_resources(my-app)
```

Skia's first build is long. `-DPSYCHIC_UI_SKIA_DIR=<dir>` links an already built Skia from that directory
instead of building one, and only reads it, so several build directories share one build: point it at the
`extlib/skia/build` of a Psychic UI build directory configured without it, once its `skia-build` target is built.

Relative font paths resolve against the style manager's `resourceRoot()`, the executable's directory by default.
To keep the resources elsewhere, set it before installing a skin set, which loads the theme's fonts once:

```cpp
psychic_ui::StyleManager::getInstance()->resourceRoot("/usr/share/my-app");
window->skinSet<psychic_ui::DefaultSkins>();
```

## Sample App

A sample app that is used for development and demonstration is available in the `example` directory.
It can be built with the rest of the library with the CMake option `PSYCHIC_UI_BUILD_EXAMPLE` (`ON` by default when Psychic UI is the top-level project, `OFF` inside another one).
    
## Building

### Dependencies

#### Linux

    sudo apt install python3 ninja-build libgl-dev libicu-dev libharfbuzz-dev libfreetype-dev libfontconfig-dev

Skia is built from source the first time: its build downloads the `gn` it pins (and `ninja` when none is installed).
    
SDL3 and GLFW are found with CMake's `find_package` when installed (SDL3 3.2+, GLFW 3.4+), otherwise CMake
downloads and builds them. Building them needs their X11 and Wayland development packages, on Ubuntu:

    sudo apt install libx11-dev libxext-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
        libxfixes-dev libxkbcommon-dev libwayland-dev libdecor-0-dev libegl-dev \
        libdbus-1-dev libibus-1.0-dev

#### Mac

    brew install cmake ninja sdl3 glfw icu4c harfbuzz
    
### Building

    mkdir build
    cd build
    cmake ..
    make -j"$(nproc)"

