#include <catch2/catch_test_macros.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <psychic-ui/applications/GLFWSystemWindow.hpp>
#include "fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    // What the host's own callbacks saw
    int hostKeys    = 0;
    int hostButtons = 0;

    void hostKeyCallback(GLFWwindow *, int, int, int, int) { ++hostKeys; }
    void hostButtonCallback(GLFWwindow *, int, int, int) { ++hostButtons; }

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

        ~GLFWHost() {
            glfwDestroyWindow(glfwWindow);
            glfwTerminate();
        }

        void layout() {
            window->layout(surface->getCanvas());
        }

        // The callback installed right now
        GLFWkeyfun keyCallback() const {
            auto current = glfwSetKeyCallback(glfwWindow, nullptr);
            glfwSetKeyCallback(glfwWindow, current);
            return current;
        }

        GLFWmousebuttonfun buttonCallback() const {
            auto current = glfwSetMouseButtonCallback(glfwWindow, nullptr);
            glfwSetMouseButtonCallback(glfwWindow, current);
            return current;
        }

        GLFWcursorposfun cursorCallback() const {
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
    div.onMousePress([&](int, int, MouseButton, Mod) { ++downs; });

    {
        GLFWSystemWindow adapter(host.window, host.glfwWindow);
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
    GLFWHost host;
    GLFWSystemWindow adapter(host.window, host.glfwWindow);
    CHECK_THROWS_AS(GLFWSystemWindow(std::make_shared<TestWindow>(), host.glfwWindow), std::invalid_argument);
}

TEST_CASE("GLFW: drops, clipboard and window dragging", "[backend][glfw]") {
    GLFWHost host;
    GLFWSystemWindow adapter(host.window, host.glfwWindow);

    auto drop = glfwSetDropCallback(host.glfwWindow, nullptr);
    glfwSetDropCallback(host.glfwWindow, drop);
    const char *paths[] = {"/tmp/a.txt", "/tmp/b.png"};
    drop(host.glfwWindow, 2, paths);
    CHECK(host.window->dropped == std::vector<std::string>{"/tmp/a.txt", "/tmp/b.png"});

    host.window->setClipboardText("from Psychic UI ✓");
    CHECK(host.window->clipboardText() == "from Psychic UI ✓");

    // Only Wayland can't move windows from inside
    CHECK(glfwGetPlatform() == GLFW_PLATFORM_NULL);
    CHECK(host.window->supportsDrag());
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
