#include <memory>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/Scroller.hpp>
#include <psychic-ui/components/TextArea.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/skins/base/ButtonSkinBase.hpp>
#include <psychic-ui/skins/base/CheckBoxSkinBase.hpp>
#include <psychic-ui/skins/default/DefaultButtonSkin.hpp>
#include <psychic-ui/skins/default/DefaultBasicButtonSkin.hpp>
#include <psychic-ui/skins/default/DefaultCheckBoxSkin.hpp>
#include <psychic-ui/skins/default/DefaultMenuButtonSkin.hpp>
#include <psychic-ui/skins/default/DefaultScrollBarSkin.hpp>
#include <psychic-ui/skins/default/DefaultSliderSkin.hpp>
#include <psychic-ui/skins/default/DefaultTextAreaSkin.hpp>
#include <psychic-ui/skins/default/DefaultTextInputSkin.hpp>
#include <psychic-ui/skins/default/TitleBarButtonSkin.hpp>
#include <psychic-ui/themes/one-light.hpp>
#include "../backends/fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

namespace {
    class GlowButtonSkin : public ButtonSkinBase {};

    class DangerButtonSkin : public ButtonSkinBase {};

    class FlatCheckBoxSkin : public CheckBoxSkinBase {};

    /**
     * A partial set: two button skins and a check box, the rest from the default set
     */
    class PartialSkins : public SkinSet {
    public:
        std::string_view name() const override { return "Partial"; }

        const Theme &defaultTheme() const override { return themes::oneLight; }

        std::unique_ptr<SkinSet> fallback() const override { return std::make_unique<DefaultSkins>(); }

        void registerSkins(SkinRegistry &skins) const override {
            skins.add<ButtonSkin, GlowButtonSkin>()
                 .add<ButtonSkin, DangerButtonSkin>("danger")
                 .add<CheckBoxSkin, FlatCheckBoxSkin>();
        }

        void load(StyleManager *manager) override {
            manager->style("Button").padding(manager->theme().metrics.gutterCard);
        }
    };

    template<class T>
    bool is(const std::shared_ptr<internal::SkinBase> &skin) {
        return std::dynamic_pointer_cast<T>(skin) != nullptr;
    }

    /**
     * Whether a descendant of the div has the class
     */
    bool hasPart(const Div &div, const std::string &className) {
        for (const auto &child: const_cast<Div &>(div).children()) {
            if (child->classNames().contains(className) || hasPart(*child, className)) {
                return true;
            }
        }
        return false;
    }
}

TEST_CASE("the registry finds a variant, else the interface's default", "[skins][registry]") {
    SkinRegistry skins;
    REQUIRE(skins.make<ButtonSkin>() == nullptr);

    skins.add<ButtonSkin, DefaultButtonSkin>()
         .add<ButtonSkin, DefaultBasicButtonSkin>("basic");

    CHECK(is<DefaultButtonSkin>(skins.make<ButtonSkin>()));
    CHECK(is<DefaultBasicButtonSkin>(skins.make<ButtonSkin>("basic")));
    CHECK(is<DefaultButtonSkin>(skins.make<ButtonSkin>("unknown")));
    CHECK(skins.make<CheckBoxSkin>() == nullptr); // Another interface: nothing registered for it
    CHECK(skins.make<ButtonSkin>() != skins.make<ButtonSkin>()); // A new one each time

    skins.add<ButtonSkin, TitleBarButtonSkin>(); // Replaces the default
    CHECK(is<TitleBarButtonSkin>(skins.make<ButtonSkin>()));
}

TEST_CASE("a skin set falls back on its fallback set, never on no skin", "[skins][registry]") {
    StyleManager manager;
    CHECK(manager.skinSet() == nullptr);
    CHECK(manager.skin<ButtonSkin>() == nullptr); // No set yet

    manager.skinSet<PartialSkins>();
    REQUIRE(manager.skinSet() != nullptr);
    CHECK(manager.skinSet()->name() == "Partial");
    CHECK(manager.theme().name == themes::oneLight.name); // The set's theme

    CHECK(is<GlowButtonSkin>(manager.skin<ButtonSkin>()));
    CHECK(is<DangerButtonSkin>(manager.skin<ButtonSkin>("danger")));
    CHECK(is<DefaultBasicButtonSkin>(manager.skin<ButtonSkin>("basic")));    // The fallback's variant
    CHECK(is<TitleBarButtonSkin>(manager.skin<ButtonSkin>("title-bar")));
    CHECK(is<GlowButtonSkin>(manager.skin<ButtonSkin>("unknown")));          // The set's default
    CHECK(is<FlatCheckBoxSkin>(manager.skin<CheckBoxSkin>()));
    CHECK(is<DefaultSliderSkin>(manager.skin<RangeSkin>()));                 // Only in the fallback
    CHECK(is<DefaultScrollBarSkin>(manager.skin<ScrollBarSkin>("unknown")));
    CHECK(is<DefaultTextInputSkin>(manager.skin<TextInputSkin>()));
    CHECK(is<DefaultTextAreaSkin>(manager.skin<TextAreaSkin>()));
    CHECK(is<DefaultSubMenuButtonSkin>(manager.skin<MenuButtonSkin>("submenu")));
}

TEST_CASE("a skin set's rules load over its fallback's", "[skins][registry]") {
    auto button = std::make_shared<Button>("Button");
    StyleManager manager;
    manager.skinSet<DefaultSkins>();
    const Length defaultPadding = manager.computeStyle(button.get()).padding();
    REQUIRE(defaultPadding != Length{themes::oneLight.metrics.gutterCard});
    manager.skinSet<PartialSkins>();
    CHECK(manager.computeStyle(button.get()).padding() == Length{themes::oneLight.metrics.gutterCard});
}

