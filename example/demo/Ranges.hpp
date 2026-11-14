#pragma once

#include <string>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Range.hpp>

namespace psychic_ui {

    class Ranges : public Div {
    public:
        Ranges() {
            tag("Ranges")
                .classNames({"demo-panel"});

            auto &container = add<HBox>().addClassName("columns");

            auto &left = container.add<VBox>().addClassName("column");
            left.add<Label>("Horizontal").addClassName("h4");
            left.add<Label>("A thin track, the accent up to the value, the value after it").role(TextRole::Tertiary);

            const auto field = [](Div &parent, const std::string &name) -> Div & {
                auto &row = parent.add<HBox>().addClassName("field");
                row.add<Label>(name).addClassName("name").role(TextRole::Tertiary);
                return row;
            };

            field(left, "Intensity").add<Range<int>>(0, 100, 60, nullptr).grow(1);
            field(left, "Zoom").add<Range<float>>(1.23f, 4.56f, 2.0f, nullptr).grow(1);
            field(left, "Disabled").add<Range<int>>(0, 10, 3, nullptr).grow(1).enabled(false);

            auto &right = container.add<VBox>().addClassName("column");
            right.add<Label>("Vertical").addClassName("h4");
            right.add<Label>("Filled from the bottom; no room for the value").role(TextRole::Tertiary);

            auto &verticals = right.add<HBox>().addClassName("verticals");

            verticals.add<Range<int>>(0, 10, 7, nullptr)
                     .orientation(Orientation::Vertical);

            verticals.add<Range<float>>(1.23f, 4.56f, 2.0f, nullptr)
                     .orientation(Orientation::Vertical);
        }
    };
}
