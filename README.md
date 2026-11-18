# Psychic UI

[![CI](https://github.com/ubald/psychic-ui/actions/workflows/ci.yml/badge.svg)](https://github.com/ubald/psychic-ui/actions/workflows/ci.yml)

A retained-mode C++23 UI library: [Skia](https://skia.org/) draws, [Yoga](https://www.yogalayout.dev/) lays out
with flexbox, and a CSS-like style system drives both. Components get their looks from skin sets and themes,
with light and dark, pointer and touch densities, motion, popups, lists and text editing built in. It runs on its
own with [SDL3](https://www.libsdl.org/), or inside an existing SDL3 or GLFW application, on Linux, macOS and
Windows.

**See it, with screenshots of every component under each skin set: https://ubald.github.io/psychic-ui/**

[![Psychic UI's demo app](https://ubald.github.io/psychic-ui/showcase/app-default-dark.png)](https://ubald.github.io/psychic-ui/)

## Building

CMake 3.24 or newer and a C++23 compiler (GCC 14+, Clang 18+, Apple Clang 16+, or clang-cl on Windows). Skia, Yoga
and psychic-color are submodules; Skia is built by CMake on the first build, which takes a while.

```sh
git submodule update --init --recursive
cmake --preset debug
cmake --build --preset debug -j"$(nproc)"
./build/debug/example/psychic-ui-demo   # the demo app
ctest --preset debug                     # the tests
```

On Linux, Skia uses the system's ICU, HarfBuzz, FreeType and Fontconfig, and SDL3 and GLFW are downloaded when
they aren't installed; [CLAUDE.md](CLAUDE.md) has the details for every platform, the CMake options and the
architecture.

## Using it in your project

Inside your own CMake project it builds as a static library, without its tests and examples:

```cmake
add_subdirectory(psychic-ui)
target_link_libraries(my-app PRIVATE psychic-ui::psychic-ui)
psychic_ui_copy_resources(my-app)   # the bundled fonts, next to the executable
```

```cpp
psychic_ui::SDL3Application app;
app.open(std::make_shared<MyWindow>());
app.run();
```

`-DPSYCHIC_UI_SKIA_DIR=<dir>` links an already built Skia instead of building one. To embed it in an application
that already has a window and a loop, see the adapters in `psychic-ui/applications/` and `example/glfw-host`.

## License

MIT, see [LICENSE](LICENSE). The bundled fonts keep their own licenses (Inter: SIL OFL 1.1, Ubuntu: the Ubuntu
Font Licence, in `res/fonts/`), as do the Lucide icons (ISC, `psychic-ui/icons/LICENSE-Lucide.txt`).

Thanks to everyone who has starred, used or contributed to it.
