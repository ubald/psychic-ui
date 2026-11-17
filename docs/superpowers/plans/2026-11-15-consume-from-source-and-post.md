# Consume from Source and Window::post Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let another CMake project build Psychic UI from its source (`add_subdirectory`, or FetchContent with `FETCHCONTENT_SOURCE_DIR_<NAME>`) so edits to the library rebuild with the app, and let other threads hand work to the UI thread with `Window::post`.

**Architecture:** The root `CMakeLists.txt` stops leaking into a parent project: its extras default to `PROJECT_IS_TOP_LEVEL`, its shared/static choice gets its own option, it exports an alias, it can reuse an existing Skia build, and it gives consumers a function that copies the resources next to their executables. Relative font paths are resolved against the executable's directory instead of the working directory. A consumer project under `tests/consumer/` proves all of it through CTest. `Window::post` is a locked queue that `drawAll` empties first thing; posting wakes the loop through the existing `requestFrame()`/`SystemWindow::wake()` path.

**Tech Stack:** CMake 3.24+, C++23, Catch2 v3, SDL3, GLFW.

**Spec:** this conversation (2026-11-15): the review's item B4 in `docs/review-2026-11.md`, plus a shared Skia build, resources for consumers, and `Window::post`. The motivating consumer is a C++ psychic-lights (vault note `dogfood-psychic-lights`).

## Global Constraints

