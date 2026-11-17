# Windows Build Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build Psychic UI, its tests and its demo on Windows with Visual Studio Build Tools 2026 and clang-cl, from CLion or the command line, with every test passing, while the Linux and macOS builds stay as they are.

**Architecture:** Skia's build script goes away: CMake downloads gn (pinned, hashed) into the build directory, writes the args to a file gn reads, and runs gn and Ninja from its custom command on every platform, so the build never writes into Skia's checkout. Windows gets its own branch in `extlib/skia/CMakeLists.txt` (clang-cl through gn's `clang_win`, DirectWrite fonts) and in the font manager. ICU and HarfBuzz, which Windows doesn't have, come from vcpkg in manifest mode (`vcpkg.json`), release only, and everything links the release runtime (`/MD`) in every configuration, since Skia and vcpkg's libraries are release builds. A `windows` preset holds all of that, so CLion and the command line configure the same way.

**Tech Stack:** CMake 3.24+, C++23, clang-cl (Build Tools 2026's LLVM), MSVC 14.52 libraries and Windows SDK 10.0.26100, vcpkg (Build Tools' copy), Ninja, Python 3, Skia m159 with gn.

**Spec:** this conversation (2026-11-17): build the Windows version from CLion; the review's Windows bullet in B8 (`docs/review-2026-11.md:270`) lists what breaks: `-Wextra`, Skia built by a bash script, `M_LOG10E`, no DLL exports.

## Global Constraints

- The Linux and macOS builds don't change behaviour: same Skia libraries, gn args and defines (apart from the args' file format), same tests passing. CI stays Linux and macOS only (no Windows job: free minutes are limited; vault note `ci-credits`). No pushes just to run CI.
- The build must keep working without Nix (CLAUDE.md, Building); nothing Windows-specific may require Nix either.
- Windows compiler: clang-cl from Build Tools 2026 (`VC/Tools/Llvm/x64/bin/clang-cl.exe`), for Skia and for everything else. The root `CMakeLists.txt` version check stays as it is (clang-cl reports `Clang`); MSVC's `cl` is not a goal of this plan.
- Every target links the release dynamic runtime: `CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`, in Debug too, since Skia and vcpkg's libraries are release builds (a `/MDd` object next to them fails with LNK2038).
- vcpkg triplet `x64-windows-release` (community triplet: DLLs, dynamic runtime, release only). The manifest pins a `builtin-baseline` (Build Tools' vcpkg requires one).
- `PSYCHIC_UI_SHARED` is not supported on Windows yet (no exports): it defaults to `OFF` there and configure stops with a fatal error if it is `ON`.
- Never open visible windows: on Windows, run the tests (SDL's `dummy` driver, GLFW's null platform) but not the demo; Ubald runs the demo himself.
- Never delete files outside the WSL checkout (vault note `no-deletes-outside-repo`): the Windows clone is updated with `git fetch`/`checkout`, never cleaned with `rm`; a build directory to start over goes in a new directory, and the old one is listed for Ubald.
- Simplicity first (vault note `project-goal`): no helper scripts that hide steps; Windows adds only what it can't build without.
- Every change to the build updates CLAUDE.md in the same commit.
- Commits end with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.

### Building on Windows from WSL

The Windows clone is `C:\src\psychic-ui` (`/mnt/c/src/psychic-ui` from WSL), with Git for Windows, a remote `wsl` pointing at the WSL repository (`//wsl.localhost/NixOS/home/ubald/src/psychic-ui`). A task's branch reaches it with `git -c safe.directory=* fetch wsl <branch>` and `git checkout <branch>` (run through `cmd.exe`). Builds never run from `\\wsl.localhost` paths.

Run Windows commands from `/mnt/c` (cmd.exe refuses a UNC working directory), inside the Build Tools environment:

```sh
cd /mnt/c && cmd.exe /c 'call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul && cd /d C:\src\psychic-ui && cmake --preset windows && cmake --build --preset windows && ctest --preset windows'
```

`vcvars64.bat` puts clang-cl, CMake, Ninja and the SDK on the path and sets `VCPKG_ROOT` (Build Tools' vcpkg) and `VCINSTALLDIR`.

## Review Focus

1. A Debug build on Windows next to release Skia and vcpkg libraries: links without LNK2038 runtime or `_ITERATOR_DEBUG_LEVEL` mismatches (Task 3 builds the Debug `windows` preset).
2. Paths with spaces and backslashes (`C:\Program Files (x86)\…`) in gn's `clang_win`, `win_vc` and include flags: the args reach gn intact (Task 1 writes them to `args.gn` and lets CMake quote the commands; Task 3 builds from Build Tools' path).
3. The Linux build after the script rewrite: the same gn args and the same six libraries, the tests still pass (Task 1 compares `gn args <out> --list --short` before and after).
4. Bundled fonts on Windows: Inter loads from its file and isn't silently replaced by a system fallback (Task 3 runs `tests/text/font_tests.cpp`, which checks exactly that).
5. A fresh Windows clone: the first configure downloads `gn.exe` and builds Skia with no step done by hand beyond the prerequisites (Task 3 configures `C:\src\psychic-ui` from scratch).

---

### Task 1: Build Skia from CMake, without a script or writes into Skia's checkout

**Files:**
- Delete: `extlib/skia/skia_build.sh.in` (our script, not Skia's: `extlib/skia/` is this repository's; Skia's checkout is `extlib/external/skia/`)
- Modify: `extlib/skia/CMakeLists.txt` (`find_program(SKIA_NINJA …)` at 10-13, the `configure_file` at 102, the custom command at 122-126)
- Modify: `.github/workflows/ci.yml:49,94` (the cache keys hash `skia_build.sh.in`: hash only `extlib/skia/CMakeLists.txt`)
- Modify: `CLAUDE.md` (Building, the Skia bullet; nix-ld still runs the downloaded gn)

**Interfaces:**
- Produces: nothing is written into `extlib/external/skia` by the build any more: no `bin/fetch-gn` (it extracts `bin/gn` and `third_party/gn/gn` there) and no `bin/fetch-ninja` (it writes `third_party/ninja/`).
- Produces: gn downloaded by CMake at configure time into `${CMAKE_CURRENT_BINARY_DIR}/gn/` (`file(DOWNLOAD … EXPECTED_HASH SHA256=…)` then `file(ARCHIVE_EXTRACT)`), only when Skia is built here (empty `PSYCHIC_UI_SKIA_DIR`) and the binary isn't there yet. The URL and revision are `fetch-gn`'s: `https://chrome-infra-packages.appspot.com/dl/gn/gn/<os>-<cpu>/+/git_revision:b2afae122eeb6ce09c52d63f67dc53fc517dbdc8`, with `<os>-<cpu>` one of `linux-amd64`, `mac-amd64`, `mac-arm64`, `windows-amd64` (others: `FATAL_ERROR` naming the platform). One SHA-256 per platform, computed by downloading each zip once; a comment says the revision is `bin/fetch-gn`'s and to update both together when Skia moves. The binary is `gn${CMAKE_EXECUTABLE_SUFFIX}`.
- Produces: Ninja is required: `find_program(SKIA_NINJA ninja REQUIRED)` (the flake, both CI jobs and Build Tools provide it).
- Produces: the gn args, one per line, written to `${SKIA_OUTPUT}/args.gn` at configure time (`file(WRITE)`, only when Skia is built here: a prebuilt Skia is never written to); `gn gen` reads an `args.gn` already in its output directory, so no args go on a command line.
- Produces: the custom command runs, with `VERBATIM` and `WORKING_DIRECTORY ${SKIA_SOURCE_DIR}`: `<gn> gen ${SKIA_OUTPUT}`, then `${SKIA_NINJA} -C ${SKIA_OUTPUT} skia modules/skparagraph modules/skshaper modules/skunicode`. Its outputs, `USES_TERMINAL` and the rest stay as they are (a change of args doesn't rebuild Skia, as today).

- [ ] **Step 1: Record the current gn args.** `/home/ubald/src/psychic-ui/extlib/external/skia/bin/gn args /home/ubald/src/psychic-ui/build/debug/extlib/skia/build --list --short > <scratchpad>/args-before.txt`.
- [ ] **Step 2: Implement the download, the `args.gn` file and the commands**, and delete `skia_build.sh.in`.
- [ ] **Step 3: Rebuild Skia from scratch in a new build directory of the worktree, whose Skia checkout has no `bin/gn`.** `cmake --preset debug -B build/skia-cmake && cmake --build build/skia-cmake --target skia-build`. Expected: the six libraries in `build/skia-cmake/extlib/skia/build`, and `git -C extlib/external/skia status --short --ignored` empty afterwards.
- [ ] **Step 4: Compare the args.** `build/skia-cmake/extlib/skia/gn/gn args build/skia-cmake/extlib/skia/build --list --short` against `args-before.txt`: identical.
- [ ] **Step 5: Build and run the tests on it.** `cmake --build build/skia-cmake -j"$(nproc)" && ctest --test-dir build/skia-cmake --output-on-failure`. Expected: all pass.
- [ ] **Step 6: Update the CI cache keys and CLAUDE.md**, then commit. List `build/skia-cmake` for Ubald to delete.

### Task 2: Make the library portable to MSVC-style compilers

**Files:**
- Delete: `extlib/glad/CMakeLists.txt`
- Modify: `CMakeLists.txt` (`add_subdirectory(extlib/glad)` at 63-64, the GLAD block at 422-429, the warnings at 389, `PSYCHIC_UI_SHARED` at 43)
- Modify: `psychic-ui/components/Range.hpp:285,288`
- Modify: `docs/review-2026-11.md` (B5: GLAD done; B8: the Windows bullet)
- Modify: `CLAUDE.md` (Building, the options bullet)

**Interfaces:**
- Produces: on `WIN32`, `PSYCHIC_UI_SHARED` defaults to `OFF`, and `ON` stops configure with `FATAL_ERROR` "Psychic UI can't be built as a shared library on Windows yet: it exports no symbols. Configure with -DPSYCHIC_UI_SHARED=OFF.". Elsewhere it defaults to `PROJECT_IS_TOP_LEVEL` as now.
- Produces: the library's warnings are `/W4` when `CMAKE_CXX_COMPILER_FRONTEND_VARIANT` is `MSVC`, else `-Wall -Wextra` (as `lib/psychic-color` does with `MSVC`, but on the frontend so that clang-cl gets `/W4`).

- [ ] **Step 1: Remove GLAD.** Delete `extlib/glad/CMakeLists.txt`, the `add_subdirectory` and the `PSYCHIC_UI_USE_GLAD` block. Nothing in the sources uses it (`grep -rn GLAD psychic-ui example playground tests` is empty): the GL interface comes from the system window's procs. On Windows it would fetch GLAD's unpinned `master`.
- [ ] **Step 2: Replace `M_LOG10E`** with `std::numbers::log10e_v<float>` (`<numbers>`), which needs no define.
- [ ] **Step 3: Warnings by frontend, and the `PSYCHIC_UI_SHARED` rule on Windows**, as in Interfaces.
- [ ] **Step 4: Build and test on Linux.** `cmake --preset debug && cmake --build --preset debug -j"$(nproc)" && ctest --preset debug`. Expected: all pass, no new warnings.
- [ ] **Step 5: Update the review and CLAUDE.md, then commit.** In B5, note GLAD as removed (the item stays open for the rest); in B8's Windows bullet, strike `-Wextra` and `M_LOG10E` and say shared builds are refused on Windows.

### Task 3: Build and test on Windows

Depends on Tasks 1 and 2 (merged into its branch first).

**Files:**
- Create: `vcpkg.json`
- Modify: `CMakePresets.json` (a `windows` configure, build and test preset)
- Modify: `extlib/skia/CMakeLists.txt` (Windows gn args, link libraries and defines; Freetype only on Linux/BSD)
- Modify: `psychic-ui/style/StyleManager.cpp:7-12,114-121` (DirectWrite on Windows)
- Modify: `CLAUDE.md` (Building: a Windows paragraph; Tests: the Windows command)
- Modify: `docs/review-2026-11.md` (B8: the Windows bullet ticked off)

**Interfaces:**
- Consumes: Skia's build as CMake commands (Task 1), the portable `CMakeLists.txt` (Task 2).
- Produces: `vcpkg.json`: `name` `psychic-ui`, dependencies `icu` and `harfbuzz` (`"default-features": false`), `builtin-baseline` the current `HEAD` of `https://github.com/microsoft/vcpkg` (`git ls-remote … HEAD`). vcpkg only reads it when its toolchain file is in use, so other platforms ignore it.
- Produces: the `windows` configure preset: inherits `debug`, `"generator": "Ninja"`, `"condition": {"type": "equals", "lhs": "${hostSystemName}", "rhs": "Windows"}`, `"toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"`, cache variables `CMAKE_C_COMPILER` and `CMAKE_CXX_COMPILER` `clang-cl`, `VCPKG_TARGET_TRIPLET` `x64-windows-release`, `CMAKE_MSVC_RUNTIME_LIBRARY` `MultiThreadedDLL`; build and test presets of the same name.
- Produces: on Windows, gn gets no `cc`/`cxx` but `clang_win` (the compiler's directory two levels up, the LLVM root holding `bin/clang-cl.exe`) when the compiler is clang-cl, and `win_vc` from `$ENV{VCINSTALLDIR}` (trailing separator removed) when it is set, else gn finds it with vswhere; `extra_ldflags` is left out. Skia's library names come from `CMAKE_STATIC_LIBRARY_PREFIX`/`SUFFIX` already (`skia.lib`).
- Produces: `StyleManager::fontManager()` returns `SkFontMgr_New_DirectWrite()` (`include/ports/SkTypeface_win.h`) under `_WIN32`.

- [ ] **Step 1: Prerequisites (Ubald).** Python 3 installed for Windows (`winget install Python.Python.3.13`; the `python.exe` in `WindowsApps` is the Store's stub and doesn't count). Check from WSL that `python --version` works inside the `vcvars64.bat` environment.
- [ ] **Step 2: Make the Windows clone.** `git clone` the GitHub origin into `C:\src\psychic-ui` with Git for Windows, add the `wsl` remote, fetch this task's branch, `git submodule update --init --recursive` (Skia is large: minutes).
- [ ] **Step 3: Write `vcpkg.json`, the preset, the Skia branch and the font manager.** For Skia's link libraries and defines on Windows, build once, then read them from gn: `bin\gn.exe desc <out> //:skia libs` and `… defines`; add the Windows-only ones under `$<$<PLATFORM_ID:Windows>:…>` (defines) and `if (WIN32)` (libraries, beside the existing `if (APPLE)`). Change the Freetype/Fontconfig `else ()` to `elseif (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD")`.
- [ ] **Step 4: Configure, build and test on Windows** with the command in "Building on Windows from WSL". Expected: configure prints vcpkg installing `icu` and `harfbuzz` on its first run, Skia builds, and `ctest --preset windows` passes every test. A test failing on Windows only is reported with its output before any test is changed: if the behaviour is right and the test assumed Linux (a FreeType metric, a path separator), fix the test; otherwise fix the library.
- [ ] **Step 5: Check the Linux build still configures and passes** (`cmake --preset debug && cmake --build --preset debug -j"$(nproc)" && ctest --preset debug`).
- [ ] **Step 6: Document and commit.** CLAUDE.md: the prerequisites (Build Tools 2026 with "Desktop development with C++" and "C++ Clang tools for Windows", Python 3), the `windows` preset, `/MD` everywhere and why, release-only vcpkg libraries, no shared library, and CLion's setup (a Visual Studio toolchain on Build Tools, `amd64`, with the `windows` preset enabled). Tick B8's Windows bullet.

---

After the branch is merged: a vault note in `~/vault/projects/psychic-ui/` with the decisions (clang-cl, `/MD` everywhere, vcpkg release-only, static only on Windows, no Windows CI).
