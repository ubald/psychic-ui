#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Skin.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/MenuButton.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/ScrollBar.hpp>
#include <psychic-ui/components/Scroller.hpp>
#include <psychic-ui/components/TextArea.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/skins/SkinSet.hpp>
#include <psychic-ui/skins/base/ButtonSkinBase.hpp>
#include <psychic-ui/skins/base/CheckBoxSkinBase.hpp>
#include <psychic-ui/skins/base/MenuButtonSkinBase.hpp>
#include <psychic-ui/skins/base/RangeSkinBase.hpp>
#include <psychic-ui/skins/base/ScrollBarSkinBase.hpp>
#include <psychic-ui/skins/base/TextAreaSkinBase.hpp>
#include <psychic-ui/skins/base/TextInputSkinBase.hpp>
#include <psychic-ui/skins/pivotal/PivotalSkins.hpp>
#include <psychic-ui/skins/pivotal/TabButtonSkin.hpp>
#include <psychic-ui/skins/pivotal/TitleBarButtonSkin.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/themes/Theme.hpp>
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

        std::unique_ptr<SkinSet> fallback() const override { return std::make_unique<PivotalSkins>(); }

        void registerSkins(SkinRegistry &skins) const override {
            skins.add<ButtonSkin, GlowButtonSkin>()
                 .add<ButtonSkin, DangerButtonSkin>("danger")
                 .add<CheckBoxSkin, FlatCheckBoxSkin>();
        }

        void load(StyleManager *manager) override {
            manager->style("Button").padding(manager->theme().metrics.gutterCard);
        }
    };

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
     * Whether a descendant of the div has the class
     */
    bool hasPart(const Div &div, const std::string &className) {
        return std::ranges::any_of(div.children(), [&className](const auto &child) {
            return child->classNames().contains(className) || hasPart(*child, className);
        });
    }
}

TEST_CASE("the registry finds a variant, else the interface's default", "[skins][registry]") {
    SkinRegistry skins;
    REQUIRE(skins.make<ButtonSkin>() == nullptr);

    skins.add<ButtonSkin, ButtonSkinBase>()
         .add<ButtonSkin, ButtonSkinBase>("primary")
         .add<ButtonSkin, TabButtonSkin>("tab");

    CHECK(is<ButtonSkinBase>(skins.make<ButtonSkin>()));
    CHECK(is<TabButtonSkin>(skins.make<ButtonSkin>("tab")));
    CHECK(is<ButtonSkinBase>(skins.make<ButtonSkin>("unknown")));
    // A variant's skins have its name as a class, the default's and the fallbacks' none
    CHECK(skins.make<ButtonSkin>("primary")->classNames().contains("primary"));
    CHECK(skins.make<ButtonSkin>("tab")->classNames().contains("tab"));
    CHECK_FALSE(skins.make<ButtonSkin>("unknown")->classNames().contains("unknown"));
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
    CHECK(is<TabButtonSkin>(manager.skin<ButtonSkin>("tab")));               // The fallback's variant
    CHECK(is<TitleBarButtonSkin>(manager.skin<ButtonSkin>("title-bar")));
    CHECK(is<GlowButtonSkin>(manager.skin<ButtonSkin>("unknown")));          // The set's default
    CHECK(is<FlatCheckBoxSkin>(manager.skin<CheckBoxSkin>()));
    CHECK(is<RangeSkinBase>(manager.skin<RangeSkin>()));                 // Only in the fallback
    CHECK(is<ScrollBarSkinBase>(manager.skin<ScrollBarSkin>("unknown")));
    CHECK(is<TextInputSkinBase>(manager.skin<TextInputSkin>()));
    CHECK(is<TextAreaSkinBase>(manager.skin<TextAreaSkin>()));
    CHECK(is<MenuButtonSkinBase>(manager.skin<MenuButtonSkin>()));
}

