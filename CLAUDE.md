# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Status

The project is being resumed after years idle (the README still says it's paused). Dependencies are pinned through git submodules; Yoga is at 3.2.2, while Skia m73 and GLFW 3.3-dev are still 2018-era versions. The goal is to upgrade them all to current versions. `.travis.yml` is dead.

## Building

The toolchain and system libraries come from the Nix flake's dev shell (`flake.nix`), loaded by direnv (`.envrc`: `use flake`); outside direnv, prefix commands with `nix develop -c`. The flake is a convenience for local development only: the build must also work without Nix, so never make a dependency reachable only through it. Dependencies are git submodules (GLFW, Yoga, Skia, depot_tools, `lib/psychic-color`); CMake stops with a fatal error if they are missing. SDL2 comes from the system (`find_package(SDL2 CONFIG)`). `CMakePresets.json` has `debug` and `release` presets that build in `build/<preset>` and export `compile_commands.json`:

```sh
git submodule update --init --recursive
cmake --preset debug
cmake --build --preset debug -j"$(nproc)"
```

No preset sets a generator, so it stays CMake's default (Make), not Ninja; pass `-j` yourself.

- CMake 3.24 or newer. The root `CMakeLists.txt` sets `CMAKE_POLICY_VERSION_MINIMUM` to 3.5 around the GLFW and psychic-color subdirectories, whose old `cmake_minimum_required` CMake 4 otherwise rejects; nothing needs to set it from outside.
- Everything is a target: Yoga is its own static `yogacore` target (`extlib/external/yoga/yoga/CMakeLists.txt`, added from `extlib/yoga/`; it compiles as C++20 with `-Wall -Werror` in its own directory), Skia the imported `Skia::Skia`, GLFW the static `glfw`. ICU and Fontconfig use CMake's own Find modules; there's no `cmake/` directory any more. On macOS the Homebrew prefix comes from `brew --prefix`.
- Skia is built by `extlib/skia/skia_build.sh.in`, which CMake runs as the `skia-build` custom command when `<build dir>/extlib/skia/build/libskia.a` is missing: it calls `tools/git-sync-deps` (needs network), then `gn` and `ninja` from depot_tools (the downloaded `gn` binary runs through nix-ld). These scripts need **Python 2**, which nixpkgs unstable dropped, so the flake takes `python27` from a second `nixos-25.11` input; it isn't in the binary cache and builds from source once. The first Skia build is slow; copying an existing `extlib/skia/build` into a new build directory (`cp -a`) before its first build skips it.
- Skia's gn args in `extlib/skia/CMakeLists.txt` work around modern GCC/ICU: `-include cstdint` (GCC 13+) and `skia_use_sfntly=false` (sfntly is C++14 and can't include ICU 64+ headers).
- The library requires C++20 (`target_compile_features(psychic-ui PUBLIC cxx_std_20)`), so whatever links it builds as C++20 too. GCC 16 defaults to C++20, so the compile flags may show no `-std` at all.
- CMake options (all `ON` by default): `BUILD_SHARED_LIBS` (builds psychic-ui as a shared library; GLFW, Yoga, psychic-color and a fetched Catch2 stay static either way), `PSYCHIC_UI_BUILD_GLFW`, `PSYCHIC_UI_BUILD_SDL2`, `PSYCHIC_UI_BUILD_TESTS`, `PSYCHIC_UI_BUILD_EXAMPLE`, `PSYCHIC_UI_BUILD_PLAYGROUND`.
- Non-Debug shared builds strip the library after linking; use the `debug` preset when debugging.
- `make doc` generates Doxygen docs when Doxygen is installed. `docs/html/` holds committed generated output, so don't edit it by hand.
- **Library sources are listed explicitly in `SOURCE_FILES` in the root `CMakeLists.txt`.** A new `.cpp`/`.hpp` must be added there; test files go in `tests/CMakeLists.txt`.

## Tests

