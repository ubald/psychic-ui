# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Status

The project is being resumed after years idle (the README still says it's paused). Yoga (3.2.2), SDL3 and GLFW are current; Skia is still m73 from 2018. The goal is to upgrade them all to current versions.

## Building

The toolchain and system libraries come from the Nix flake's dev shell (`flake.nix`), loaded by direnv (`.envrc`: `use flake`); outside direnv, prefix commands with `nix develop -c`. The flake is a convenience for local development only: the build must also work without Nix, so never make a dependency reachable only through it. Yoga, Skia, depot_tools and `lib/psychic-color` are git submodules; CMake stops with a fatal error if they are missing. SDL3 and GLFW come from `find_package` (`SDL3 3.2`, `glfw3 3.4`, both in the flake), otherwise FetchContent downloads a pinned release (SDL 3.4.20, GLFW 3.5.1, by URL and SHA-256) and builds it static: SDL3 without audio, joystick, haptics, sensors, camera, GPU, render, Vulkan or the X11 extensions it would otherwise stop on when their headers are missing (XScrnSaver, XShape, XTest, Xdbe), GLFW with Wayland only when its development files are found and SDL3 isn't fetched too (both static, their generated Wayland protocol symbols would clash). `-DCMAKE_DISABLE_FIND_PACKAGE_SDL3=ON -DCMAKE_DISABLE_FIND_PACKAGE_glfw3=ON` forces the downloads. `CMakePresets.json` has `debug` and `release` presets that build in `build/<preset>` and export `compile_commands.json`:

```sh
git submodule update --init --recursive
cmake --preset debug
cmake --build --preset debug -j"$(nproc)"
```

No preset sets a generator, so it stays CMake's default (Make), not Ninja; pass `-j` yourself.

- CMake 3.24 or newer.
- Everything is a target: psychic-color is the static `psychic-color::psychic-color`, Yoga is its own static `yogacore` target (`extlib/external/yoga/yoga/CMakeLists.txt`, added from `extlib/yoga/`; it compiles as C++20 with `-Wall -Werror` in its own directory), Skia the imported `Skia::Skia`, SDL3 `SDL3::SDL3`, GLFW `glfw`. ICU and Fontconfig use CMake's own Find modules; there's no `cmake/` directory any more. On macOS the Homebrew prefix comes from `brew --prefix`.
- Skia is built by `extlib/skia/skia_build.sh.in`, which CMake runs as the `skia-build` custom command when `<build dir>/extlib/skia/build/libskia.a` is missing: it calls `tools/git-sync-deps` (needs network), then `gn` and `ninja` from depot_tools (the downloaded `gn` binary runs through nix-ld). These scripts need **Python 2**, which nixpkgs unstable dropped, so the flake takes `python27` from a second `nixos-25.11` input; it isn't in the binary cache and builds from source once. The first Skia build is slow; copying an existing `extlib/skia/build` into a new build directory (`cp -a`) before its first build skips it.
- CI (`.github/workflows/ci.yml`, GitHub Actions) proves the build works without Nix, from system packages: Linux on `ubuntu-22.04`, the last Ubuntu that packages Python 2 (it has no SDL3 or GLFW 3.4, so they're fetched, with their X11/Wayland development packages installed), and macOS, allowed to fail until the Skia upgrade. Both cache Skia's output directory per Skia commit and build script. Jammy's GCC 11 is too old for C++23, so the Linux job installs `g++-14` from the `ppa:ubuntu-toolchain-r/test` PPA and passes it to CMake only (`-DCMAKE_CXX_COMPILER=g++-14`); Skia's gn build keeps the default `cc`/`c++`.
- Skia's gn args in `extlib/skia/CMakeLists.txt` work around modern GCC/ICU: `-include cstdint` (GCC 13+) and `skia_use_sfntly=false` (sfntly is C++14 and can't include ICU 64+ headers).
- `Skia::Skia` defines `_LIBCPP_ENABLE_CXX20_REMOVED_TYPE_TRAITS` for its consumers: the m73 headers use `std::result_of`, which libc++ (macOS) drops under C++20. A stopgap until the Skia upgrade; libstdc++ ignores it.
- The library requires C++23 (`target_compile_features(psychic-ui PUBLIC cxx_std_23)`), so whatever links it builds as C++23 too: the chainable API uses explicit object parameters ("deducing this"), which need GCC 14+, Clang 18+, Apple Clang 16+ or MSVC 17.2+. The root `CMakeLists.txt` stops at configure time with an older compiler, which would otherwise accept `-std=c++23` and fail deep in the headers.
- CMake options (all `ON` by default): `BUILD_SHARED_LIBS` (builds psychic-ui as a shared library; Yoga, psychic-color and a fetched SDL3, GLFW or Catch2 stay static either way), `PSYCHIC_UI_BUILD_SDL3` (the runner and the SDL adapter; the demo and playground need it), `PSYCHIC_UI_BUILD_GLFW_ADAPTER`, `PSYCHIC_UI_BUILD_TESTS`, `PSYCHIC_UI_BUILD_EXAMPLE`, `PSYCHIC_UI_BUILD_PLAYGROUND`.
- Non-Debug shared builds strip the library after linking; use the `debug` preset when debugging.
- `make doc` generates Doxygen docs when Doxygen is installed. `docs/html/` holds committed generated output, so don't edit it by hand.
- **Library sources are listed explicitly in `SOURCE_FILES` in the root `CMakeLists.txt`.** A new `.cpp`/`.hpp` must be added there; test files go in `tests/CMakeLists.txt`.

## Tests

Catch2 v3 into one executable, linked to `Catch2::Catch2WithMain` (there's no `main.cpp`) and registered with CTest. `tests/CMakeLists.txt` uses an installed Catch2 3 when `find_package` finds one (the flake provides `catch2_3`); otherwise FetchContent downloads `v3.16.1` into `<build dir>/_deps`. `-DCMAKE_DISABLE_FIND_PACKAGE_Catch2=ON` forces the download. `lib/psychic-color`'s own tests (`PSYCHIC_COLOR_BUILD_TESTS`) default off when it isn't the top-level project, so they aren't built here.

The backend tests (`tests/backends/`) need no display: a fake `SystemWindow`, SDL3 on its `dummy` video driver fed synthetic events, and GLFW on its null platform with its installed callbacks called directly.

```sh
ctest --preset debug                                       # all tests, through CTest
./build/debug/tests/psychic-ui-tests                       # all tests
./build/debug/tests/psychic-ui-tests "[style]"             # by tag
./build/debug/tests/psychic-ui-tests "basic style getter"  # by test name
```

## Executables

- `example/` builds `psychic-ui-demo` (SDL3): the demo app, where new components are developed before they move into the library. It also builds `psychic-ui-glfw-host`, a GLFW application that embeds a Psychic UI window through the GLFW adapter (Escape quits).
- `playground/` builds `psychic-ui-playground` (SDL3), a scratch app.

Resources (`res/`, `example/fonts/`) are copied into the build directory, and fonts load by relative path, so run binaries from their build directory.

- `SDL_LOGGING=app=debug` makes the SDL3 runner print the video driver, GL renderer, samples, stencil bits, hit testing and pixel density it got.
- SDL picks the video driver (decision D4). Under WSLg it falls back to XWayland by itself (the compositor lacks fifo-v1); `SDL_VIDEO_DRIVER=wayland` forces Wayland, which works too. Nix's Mesa only finds WSL's GPU driver with `LD_LIBRARY_PATH=/usr/lib/wsl/lib`; otherwise GL is llvmpipe (software).
- Under Xvfb, unset `WAYLAND_DISPLAY` and point `XDG_RUNTIME_DIR` at an empty directory: otherwise GLFW (and SDL when forced) connects to WSLg's `wayland-0` socket and opens windows on the real display.

## Architecture

Retained-mode UI: **Skia** draws, **Yoga** does flexbox layout, and a CSS-like **style system** drives both. Everything lives in `namespace psychic_ui`.

- **`Div`** (`psychic-ui/Div.hpp`) is the base node, like an HTML div. It owns a Yoga node, children (`std::shared_ptr<Div>`, hence `enable_shared_from_this`), tag and class names, mouse/focus handling and signals. Override `draw(SkCanvas*)` for custom drawing; `render()` recurses over children. The computed style is the only input of its Yoga node (`updateLayout` pushes it): `setX`/`setY`/`setPosition`/`setWidth`/`setHeight`/`setSize` only write the inline style, and `x()`/`y()`/`getWidth()`/`getHeight()` read the last layout. A component's own defaults go in `_defaultStyle`, which stylesheets override.
- **`Window`** is the root `Div`. `Window::drawAll()` runs each frame in this order, the first two in `updateStylesAndLayout()` (no canvas needed, which is how the tests drive it): styles (the whole tree if the `StyleManager` is invalid, otherwise `updateDirtyStyles`, which follows the `_descendantStyleDirty` flags that `invalidateStyle` sets up the parent chain, visible or not; repeated up to `maxStylePasses` times while `styleUpdated` callbacks invalidate more), then layout if the Yoga root is dirty or the window size changed (`YGNodeCalculateLayout` at `windowWidth()`/`windowHeight()`, which the root, with no size of its own, fills; then `layoutUpdated()`), then `render()` on the Skia canvas, which only paints. Skia is set up lazily on the first frame, with a GL interface assembled from the system window's `glProcAddress` (not Skia's native one, which is GLX-only on m73); the surface is the framebuffer's pixel size, remade on the frame after a resize, and the canvas is scaled by the pixel ratio. Every Yoga node is created with one shared config (web defaults, rounding disabled), owned by `Div.cpp`.
- **Backends** (`psychic-ui/applications/`): `SystemWindow` (`SystemWindow.hpp`) is the platform side of a `Window`: sizes (window units for input and layout, framebuffer pixels for drawing), GL proc addresses, window operations, drag support, text input, clipboard. Adapters implement it over a window they don't own and translate its events: `SDL3SystemWindow` (`handleEvent(const SDL_Event &)`, `installHitTest()`) and `GLFWSystemWindow` (installs its callbacks over the host's and chains to them, like Dear ImGui's `imgui_impl_glfw`). `SDL3Application` is the only runner: SDL init, one SDL window and GL context per `Window`, the loop. There is no application interface; embedding hosts have no application object. A system window detaches from its `Window` in its destructor, with its context still current.
- **Window dragging**: `Window::addDragArea(div)` (`TitleBar` registers itself) and `Window::isDragArea(x, y)`, which SDL's hit test asks before a press is delivered, so the window manager or compositor moves the window (on Wayland too). `startDrag()`/`stopDrag()` remain as the fallback where there's no hit testing; `Window::supportsDrag()` is false where neither works (GLFW on Wayland).
- **Style system** (`psychic-ui/style/`): `StyleManager` holds selector→`StyleDeclaration` rules (descendant selectors on tags and `.class`es, matched case-insensitively), fonts and skin factories. The cascade follows CSS: matching rules apply by weight (specificity), and on equal weight the rule declared last wins (rules are kept in declaration order). An unparsable selector throws `std::invalid_argument`. `reset()` empties the rules but keeps them allocated, so a `Style *` from `style()` never dangles; a rule declared again starts empty, after the others. A `StyleSheet` subclass implements `load(StyleManager*)`. Windows apply them with `loadStyleSheet<T>(reset)`, and loading several stacks them (e.g. `OneDarkStyleSheet` then `DemoStyleSheet`). Each `Div` gets a computed `Style` (`computeStyle`). `styleUpdated()` is the hook for reacting to it. Some properties are inherited down the tree (`inheritableValues()`).
- **Component/Skin split**: `Component<SkinT>` (`Component.hpp`) is a `Div` whose visuals come from a `Skin<ComponentT>` child. The skin is chosen by the computed `skin` string style property and built from a factory registered with `StyleManager::registerSkin(name, SkinType::make(...))` (`Hatcher`). The skin is always child 0 and draws the component; the component's own `draw` is a no-op and it ignores internal layout constraints. Built-in themes and skin registrations live in `psychic-ui/themes/default.hpp`; skin implementations are in `psychic-ui/skins/`, widgets in `psychic-ui/components/`.
- **Signals** (`psychic-ui/signals/`): header-only `Signal`/`Slot`/`Observer`. `Div` is an `Observer`, so its subscriptions are torn down with it.
- **Text**: `TextBase` plus `utils/TextBox` (Skia text layout, ICU `UnicodeString`) back `Text`, `Label`, `TextInput` and `TextArea`.
- `lib/psychic-color` is the author's own color library (submodule), used for theme colors.
- `extlib/rxcpp` exists but is not wired into the root build.
