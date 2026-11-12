# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Status

The project is being resumed after years idle (the README still says it's paused). Dependencies are pinned through git submodules at 2018-era versions (Skia m73, Yoga 1.8, GLFW 3.2); the goal is to upgrade them all to current versions. `.travis.yml` is dead.

## Building

The toolchain and system libraries come from the Nix flake's dev shell (`flake.nix`), loaded by direnv (`.envrc`: `use flake`); outside direnv, prefix commands with `nix develop -c`. Dependencies are git submodules (GLFW, Yoga, Skia, depot_tools, `lib/psychic-color`); CMake stops with a fatal error if they are missing:

```sh
git submodule update --init --recursive
mkdir build && cd build
cmake ..
make -j"$(nproc)"
```

The generator stays CMake's default (Make), not Ninja, so pass `-j` yourself.

- The dev shell sets `CMAKE_POLICY_VERSION_MINIMUM=3.5`: CMake 4 otherwise rejects the old `cmake_minimum_required` in this repo and in the GLFW/Yoga submodules.
- Skia is built by `extlib/skia/skia_build.sh.in`, which CMake runs as a custom command: it calls `tools/git-sync-deps` (needs network), then `gn` and `ninja` from depot_tools (the downloaded `gn` binary runs through nix-ld). These scripts need **Python 2**, which nixpkgs unstable dropped, so the flake takes `python27` from a second `nixos-25.11` input; it isn't in the binary cache and builds from source once. The first Skia build is slow.
- Skia's gn args in `extlib/skia/CMakeLists.txt` work around modern GCC/ICU: `-include cstdint` (GCC 13+) and `skia_use_sfntly=false` (sfntly is C++14 and can't include ICU 64+ headers).
- The project builds as C++17 because the ICU headers require it.
- CMake options (all `ON` by default): `PSYCHIC_UI_BUILD_GLFW`, `PSYCHIC_UI_BUILD_SDL2`, `PSYCHIC_UI_BUILD_SHARED`, `PSYCHIC_UI_BUILD_TESTS`, `PSYCHIC_UI_BUILD_EXAMPLE`, `PSYCHIC_UI_BUILD_PLAYGROUND`.
- Non-Debug shared builds strip the library after linking; use `-DCMAKE_BUILD_TYPE=Debug` when debugging.
- `make doc` generates Doxygen docs when Doxygen is installed. `docs/html/` holds committed generated output, so don't edit it by hand.
- **Library sources are listed explicitly in `SOURCE_FILES` in the root `CMakeLists.txt`.** A new `.cpp`/`.hpp` must be added there; test files go in `tests/CMakeLists.txt`.

## Tests

Catch2 v2 (`v2.13.10`, fetched via `ExternalProject_Add`) into one executable; `lib/psychic-color`'s own tests are turned off from the root `CMakeLists.txt` because they'd declare the same `catch` target:

```sh
./build/tests/psychic-ui-tests                  # all tests
./build/tests/psychic-ui-tests "[style]"        # by tag
./build/tests/psychic-ui-tests "basic style getter"   # by test name
```

## Executables

- `example/` builds `psychic-ui-demo-glfw` and `psychic-ui-demo-sdl2`: the demo app, where new components are developed before they move into the library.
- `playground/` builds `psychic-ui-playground-{glfw,sdl2}`, a scratch app.

Each executable is compiled twice from the same sources, with `-DUSE_GLFW` or `-DUSE_SDL2`. `psychic-ui/Application.hpp` uses that define to alias `psychic_ui::Application` to `GLFWApplication` or `SDL2Application`. The library itself is built with `WITH_GLFW` / `WITH_SDL2` according to the CMake options. Resources (`res/`, `example/fonts/`) are copied into the build directory, and fonts load by relative path, so run binaries from their build directory.

## Architecture

Retained-mode UI: **Skia** draws, **Yoga** does flexbox layout, and a CSS-like **style system** drives both. Everything lives in `namespace psychic_ui`.

- **`Div`** (`psychic-ui/Div.hpp`) is the base node, like an HTML div. It owns a Yoga node, children (`std::shared_ptr<Div>`, hence `enable_shared_from_this`), tag and class names, mouse/focus handling and signals. Override `draw(SkCanvas*)` for custom drawing; `render()` recurses over children.
- **`Window`** is the root `Div`. `Window::drawAll()` runs each frame in this order: if the `StyleManager` is invalid, it recomputes styles recursively (`updateStyleRecursive`); if the Yoga root is dirty, it runs `YGNodeCalculateLayout` and then `layoutUpdated()`; finally it calls `render()` on the Skia canvas. Window-level Yoga config (web defaults, rounding disabled) is set in `Window.cpp`.
- **Backends**: `ApplicationBase` and `SystemWindow` (`ApplicationBase.hpp`) are abstract. `applications/GLFWApplication` and `applications/SDL2Application` implement them, creating the GL context and the Skia surface and translating input events.
- **Style system** (`psychic-ui/style/`): `StyleManager` holds selector→`StyleDeclaration` rules (descendant selectors on tags and `.class`es, matched case-insensitively), fonts and skin factories. A `StyleSheet` subclass implements `load(StyleManager*)`. Windows apply them with `loadStyleSheet<T>(reset)`, and loading several stacks them (e.g. `OneDarkStyleSheet` then `DemoStyleSheet`). Each `Div` gets a computed `Style` (`computeStyle`). `styleUpdated()` is the hook for reacting to it. Some properties are inherited down the tree (`inheritableValues()`).
- **Component/Skin split**: `Component<SkinT>` (`Component.hpp`) is a `Div` whose visuals come from a `Skin<ComponentT>` child. The skin is chosen by the computed `skin` string style property and built from a factory registered with `StyleManager::registerSkin(name, SkinType::make(...))` (`Hatcher`). The skin is always child 0 and draws the component; the component's own `draw` is a no-op and it ignores internal layout constraints. Built-in themes and skin registrations live in `psychic-ui/themes/default.hpp`; skin implementations are in `psychic-ui/skins/`, widgets in `psychic-ui/components/`.
- **Signals** (`psychic-ui/signals/`): header-only `Signal`/`Slot`/`Observer`. `Div` is an `Observer`, so its subscriptions are torn down with it.
- **Text**: `TextBase` plus `utils/TextBox` (Skia text layout, ICU `UnicodeString`) back `Text`, `Label`, `TextInput` and `TextArea`.
- `lib/psychic-color` is the author's own color library (submodule), used for theme colors.
- `extlib/rxcpp` exists but is not wired into the root build.