- CMake minimum stays `3.24`: no `copy_directory_if_different` (3.26).
- The build must work without Nix (CLAUDE.md, Building).
- The library's own top-level build keeps today's defaults: shared library, tests, demo and playground on.
- Defaults that depend on being the top-level project use `PROJECT_IS_TOP_LEVEL`, as `lib/psychic-color` does: `project()` in a subdirectory (psychic-color's) only sets it in that subdirectory's scope, so the root and its other subdirectories still see psychic-ui's value (checked 2026-11-15 with a toy project).
- The UI stays single-threaded: `post` and `requestFrame` are the only `Window` calls allowed from another thread.
- Use `std::function<void()>`, not `std::move_only_function`: Apple Clang 16's libc++ lacks it.
- Commits end with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Every change to the build, the options or the frame loop updates CLAUDE.md in the same commit.

## Review Focus

1. A consumer that sets `BUILD_SHARED_LIBS ON` for its own libraries: psychic-ui stays static and `BUILD_SHARED_LIBS` is still `ON` afterwards (Task 4's consumer sets it).
2. A consumer executable run from a working directory other than its own: the fonts still load (Task 4 runs it from another directory).
3. A post landing between `runPosted` and the moment `updateStylesAndLayout` clears `_frameRequested`: it must still get a frame, so `needsFrame()` reads the queue's own flag (Task 5's "posted from other threads" test, plus the ordering in Step 3).
4. A posted task that throws: the rest of its batch isn't lost (Task 5 test).
5. Tasks still queued when the window is destroyed: destroyed unrun, on the destroying thread (Task 5 test).

---

### Task 1: Stop leaking into a parent project

**Files:**
- Modify: `CMakeLists.txt` (CPack block at lines 15–78, `option(BUILD_SHARED_LIBS …)` at 81–82, `add_library` at 413, the GLAD and strip checks at 454–459, `enable_testing`/subdirectories at 465–469, `feature_summary` at the end)
- Modify: `tests/CMakeLists.txt:1`, `example/CMakeLists.txt:1`, `playground/CMakeLists.txt:1`
- Modify: `CMakePresets.json:25` (asan preset)
- Modify: `CLAUDE.md` (Building: the options bullet)

**Interfaces:**
- Produces: option `PSYCHIC_UI_SHARED` (default `${PROJECT_IS_TOP_LEVEL}`); target alias `psychic-ui::psychic-ui`; `PSYCHIC_UI_BUILD_TESTS`, `PSYCHIC_UI_BUILD_EXAMPLE`, `PSYCHIC_UI_BUILD_PLAYGROUND` default `${PROJECT_IS_TOP_LEVEL}`. Nothing sets a cache `BUILD_SHARED_LIBS` any more.

- [ ] **Step 1: Delete the CPack block and the `PACKAGE_*` variables.** Grep for `PACKAGE_` and `CPACK_` first; delete whichever ones only CPack uses.
- [ ] **Step 2: Replace `option(BUILD_SHARED_LIBS …)` with `option(PSYCHIC_UI_SHARED "Build Psychic UI as a shared library" ${PROJECT_IS_TOP_LEVEL})`.** Then:
  - Create the library with `add_library(psychic-ui ${_type} ${SOURCE_FILES})`, where `_type` is `SHARED` or `STATIC` from that option.
  - Add `add_library(psychic-ui::psychic-ui ALIAS psychic-ui)`.
  - Switch the GLAD export and strip conditions to `PSYCHIC_UI_SHARED`.
  - Keep the fetched dependencies' local `set(BUILD_SHARED_LIBS OFF)` … `unset(BUILD_SHARED_LIBS)` pairs as they are.
  - Change the asan preset's `"BUILD_SHARED_LIBS": "OFF"` to `"PSYCHIC_UI_SHARED": "OFF"`.
- [ ] **Step 3: Default the three extras' options to `${PROJECT_IS_TOP_LEVEL}`.** Call `enable_testing()` only when `PSYCHIC_UI_BUILD_TESTS` is on, and call `feature_summary` only when `PROJECT_IS_TOP_LEVEL`.
- [ ] **Step 4: Verify the top-level build is unchanged.**
  Run: `cmake --preset debug && cmake --build --preset debug -j"$(nproc)" && ctest --preset debug`
  Expected: the configure output still lists tests, example and playground; `build/debug/libpsychic-ui.so` exists; all tests pass.
- [ ] **Step 5: Update CLAUDE.md's options bullet** (`PSYCHIC_UI_SHARED` replaces `BUILD_SHARED_LIBS`; the defaults follow whether psychic-ui is the top-level project) **and commit:** "Keep the build options, the shared library choice and CPack out of a parent project, and add the psychic-ui::psychic-ui alias".

### Task 2: Reuse an existing Skia build

**Files:**
- Modify: `extlib/skia/CMakeLists.txt` (the `SKIA_OUTPUT` line near 99)
- Modify: `CLAUDE.md` (Building: the Skia bullet that tells you to `cp -a` an existing build)

**Interfaces:**
- Produces:
  - Cache `PATH` `PSYCHIC_UI_SKIA_DIR`, default empty. When set, Skia is built into and linked from that directory instead of `${CMAKE_CURRENT_BINARY_DIR}/build`.
  - Internal cache variable `PSYCHIC_UI_SKIA_OUTPUT`: the directory in use. Task 4 reads it.

- [ ] **Step 1: Implement it.** Set `SKIA_OUTPUT` to `PSYCHIC_UI_SKIA_DIR` when it isn't empty, otherwise keep today's default, then `set(PSYCHIC_UI_SKIA_OUTPUT "${SKIA_OUTPUT}" CACHE INTERNAL "")`. The custom command already builds only when a library is missing, so a shared directory that's already full is used as is.
- [ ] **Step 2: Verify the option is used.**
  Run: `cmake -S . -B build/skia-dir-check -DPSYCHIC_UI_SKIA_DIR=$PWD/build/debug/extlib/skia/build -DPSYCHIC_UI_BUILD_TESTS=OFF -DPSYCHIC_UI_BUILD_EXAMPLE=OFF -DPSYCHIC_UI_BUILD_PLAYGROUND=OFF && cmake --build build/skia-dir-check -j"$(nproc)"`
  Expected: the build doesn't run `skia_build.sh` (no gn or ninja output from Skia) and links. Then `rm -rf build/skia-dir-check`.
- [ ] **Step 3: Update CLAUDE.md** (`-DPSYCHIC_UI_SKIA_DIR=<dir>` replaces the `cp -a` advice for new build directories; one directory per compiler and platform; don't build two directories into it at the same time) **and commit:** "Let a build directory use an existing Skia build through PSYCHIC_UI_SKIA_DIR".

### Task 3: Resources for any executable

**Files:**
- Create: `psychic-ui/utils/ExecutablePath.hpp`, `psychic-ui/utils/ExecutablePath.cpp`; add both to `SOURCE_FILES`
- Modify: `psychic-ui/style/StyleManager.hpp`/`.cpp` (`loadFont` at `StyleManager.cpp:124`)
- Modify: `psychic-ui/themes/Theme.hpp:107-109` and `psychic-ui/themes/ubuntu-type.hpp`: drop the leading `../` from every font path (grep the rest of `psychic-ui/` for `"../res`)
- Modify: `CMakeLists.txt` (replace `file(COPY res …)` with the function below), `example/CMakeLists.txt`, `playground/CMakeLists.txt`
- Modify: `CLAUDE.md` (the "Resources … load by relative path" paragraph)
- Test: `tests/text/font_tests.cpp`

**Interfaces:**
- Produces:
  - `std::filesystem::path psychic_ui::executableDirectory()`: the running executable's directory. It reads `/proc/self/exe` on Linux, `_NSGetExecutablePath` on macOS and `GetModuleFileNameW` on Windows, and returns an empty path on failure.
  - `StyleManager &StyleManager::resourceRoot(std::filesystem::path)` and `const std::filesystem::path &StyleManager::resourceRoot() const`, which defaults to `executableDirectory()`.
  - `loadFont` resolves a relative path against `resourceRoot()`. Absolute paths are unchanged.
  - CMake `psychic_ui_copy_resources(<target>)`: a `POST_BUILD` `${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/res" "$<TARGET_FILE_DIR:<target>>/res"`.

- [ ] **Step 1: Write the failing tests** in `tests/text/font_tests.cpp`:
  - `TEST_CASE("The executable directory holds the running executable", "[font][resources]")`: `CHECK(std::filesystem::exists(executableDirectory() / "psychic-ui-tests"))`.
  - `TEST_CASE("A relative font path resolves against the resource root", "[font][resources]")`:
    - set the root to the source directory: `std::filesystem::path(PSYCHIC_UI_TEST_RES).parent_path().parent_path()`, since the macro ends in `res/`;
    - `loadFont("Inter", "res/fonts/Inter/Inter-Regular.ttf")`;
    - assert the family loaded, the way the existing font tests do;
    - assert that a fresh manager's `resourceRoot()` equals `executableDirectory()`.
- [ ] **Step 2: Run them and see them fail.**
  Run: `cmake --build --preset debug -j"$(nproc)" && ./build/debug/tests/psychic-ui-tests "[resources]"`
  Expected: a compile error (`executableDirectory`, `resourceRoot` undeclared).
- [ ] **Step 3: Implement the Interfaces above.** Then make the library's own executables use the function: call `psychic_ui_copy_resources` on `psychic-ui-demo`, `psychic-ui-glfw-host` and `psychic-ui-playground`, and keep example's `file(COPY fonts …)`.
- [ ] **Step 4: Run the tests and the demo.**
  Run: `ctest --preset debug`, then, headless as CLAUDE.md describes (Xvfb, `WAYLAND_DISPLAY` unset), `cd /tmp && timeout 3 <repo>/build/debug/example/psychic-ui-demo --no-fps`.
  Expected: all tests pass, and the demo prints no font errors on stderr when started from `/tmp`.
- [ ] **Step 5: Update CLAUDE.md** (fonts resolve against the executable's directory; executables get `res/` through `psychic_ui_copy_resources`) **and commit:** "Resolve relative font paths against the executable's directory and give executables their resources with psychic_ui_copy_resources".

### Task 4: A consumer project that proves it

**Files:**
- Create: `tests/consumer/CMakeLists.txt`, `tests/consumer/main.cpp`
- Modify: `tests/CMakeLists.txt` (new option and test)
- Modify: `.github/workflows/ci.yml` (Linux job's Configure step only: add `-DPSYCHIC_UI_TEST_CONSUMER=ON`)
- Modify: `README.md` (new section "Using Psychic UI in your project"), `CLAUDE.md` (Tests), `docs/review-2026-11.md` (tick B4)

**Interfaces:**
- Consumes: `psychic-ui::psychic-ui`, `PSYCHIC_UI_SHARED` (Task 1); `PSYCHIC_UI_SKIA_DIR`, `PSYCHIC_UI_SKIA_OUTPUT` (Task 2); `psychic_ui_copy_resources`, `StyleManager::resourceRoot` (Task 3).
- Produces: option `PSYCHIC_UI_TEST_CONSUMER` (default `OFF`); CTest test `psychic-ui-consumer`.

- [ ] **Step 1: Write the consumer.** `tests/consumer/CMakeLists.txt`:
  - `project(psychic-ui-consumer C CXX)`, then `set(BUILD_SHARED_LIBS ON)`, standing in for an app that builds its own libraries shared;
  - `add_subdirectory(${PSYCHIC_UI_SOURCE_DIR} psychic-ui)`;
  - then `FATAL_ERROR` if any of these holds:
    - `TARGET psychic-ui-tests`, `psychic-ui-demo` or `psychic-ui-playground` exists;
    - `get_target_property(... psychic-ui TYPE)` isn't `STATIC_LIBRARY`;
    - `BUILD_SHARED_LIBS` isn't still `ON`;
    - `DEFINED CACHE{BUILD_SHARED_LIBS}`;
  - `add_executable(consumer-app main.cpp)`, linked `PRIVATE psychic-ui::psychic-ui`, with `psychic_ui_copy_resources(consumer-app)`.

  `main.cpp`:
  - make a `Window` with no system window and install `DefaultSkins`;
  - return 1 if the theme's regular font isn't loaded (Task 3's check);
  - `post` a lambda that sets a flag, call `runPosted()` (Task 5), and return 1 unless the flag is set; return 0 otherwise.

  If Task 5 isn't merged yet, leave the post check out and add it in Task 5.
- [ ] **Step 2: Register the test** in `tests/CMakeLists.txt` under `if (PSYCHIC_UI_TEST_CONSUMER)`:
  - `add_test(NAME psychic-ui-consumer COMMAND ${CMAKE_CTEST_COMMAND} --build-and-test ${PROJECT_SOURCE_DIR}/tests/consumer ${CMAKE_CURRENT_BINARY_DIR}/consumer --build-generator ${CMAKE_GENERATOR} --build-options -DPSYCHIC_UI_SOURCE_DIR=${PROJECT_SOURCE_DIR} -DPSYCHIC_UI_SKIA_DIR=${PSYCHIC_UI_SKIA_OUTPUT} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_BUILD_TYPE=Debug --test-command ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/consumer/consumer-app)`. The `chdir` runs it from a directory other than its own (Review Focus 2).
  - Set the properties `TIMEOUT 1800` and `ENVIRONMENT CMAKE_BUILD_PARALLEL_LEVEL=<cores>`, with the core count from `cmake_host_system_information(RESULT … QUERY NUMBER_OF_LOGICAL_CORES)`.
- [ ] **Step 3: Run it.**
  Run: `cmake --preset debug -DPSYCHIC_UI_TEST_CONSUMER=ON && cmake --build --preset debug -j"$(nproc)" && ctest --preset debug -R psychic-ui-consumer --output-on-failure`
  Expected: PASS, without Skia being built again.
  Then reconfigure with `-DPSYCHIC_UI_TEST_CONSUMER=OFF`, so the local `ctest --preset debug` stays fast.
- [ ] **Step 4: Prove the test can fail.** Temporarily restore `option(BUILD_SHARED_LIBS … ON)` in the root and rerun Step 3.
  Expected: FAIL with the consumer's `FATAL_ERROR` message. Then revert.
- [ ] **Step 5: Document it.**
  - README: both ways to consume the library: `add_subdirectory(../psychic-ui psychic-ui)`, and `FetchContent_Declare(psychic_ui GIT_REPOSITORY … GIT_TAG …)` with `-DFETCHCONTENT_SOURCE_DIR_PSYCHIC_UI=<checkout>` for local edits. Plus `PSYCHIC_UI_SKIA_DIR`, `psychic_ui_copy_resources`, and the submodules the checkout needs.
  - CLAUDE.md Tests: the consumer test, off by default, on in the Linux CI job.
  - Tick B4.
- [ ] **Step 6: Commit:** "Prove the library builds from source inside another project with a consumer test, on in Linux CI".

### Task 5: `Window::post`

**Files:**
- Modify: `psychic-ui/Window.hpp` (region Frames, near `requestFrame` at line 144; members near line 682), `psychic-ui/Window.cpp` (`drawAll` at 478, `needsFrame` at 532, `requestFrame` at 582, the destructor)
- Modify: `tests/backends/fake_system_window.hpp` (count `wake()` calls)
- Create: `tests/backends/post_tests.cpp`; add it to `tests/CMakeLists.txt`
- Modify: `tests/backends/sdl3_tests.cpp` (one test after the one at line 721)
- Modify: `CLAUDE.md` (Architecture: "Frames and idling"; the "Time is the tests' own" paragraph lists `runPosted`)

**Interfaces:**
- Produces:
  - `void Window::post(std::function<void()> task)`. It may be called from any thread. It queues the task and calls `requestFrame()`.
  - `void Window::runPosted()`, UI thread only. It runs the queued tasks in posting order. A task posted while it runs waits for the next call. If a task throws, the rest of its batch goes back to the front of the queue and the exception propagates.
  - `drawAll` calls `runPosted()` first, before `updateGestures`.
  - `needsFrame()` is true while the queue isn't empty.
  - On destruction, queued tasks are destroyed without running. The caller must stop posting before the window is destroyed.
  - `FakeSystemWindow::wakes` (an `int`).

- [ ] **Step 1: Write the failing tests** in `tests/backends/post_tests.cpp`, tag `[post]`, each on a `TestWindow` with a `FakeSystemWindow` laid out once:
  - `"Posted tasks run in posting order at the next runPosted, not before"`: post the pushes of 1 and 2 into a vector. Before `runPosted`: `CHECK(order.empty())` and `CHECK(window.needsFrame())`. After: `CHECK(order == std::vector{1, 2})`; then after `updateStylesAndLayout()`, `CHECK_FALSE(window.needsFrame())`.
  - `"A task posted by a task runs at the next runPosted"`: the first call runs only the outer task, then `CHECK(window.needsFrame())`; the second runs the inner one.
  - `"Tasks posted from other threads all run, waking the system window once until the next frame"`: 4 `std::thread`s × 1000 posts of `++count` (a plain `int`, since tasks run on the test thread), joined. Then `CHECK(fake.wakes == 1)` and, after `runPosted`, `CHECK(count == 4000)`.
  - `"A post after runPosted still needs a frame after the layout clears the request"`: `runPosted()`, then `post(...)`, then `updateStylesAndLayout()`, then `CHECK(window.needsFrame())`. This pins Review Focus 3.
  - `"A task that throws leaves the rest of its batch for the next runPosted"`: post a, then a throw of `std::runtime_error`, then c. `CHECK_THROWS_AS(window.runPosted(), std::runtime_error)`, then a has run and c hasn't. After a second `runPosted`, c has run.
  - `"Tasks still queued when the window goes are destroyed without running"`: post a lambda that captures a `shared_ptr` sentinel and sets a flag, keeping a `weak_ptr` to the sentinel. Destroy the window. `CHECK(weak.expired())` and `CHECK_FALSE(ran)`.

  In `sdl3_tests.cpp`:
  - `TEST_CASE("SDL3: a post from another thread wakes a loop waiting for events", "[backend][sdl3][frames][post]")`: flush events, then `std::thread([&] { sdl.window->post([] {}); }).join()`. `REQUIRE(SDL_WaitEventTimeout(&event, 1000))` and `CHECK(event.type == SDL3SystemWindow::wakeEventType())`.
- [ ] **Step 2: Run them and see them fail.**
  Run: `cmake --build --preset debug -j"$(nproc)" && ./build/debug/tests/psychic-ui-tests "[post]"`
  Expected: a compile error (`post`, `runPosted`, `wakes` undeclared).
- [ ] **Step 3: Implement the Interfaces.**
  - Members: `std::mutex _postedMutex`, `std::vector<std::function<void()>> _posted`, `std::atomic<bool> _hasPosted{false}`.
  - `post` pushes under the lock, sets `_hasPosted`, unlocks, then calls `requestFrame()`.
  - `runPosted` swaps the vector out and clears `_hasPosted` under the lock, then runs the batch outside the lock. If a task throws, it puts the remainder back at the front under the lock, sets `_hasPosted`, and rethrows.
  - `needsFrame` ORs in `_hasPosted`. This closes the race in Review Focus 3: `updateStylesAndLayout` clears `_frameRequested` after `runPosted`, so the request flag alone can't be trusted.
  - Update `requestFrame`'s doc comment to say "one of the two calls that may come from another thread", and give `post`'s doc comment the lifetime rule: stop posting before the window is destroyed.
- [ ] **Step 4: Run the tests.**
  Run: `ctest --preset debug`
  Expected: all pass, including `[post]`. Then build the asan preset and run `"[post]"` from `build/asan/tests` as CLAUDE.md describes; expected: no ASan or UBSan reports.
- [ ] **Step 5: Add the post check to the consumer's `main.cpp`** if Task 4 left it out, and rerun Task 4 Step 3.
- [ ] **Step 6: Update CLAUDE.md**:
  - Frames and idling: `post`, `runPosted` first in `drawAll`, `needsFrame` including the queue, and the lifetime rule.
  - The "Time is the tests' own" paragraph: tests call `runPosted` themselves.
  - Then commit: "Add Window::post for other threads to hand work to the UI thread, run first in each frame".