Catch2 v3 into one executable, linked to `Catch2::Catch2WithMain` (there's no `main.cpp`) and registered with CTest. `tests/CMakeLists.txt` uses an installed Catch2 3 when `find_package` finds one (the flake provides `catch2_3`); otherwise FetchContent downloads `v3.16.1` into `<build dir>/_deps`. `-DCMAKE_DISABLE_FIND_PACKAGE_Catch2=ON` forces the download. `lib/psychic-color`'s own tests are turned off from the root `CMakeLists.txt`: they download Catch's `master` branch at build time, which no longer exists.

```sh
ctest --preset debug                                       # all tests, through CTest
./build/debug/tests/psychic-ui-tests                       # all tests
./build/debug/tests/psychic-ui-tests "[style]"             # by tag
./build/debug/tests/psychic-ui-tests "basic style getter"  # by test name
```

## Executables

- `example/` builds `psychic-ui-demo-glfw` and `psychic-ui-demo-sdl2`: the demo app, where new components are developed before they move into the library.
- `playground/` builds `psychic-ui-playground-{glfw,sdl2}`, a scratch app.

Each executable is compiled twice from the same sources, with `-DUSE_GLFW` or `-DUSE_SDL2`. `psychic-ui/Application.hpp` uses that define to alias `psychic_ui::Application` to `GLFWApplication` or `SDL2Application`. The library itself is built with `WITH_GLFW` / `WITH_SDL2` according to the CMake options. Resources (`res/`, `example/fonts/`) are copied into the build directory, and fonts load by relative path, so run binaries from their build directory.

## Architecture

Retained-mode UI: **Skia** draws, **Yoga** does flexbox layout, and a CSS-like **style system** drives both. Everything lives in `namespace psychic_ui`.

- **`Div`** (`psychic-ui/Div.hpp`) is the base node, like an HTML div. It owns a Yoga node, children (`std::shared_ptr<Div>`, hence `enable_shared_from_this`), tag and class names, mouse/focus handling and signals. Override `draw(SkCanvas*)` for custom drawing; `render()` recurses over children.
- **`Window`** is the root `Div`. `Window::drawAll()` runs each frame in this order: if the `StyleManager` is invalid, it recomputes styles recursively (`updateStyleRecursive`); if the Yoga root is dirty, it runs `YGNodeCalculateLayout` and then `layoutUpdated()`; finally it calls `render()` on the Skia canvas. Every Yoga node is created with one shared config (web defaults, rounding disabled), owned by `Div.cpp`.
- **Backends**: `ApplicationBase` and `SystemWindow` (`ApplicationBase.hpp`) are abstract. `applications/GLFWApplication` and `applications/SDL2Application` implement them, creating the GL context and the Skia surface and translating input events.
- **Style system** (`psychic-ui/style/`): `StyleManager` holds selector→`StyleDeclaration` rules (descendant selectors on tags and `.class`es, matched case-insensitively), fonts and skin factories. A `StyleSheet` subclass implements `load(StyleManager*)`. Windows apply them with `loadStyleSheet<T>(reset)`, and loading several stacks them (e.g. `OneDarkStyleSheet` then `DemoStyleSheet`). Each `Div` gets a computed `Style` (`computeStyle`). `styleUpdated()` is the hook for reacting to it. Some properties are inherited down the tree (`inheritableValues()`).
- **Component/Skin split**: `Component<SkinT>` (`Component.hpp`) is a `Div` whose visuals come from a `Skin<ComponentT>` child. The skin is chosen by the computed `skin` string style property and built from a factory registered with `StyleManager::registerSkin(name, SkinType::make(...))` (`Hatcher`). The skin is always child 0 and draws the component; the component's own `draw` is a no-op and it ignores internal layout constraints. Built-in themes and skin registrations live in `psychic-ui/themes/default.hpp`; skin implementations are in `psychic-ui/skins/`, widgets in `psychic-ui/components/`.
- **Signals** (`psychic-ui/signals/`): header-only `Signal`/`Slot`/`Observer`. `Div` is an `Observer`, so its subscriptions are torn down with it.
- **Text**: `TextBase` plus `utils/TextBox` (Skia text layout, ICU `UnicodeString`) back `Text`, `Label`, `TextInput` and `TextArea`.
- `lib/psychic-color` is the author's own color library (submodule), used for theme colors.
- `extlib/rxcpp` exists but is not wired into the root build.
