#include <memory>
#include <string>
#include <typeinfo>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/AppShell.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/ColorSwatches.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/skins/base/ButtonSkinBase.hpp>
#include <psychic-ui/skins/base/CheckBoxSkinBase.hpp>
#include <psychic-ui/skins/base/ColorSwatchesSkinBase.hpp>
#include <psychic-ui/skins/base/MenuButtonSkinBase.hpp>
#include <psychic-ui/skins/base/RangeSkinBase.hpp>
#include <psychic-ui/skins/base/ScrollBarSkinBase.hpp>
#include <psychic-ui/skins/base/SwitchSkinBase.hpp>
#include <psychic-ui/skins/base/TextInputSkinBase.hpp>
#include <psychic-ui/skins/desk/DeskSkins.hpp>
#include <psychic-ui/skins/float/FloatSkins.hpp>
#include <psychic-ui/skins/pivotal/PivotalSkins.hpp>
#include <psychic-ui/themes/desk.hpp>
#include <psychic-ui/themes/float.hpp>
#include <psychic-ui/themes/pivotal.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    /**
     * Whether the skin is a T, not a subclass of it
     */
    template<class T>
    bool is(const std::shared_ptr<internal::SkinBase> &skin) {
        if (!skin) {
            return false;
        }
        // Dereferenced outside typeid, which would otherwise evaluate a call in its operand
        const internal::SkinBase &base = *skin;
        return typeid(base) == typeid(T);
    }

    /**
     * A skin's part, by its class
     */
    Div *part(Div &root, const std::string &className) {
        if (root.classNames().contains(className)) {
            return &root;
        }
        for (const auto &child: root.children()) {
            if (Div *found = part(*child, className)) {
                return found;
            }
        }
        return nullptr;
    }

    struct Fixture {
        std::shared_ptr<TestWindow> window{std::make_shared<TestWindow>()};
        FakeSystemWindow            systemWindow{window, 1000, 700};

        Fixture() {
            window->skinSet<DeskSkins>();
        }

        Fixture(const Fixture &) = delete;
        Fixture &operator=(const Fixture &) = delete;
        Fixture(Fixture &&) = delete;
        Fixture &operator=(Fixture &&) = delete;

        ~Fixture() {
            // The style manager is shared with the other tests
            window->handleInputModeChange(InputMode::Pointer);
            window->skinSet<PivotalSkins>();
        }

        void layout() { systemWindow.layout(); }

        void touch(const bool on) {
            window->handleInputModeChange(on ? InputMode::Touch : InputMode::Pointer);
            layout();
        }
    };
}

TEST_CASE("Desk skins: the set installs with its dark theme, its light one for a light system", "[skins][desk]") {
    const DeskSkins set;
    CHECK(&set.theme(ColorScheme::Light) == &themes::deskLight);
    CHECK(&set.theme(ColorScheme::Dark) == &themes::deskDark);
    CHECK(&set.theme(ColorScheme::Unknown) == &themes::deskDark);
    REQUIRE(set.themes().size() == 2);
    CHECK(set.hasTheme(themes::deskLight));
    CHECK_FALSE(set.hasTheme(themes::pivotalDark));

    StyleManager manager;
    manager.skinSet<DeskSkins>();
    REQUIRE(manager.skinSet() != nullptr);
    CHECK(manager.skinSet()->name() == "Desk");
    CHECK(manager.theme().name == themes::deskDark.name);

    // A window follows the system with the set's pair
    const Fixture f;
    CHECK(f.window->theme().name == themes::deskDark.name);
    f.window->handleColorSchemeChange(ColorScheme::Light);
    CHECK(f.window->theme().name == themes::deskLight.name);
    f.window->handleColorSchemeChange(ColorScheme::Dark);
    CHECK(f.window->theme().name == themes::deskDark.name);
    f.window->handleColorSchemeChange(ColorScheme::Unknown);
}

TEST_CASE("Desk skins: a picked theme follows the scheme across sets", "[skins][desk][theme]") {
    const Fixture f;
    f.window->theme(themes::deskLight);
    f.window->skinSet<FloatSkins>();
    CHECK(f.window->theme().name == themes::floatLight.name);
    f.window->theme(themes::pivotalDark);
    f.window->skinSet<DeskSkins>();
    CHECK(f.window->theme().name == themes::deskDark.name);
    CHECK_FALSE(f.window->followsSystemTheme());
}

