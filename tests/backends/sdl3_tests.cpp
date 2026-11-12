#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/applications/SDL3SystemWindow.hpp>
#include <psychic-ui/components/TitleBar.hpp>
#include "fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    /**
     * SDL on its dummy video driver: real SDL windows, no display, no GL. Events are made up and
     * handed to the adapter, as a host or the runner would.
     */
    struct SDL3Fixture {
        SDL_Window                        *sdlWindow{nullptr};
        std::shared_ptr<TestWindow>       window{std::make_shared<TestWindow>()};
        std::unique_ptr<SDL3SystemWindow> adapter{};
        sk_sp<SkSurface>                  surface{SkSurface::MakeRasterN32Premul(800, 600)};

        explicit SDL3Fixture(SDL_WindowFlags flags = 0) {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
            REQUIRE(SDL_Init(SDL_INIT_VIDEO));
            sdlWindow = SDL_CreateWindow("Test", 800, 600, flags);
            REQUIRE(sdlWindow != nullptr);
            adapter = std::make_unique<SDL3SystemWindow>(window, sdlWindow, nullptr);
        }

        ~SDL3Fixture() {
            adapter.reset();
            window.reset();
            SDL_DestroyWindow(sdlWindow);
            SDL_Quit();
        }

        void layout() {
            window->layout(surface->getCanvas());
        }

        SDL_Event event(SDL_EventType type) const {
            SDL_Event e{};
            e.type = type;
            // windowID is at the same place in every event that has one
            e.window.windowID = SDL_GetWindowID(sdlWindow);
            return e;
        }

        bool button(SDL_EventType type, float x, float y) {
            auto e = event(type);
            e.button.button = SDL_BUTTON_LEFT;
            e.button.down   = type == SDL_EVENT_MOUSE_BUTTON_DOWN;
            e.button.x      = x;
            e.button.y      = y;
            return adapter->handleEvent(e);
        }
    };
}

TEST_CASE("SDL3: the adapter reads the SDL window", "[backend][sdl3]") {
    SDL3Fixture sdl;
    CHECK(sdl.window->systemWindow() == sdl.adapter.get());
    CHECK(sdl.window->windowWidth() == 800);
    CHECK(sdl.window->windowHeight() == 600);
    CHECK(sdl.adapter->getFramebufferWidth() == 800);
}

TEST_CASE("SDL3: events for another window are left alone", "[backend][sdl3]") {
    SDL3Fixture sdl;
    auto e = sdl.event(SDL_EVENT_WINDOW_CLOSE_REQUESTED);
    e.window.windowID += 1000;
    CHECK_FALSE(sdl.adapter->handleEvent(e));
    CHECK(sdl.window->windowVisible());
}

TEST_CASE("SDL3: mouse events reach the div under the pointer", "[backend][sdl3]") {
    SDL3Fixture sdl;
    auto &div = sdl.window->appContainer().add<Div>();
    div.width(100.0f).height(100.0f);
    sdl.layout();

    int downs  = 0;
    int clicks = 0;
    int downX  = -1;
    int downY  = -1;
    div.onMouseDown([&](int x, int y, MouseButton button, Mod) {
        ++downs;
        downX = x;
        downY = y;
        CHECK(button == MouseButton::LEFT);
    });
    div.onClicked([&]() { ++clicks; });

    auto motion = sdl.event(SDL_EVENT_MOUSE_MOTION);
    motion.motion.x = 40.5f;
    motion.motion.y = 60.5f;
    CHECK(sdl.adapter->handleEvent(motion));
    CHECK(div.mouseOver());

    CHECK(sdl.button(SDL_EVENT_MOUSE_BUTTON_DOWN, 40.5f, 60.5f));
    CHECK(downs == 1);
    // Float coordinates truncated, without the 2018 backends' (-1, -2) offset, in the div's coordinates
    int divX = 0;
    int divY = 0;
    div.localToGlobal(divX, divY);
    CHECK(downX == 40 - divX);
    CHECK(downY == 60 - divY);
    CHECK(sdl.button(SDL_EVENT_MOUSE_BUTTON_UP, 40.5f, 60.5f));
    CHECK(clicks == 1);

    // Outside
    CHECK(sdl.button(SDL_EVENT_MOUSE_BUTTON_DOWN, 300.0f, 300.0f));
    CHECK(downs == 1);
}

TEST_CASE("SDL3: wheel events scroll at the pointer", "[backend][sdl3]") {
    SDL3Fixture sdl;
    auto &div = sdl.window->appContainer().add<Div>();
    div.width(100.0f).height(100.0f);
    sdl.layout();

    double scrolledY = 0;
    div.onMouseScrolled([&](int, int, double, double y) { scrolledY = y; });

    auto wheel = sdl.event(SDL_EVENT_MOUSE_WHEEL);
    wheel.wheel.mouse_x   = 50;
    wheel.wheel.mouse_y   = 50;
    wheel.wheel.y         = 2.0f;
    wheel.wheel.direction = SDL_MOUSEWHEEL_FLIPPED;
    sdl.adapter->handleEvent(wheel);
    CHECK(scrolledY == -2.0);
}

