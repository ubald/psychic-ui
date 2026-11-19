#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/WindowScale.hpp>

using namespace psychic_ui;

// Each platform's case: what SDL reports (pixel density, display scale) or GLFW (window size, framebuffer
// size, content scale), and the logical size, paint ratio and window size it makes

TEST_CASE("Window scale: Windows at 200 % works in pixels, the scale apart", "[backend][scale]") {
    // SDL: window units are pixels (density 1), the user's scale is in the display scale
    const auto scale = WindowScale::fromDisplay(1.0f, 2.0f);
    CHECK(scale.logicalPerWindowUnit == 0.5f);

    // A 2880×1800 pixel window is laid out at 1440×900 and painted at 2 pixels per unit
    CHECK(scale.toLogical(2880) == 1440);
    CHECK(scale.toLogical(1800) == 900);
    CHECK(WindowScale::pixelRatio(2880, scale.toLogical(2880)) == 2.0f);

    // A 1440×900 window asked for is 2880×1800 pixels
    CHECK(scale.toWindow(1440) == 2880);
    CHECK(scale.toWindow(900) == 1800);
}

TEST_CASE("Window scale: Windows at 150 %", "[backend][scale]") {
    const auto scale = WindowScale::fromDisplay(1.0f, 1.5f);
    CHECK(scale.toLogical(1500) == 1000);
    CHECK(WindowScale::pixelRatio(1500, scale.toLogical(1500)) == 1.5f);
    CHECK(scale.toWindow(1000) == 1500);
    // Odd pixel sizes round to the nearest unit
    CHECK(scale.toLogical(1366) == 911);
}

TEST_CASE("Window scale: macOS Retina has the scale in the density only", "[backend][scale]") {
    // SDL: window units are points, the framebuffer twice as large; the display scale counts the density
    const auto scale = WindowScale::fromDisplay(2.0f, 2.0f);
    CHECK(scale.logicalPerWindowUnit == 1.0f);
    CHECK(scale.toLogical(1440) == 1440);
    CHECK(WindowScale::pixelRatio(2880, scale.toLogical(1440)) == 2.0f);
    CHECK(scale.toWindow(1440) == 1440);
}

TEST_CASE("Window scale: Wayland's fractional 150 % has the scale in the density", "[backend][scale]") {
    const auto scale = WindowScale::fromDisplay(1.5f, 1.5f);
    CHECK(scale.logicalPerWindowUnit == 1.0f);
    CHECK(scale.toLogical(1000) == 1000);
    CHECK(WindowScale::pixelRatio(1500, scale.toLogical(1000)) == 1.5f);
}

TEST_CASE("Window scale: a plain 1× display", "[backend][scale]") {
    const auto scale = WindowScale::fromDisplay(1.0f, 1.0f);
    CHECK(scale.logicalPerWindowUnit == 1.0f);
    CHECK(scale.toLogical(800) == 800);
    CHECK(WindowScale::pixelRatio(800, 800) == 1.0f);
}

TEST_CASE("Window scale: an unknown density or scale leaves window units as they are", "[backend][scale]") {
    // SDL returns 0 on failure
    CHECK(WindowScale::fromDisplay(0.0f, 2.0f).logicalPerWindowUnit == 1.0f);
    CHECK(WindowScale::fromDisplay(1.0f, 0.0f).logicalPerWindowUnit == 1.0f);
    CHECK(WindowScale::pixelRatio(800, 0) == 1.0f);
}

TEST_CASE("Window scale: GLFW, from the framebuffer and the content scale", "[backend][scale]") {
    // Windows at 200 %: the framebuffer is the window's size, the content scale apart
    CHECK(WindowScale::fromFramebuffer(2880, 2880, 2.0f).logicalPerWindowUnit == 0.5f);
    // macOS Retina: the content scale is the framebuffer's density
    CHECK(WindowScale::fromFramebuffer(1440, 2880, 2.0f).logicalPerWindowUnit == 1.0f);
    // Wayland at 150 %: a framebuffer rounded to whole pixels is still the density
    CHECK(WindowScale::fromFramebuffer(1001, 1502, 1.5f).logicalPerWindowUnit == 1.0f);
    // Minimized: no size to tell the density from, so window units are left as they are
    CHECK(WindowScale::fromFramebuffer(0, 0, 2.0f).logicalPerWindowUnit == 1.0f);
}

TEST_CASE("Window scale: mouse coordinates in logical units", "[backend][scale]") {
    const auto windows = WindowScale::fromDisplay(1.0f, 2.0f);
    CHECK(windows.toLogical(401.0f) == 200.5f);
    CHECK(windows.toLogical(0.0f) == 0.0f);

    const auto mac = WindowScale::fromDisplay(2.0f, 2.0f);
    CHECK(mac.toLogical(200.5f) == 200.5f);

    const auto windows150 = WindowScale::fromDisplay(1.0f, 1.5f);
    CHECK(windows150.toLogical(300.0f) == 200.0f);
}
