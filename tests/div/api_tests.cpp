#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/CheckBox.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Range.hpp>
#include <psychic-ui/components/Text.hpp>
#include <psychic-ui/style/Length.hpp>
#include <psychic-ui/style/Values.hpp>

using namespace psychic_ui;

namespace {
    // A component of the user's: chains through its own and every base's setters
    class Toolbar : public Button {
    public:
        Toolbar() : Button("toolbar") {
            tag("Toolbar").padding(2);
        }

        template<class Self>
        Self &compact(this Self &self, bool compact) {
            static_cast<Toolbar &>(self)._compact = compact;
            return self;
        }

        bool compact() const { return _compact; }

        int enabledChanges = 0;

    protected:
        bool _compact{false};

        void enabledChanged() override {
            Button::enabledChanged();
            ++enabledChanges;
        }
    };

    template<class D> concept VisibleFromString = requires(D div) { div.visible("false"); };
    template<class D> concept ColorFromInt = requires(D div) { div.color(0xFF0000); };
    template<class D> concept FontFromZero = requires(D div) { div.fontFamily(0); };
    template<class D> concept OverflowFromString = requires(D div) { div.overflow("scroll"); };
    template<class D> concept EnabledFromString = requires(D div) { div.enabled("false"); };
    template<class D> concept ToggleFromInt = requires(D div) { div.toggle(1); };
    template<class D> concept MutableStyle = requires(D div) { div.style().padding(1); };
    template<class D> concept PublicTag = requires(D div) { div.tag("x"); };
    template<class D> concept OldSetters = requires(D div) { div.setWidth(1); } || requires(D div) { div.setEnabled(true); };
}

// The base's setters return the derived type, whatever the order
static_assert(std::is_same_v<decltype(std::declval<Button &>().enabled(false)), Button &>);
static_assert(std::is_same_v<decltype(std::declval<Button &>().padding(24)), Button &>);
static_assert(std::is_same_v<decltype(std::declval<Button &>().enabled(false).toggle(true).padding(24)), Button &>);
static_assert(std::is_same_v<decltype(std::declval<Toolbar &>().padding(1).toggle(true).compact(true)), Toolbar &>);
static_assert(std::is_same_v<decltype(std::declval<Range<int> &>().margin(12).range(0, 10).value(5)), Range<int> &>);
static_assert(std::is_same_v<decltype(std::declval<Text &>().text("a").padding(2).multiline(true)), Text &>);
static_assert(std::is_same_v<decltype(std::declval<Div &>().add<Label>("x")), Label &>);
// A component's skin setters: the `skin` style property by name, or a skin built by hand
static_assert(std::is_same_v<decltype(std::declval<Toolbar &>().skin("flat").compact(true)), Toolbar &>);
static_assert(std::is_same_v<decltype(std::declval<Toolbar &>().skin(std::shared_ptr<ButtonSkin>{}).compact(true)), Toolbar &>);
static_assert(std::is_same_v<decltype(std::declval<const Button &>().skin()), std::shared_ptr<ButtonSkin>>);
static_assert(std::is_same_v<decltype(std::declval<const Div &>().children()), const std::vector<std::shared_ptr<Div>> &>);

// Misuse doesn't compile on Div either
static_assert(!VisibleFromString<Div>);
static_assert(!ColorFromInt<Div>);
static_assert(!FontFromZero<Button>);
static_assert(!OverflowFromString<Button>);
static_assert(!EnabledFromString<Div>);
static_assert(!ToggleFromInt<Button>);
static_assert(!MutableStyle<Div>, "the inline style is set through the div's own setters");
static_assert(!OldSetters<Div>);
static_assert(!PublicTag<Div>, "a div's tags name its classes, set from their constructors");

TEST_CASE("setters chain across the hierarchy", "[div][api]") {
    auto root = std::make_shared<Div>();

    auto &button = root->add<Button>("Toggle").enabled(false).toggle(true).padding(24).selected(true);
    REQUIRE_FALSE(button.enabled());
    REQUIRE(button.toggle());
    REQUIRE(button.selected());
    REQUIRE(button.style().padding() == 24);

    auto &toolbar = root->add<Toolbar>().padding(1).compact(true).enabled(false).direction(Direction::Row);
    REQUIRE(toolbar.compact());
    REQUIRE(toolbar.tags().back() == "toolbar");
    REQUIRE(toolbar.style().padding() == 1);
    REQUIRE(toolbar.style().direction() == Direction::Row);

    auto &range = root->add<Range<int>>().margin(12).range(0, 10).value(5);
    REQUIRE(range.value() == 5);

    auto &checkBox = root->add<CheckBox>("Check").checked(true).label("Checked");
    REQUIRE(checkBox.checked());
    REQUIRE(checkBox.label() == "Checked");
}

TEST_CASE("helpers are shortcuts for properties", "[div][api]") {
    Div div;
    div.row();
    REQUIRE(div.style().direction() == Direction::Row);
    div.column().size(10, 50_pct);
    REQUIRE(div.style().direction() == Direction::Column);
    REQUIRE(div.style().width() == 10);
    REQUIRE(div.style().height() == 50_pct);

    const Style card = Style{}.padding(12).borderRadius(4);
    div.style(card).grow(1);
    REQUIRE(div.style().padding() == 12);
    REQUIRE(div.style().borderRadius() == 4);
    REQUIRE(div.style().grow() == 1);
}

TEST_CASE("enabled calls its hook only on a change", "[div][api]") {
    Toolbar toolbar;
    toolbar.enabled(true);
    REQUIRE(toolbar.enabledChanges == 0);
    toolbar.enabled(false).enabled(false);
    REQUIRE(toolbar.enabledChanges == 1);
    toolbar.enabled(true);
    REQUIRE(toolbar.enabledChanges == 2);
}

TEST_CASE("the parent owns what add<T> creates", "[div][api]") {
    auto               parent = std::make_shared<Div>();
    std::weak_ptr<Div> child;
    {
        auto &label = parent->add<Label>("owned");
        child = label.weak_from_this();
        REQUIRE(child.use_count() == 1); // Only the parent holds it
        REQUIRE(label.parent() == parent.get());
        REQUIRE(parent->children().front().get() == &label);
    }
    REQUIRE_FALSE(child.expired());

    SECTION("removing it frees it") {
        parent->remove(*parent->at(0));
        REQUIRE(child.expired());
    }

    SECTION("so does destroying the parent") {
        parent.reset();
        REQUIRE(child.expired());
    }

    SECTION("an existing child can be added, and is returned") {
        auto  text  = std::make_shared<Text>("shared");
        Text &added = parent->add(0, text).multiline(true);
        REQUIRE(&added == text.get());
        REQUIRE(parent->childIndex(text) == 0);
        REQUIRE(text->multiline());
    }
}
