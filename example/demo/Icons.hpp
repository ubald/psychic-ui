#pragma once

#include <string>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/components/Icon.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/ScrollBar.hpp>
#include <psychic-ui/icons/IconRegistry.hpp>

namespace psychic_ui {

    /**
     * Every registered icon at 14 and 20 px with its name, and Inter at the sizes Pivotal uses
     */
    class IconsPanel : public Div {
    public:
        IconsPanel();
    };

    inline IconsPanel::IconsPanel() : Div() {
        tag("Icons")
            .classNames({"demo-panel"})
            .direction(Direction::Row)
            .padding(0); // We're using a scroller so no padding

        auto &scroller = add<Div>()
            .shrink(1)
            .grow(1)
            .height(100_pct)
            .padding(24)
            .overflow(Overflow::Scroll);

        add<ScrollBar>(scroller)
            .height(100_pct);

        // Inter, from Pivotal's extra-small to medium text
        auto &samples = scroller.add<Div>().addClassName("inter-samples");
        for (auto family: {"Inter", "Inter Medium"}) {
            auto &column = samples.add<Div>().addClassName("column");
            for (const float size: {11.0f, 12.0f, 13.0f, 14.0f}) {
                column.add<Label>(std::string{family} + " " + std::to_string(static_cast<int>(size)) + " px: The quick brown fox jumps over the lazy dog 0123456789")
                       .fontFamily(family)
                       .fontSize(size);
            }
        }

        // Rows of a fixed count, which fit the demo window
        constexpr std::size_t perRow = 9;
        auto     &grid = scroller.add<Div>().addClassName("icon-grid");
        Div      *row  = nullptr;
        std::size_t index = 0;
        for (auto name: IconRegistry::shared().names()) {
            if (index++ % perRow == 0) {
                row = &grid.add<Div>().addClassName("icon-row");
            }
            auto &cell = row->add<Div>().addClassName("icon-cell");
            auto &icons = cell.add<Div>().addClassName("icons");
            icons.add<Icon>(name).size(14);
            icons.add<Icon>(name).size(20);
            cell.add<Label>(std::string{name});
        }
    }
}