TEST_CASE("Desk skins: its themes are Pivotal's colors at desk sizes", "[skins][desk][theme]") {
    CHECK(themes::deskDark.scheme == ColorScheme::Dark);
    CHECK(themes::deskLight.scheme == ColorScheme::Light);
    CHECK(themes::deskDark.palette.accent == themes::pivotalDark.palette.accent);
    CHECK(themes::deskDark.palette.base == themes::pivotalDark.palette.base);
    CHECK(themes::deskLight.palette.text == themes::pivotalLight.palette.text);
    CHECK(themes::deskLight.palette.base == rgb(0xFFFFFF)); // White, not Pivotal's near-white
    CHECK(themes::deskLight.palette.page == rgb(0xFFFFFF));

    for (const Theme *theme: {&themes::deskDark, &themes::deskLight}) {
        INFO(theme->name);
        // What every named theme has: a canvas's dots, a panel elevation, Pivotal's motion
        CHECK(theme->palette.canvasDot.alpha() > 0);
        CHECK(theme->elevation.panel.color.alpha() > 0);
        CHECK(theme->elevation.panelContact.color.alpha() > 0);
        CHECK(theme->metrics.motion.fast == themes::pivotalDark.metrics.motion.fast);
        CHECK(theme->metrics.motion.medium == themes::pivotalDark.metrics.motion.medium);

        const Metrics &m = theme->metrics;
        CHECK(m.radius.button == 4); // Not pills
        CHECK(m.radius.page == 0);   // Flush panes
        CHECK(m.frameInset == 0);
        CHECK(m.pointer.controlMd == 24);
        CHECK(m.pointer.rowSm == 26);
        CHECK(m.pointer.hitSlop == 0);
        CHECK(m.pointer.iconStep == 0);
        CHECK(m.touch.controlMd == 40);
        CHECK(m.touch.rowSm == 40);
        CHECK(m.touch.textStep == 1);
        CHECK(m.touch.iconStep == 2);
        CHECK(m.touch.rowSm + (2 * m.touch.hitSlop) >= 44);
        CHECK(theme->type.sm.size == 11.5f);
    }
}

TEST_CASE("Desk skins: every skin is the default set's", "[skins][desk][registry]") {
    StyleManager manager;
    manager.skinSet<DeskSkins>();
    CHECK(is<RangeSkinBase>(manager.skin<RangeSkin>()));
    CHECK(is<ButtonSkinBase>(manager.skin<ButtonSkin>()));
    CHECK(is<ButtonSkinBase>(manager.skin<ButtonSkin>("primary")));
    CHECK(manager.skin<ButtonSkin>("primary")->classNames().contains("primary"));
    CHECK(is<CheckBoxSkinBase>(manager.skin<CheckBoxSkin>()));
    CHECK(is<ScrollBarSkinBase>(manager.skin<ScrollBarSkin>()));
    CHECK(is<TextInputSkinBase>(manager.skin<TextInputSkin>()));
    CHECK(is<MenuButtonSkinBase>(manager.skin<MenuButtonSkin>()));
    CHECK(is<SwitchSkinBase>(manager.skin<SwitchSkin>()));
    CHECK(is<ColorSwatchesSkinBase>(manager.skin<ColorSwatchesSkin>()));
}

TEST_CASE("Desk skins: its rules go over the default set's", "[skins][desk]") {
    Fixture f;
    auto &shell  = f.window->appContainer().add<AppShell>();
    auto &button = shell.page().add<Button>("Button");
    auto &input  = shell.page().add<TextInput>("Text");
    f.layout();
    CHECK(button.getHeight() == 24);
    CHECK(button.computedStyle().borderRadius() == 4); // Not a pill
    CHECK(input.getHeight() == 24);
    CHECK(input.computedStyle().fontSize() == 12);

    // Flush panes: no inset, no corners, the sidebar on the surface behind a hairline
    const Palette &p = themes::deskDark.palette;
    CHECK(shell.page().computedStyle().borderRadius() == 0);
    CHECK(shell.page().x() == 0);
    CHECK(shell.page().y() == 0);
    CHECK(shell.page().parent()->x() == shell.sidebar().getWidth());
    CHECK(shell.sidebar().getWidth() == themes::deskDark.metrics.sidebarWidth);
    CHECK(shell.sidebar().computedStyle().borderRight() == 1);
    CHECK(shell.sidebar().computedStyle().backgroundColor() == p.surface);

    // The default set, for comparison
    f.window->skinSet<PivotalSkins>();
    f.layout();
    CHECK(button.getHeight() == 28);
    CHECK(button.computedStyle().borderRadius() == 999);
}