TEST_CASE("a skin set's rules load over its fallback's", "[skins][registry]") {
    auto button = std::make_shared<Button>("Button");
    StyleManager manager;
    manager.skinSet<PivotalSkins>();
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
    auto &tab      = app.add<Button>("Tab").skin("tab");
    auto &checkBox = app.add<CheckBox>("Check");
    auto &range    = app.add<Range<float>>();
    auto &input    = app.add<TextInput>("Some text");
    auto &area     = app.add<TextArea>("Some more text");
    auto content   = std::make_shared<Div>();
    content->size(2000, 2000);
    auto &scroller = app.add<Scroller>(content);
    scroller.size(100, 100);
    systemWindow.layout();

    REQUIRE(is<ButtonSkinBase>(button.skin()));
    REQUIRE(is<TabButtonSkin>(tab.skin()));
    REQUIRE(is<CheckBoxSkinBase>(checkBox.skin()));
    REQUIRE(is<RangeSkinBase>(range.skin()));
    REQUIRE(input.skin()->textView()->parent() == input.skin().get());
    input.selection(2, 6);
    const auto rangeSkin  = range.skin();
    const auto inputSkin  = input.skin();
    const auto areaSkin   = area.skin();
    const auto scrollBars = scroller.children();

    SECTION("a theme switch keeps the skins") {
        window->theme(themes::oneLight);
        systemWindow.layout();
        CHECK(range.skin() == rangeSkin);
        CHECK(input.skin() == inputSkin);
        CHECK(is<ButtonSkinBase>(button.skin()));
    }

    SECTION("a skin set switch replaces them all") {
        window->skinSet<PartialSkins>();
        systemWindow.layout();

        CHECK(is<GlowButtonSkin>(button.skin()));
        CHECK(is<TabButtonSkin>(tab.skin())); // The variant, from the fallback set
        CHECK(is<FlatCheckBoxSkin>(checkBox.skin()));
        CHECK(is<RangeSkinBase>(range.skin()));
        CHECK(range.skin() != rangeSkin); // Same type, new skin
        CHECK(area.skin() != areaSkin);
        CHECK(window->theme().name == themes::oneLight.name);

        // The text input's new skin has a view of its own, showing its text and selection
        REQUIRE(input.skin() != inputSkin);
        Text *view = input.skin()->textView();
        REQUIRE(view);
        CHECK(view != inputSkin->textView());
        CHECK(view->parent() == input.skin().get());
        CHECK(input.text() == "Some text");
        CHECK(view->text() == "Some text");
        CHECK(input.selection() == std::pair{2u, 6u});
        CHECK(view->selection() == std::pair{2u, 6u});
        // So does the text area's, inside its own viewport
        Text *areaView = area.skin()->textView();
        REQUIRE(areaView);
        CHECK(areaView != areaSkin->textView());
        CHECK(areaView->text() == "Some more text");
        Div *ancestor = areaView->parent();
        while (ancestor && ancestor != area.skin().get()) {
            ancestor = ancestor->parent();
        }
        CHECK(ancestor == area.skin().get());

        // Every component in the scroller too (its scroll bars)
        for (const auto &child: scroller.children()) {
            if (auto scrollBar = std::dynamic_pointer_cast<ScrollBar>(child)) {
                CHECK(is<ScrollBarSkinBase>(scrollBar->skin()));
            }
        }

        // The button's label survived the switch
        auto glow = std::dynamic_pointer_cast<ButtonSkinBase>(button.skin());
        REQUIRE(glow);
        CHECK(hasPart(*glow, "label"));
    }

    window->skinSet<PivotalSkins>(); // The style manager is shared with the other tests
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

    window->skinSet<PivotalSkins>();
    systemWindow.layout();
    CHECK(is<ButtonSkinBase>(button.skin()));
}

TEST_CASE("base skins create the parts contract", "[skins][parts]") {
    StyleManager manager;
    manager.skinSet<PivotalSkins>();

    // Button .label, in every variant built on the base
    CHECK(hasPart(*manager.skin<ButtonSkin>(), "label"));
    CHECK(hasPart(*manager.skin<ButtonSkin>("tab"), "label"));
    // CheckBox .box .label
    CHECK(hasPart(*manager.skin<CheckBoxSkin>(), "box"));
    CHECK(hasPart(*manager.skin<CheckBoxSkin>(), "label"));
    // ScrollBar .track .thumb
    CHECK(hasPart(*manager.skin<ScrollBarSkin>(), "track"));
    CHECK(hasPart(*manager.skin<ScrollBarSkin>(), "thumb"));
    // Range .value
    CHECK(hasPart(*manager.skin<RangeSkin>(), "value"));
    // MenuButton .label .shortcut .arrow (displayed for a submenu)
    CHECK(hasPart(*manager.skin<MenuButtonSkin>(), "label"));
    CHECK(hasPart(*manager.skin<MenuButtonSkin>(), "shortcut"));
    CHECK(hasPart(*manager.skin<MenuButtonSkin>(), "arrow"));

    // A set's own skins built on the bases get them too
    CHECK(hasPart(*std::make_shared<GlowButtonSkin>(), "label"));
    CHECK(hasPart(*std::make_shared<FlatCheckBoxSkin>(), "box"));
}