TEST_CASE("switching skin sets hatches every component again", "[skins][switch]") {
    auto window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    auto &app      = window->appContainer();
    auto &button   = app.add<Button>("Button");
    auto &basic    = app.add<Button>("Basic").skin("basic");
    auto &checkBox = app.add<CheckBox>("Check");
    auto &range    = app.add<Range<float>>();
    auto &input    = app.add<TextInput>("Some text");
    auto &area     = app.add<TextArea>("Some more text");
    auto content   = std::make_shared<Div>();
    content->size(2000, 2000);
    auto &scroller = app.add<Scroller>(content);
    scroller.size(100, 100);
    systemWindow.layout();

    REQUIRE(is<DefaultButtonSkin>(button.skin()));
    REQUIRE(is<DefaultBasicButtonSkin>(basic.skin()));
    REQUIRE(is<DefaultCheckBoxSkin>(checkBox.skin()));
    REQUIRE(is<DefaultSliderSkin>(range.skin()));
    REQUIRE(input.textDisplay()->parent() == input.skin().get());
    const auto rangeSkin  = range.skin();
    const auto inputSkin  = input.skin();
    const auto areaSkin   = area.skin();
    const auto scrollBars = scroller.children();

    SECTION("a theme switch keeps the skins") {
        window->theme(themes::oneLight);
        systemWindow.layout();
        CHECK(range.skin() == rangeSkin);
        CHECK(input.skin() == inputSkin);
        CHECK(is<DefaultButtonSkin>(button.skin()));
    }

    SECTION("a skin set switch replaces them all") {
        window->skinSet<PartialSkins>();
        systemWindow.layout();

        CHECK(is<GlowButtonSkin>(button.skin()));
        CHECK(is<DefaultBasicButtonSkin>(basic.skin())); // The variant, from the fallback set
        CHECK(is<FlatCheckBoxSkin>(checkBox.skin()));
        CHECK(is<DefaultSliderSkin>(range.skin()));
        CHECK(range.skin() != rangeSkin); // Same type, new skin
        CHECK(area.skin() != areaSkin);
        CHECK(window->theme().name == themes::oneLight.name);

        // The text input's text display moved to its new skin, with its text
        REQUIRE(input.skin() != inputSkin);
        CHECK(input.textDisplay()->parent() == input.skin().get());
        CHECK(inputSkin->children().empty());
        CHECK(input.text() == "Some text");
        CHECK(input.textDisplay()->text() == "Some text");
        CHECK(area.textScroller()->parent() == area.skin().get());

        // Every component in the scroller too (its scroll bars)
        for (const auto &child: scroller.children()) {
            if (auto scrollBar = std::dynamic_pointer_cast<ScrollBar>(child)) {
                CHECK(is<DefaultScrollBarSkin>(scrollBar->skin()));
            }
        }

        // The button's label survived the switch
        auto glow = std::dynamic_pointer_cast<ButtonSkinBase>(button.skin());
        REQUIRE(glow);
        CHECK(hasPart(*glow, "label"));
    }

    window->skinSet<DefaultSkins>(); // The style manager is shared with the other tests
}

TEST_CASE("a skin built by hand stays until the skin set changes", "[skins][switch]") {
    auto window = std::make_shared<TestWindow>();
    FakeSystemWindow systemWindow(window);
    auto &button = window->appContainer().add<Button>("Button");
    systemWindow.layout();

    auto mine = std::make_shared<GlowButtonSkin>();
    button.skin(mine);
    systemWindow.layout();
    button.enabled(false); // A style change
    systemWindow.layout();
    CHECK(button.skin() == mine);

    window->skinSet<DefaultSkins>();
    systemWindow.layout();
    CHECK(is<DefaultButtonSkin>(button.skin()));
}

TEST_CASE("base skins create the parts contract", "[skins][parts]") {
    StyleManager manager;
    manager.skinSet<DefaultSkins>();

    // Button .label, in every variant built on the base
    CHECK(hasPart(*manager.skin<ButtonSkin>(), "label"));
    CHECK(hasPart(*manager.skin<ButtonSkin>("basic"), "label"));
    // CheckBox .box .label
    CHECK(hasPart(*manager.skin<CheckBoxSkin>(), "box"));
    CHECK(hasPart(*manager.skin<CheckBoxSkin>(), "label"));
    // ScrollBar .track .thumb
    CHECK(hasPart(*manager.skin<ScrollBarSkin>(), "track"));
    CHECK(hasPart(*manager.skin<ScrollBarSkin>(), "thumb"));
    // Range .value
    CHECK(hasPart(*manager.skin<RangeSkin>(), "value"));
    // MenuButton .label .shortcut, and .arrow for a submenu
    CHECK(hasPart(*manager.skin<MenuButtonSkin>(), "label"));
    CHECK(hasPart(*manager.skin<MenuButtonSkin>(), "shortcut"));
    CHECK_FALSE(hasPart(*manager.skin<MenuButtonSkin>(), "arrow"));
    CHECK(hasPart(*manager.skin<MenuButtonSkin>("submenu"), "arrow"));

    // A set's own skins built on the bases get them too
    CHECK(hasPart(*std::make_shared<GlowButtonSkin>(), "label"));
    CHECK(hasPart(*std::make_shared<FlatCheckBoxSkin>(), "box"));
}