TEST_CASE("Desk skins: the slider is a value field, the value inside it", "[skins][desk][range]") {
    Fixture f;
    auto &range = f.window->appContainer().add<Range<float>>(0.0f, 1.0f, 0.25f, nullptr).width(200);
    f.layout();
    REQUIRE(is<RangeSkinBase>(range.skin()));
    const Div *track = part(*range.skin(), "track");
    const Div *fill  = part(*range.skin(), "fill");
    const Div *thumb = part(*range.skin(), "thumb");
    const Div *value = part(*range.skin(), "value");
    REQUIRE(track);
    REQUIRE(fill);
    REQUIRE(thumb);
    REQUIRE(value);

    CHECK(range.getHeight() == 18);
    CHECK(track->getWidth() == 200); // The whole width: the value is inside
    CHECK(track->getHeight() == 18);
    CHECK(fill->getWidth() == 200 / 4.0f);
    CHECK(fill->computedStyle().borderRadius() == 0);
    CHECK(thumb->computedStyle().display() == Display::None);
    CHECK(value->x() + value->getWidth() <= 200);
    CHECK(value->x() > 100);

    // A press picks the value under it, along the field
    CHECK(range.skin()->valueAt(100, 9) == 0.5f);
}

TEST_CASE("Desk skins: touch switches to touch sizes, and back", "[skins][desk][touch]") {
    Fixture f;
    auto &button   = f.window->appContainer().add<Button>("Button");
    auto &checkBox = f.window->appContainer().add<CheckBox>("Check");
    auto &range    = f.window->appContainer().add<Range<float>>(0.0f, 1.0f, 0.5f, nullptr).width(200);
    auto &toggle   = f.window->appContainer().add<Switch>("Switch");
    auto &swatches = f.window->appContainer().add<ColorSwatches>(std::vector<Color>{rgb(0xFF0000), rgb(0x00FF00)});
    f.layout();
    const Div *box    = part(*checkBox.skin(), "box");
    const Div *thumb  = part(*range.skin(), "thumb");
    const Div *track  = part(*toggle.skin(), "track");
    const Div *swatch = part(*swatches.skin(), "swatch");
    REQUIRE(box);
    REQUIRE(thumb);
    REQUIRE(track);
    REQUIRE(swatch);

    CHECK(button.getHeight() == 24);
    CHECK(box->getHeight() == 14);
    CHECK(range.getHeight() == 18);
    CHECK(track->getHeight() == 14);
    CHECK(swatch->getHeight() == 16);
    CHECK(button.computedStyle().fontSize() == 11.5f);
    CHECK(button.computedStyle().iconSize() == themes::deskDark.metrics.iconSm);
    CHECK(button.computedStyle().hitSlop() == 0);
    CHECK(checkBox.computedStyle().hitSlop() == 0);

    f.touch(true);
    CHECK(f.window->styleManager()->density() == Density::Touch);
    CHECK(button.getHeight() == 40);
    CHECK(box->getHeight() == 20);
    CHECK(range.getHeight() == 32);
    CHECK(thumb->computedStyle().display() == Display::Flex); // The grab tab
    CHECK(track->getHeight() == 22);
    CHECK(swatch->getHeight() == 26);
    CHECK(button.computedStyle().fontSize() == 12.5f);
    CHECK(button.computedStyle().iconSize() == themes::deskDark.metrics.iconSm + 2);

    // Hit slop fills the rest up to 44 px
    CHECK(button.getHeight() + (2 * button.computedStyle().hitSlop()) >= 44);
    CHECK(box->getHeight() + (2 * checkBox.computedStyle().hitSlop()) >= 44);
    CHECK(range.getHeight() + (2 * range.computedStyle().hitSlop()) >= 44);
    CHECK(track->getHeight() + (2 * toggle.computedStyle().hitSlop()) >= 44);
    CHECK(swatch->getHeight() + (2 * swatches.computedStyle().hitSlop()) >= 44);

    // Back with the next mouse motion
    f.touch(false);
    CHECK(button.getHeight() == 24);
    CHECK(box->getHeight() == 14);
    CHECK(range.getHeight() == 18);
    CHECK(thumb->computedStyle().display() == Display::None);
    CHECK(button.computedStyle().hitSlop() == 0);
}

TEST_CASE("Desk skins: the sidebar widens by touch", "[skins][desk][touch]") {
    Fixture f;
    auto &shell = f.window->appContainer().add<AppShell>();
    f.layout();
    const auto pointer = static_cast<float>(shell.sidebar().getWidth());
    f.touch(true);
    CHECK(shell.sidebar().getWidth() > pointer);
    f.touch(false);
    CHECK(shell.sidebar().getWidth() == pointer);
}

TEST_CASE("the default set's icons and sizes are the same in both densities", "[skins][touch]") {
    // Desk grows its icons by touch through `Sizes::iconStep`; the default and Float sets keep theirs
    for (const Theme *theme: {&themes::pivotalDark, &themes::floatLight}) {
        INFO(theme->name);
        CHECK(theme->metrics.touch.iconStep == 0);
        CHECK(theme->metrics.touch.textStep == 0);
    }
}
