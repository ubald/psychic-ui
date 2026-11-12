#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Text.hpp>
#include <psychic-ui/components/TitleBar.hpp>
#include "fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

TEST_CASE("The window reads its size from the system window", "[backend]") {
    auto window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window, 640, 480, 2.0f);
    CHECK(window->windowWidth() == 640);
    CHECK(window->windowHeight() == 480);
    CHECK(systemWindow.getFramebufferWidth() == 1280);
    CHECK(systemWindow.getPixelRatio() == 2.0f);
}

TEST_CASE("A system window detaches from its window when it goes away", "[backend]") {
    auto window = std::make_shared<TestWindow>();
    {
        FakeSystemWindow systemWindow(window);
        CHECK(window->systemWindow() == &systemWindow);
    }
    CHECK(window->systemWindow() == nullptr);
    window->startDrag(); // Nothing to forward to, nothing breaks
}

TEST_CASE("The clipboard goes through the system window", "[backend][clipboard]") {
    auto window = std::make_shared<TestWindow>();
    CHECK(window->clipboardText().empty()); // No system window yet
    window->setClipboardText("lost");

    FakeSystemWindow systemWindow(window);
    CHECK(window->clipboardText().empty());
    window->setClipboardText("Ünïcödé");
    CHECK(systemWindow.getClipboardText() == "Ünïcödé");
    CHECK(window->clipboardText() == "Ünïcödé");
}

TEST_CASE("Drag areas", "[backend][drag]") {
    auto window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    auto titleBar = window->appContainer()->add<TitleBar>();
    auto below    = window->appContainer()->add<Div>();
    below->style().height(100.0f);
    systemWindow.layout();

    // The title bar's empty part, right of its buttons and label
    int barX = 0;
    int barY = 0;
    titleBar->localToGlobal(barX, barY, titleBar->getWidth() - 10, titleBar->getHeight() / 2);
    REQUIRE(titleBar->getHeight() > 0);
    CHECK(window->isDragArea(barX, barY));

    // Its buttons still take presses
    std::shared_ptr<Div> close;
    for (const auto &child : titleBar->children()) {
        if (std::dynamic_pointer_cast<Button>(child)) {
            close = child;
            break;
        }
    }
    REQUIRE(close != nullptr);
    int closeX = 0;
    int closeY = 0;
    close->localToGlobal(closeX, closeY, close->getWidth() / 2, close->getHeight() / 2);
    CHECK_FALSE(window->isDragArea(closeX, closeY));

    // Outside of it
    int belowX = 0;
    int belowY = 0;
    below->localToGlobal(belowX, belowY, 10, 10);
    CHECK_FALSE(window->isDragArea(belowX, belowY));

    SECTION("not while a menu is open") {
        window->openMenu({Menu::item("Item", []() {})}, 0, 0);
        systemWindow.layout();
        CHECK_FALSE(window->isDragArea(barX, barY));
    }

    SECTION("not once the area is gone") {
        window->appContainer()->remove(titleBar);
        titleBar.reset();
        systemWindow.layout();
        CHECK_FALSE(window->isDragArea(barX, barY));
    }
}

TEST_CASE("Text copies, cuts and pastes through the clipboard", "[backend][clipboard][text]") {
    auto window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    auto text = window->appContainer()->add<Text>("Hello wörld");
    text->setEditable(true);
    systemWindow.layout();
    window->requestFocus(text.get());

    Mod ctrl{};
    ctrl.ctrl = true;

    text->setSelection(6, 11);
    window->keyDown(Key::C, ctrl);
    CHECK(window->clipboardText() == "wörld");
    CHECK(text->text() == "Hello wörld");

    window->keyDown(Key::X, ctrl);
    CHECK(text->text() == "Hello ");

    // Single-line text pastes line breaks as spaces
    window->setClipboardText("big\nworld");
    window->keyDown(Key::V, ctrl);
    CHECK(text->text() == "Hello big world");
    CHECK(text->getCaret() == 15);

    // Without the modifier, it's just a key
    window->keyDown(Key::V, Mod{});
    CHECK(text->text() == "Hello big world");
}
