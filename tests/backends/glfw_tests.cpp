#include <array>
#include <chrono>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include <psychic-ui/psychic-ui.hpp>
#include <psychic-ui/style/Values.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <psychic-ui/applications/GLFWSystemWindow.hpp>
#include "fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    // What the host's own callbacks saw
    int hostKeys    = 0; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables): GLFW's C callbacks capture nothing
    int hostButtons = 0; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables): GLFW's C callbacks capture nothing

    void hostKeyCallback(GLFWwindow * /*window*/, int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/) { ++hostKeys; }
    void hostButtonCallback(GLFWwindow * /*window*/, int /*button*/, int /*action*/, int /*mods*/) { ++hostButtons; }

    /**
     * A host on GLFW's null platform: real GLFW windows, no display, no GL context.
     * The adapter's callbacks are called directly, as GLFW would.
     */
    struct GLFWHost {
        GLFWwindow                  *glfwWindow{nullptr};
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        sk_sp<SkSurface>            surface{SkSurfaces::Raster(SkImageInfo::MakeN32Premul(800, 600))};

        GLFWHost() {
            hostKeys    = 0;
            hostButtons = 0;
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_NULL);
            REQUIRE(glfwInit());
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindow = glfwCreateWindow(800, 600, "Host", nullptr, nullptr);
            REQUIRE(glfwWindow != nullptr);
            glfwSetKeyCallback(glfwWindow, hostKeyCallback);
            glfwSetMouseButtonCallback(glfwWindow, hostButtonCallback);
        }

        GLFWHost(const GLFWHost &) = delete;
        GLFWHost &operator=(const GLFWHost &) = delete;
        GLFWHost(GLFWHost &&) = delete;
        GLFWHost &operator=(GLFWHost &&) = delete;

        ~GLFWHost() {
            glfwDestroyWindow(glfwWindow);
            glfwTerminate();
        }

        void layout() {
            window->layout(surface->getCanvas());
        }

        // The callback installed right now
        [[nodiscard]] GLFWkeyfun keyCallback() const {
            auto current = glfwSetKeyCallback(glfwWindow, nullptr);
            glfwSetKeyCallback(glfwWindow, current);
            return current;
        }

        [[nodiscard]] GLFWmousebuttonfun buttonCallback() const {
            auto current = glfwSetMouseButtonCallback(glfwWindow, nullptr);
            glfwSetMouseButtonCallback(glfwWindow, current);
            return current;
        }

        [[nodiscard]] GLFWcursorposfun cursorCallback() const {
            auto current = glfwSetCursorPosCallback(glfwWindow, nullptr);
            glfwSetCursorPosCallback(glfwWindow, current);
            return current;
        }
    };
}

