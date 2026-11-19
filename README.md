<h1 align="center">Psychic UI</h1>

<p align="center">
  <b>A retained-mode C++23 UI library that looks like a modern design system, out of the box.</b><br>
  <a href="https://skia.org/">Skia</a> draws, <a href="https://www.yogalayout.dev/">Yoga</a> lays out with flexbox,
  and a CSS-like style system drives both.
</p>

<p align="center">
  <a href="https://github.com/ubald/psychic-ui/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/ubald/psychic-ui/actions/workflows/ci.yml/badge.svg"></a>
  <a href="LICENSE"><img alt="License: MIT" src="https://img.shields.io/badge/license-MIT-blue.svg"></a>
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-informational.svg">
  <img alt="Platforms: Linux, macOS, Windows" src="https://img.shields.io/badge/platforms-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg">
</p>

<p align="center">
  <a href="https://ubald.github.io/psychic-ui/"><b>Website and screenshots</b></a> ·
  <a href="https://ubald.github.io/psychic-ui/html/"><b>API reference</b></a> ·
  <a href="#quick-start"><b>Quick start</b></a>
</p>

<p align="center">
  <a href="https://ubald.github.io/psychic-ui/">
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="https://ubald.github.io/psychic-ui/showcase/canvas-float-dark.png">
      <img alt="A canvas app built from Psychic UI components: a layer tree, an inspector and a tool bar floating over a dotted canvas" src="https://ubald.github.io/psychic-ui/showcase/canvas-float-light.png" width="840">
    </picture>
  </a>
  <br>
  <sub>Every screenshot here is captured by the library itself, headless, and follows your GitHub theme.</sub>
</p>

## Why Psychic UI

- **Flexbox layout and a CSS cascade, typed.** Yoga does the layout; rules with descendant selectors, classes and
  pseudo-classes (`:hover`, `:focus-visible`, `:pressed`...) style everything. Every style property is a chainable,
  type-checked setter: `add<Button>("Save").skin("primary").padding(12)`. A wrong type or unit doesn't compile.
