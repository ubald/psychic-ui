#include <thread>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/applications/SDL3SystemWindow.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/Scroller.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/components/TitleBar.hpp>
#include <psychic-ui/themes/one-dark.hpp>
#include <psychic-ui/themes/one-light.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    /**
     * Touch devices the dummy driver doesn't have: SDL can't make them up, so the adapter is told
     * that touch device 1 is a touch screen and 2 a trackpad
     */
    constexpr SDL_TouchID touchScreen = 1;
    constexpr SDL_TouchID trackpad    = 2;

    struct TouchAdapter : SDL3SystemWindow {
        using SDL3SystemWindow::SDL3SystemWindow;

        /**
         * The system's light or dark mode, made up: the dummy driver doesn't have one
         */
        std::optional<ColorScheme> scheme{};

        ColorScheme colorScheme() const override {
            return scheme ? *scheme : SDL3SystemWindow::colorScheme();
        }

        SDL_TouchDeviceType touchDeviceType(SDL_TouchID touchID) const override {
            if (touchID == touchScreen) {
                return SDL_TOUCH_DEVICE_DIRECT;
            }
            if (touchID == trackpad) {
                return SDL_TOUCH_DEVICE_INDIRECT_ABSOLUTE;
            }
            return SDL_TOUCH_DEVICE_INVALID;
        }
    };

    /**
     * SDL on its dummy video driver: real SDL windows, no display, no GL. Events are made up and
     * handed to the adapter, as a host or the runner would.
     */
    struct SDL3Fixture {
        SDL_Window                        *sdlWindow{nullptr};
        std::shared_ptr<TestWindow>       window{std::make_shared<TestWindow>()};
        std::unique_ptr<SDL3SystemWindow> adapter{};
        sk_sp<SkSurface>                  surface{SkSurfaces::Raster(SkImageInfo::MakeN32Premul(800, 600))};

        explicit SDL3Fixture(SDL_WindowFlags flags = 0) {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
            REQUIRE(SDL_Init(SDL_INIT_VIDEO));
            sdlWindow = SDL_CreateWindow("Test", 800, 600, flags);
            REQUIRE(sdlWindow != nullptr);
            adapter = std::make_unique<TouchAdapter>(window, sdlWindow, nullptr);
        }

        SDL3Fixture(const SDL3Fixture &) = delete;
        SDL3Fixture &operator=(const SDL3Fixture &) = delete;
        SDL3Fixture(SDL3Fixture &&) = delete;
        SDL3Fixture &operator=(SDL3Fixture &&) = delete;

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

        /**
         * A finger event, at window coordinates, timed in milliseconds
         */
        bool finger(SDL_EventType type, float x, float y, Uint64 milliseconds, SDL_TouchID device = touchScreen, SDL_FingerID id = 1) const {
            SDL_Event e{};
            e.type              = type;
            e.tfinger.timestamp = milliseconds * 1'000'000;
            e.tfinger.windowID  = SDL_GetWindowID(sdlWindow);
            e.tfinger.touchID   = device;
            e.tfinger.fingerID  = id;
            e.tfinger.x         = x / 800.0f;
            e.tfinger.y         = y / 600.0f;
            return adapter->handleEvent(e);
        }

        /**
         * The mouse events SDL makes of a touch (or of a pen, or a real mouse's, by `which`)
         */
        void mouse(SDL_EventType type, float x, float y, SDL_MouseID which) const {
            auto e = event(type);
            if (type == SDL_EVENT_MOUSE_MOTION) {
                e.motion.which = which;
                e.motion.x     = x;
                e.motion.y     = y;
            } else {
                e.button.which  = which;
                e.button.button = SDL_BUTTON_LEFT;
                e.button.down   = type == SDL_EVENT_MOUSE_BUTTON_DOWN;
                e.button.x      = x;
                e.button.y      = y;
            }
            adapter->handleEvent(e);
        }

        bool button(SDL_EventType type, float x, float y) const {
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

TEST_CASE("SDL3: the theme follows the system's light and dark mode", "[backend][sdl3][theme]") {
    SDL3Fixture sdl;
    auto &adapter = dynamic_cast<TouchAdapter &>(*sdl.adapter);
    auto &div     = sdl.window->appContainer().add<Div>().addClassName("probe");
    std::vector<ColorScheme> changes{};
    sdl.window->onColorSchemeChange([&](ColorScheme scheme) { changes.push_back(scheme); });

    const auto systemGoes = [&](ColorScheme scheme) {
        adapter.scheme = scheme;
        SDL_Event e{};
        e.type = SDL_EVENT_SYSTEM_THEME_CHANGED; // Names no window
        CHECK(sdl.adapter->handleEvent(e));
        sdl.layout();
    };

    // The dummy driver doesn't say: dark
    sdl.layout();
    CHECK(sdl.window->followsSystemTheme());
    CHECK(sdl.window->theme().name == themes::pivotalDark.name);
    CHECK(div.computedStyle().color() == themes::pivotalDark.palette.text);

    systemGoes(ColorScheme::Light);
    CHECK(sdl.window->colorScheme() == ColorScheme::Light);
    CHECK(sdl.window->theme().name == themes::pivotalLight.name);
    CHECK(div.computedStyle().color() == themes::pivotalLight.palette.text); // Restyled live

    systemGoes(ColorScheme::Dark);
    CHECK(sdl.window->theme().name == themes::pivotalDark.name);

    systemGoes(ColorScheme::Unknown);
    CHECK(sdl.window->theme().name == themes::pivotalDark.name); // Dark when unknown
    CHECK(changes == std::vector{ColorScheme::Light, ColorScheme::Dark, ColorScheme::Unknown});

    // An explicit theme stops following
    sdl.window->theme(themes::oneLight);
    CHECK_FALSE(sdl.window->followsSystemTheme());
    systemGoes(ColorScheme::Dark);
    CHECK(sdl.window->theme().name == themes::oneLight.name);

    // Following again picks the system's at once, and a pair of themes replaces the set's
    sdl.window->followSystemTheme();
    CHECK(sdl.window->theme().name == themes::pivotalDark.name);
    sdl.window->followSystemTheme(themes::oneLight, themes::oneDark);
    CHECK(sdl.window->theme().name == themes::oneDark.name);
    systemGoes(ColorScheme::Light);
    CHECK(sdl.window->theme().name == themes::oneLight.name);
}

TEST_CASE("SDL3: a window opening on a light system starts light", "[backend][sdl3][theme]") {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
    REQUIRE(SDL_Init(SDL_INIT_VIDEO));
    SDL_Window *sdlWindow = SDL_CreateWindow("Test", 800, 600, 0);
    auto       window     = std::make_shared<TestWindow>();
    CHECK(window->theme().name == themes::pivotalDark.name); // Before it opens, it doesn't know

    struct LightAdapter : SDL3SystemWindow {
        using SDL3SystemWindow::SDL3SystemWindow;
        ColorScheme colorScheme() const override { return ColorScheme::Light; }
    };
    {
        LightAdapter adapter(window, sdlWindow, nullptr);
        window->open(&adapter); // As the base constructor did, through the adapter's override this time
        CHECK(window->theme().name == themes::pivotalLight.name);
    }
    window.reset();
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
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
    div.onMousePress([&](const MouseEvent &event) {
        ++downs;
        downX = static_cast<int>(event.x);
        downY = static_cast<int>(event.y);
        CHECK(event.button == MouseButton::LEFT);
    });
    div.onClick([&]() { ++clicks; });

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
    div.onMouseScroll([&](const MouseEvent &event) { scrolledY = event.scrollY; });

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
    div.onKeyPress([&](Key key, Mod mod) { downs.push_back(key); lastMod = mod; });
    div.onKeyRepeat([&](Key, Mod) { ++repeats; });
    div.onKeyRelease([&](Key, Mod) { ++ups; });
    div.onType([&](const icu::UnicodeString &character) { typed += character; ++characters; });

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
    const Button &close = titleBar.controls().closeButton();
    int closeX = 0;
    int closeY = 0;
    close.localToGlobal(closeX, closeY, close.getWidth() / 2, close.getHeight() / 2);
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
    const SDL3Fixture sdl;
    sdl.window->setClipboardText("from Psychic UI ✓");
    CHECK(sdl.window->clipboardText() == "from Psychic UI ✓");
    CHECK(SDL_HasClipboardText());
}

namespace {
    bool moveTo(SDL3Fixture &sdl, float x, float y) {
        auto motion = sdl.event(SDL_EVENT_MOUSE_MOTION);
        motion.motion.x = x;
        motion.motion.y = y;
        return sdl.adapter->handleEvent(motion);
    }

    /**
     * Where a skin's point is in the window
     */
    SkPoint inWindow(const Div &skin, float x, float y) {
        int windowX = 0;
        int windowY = 0;
        skin.localToGlobal(windowX, windowY, static_cast<int>(x), static_cast<int>(y));
        return SkPoint::Make(static_cast<float>(windowX), static_cast<float>(windowY));
    }

    /**
     * A range drawn as a dial: its values go down its height, whatever the mouse's x
     */
    class DialSkin : public RangeSkin {
    public:
        void setValue(float /*value*/) override {}

        float valueAt(int /*x*/, int y) const override {
            return static_cast<float>(y) / static_cast<float>(_height);
        }
    };
}

TEST_CASE("SDL3: dragging a range follows the mouse, through its skin's geometry", "[backend][sdl3][drag]") {
    SDL3Fixture sdl;
    auto &range = sdl.window->appContainer().add<Range<float>>();
    sdl.layout();
    auto skin = range.skin();
    REQUIRE(skin);
    // The default slider's values are along its `.track` part, its first child
    const Div  &track  = *skin->children().front();
    REQUIRE(track.classNames().contains("track"));
    const auto  trackX = static_cast<float>(track.x());
    const auto  width  = static_cast<float>(track.getWidth());
    REQUIRE(width > 100.0f);
    const SkPoint quarter = inWindow(*skin, trackX + (width * 0.25f), 10.0f);
    CHECK(sdl.button(SDL_EVENT_MOUSE_BUTTON_DOWN, quarter.x(), quarter.y()));
    CHECK(range.value() == Catch::Approx(0.25f).margin(0.01f));
    CHECK(range.active());

    // Dragging, even off the range, keeps following the mouse
    const SkPoint threeQuarters = inWindow(*skin, trackX + (width * 0.75f), 300.0f);
    moveTo(sdl, threeQuarters.x(), threeQuarters.y());
    CHECK(range.value() == Catch::Approx(0.75f).margin(0.01f));
    moveTo(sdl, quarter.x() - 500.0f, quarter.y());
    CHECK(range.value() == 0.0f); // Clamped

    // Released, it stops
    CHECK(sdl.button(SDL_EVENT_MOUSE_BUTTON_UP, quarter.x() - 500.0f, quarter.y()));
    moveTo(sdl, threeQuarters.x(), threeQuarters.y());
    CHECK(range.value() == 0.0f);

    // A skin with another geometry: the range asks it where values are
    range.skin(std::make_shared<DialSkin>()).height(100.0f);
    sdl.layout();
    const SkPoint middle = inWindow(*range.skin(), 10.0f, 50.0f);
    CHECK(sdl.button(SDL_EVENT_MOUSE_BUTTON_DOWN, middle.x(), middle.y()));
    CHECK(range.value() == Catch::Approx(0.5f).margin(0.01f));
    CHECK(sdl.button(SDL_EVENT_MOUSE_BUTTON_UP, middle.x(), middle.y()));

    // Disabled, it doesn't move
    range.enabled(false);
    sdl.layout();
    const SkPoint low = inWindow(*range.skin(), 10.0f, 90.0f);
    sdl.button(SDL_EVENT_MOUSE_BUTTON_DOWN, low.x(), low.y());
    CHECK(range.value() == Catch::Approx(0.5f).margin(0.01f));
    sdl.button(SDL_EVENT_MOUSE_BUTTON_UP, low.x(), low.y());
}

TEST_CASE("SDL3: dragging a scroll bar's thumb scrolls its viewport", "[backend][sdl3][drag]") {
    SDL3Fixture sdl;
    auto content = std::make_shared<Div>();
    content->size(100, 1000).shrink(0);
    auto &scroller = sdl.window->appContainer().add<Scroller>(content);
    scroller.size(200, 200);
    sdl.layout();
    sdl.layout(); // The thumb is sized from the first layout

    const ScrollBar *scrollBar = nullptr;
    for (const auto &child: scroller.children()) {
        for (const auto &grandChild: child->children()) { // Beside the viewport, in a row
            auto bar = std::dynamic_pointer_cast<ScrollBar>(grandChild);
            if (bar && bar->scrollDirection() == ScrollDirection::Vertical) {
                scrollBar = bar.get();
            }
        }
    }
    REQUIRE(scrollBar);
    auto skin = scrollBar->skin();
    REQUIRE(skin);
    const SkRect track = skin->trackRect();
    const SkRect thumb = skin->thumbRect();
    REQUIRE(thumb.height() < track.height());
    REQUIRE(scroller.viewport()->scrollY() == 0);

    // A press on the track, below the thumb, doesn't drag
    const SkPoint below = inWindow(*skin, thumb.centerX(), thumb.bottom() + 20.0f);
    sdl.button(SDL_EVENT_MOUSE_BUTTON_DOWN, below.x(), below.y());
    moveTo(sdl, below.x(), below.y() + 50.0f);
    sdl.button(SDL_EVENT_MOUSE_BUTTON_UP, below.x(), below.y() + 50.0f);
    CHECK(scroller.viewport()->scrollY() == 0);

    // Pressed in its middle, the thumb follows the mouse: halfway down its free length is halfway down the content
    const SkPoint grab = inWindow(*skin, thumb.centerX(), thumb.centerY());
    CHECK(sdl.button(SDL_EVENT_MOUSE_BUTTON_DOWN, grab.x(), grab.y()));
    const float free          = track.height() - thumb.height();
    const int   contentScroll = scroller.viewport()->contentHeight() - scroller.viewport()->getHeight();
    REQUIRE(contentScroll > 0);
    moveTo(sdl, grab.x() + 300.0f, grab.y() + (free / 2.0f)); // Off the bar sideways too
    CHECK(scroller.viewport()->scrollY() == Catch::Approx(-contentScroll / 2.0).margin((contentScroll / free) + 1.0));
    moveTo(sdl, grab.x(), grab.y() + (10 * free)); // Past the end
    CHECK(scroller.viewport()->scrollY() == -contentScroll);

    // Released, it stops
    CHECK(sdl.button(SDL_EVENT_MOUSE_BUTTON_UP, grab.x(), grab.y() + (10 * free)));
    moveTo(sdl, grab.x(), grab.y());
    CHECK(scroller.viewport()->scrollY() == -contentScroll);
}

TEST_CASE("SDL3: a tap on a touch screen clicks once, without SDL's mouse events", "[backend][sdl3][touch]") {
    SDL3Fixture sdl;
    auto &div = sdl.window->appContainer().add<Div>();
    div.width(100.0f).height(100.0f);
    sdl.layout();
    int presses = 0;
    int clicks  = 0;
    div.onMousePress([&]() { ++presses; });
    div.onClick([&]() { ++clicks; });

    // What SDL sends for a tap: the finger, and the mouse events it makes of it
    CHECK(sdl.finger(SDL_EVENT_FINGER_DOWN, 40.0f, 40.0f, 1000));
    sdl.mouse(SDL_EVENT_MOUSE_MOTION, 40.0f, 40.0f, SDL_TOUCH_MOUSEID);
    sdl.mouse(SDL_EVENT_MOUSE_BUTTON_DOWN, 40.0f, 40.0f, SDL_TOUCH_MOUSEID);
    CHECK(sdl.window->inputMode() == InputMode::Touch);
    CHECK(sdl.finger(SDL_EVENT_FINGER_UP, 40.0f, 40.0f, 1080));
    sdl.mouse(SDL_EVENT_MOUSE_BUTTON_UP, 40.0f, 40.0f, SDL_TOUCH_MOUSEID);

    CHECK(presses == 1);
    CHECK(clicks == 1);
    CHECK(sdl.window->inputMode() == InputMode::Touch); // SDL's mouse events don't switch back

    // A real mouse does
    sdl.mouse(SDL_EVENT_MOUSE_MOTION, 300.0f, 300.0f, 1);
    CHECK(sdl.window->inputMode() == InputMode::Pointer);
    sdl.mouse(SDL_EVENT_MOUSE_BUTTON_DOWN, 40.0f, 40.0f, 1);
    sdl.mouse(SDL_EVENT_MOUSE_BUTTON_UP, 40.0f, 40.0f, 1);
    CHECK(clicks == 2);
}

TEST_CASE("SDL3: a trackpad's fingers don't switch to touch", "[backend][sdl3][touch]") {
    SDL3Fixture sdl;
    auto &div = sdl.window->appContainer().add<Div>();
    div.width(100.0f).height(100.0f);
    sdl.layout();
    int presses = 0;
    div.onMousePress([&]() { ++presses; });

    sdl.finger(SDL_EVENT_FINGER_DOWN, 40.0f, 40.0f, 1000, trackpad);
    sdl.finger(SDL_EVENT_FINGER_UP, 40.0f, 40.0f, 1080, trackpad);
    CHECK(sdl.window->inputMode() == InputMode::Pointer);
    CHECK(presses == 0); // The mouse it moves does the clicking
}

TEST_CASE("SDL3: touch drags, long presses and a second finger", "[backend][sdl3][touch]") {
    SDL3Fixture sdl;
    auto &div = sdl.window->appContainer().add<Div>();
    div.width(100.0f).height(100.0f);
    sdl.layout();
    std::vector<MouseButton> presses{};
    int                      moves = 0;
    div.onMousePress([&](const MouseEvent &event) { presses.push_back(event.button); });
    sdl.window->onMouseMove([&]() { ++moves; });

    // Past the slop, a drag
    sdl.finger(SDL_EVENT_FINGER_DOWN, 20.0f, 20.0f, 1000);
    sdl.finger(SDL_EVENT_FINGER_MOTION, 24.0f, 20.0f, 1010);
    CHECK(presses.empty());
    sdl.finger(SDL_EVENT_FINGER_MOTION, 60.0f, 20.0f, 1020);
    CHECK(presses == std::vector{MouseButton::LEFT});
    const int movesBefore = moves;
    // A second finger is ignored
    sdl.finger(SDL_EVENT_FINGER_DOWN, 80.0f, 80.0f, 1025, touchScreen, 2);
    sdl.finger(SDL_EVENT_FINGER_MOTION, 90.0f, 90.0f, 1030, touchScreen, 2);
    CHECK(moves == movesBefore);
    sdl.finger(SDL_EVENT_FINGER_UP, 60.0f, 20.0f, 1040);
    sdl.finger(SDL_EVENT_FINGER_UP, 90.0f, 90.0f, 1045, touchScreen, 2);

    // Held, a right button press, timed by SDL's timestamps
    sdl.finger(SDL_EVENT_FINGER_DOWN, 20.0f, 20.0f, 2000);
    sdl.window->updateGestures(std::chrono::milliseconds(2400));
    CHECK(presses.size() == 1);
    sdl.window->updateGestures(std::chrono::milliseconds(2500));
    CHECK(presses == std::vector{MouseButton::LEFT, MouseButton::RIGHT});
    sdl.finger(SDL_EVENT_FINGER_CANCELED, 20.0f, 20.0f, 2600);
    CHECK(presses.size() == 2);
}

TEST_CASE("SDL3: a pen works like a mouse, in pen mode", "[backend][sdl3][touch]") {
    SDL3Fixture sdl;
    auto &div = sdl.window->appContainer().add<Div>();
    div.width(100.0f).height(100.0f);
    sdl.layout();
    std::vector<MouseButton> presses{};
    int                      clicks = 0;
    div.onMousePress([&](const MouseEvent &event) { presses.push_back(event.button); });
    div.onClick([&]() { ++clicks; });

    auto motion = sdl.event(SDL_EVENT_PEN_MOTION);
    motion.pmotion.windowID = SDL_GetWindowID(sdl.sdlWindow);
    motion.pmotion.x        = 40.0f;
    motion.pmotion.y        = 40.0f;
    sdl.adapter->handleEvent(motion);
    CHECK(sdl.window->inputMode() == InputMode::Pen);
    CHECK(div.mouseOver());

    auto tip = sdl.event(SDL_EVENT_PEN_DOWN);
    tip.ptouch.windowID = SDL_GetWindowID(sdl.sdlWindow);
    tip.ptouch.x        = 40.0f;
    tip.ptouch.y        = 40.0f;
    tip.ptouch.down     = true;
    sdl.adapter->handleEvent(tip);
    sdl.mouse(SDL_EVENT_MOUSE_BUTTON_DOWN, 40.0f, 40.0f, SDL_PEN_MOUSEID); // SDL's copy, ignored
    tip.type        = SDL_EVENT_PEN_UP;
    tip.ptouch.down = false;
    sdl.adapter->handleEvent(tip);
    sdl.mouse(SDL_EVENT_MOUSE_BUTTON_UP, 40.0f, 40.0f, SDL_PEN_MOUSEID);
    CHECK(presses == std::vector{MouseButton::LEFT});
    CHECK(clicks == 1);

    // The barrel button is the right button
    auto barrel = sdl.event(SDL_EVENT_PEN_BUTTON_DOWN);
    barrel.pbutton.windowID = SDL_GetWindowID(sdl.sdlWindow);
    barrel.pbutton.button   = 1;
    barrel.pbutton.down     = true;
    barrel.pbutton.x        = 40.0f;
    barrel.pbutton.y        = 40.0f;
    sdl.adapter->handleEvent(barrel);
    CHECK(presses == std::vector{MouseButton::LEFT, MouseButton::RIGHT});
    CHECK(sdl.window->inputMode() == InputMode::Pen);
    barrel.type         = SDL_EVENT_PEN_BUTTON_UP;
    barrel.pbutton.down = false;
    sdl.adapter->handleEvent(barrel);
    CHECK(clicks == 1); // Only the tip clicks
}

TEST_CASE("SDL3: a focused text field tells SDL where its caret is, by mouse or by touch", "[backend][sdl3][touch]") {
    SDL3Fixture sdl;
    auto &input = sdl.window->appContainer().add<TextInput>("Hello");
    input.width(200.0f);
    sdl.layout();
    int left = 0;
    int top  = 0;
    input.localToGlobal(left, top);
    const auto  x = static_cast<float>(left + 100);
    const float y = static_cast<float>(top) + (static_cast<float>(input.getHeight()) / 2.0f);

    // With a mouse, for an input method's candidates
    sdl.mouse(SDL_EVENT_MOUSE_MOTION, x, y, 1);
    sdl.mouse(SDL_EVENT_MOUSE_BUTTON_DOWN, x, y, 1);
    sdl.mouse(SDL_EVENT_MOUSE_BUTTON_UP, x, y, 1);
    SDL_Rect area{};
    int      cursor = -1;
    REQUIRE(SDL_GetTextInputArea(sdl.sdlWindow, &area, &cursor));
    CHECK(area.w > 0);
    CHECK(area.y >= top);

    // Blurred, then focused again by a tap, for the on-screen keyboard
    sdl.mouse(SDL_EVENT_MOUSE_BUTTON_DOWN, 700.0f, 500.0f, 1);
    sdl.mouse(SDL_EVENT_MOUSE_BUTTON_UP, 700.0f, 500.0f, 1);
    SDL_SetTextInputArea(sdl.sdlWindow, nullptr, 0);
    sdl.finger(SDL_EVENT_FINGER_DOWN, x, y, 1000);
    sdl.finger(SDL_EVENT_FINGER_UP, x, y, 1050);
    REQUIRE(SDL_GetTextInputArea(sdl.sdlWindow, &area, &cursor));
    CHECK(area.w > 0);
    CHECK(area.h > 0);
    CHECK(area.x >= left);
    CHECK(area.y >= top);
    CHECK(area.y + area.h <= top + input.getHeight());
    CHECK(cursor > 0); // After "Hello"
}

TEST_CASE("SDL3: mouse events say what made them, with SDL's click count and the modifiers", "[backend][sdl3][mouse]") {
    SDL3Fixture sdl;
    auto &div = sdl.window->appContainer().add<Div>();
    div.width(100.0f).height(100.0f);
    sdl.layout();
    std::vector<MouseEvent> presses{};
    std::vector<MouseEvent> scrolls{};
    div.onMousePress([&](const MouseEvent &event) { presses.push_back(event); });
    div.onMouseScroll([&](const MouseEvent &event) { scrolls.push_back(event); });

    // SDL counts the clicks itself
    auto press = sdl.event(SDL_EVENT_MOUSE_BUTTON_DOWN);
    press.button.button = SDL_BUTTON_LEFT;
    press.button.down   = true;
    press.button.clicks = 2;
    press.button.x      = 40.5f;
    press.button.y      = 20.0f;
    sdl.adapter->handleEvent(press);
    REQUIRE(presses.size() == 1);
    CHECK(presses[0].clickCount == 2);
    CHECK(presses[0].source == InputMode::Pointer);
    CHECK(presses[0].windowX == 40.5f); // Not rounded
    sdl.button(SDL_EVENT_MOUSE_BUTTON_UP, 40.5f, 20.0f);

    // The wheel, with the keyboard's modifiers
    SDL_SetModState(SDL_KMOD_LSHIFT);
    auto wheel = sdl.event(SDL_EVENT_MOUSE_WHEEL);
    wheel.wheel.mouse_x = 50;
    wheel.wheel.mouse_y = 50;
    wheel.wheel.y       = 1.0f;
    sdl.adapter->handleEvent(wheel);
    SDL_SetModState(SDL_KMOD_NONE);
    REQUIRE(scrolls.size() == 1);
    CHECK(scrolls[0].modifiers.shift);
    CHECK(scrolls[0].scrollY == 1.0);

    // A pen's tip
    auto tip = sdl.event(SDL_EVENT_PEN_DOWN);
    tip.ptouch.windowID = SDL_GetWindowID(sdl.sdlWindow);
    tip.ptouch.x        = 30.0f;
    tip.ptouch.y        = 30.0f;
    tip.ptouch.down     = true;
    sdl.adapter->handleEvent(tip);
    REQUIRE(presses.size() == 2);
    CHECK(presses[1].source == InputMode::Pen);
    CHECK(presses[1].button == MouseButton::LEFT);
    CHECK(presses[1].clickCount == 1); // Counted by the window
    tip.type        = SDL_EVENT_PEN_UP;
    tip.ptouch.down = false;
    sdl.adapter->handleEvent(tip);
}

TEST_CASE("SDL3: a frame asked for wakes a loop waiting for events, once until the next frame", "[backend][sdl3][frames]") {
    SDL3Fixture sdl;
    sdl.layout();
    SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);
    CHECK_FALSE(sdl.window->needsFrame());

    sdl.window->requestFrame();
    sdl.window->requestFrame(); // Already asked: no second event
    SDL_Event event{};
    REQUIRE(SDL_WaitEventTimeout(&event, 100));
    CHECK(event.type == SDL3SystemWindow::wakeEventType());
    sdl.adapter->handleEvent(event); // Nothing for the adapter to do
    CHECK_FALSE(SDL_PollEvent(&event));
    CHECK(sdl.window->needsFrame());
    CHECK(sdl.window->untilNextFrame(sdl.adapter->gestureTime()) == std::chrono::nanoseconds::zero());

    // The frame draws it; the next request wakes again
    sdl.layout();
    CHECK_FALSE(sdl.window->needsFrame());
    CHECK(sdl.window->untilNextFrame(sdl.adapter->gestureTime()) == std::nullopt);
    sdl.window->requestFrame();
    CHECK(SDL_PollEvent(&event));
    CHECK(event.type == SDL3SystemWindow::wakeEventType());
}

TEST_CASE("SDL3: a post from another thread wakes a loop waiting for events", "[backend][sdl3][frames][post]") {
    SDL3Fixture sdl;
    sdl.layout();
    SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);

    std::thread([&] { sdl.window->post([] {}); }).join();
    SDL_Event event{};
    REQUIRE(SDL_WaitEventTimeout(&event, 1000));
    CHECK(event.type == SDL3SystemWindow::wakeEventType());
}
