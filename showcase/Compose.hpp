#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <psychic-ui/Div.hpp>

namespace psychic_ui {
    class Window;
}

/**
 * What scenes are made of, with the showcase's classes (see ShowcaseStyleSheet)
 */
namespace psychic_ui::showcase::compose {
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
     * The first div under `root` that `accept` accepts, depth first; nullptr when there is none
     */
    Div *findIf(Div &root, const std::function<bool(Div &)> &accept);

    /**
     * The first div with this class name under `root` that is a T, depth first; throws when there is none
     */
    template<class T = Div>
    T &find(Div &root, const std::string &className) {
        T *found = nullptr;
        findIf(root, [&className, &found](Div &div) {
            if (div.classNames().contains(className)) {
                found = dynamic_cast<T *>(&div);
            }
            return found != nullptr;
        });
        if (!found) {
            throw std::runtime_error("No ." + className + " in the scene");
        }
        return *found;
    }
}
