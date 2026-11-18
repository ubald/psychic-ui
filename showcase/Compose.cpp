#include "Compose.hpp"

#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Label.hpp>

namespace psychic_ui::showcase::compose {
    using namespace psychic_ui::literals;

    Div &backdrop(Window &window) {
        return window.appContainer()
                     .add<Div>()
                     .addClassName("backdrop")
                     .width(100_pct)
                     .height(100_pct);
    }

    Div &panel(Div &parent, const std::string &title, const std::string &caption) {
        auto &panel = parent.add<Div>().addClassName("panel");
        if (!title.empty()) {
            auto &heading = panel.add<Div>().addClassName("heading");
            heading.add<Label>(title).addClassName("title");
            if (!caption.empty()) {
                heading.add<Label>(caption).addClassName("caption");
            }
        }
        return panel;
    }

    Div &field(Div &parent, const std::string &name) {
        auto &field = parent.add<Div>().addClassName("field");
        field.add<Label>(name).addClassName("name");
        return field.add<Div>().addClassName("control");
    }

    Div &line(Div &parent) {
        return parent.add<Div>().addClassName("line");
    }

    Div *findIf(Div &root, const std::function<bool(Div &)> &accept) {
        if (accept(root)) {
            return &root;
        }
        for (const auto &child: root.children()) {
            // NOLINTNEXTLINE(misc-const-correctness): returned as a mutable pointer, which the check misses
            if (Div *found = findIf(*child, accept)) {
                return found;
            }
        }
        return nullptr;
    }
}
