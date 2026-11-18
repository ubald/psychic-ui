#pragma once

#include <functional>
#include <string>
#include <psychic-ui/Div.hpp>

namespace psychic_ui {
    class Window;
}

namespace psychic_ui::showcase {
    /**
     * What scenes are made of, with the showcase's classes (see ShowcaseStyleSheet)
     */
    namespace compose {
        /**
         * The window's backdrop, filling it, its panels centered
         */
        Div &backdrop(Window &window);

        /**
         * A panel floating over the backdrop, with a heading when given a title
         */
        Div &panel(Div &parent, const std::string &title = "", const std::string &caption = "");

        /**
         * A line in a form: its name, then the returned div for its controls
         */
        Div &field(Div &parent, const std::string &name);

        /**
         * A line of controls
         */
        Div &line(Div &parent);

        /**
         * The first div with this class name under `root`, depth first; throws when there is none
         */
        Div &find(Div &root, const std::string &className);

        /**
         * The first div under `root` that `accept` accepts, depth first; nullptr when there is none
         */
        Div *findIf(Div &root, const std::function<bool(Div &)> &accept);
    }
}
