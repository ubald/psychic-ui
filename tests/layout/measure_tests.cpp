#include <memory>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Measure.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Values.hpp>

using namespace psychic_ui;

namespace {
    // Content of 120 x 30, measured with psychic-ui's types, not Yoga's
    class Box : public Div {
    public:
        std::vector<Fit> widthFits{};

        Box() { setMeasurable(); }

    protected:
        Size measure(Constraint width, Constraint height) override {
            widthFits.push_back(width.fit);
            return {.width = width.apply(120), .height = height.apply(30)};
        }
    };
}

TEST_CASE("constraints fit the content like CSS sizing", "[layout][measure]") {
    STATIC_REQUIRE(Constraint{50, Fit::Exact}.apply(120) == 50);
    STATIC_REQUIRE(Constraint{50, Fit::AtMost}.apply(120) == 50);
    STATIC_REQUIRE(Constraint{500, Fit::AtMost}.apply(120) == 120);
    STATIC_REQUIRE(Constraint{0, Fit::Unbounded}.apply(120) == 120);
}

TEST_CASE("a div's measure decides its size", "[layout][measure]") {
    auto window = std::make_shared<Window>("measure");
    window->setStyleManager(std::make_shared<StyleManager>());
    window->setWindowSize(800, 600);
    auto &row = window->appContainer().add<Div>().row().alignItems(Align::Start);

    SECTION("unconstrained, at its content size") {
        auto &box = row.add<Box>();
        window->updateStylesAndLayout();
        REQUIRE(box.getWidth() == 120);
        REQUIRE(box.getHeight() == 30);
        REQUIRE_FALSE(box.widthFits.empty());
    }

    SECTION("an explicit width wins") {
        auto &box = row.add<Box>().width(40);
        window->updateStylesAndLayout();
        REQUIRE(box.getWidth() == 40);
        REQUIRE(box.getHeight() == 30);
    }
}