- **Looks good before you style anything.** Components draw through skins; three skin sets ship with it: *Default*
  (Pivotal's design system), *Desk* (dense, desktop-first) and *Float* (panels floating over a canvas), each with light
  and dark themes that follow the system's color scheme, switched live.
- **Pointer and touch.** Touch, pens, taps, long presses, panning with kinetic scrolling, and a touch density with hit
  slop that grows controls when a finger shows up.
- **Motion built in.** Style transitions on curves or springs, entering and leaving styles for anything, layout
  transitions that slide reordered rows into place (FLIP), and a reduced motion switch.
- **The components an app needs.** Buttons, check boxes, switches, sliders, segmented controls, number steppers,
  search and text fields, selects, menus and menu bars, context menus, tooltips, dialogs, toasts, lists, a sortable data
  grid, a tree, an inspector, an app frame with a sidebar, and over a hundred vector icons (Pivotal's and Lucide's).
- **Real text.** SkParagraph shapes it (bidi, font fallback, wrapping, ellipsis); text fields have word and sentence
  selection, the clipboard, and tell the input method where the caret is.
- **Keyboard first.** Tab order, focus rings for the keyboard only, arrows in lists, menus, trees and grids.
- **Idles when nothing changes.** No frame is drawn unless input, a timer or an animation asks for one.
- **Runs alone or inside your app.** An SDL3 runner, or adapters that embed it in an existing SDL3 or GLFW window and
  loop. Linux, macOS and Windows.

## Gallery

<table>
  <tr>
    <td width="50%">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="https://ubald.github.io/psychic-ui/showcase/menu-float-dark.png">
        <img alt="A menu with icons, shortcuts, check marks, a submenu and a danger row" src="https://ubald.github.io/psychic-ui/showcase/menu-float-light.png" width="420">
      </picture>
      <br><sub>Menus and submenus (Float)</sub>
    </td>
    <td width="50%">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="https://ubald.github.io/psychic-ui/showcase/dialog-desk-dark.png">
        <img alt="A modal dialog with a text field and two actions" src="https://ubald.github.io/psychic-ui/showcase/dialog-desk-light.png" width="420">
      </picture>
      <br><sub>Dialogs (Desk)</sub>
    </td>
  </tr>
  <tr>
    <td colspan="2">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="https://ubald.github.io/psychic-ui/showcase/data-grid-default-dark.png">
        <img alt="A sortable data grid beside an inspector following its selection" src="https://ubald.github.io/psychic-ui/showcase/data-grid-default-light.png" width="860">
      </picture>
      <br><sub>A sortable data grid and an inspector (Default)</sub>
    </td>
  </tr>
  <tr>
    <td width="50%">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="https://ubald.github.io/psychic-ui/showcase/inputs-float-dark.png">
        <img alt="Search fields, switches, segmented controls, number steppers and color swatches" src="https://ubald.github.io/psychic-ui/showcase/inputs-float-light.png" width="420">
      </picture>
      <br><sub>Inputs (Float)</sub>
    </td>
    <td width="50%">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="https://ubald.github.io/psychic-ui/showcase/toasts-default-dark.png">
        <img alt="A stack of toasts with icons and actions" src="https://ubald.github.io/psychic-ui/showcase/toasts-default-light.png" width="420">
      </picture>
      <br><sub>Toasts (Default)</sub>
    </td>
  </tr>
  <tr>
    <td width="50%">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="https://ubald.github.io/psychic-ui/showcase/motion-sort-default-dark.png">
        <img alt="Rows sliding to their new places as a list is sorted (animated)" src="https://ubald.github.io/psychic-ui/showcase/motion-sort-default-light.png" width="420">
      </picture>
      <br><sub>Layout transitions: sorted rows slide into place</sub>
    </td>
    <td width="50%">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="https://ubald.github.io/psychic-ui/showcase/motion-menu-default-dark.png">
        <img alt="A menu scaling and fading in from its trigger (animated)" src="https://ubald.github.io/psychic-ui/showcase/motion-menu-default-light.png" width="420">
      </picture>
      <br><sub>Popups enter and leave from their anchor</sub>
    </td>
  </tr>
</table>

More on the [website](https://ubald.github.io/psychic-ui/), where you can switch skin sets and themes.

## Skin sets

The same components and the same code, under each skin set. A skin set is a stylesheet plus the skins it registers;
Desk and Float register no skin at all, only rules over the default set's.

<table>
  <tr>
    <th>Default</th>
    <th>Desk</th>
    <th>Float</th>
  </tr>
  <tr>
    <td>
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="https://ubald.github.io/psychic-ui/showcase/buttons-default-dark.png">
        <img alt="Buttons and check boxes under the Default skin set" src="https://ubald.github.io/psychic-ui/showcase/buttons-default-light.png" width="280">
      </picture>
    </td>
    <td>
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="https://ubald.github.io/psychic-ui/showcase/buttons-desk-dark.png">
        <img alt="Buttons and check boxes under the Desk skin set" src="https://ubald.github.io/psychic-ui/showcase/buttons-desk-light.png" width="280">
      </picture>
    </td>
    <td>
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="https://ubald.github.io/psychic-ui/showcase/buttons-float-dark.png">
        <img alt="Buttons and check boxes under the Float skin set" src="https://ubald.github.io/psychic-ui/showcase/buttons-float-light.png" width="280">
      </picture>
    </td>
  </tr>
</table>

## Quick start

A window with a title, a counter, a button and a switch that changes the theme, styled by a stylesheet of its own:

```cpp
#include <format>
#include <memory>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/applications/SDL3Application.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/skins/pivotal/PivotalSkins.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/StyleSheet.hpp>
#include <psychic-ui/themes/pivotal.hpp>

using namespace psychic_ui;

// Rules over the skin set's, written from the current theme (and written again when it changes)
class HelloStyleSheet : public StyleSheet {
public:
    void load(StyleManager *manager) override {
        const Metrics &metrics = manager->theme().metrics;
        manager->style(".content")
               .padding(metrics.space(6))
               .gap(metrics.space(3))
               .alignItems(Align::Start);
        manager->style(".count")
               .color(manager->theme().palette.textSecondary);
    }
};

class HelloWindow : public Window {
public:
    HelloWindow() : Window("Hello, Psychic UI") {
        skinSet<PivotalSkins>();
        loadStyleSheet<HelloStyleSheet>();

        auto &content = appContainer().add<Div>().addClassName("content");
        content.add<Label>("Hello, Psychic UI").addClassName("h3");
        auto &count = content.add<Label>("Not clicked yet").addClassName("count");
        content.add<Button>("Click me")
               .skin("primary")
               .icon("plus")
               .onClick([this, &count] { count.text(std::format("Clicked {} times", ++_clicks)); });
        content.add<Switch>("Light theme")
               .onChange([this](bool light) { theme(light ? themes::pivotalLight : themes::pivotalDark); });
    }

private:
    int _clicks = 0;
};

int main() {
    SDL3Application app;
    app.open(std::make_shared<HelloWindow>());
    app.run();
}
```

`add<T>(...)` creates a child, owned by its parent, and returns it, so setters chain. Swap `PivotalSkins` for
`DeskSkins` (`skins/desk/DeskSkins.hpp`) or `FloatSkins` (`skins/float/FloatSkins.hpp`) to change the whole look.
The [demo app](example/demo/) shows every component, page by page.

## Building

CMake 3.24 or newer and a C++23 compiler: GCC 14+, Clang 18+, Apple Clang 16+, or clang-cl on Windows. Skia, Yoga and
psychic-color are git submodules. CMake builds Skia itself the first time (it needs Ninja and Python 3, and downloads
Skia's pinned `gn` while configuring), which takes a while; after that, builds are quick.

```sh
git clone https://github.com/ubald/psychic-ui.git && cd psychic-ui
git submodule update --init --recursive
cmake --preset debug
cmake --build --preset debug -j"$(nproc)"
```

- **Linux**: Skia uses the system's ICU, HarfBuzz, FreeType and Fontconfig. SDL3 and GLFW are used when installed and
  downloaded otherwise. A Nix flake provides a ready dev shell (`nix develop`, or direnv), but nothing requires it.
- **macOS**: the same, with ICU and HarfBuzz from Homebrew.
- **Windows**: clang-cl from Visual Studio's Build Tools, with ICU and HarfBuzz from vcpkg. From a Developer prompt:
  `cmake --preset windows`, `cmake --build --preset windows`, `ctest --preset windows`.
- `-DPSYCHIC_UI_SKIA_DIR=<dir>` links an already built Skia instead of building one (for instance another build
  directory's `extlib/skia/build`).

[CLAUDE.md](CLAUDE.md) has the details for every platform, the CMake options and the whole architecture.

### The demo

```sh
./build/debug/example/psychic-ui-demo
./build/debug/example/psychic-ui-demo --skin float --theme float-dark --page canvas
./build/debug/example/psychic-ui-demo --skin desk --density touch --page lists
```

`--skin` is `default`, `desk` or `float`; `--theme` a theme's name (`pivotal-dark`, `one-light`, `desk-light`,
`float-dark`...); `--page` a page of the sidebar (`buttons`, `text-input`, `popups`, `motion`...). The skin set and the
theme also switch live from the View menu.

### The tests

```sh
ctest --preset debug
./build/debug/tests/psychic-ui-tests "[style]"   # Catch2, by tag or by name
```

They need no display: the backends are tested on SDL3's dummy driver and GLFW's null platform, with time passed
explicitly.

## Using it in your project

Add it as a submodule (with its own submodules) and, in your CMake project, as a subdirectory. It then builds as a
static library, without its tests and examples:

```sh
git submodule add https://github.com/ubald/psychic-ui.git extern/psychic-ui
git submodule update --init --recursive
```

```cmake
add_subdirectory(extern/psychic-ui)
target_link_libraries(my-app PRIVATE psychic-ui::psychic-ui)
psychic_ui_copy_resources(my-app)   # the bundled fonts, next to the executable
```

Your target builds as C++23 too. On Windows, list `icu` and `harfbuzz` in your own `vcpkg.json` and use the same
release triplet and runtime (see CLAUDE.md).

**Embedding**: an application that already has a window and a loop doesn't need `SDL3Application`. The adapters in
`psychic-ui/applications/` (`SDL3SystemWindow`, `GLFWSystemWindow`) drive a Psychic UI `Window` inside a window you
own; [`example/glfw-host`](example/glfw-host/main.cpp) embeds one in a GLFW app and only wakes up when the UI needs a
frame.

## Status

Psychic UI started in 2017 and is being revived in 2026: Skia, Yoga and SDL are current, and most of what's above is
new. A real application is being built on it, and the API may still change while that happens. There is no
accessibility support yet. Issues and feedback are welcome.

## License

MIT, see [LICENSE](LICENSE). The bundled fonts keep their own licenses (Inter: SIL OFL 1.1, Ubuntu: the Ubuntu
Font Licence, in `res/fonts/`), as do the Lucide icons (ISC, `psychic-ui/icons/LICENSE-Lucide.txt`).

Thanks to everyone who has starred, used or contributed to it.