TEST_CASE("SDL3: keys and text reach the focused div", "[backend][sdl3]") {
    SDL3Fixture sdl;
    auto &div = sdl.window->appContainer().add<Div>();
    sdl.layout();
    sdl.window->requestFocus(&div);

    std::vector<Key>   downs{};
    int                repeats = 0;
    int                ups     = 0;
    Mod                lastMod{};
    icu::UnicodeString typed{};
    int                characters = 0;
    div.onKeyDown([&](Key key, Mod mod) { downs.push_back(key); lastMod = mod; });
    div.onKeyRepeated([&](Key, Mod) { ++repeats; });
    div.onKeyUp([&](Key, Mod) { ++ups; });
    div.onCharacterTyped([&](const icu::UnicodeString &character) { typed += character; ++characters; });

    auto key = sdl.event(SDL_EVENT_KEY_DOWN);
    key.key.key = SDLK_A;
    key.key.mod = SDL_KMOD_LCTRL;
    sdl.adapter->handleEvent(key);
    key.key.repeat = true;
    sdl.adapter->handleEvent(key);
    auto up = sdl.event(SDL_EVENT_KEY_UP);
    up.key.key = SDLK_A;
    sdl.adapter->handleEvent(up);

    REQUIRE(downs.size() == 1);
    CHECK(downs[0] == Key::A);
    CHECK(lastMod.ctrl);
    CHECK(repeats == 1);
    CHECK(ups == 1);

    // An input method commits several characters at once: they arrive one code point at a time
    auto text = sdl.event(SDL_EVENT_TEXT_INPUT);
    text.text.text = "é日本";
    sdl.adapter->handleEvent(text);
    CHECK(characters == 3);
    CHECK(typed == icu::UnicodeString::fromUTF8("é日本"));
}

TEST_CASE("SDL3: resizing changes the size the window is laid out at", "[backend][sdl3]") {
    SDL3Fixture sdl;
    auto resized = sdl.event(SDL_EVENT_WINDOW_RESIZED);
    resized.window.data1 = 1024;
    resized.window.data2 = 768;
    sdl.adapter->handleEvent(resized);
    CHECK(sdl.window->windowWidth() == 1024);
    CHECK(sdl.window->windowHeight() == 768);

    auto pixels = sdl.event(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED);
    pixels.window.data1 = 2048;
    pixels.window.data2 = 1536;
    sdl.adapter->handleEvent(pixels);
    CHECK(sdl.window->windowWidth() == 1024);
    CHECK(sdl.adapter->getFramebufferWidth() == 2048);
    CHECK(sdl.adapter->getPixelRatio() == 2.0f);

    sdl.layout();
    CHECK(sdl.window->getWidth() == 1024);
}

TEST_CASE("SDL3: dropped files arrive together", "[backend][sdl3]") {
    SDL3Fixture sdl;
    sdl.adapter->handleEvent(sdl.event(SDL_EVENT_DROP_BEGIN));
    for (const char *file : {"/tmp/a.txt", "/tmp/b.png"}) {
        auto drop = sdl.event(SDL_EVENT_DROP_FILE);
        drop.drop.data = file;
        sdl.adapter->handleEvent(drop);
        CHECK(sdl.window->dropped.empty());
    }
    sdl.adapter->handleEvent(sdl.event(SDL_EVENT_DROP_COMPLETE));
    CHECK(sdl.window->dropped == std::vector<std::string>{"/tmp/a.txt", "/tmp/b.png"});
}

TEST_CASE("SDL3: a close request closes a window that agrees", "[backend][sdl3]") {
    SDL3Fixture sdl;
    sdl.adapter->handleEvent(sdl.event(SDL_EVENT_WINDOW_CLOSE_REQUESTED));
    CHECK_FALSE(sdl.window->windowVisible());
    CHECK_FALSE(sdl.adapter->render());
}

TEST_CASE("SDL3: hit testing", "[backend][sdl3][drag]") {
    SDL3Fixture sdl(SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
    auto &titleBar = sdl.window->appContainer().add<TitleBar>();
    sdl.layout();

    // The title bar's empty part moves the window, its buttons don't
    CHECK(sdl.adapter->hitTest(700, titleBar.getHeight() / 2) == SDL_HITTEST_DRAGGABLE);
    const auto close = titleBar.children().front();
    int closeX = 0;
    int closeY = 0;
    close->localToGlobal(closeX, closeY, close->getWidth() / 2, close->getHeight() / 2);
    CHECK(sdl.adapter->hitTest(closeX, closeY) == SDL_HITTEST_NORMAL);
    CHECK(sdl.adapter->hitTest(400, 300) == SDL_HITTEST_NORMAL);

    // An undecorated resizable window resizes from its edges
    CHECK(sdl.adapter->hitTest(0, 0) == SDL_HITTEST_RESIZE_TOPLEFT);
    CHECK(sdl.adapter->hitTest(799, 300) == SDL_HITTEST_RESIZE_RIGHT);
    CHECK(sdl.adapter->hitTest(400, 599) == SDL_HITTEST_RESIZE_BOTTOM);

    // The dummy driver has no hit testing (X11 and Wayland do): the window moves itself, which only
    // Wayland forbids
    CHECK_FALSE(sdl.adapter->installHitTest());
    CHECK(sdl.window->supportsDrag());
}

TEST_CASE("SDL3: the clipboard", "[backend][sdl3][clipboard]") {
    SDL3Fixture sdl;
    sdl.window->setClipboardText("from Psychic UI ✓");
    CHECK(sdl.window->clipboardText() == "from Psychic UI ✓");
    CHECK(SDL_HasClipboardText());
}