TEST_CASE("GLFW: the adapter chains to the host's callbacks and gives them back", "[backend][glfw]") {
    GLFWHost host;
    auto &div = host.window->appContainer().add<Div>();
    div.width(100.0f).height(100.0f);

    int keys  = 0;
    int downs = 0;
    div.onKeyPress([&](Key key, Mod mod) {
        ++keys;
        CHECK(key == Key::A);
        CHECK(mod.ctrl);
    });
    div.onMousePress([&]() { ++downs; });

    {
        const GLFWSystemWindow adapter(host.window, host.glfwWindow);
        CHECK(host.window->windowWidth() == 800);
        host.layout();
        host.window->requestFocus(&div);

        CHECK(host.keyCallback() != hostKeyCallback);
        host.keyCallback()(host.glfwWindow, GLFW_KEY_A, 0, GLFW_PRESS, GLFW_MOD_CONTROL);
        CHECK(hostKeys == 1);
        CHECK(keys == 1);

        host.cursorCallback()(host.glfwWindow, 50.0, 50.0);
        host.buttonCallback()(host.glfwWindow, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
        CHECK(hostButtons == 1);
        CHECK(downs == 1);
    }

    // The host has its own callbacks back, and they no longer reach the UI
    CHECK(host.keyCallback() == hostKeyCallback);
    CHECK(host.buttonCallback() == hostButtonCallback);
    CHECK(host.cursorCallback() == nullptr);
    host.keyCallback()(host.glfwWindow, GLFW_KEY_A, 0, GLFW_PRESS, 0);
    CHECK(hostKeys == 2);
    CHECK(keys == 1);
    CHECK(host.window->systemWindow() == nullptr);
}

TEST_CASE("GLFW: a window has one adapter at a time", "[backend][glfw]") {
    const GLFWHost host;
    const GLFWSystemWindow adapter(host.window, host.glfwWindow);
    CHECK_THROWS_AS(GLFWSystemWindow(std::make_shared<TestWindow>(), host.glfwWindow), std::invalid_argument);
}

TEST_CASE("GLFW: drops, clipboard and window dragging", "[backend][glfw]") {
    const GLFWHost host;
    const GLFWSystemWindow adapter(host.window, host.glfwWindow);

    auto drop = glfwSetDropCallback(host.glfwWindow, nullptr);
    glfwSetDropCallback(host.glfwWindow, drop);
    std::array<const char *, 2> paths{"/tmp/a.txt", "/tmp/b.png"};
    drop(host.glfwWindow, 2, paths.data());
    CHECK(host.window->dropped == std::vector<std::string>{"/tmp/a.txt", "/tmp/b.png"});

    host.window->setClipboardText("from Psychic UI ✓");
    CHECK(host.window->clipboardText() == "from Psychic UI ✓");

    // Only Wayland can't move windows from inside
    CHECK(glfwGetPlatform() == GLFW_PLATFORM_NULL);
    CHECK(host.window->supportsDrag());
}

TEST_CASE("GLFW: no touch input, the window stays in pointer mode", "[backend][glfw][touch]") {
    GLFWHost host;
    const GLFWSystemWindow adapter(host.window, host.glfwWindow);
    host.layout();
    host.cursorCallback()(host.glfwWindow, 50.0, 50.0);
    host.buttonCallback()(host.glfwWindow, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
    host.buttonCallback()(host.glfwWindow, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
    CHECK(host.window->inputMode() == InputMode::Pointer);

    // A host with touch input of its own hands it to the window; the adapter leaves the mode alone
    host.window->handleTouchPress(50.0f, 50.0f, std::chrono::seconds(1));
    host.window->handleTouchRelease(50.0f, 50.0f, std::chrono::seconds(1));
    host.cursorCallback()(host.glfwWindow, 50.0, 50.0);
    CHECK(host.window->inputMode() == InputMode::Touch);
}

namespace {
    /**
     * A monitor's content scale, made up: the null platform's is 1
     */
    struct ScaledAdapter : GLFWSystemWindow {
        using GLFWSystemWindow::GLFWSystemWindow;
        float scale{1.0f};

        [[nodiscard]] float contentScale() const override { return scale; }
    };
}

TEST_CASE("GLFW: at Windows' 200 %, the window is laid out in logical units and the mouse converted", "[backend][glfw][scale]") {
    // Window units are pixels: the framebuffer is the window's size, the content scale apart
    GLFWHost host;
    auto &div = host.window->appContainer().add<Div>();
    div.width(100.0f).height(100.0f);
    ScaledAdapter adapter(host.window, host.glfwWindow);
    adapter.scale = 2.0f;
    auto contentScale = glfwSetWindowContentScaleCallback(host.glfwWindow, nullptr);
    glfwSetWindowContentScaleCallback(host.glfwWindow, contentScale);
    REQUIRE(contentScale != nullptr);
    contentScale(host.glfwWindow, 2.0f, 2.0f);
    CHECK(host.window->windowWidth() == 400);
    CHECK(host.window->windowHeight() == 300);
    CHECK(adapter.getPixelRatio() == 2.0f);
    host.layout();

    float pressX = -1;
    div.onMousePress([&](const MouseEvent &event) { pressX = event.x; });
    int divX = 0;
    int divY = 0;
    div.localToGlobal(divX, divY);
    host.cursorCallback()(host.glfwWindow, 81.0, 81.0); // 40.5 units
    host.buttonCallback()(host.glfwWindow, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
    CHECK(pressX == Catch::Approx(40.5f - static_cast<float>(divX)));
    host.buttonCallback()(host.glfwWindow, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);

    // A 320×240 window asked for is 640×480 pixels
    adapter.setSize(320, 240);
    int width  = 0;
    int height = 0;
    glfwGetWindowSize(host.glfwWindow, &width, &height);
    CHECK(width == 640);
    CHECK(height == 480);

    // Back on a 100 % monitor
    adapter.scale = 1.0f;
    contentScale(host.glfwWindow, 1.0f, 1.0f);
    CHECK(host.window->windowWidth() == 640);
    CHECK(adapter.getPixelRatio() == 1.0f);
}

TEST_CASE("GLFW keys map to Psychic UI keys", "[keyboard][glfw]") {
    CHECK(GLFWSystemWindow::mapKey(GLFW_KEY_A) == Key::A);
    CHECK(GLFWSystemWindow::mapKey(GLFW_KEY_Z) == Key::Z);
    CHECK(GLFWSystemWindow::mapKey(GLFW_KEY_0) == Key::ZERO);
    CHECK(GLFWSystemWindow::mapKey(GLFW_KEY_9) == Key::NINE);
    CHECK(GLFWSystemWindow::mapKey(GLFW_KEY_F1) == Key::F1);
    CHECK(GLFWSystemWindow::mapKey(GLFW_KEY_F25) == Key::F25);
    CHECK(GLFWSystemWindow::mapKey(GLFW_KEY_KP_0) == Key::KP_0);
    CHECK(GLFWSystemWindow::mapKey(GLFW_KEY_KP_9) == Key::KP_9);
    CHECK(GLFWSystemWindow::mapKey(GLFW_KEY_ENTER) == Key::ENTER);
    CHECK(GLFWSystemWindow::mapKey(GLFW_KEY_MENU) == Key::MENU);
    CHECK(GLFWSystemWindow::mapKey(GLFW_KEY_UNKNOWN) == Key::UNKNOWN);

    const Mod mod = GLFWSystemWindow::mapMods(GLFW_MOD_SHIFT | GLFW_MOD_SUPER);
    CHECK(mod.shift);
    CHECK_FALSE(mod.ctrl);
    CHECK_FALSE(mod.alt);
    CHECK(mod.super);
}

TEST_CASE("GLFW: every cursor has a standard cursor", "[backend][glfw][drag]") {
    CHECK(GLFWSystemWindow::standardCursorShape(Cursor::Move) == GLFW_RESIZE_ALL_CURSOR);
    CHECK(GLFWSystemWindow::standardCursorShape(Cursor::NotAllowed) == GLFW_NOT_ALLOWED_CURSOR);

    // The null platform makes every standard cursor
    GLFWHost host;
    const GLFWSystemWindow adapter(host.window, host.glfwWindow);
    for (std::size_t index = 0; index < cursorCount; ++index) {
        CAPTURE(index);
        CHECK(adapter.cursor(static_cast<Cursor>(index)) != nullptr);
    }
}

TEST_CASE("GLFW: ctrl + scroll zooms instead of scrolling", "[backend][glfw][zoom]") {
    GLFWHost host;
    auto &div = host.window->appContainer().add<Div>();
    div.width(100.0f).height(100.0f);
    const GLFWSystemWindow adapter(host.window, host.glfwWindow);
    host.layout();
    std::vector<ZoomEvent> zooms{};
    int                    scrolls = 0;
    div.onZoom([&](const ZoomEvent &event) { zooms.push_back(event); });
    div.onMouseScroll([&] { ++scrolls; });

    host.cursorCallback()(host.glfwWindow, 50.0, 40.0);
    host.keyCallback()(host.glfwWindow, GLFW_KEY_LEFT_CONTROL, 0, GLFW_PRESS, GLFW_MOD_CONTROL);
    auto scroll = glfwSetScrollCallback(host.glfwWindow, nullptr);
    glfwSetScrollCallback(host.glfwWindow, scroll);
    scroll(host.glfwWindow, 0.0, -1.0);
    REQUIRE(zooms.size() == 1);
    CHECK(zooms[0].scale == Catch::Approx(1.0f / Window::wheelZoomStep));
    CHECK(zooms[0].windowX == 50);
    CHECK(zooms[0].windowY == 40);
    CHECK(scrolls == 0);
}

TEST_CASE("GLFW: scrolls carry the modifiers of the last key or button event", "[backend][glfw][mouse]") {
    GLFWHost host;
    auto &div = host.window->appContainer().add<Div>();
    div.width(100.0f).height(100.0f);
    const GLFWSystemWindow adapter(host.window, host.glfwWindow);
    host.layout();
    std::vector<MouseEvent> scrolls{};
    std::vector<MouseEvent> presses{};
    div.onMouseScroll([&](const MouseEvent &event) { scrolls.push_back(event); });
    div.onMousePress([&](const MouseEvent &event) { presses.push_back(event); });

    host.cursorCallback()(host.glfwWindow, 50.0, 50.0);
    host.keyCallback()(host.glfwWindow, GLFW_KEY_LEFT_SHIFT, 0, GLFW_PRESS, GLFW_MOD_SHIFT);
    auto scroll = glfwSetScrollCallback(host.glfwWindow, nullptr);
    glfwSetScrollCallback(host.glfwWindow, scroll);
    scroll(host.glfwWindow, 0.0, -1.0);
    REQUIRE(scrolls.size() == 1);
    CHECK(scrolls[0].modifiers.shift);
    CHECK(scrolls[0].scrollY == -1.0);

    // The window counts GLFW's clicks
    for (int i = 0; i < 2; ++i) {
        host.buttonCallback()(host.glfwWindow, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
        host.buttonCallback()(host.glfwWindow, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
    }
    REQUIRE(presses.size() == 2);
    CHECK(presses[0].clickCount == 1);
    CHECK(presses[1].clickCount == 2);
    CHECK(presses[1].source == InputMode::Pointer);
}
