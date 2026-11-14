#pragma once

#include <memory>
#include "Label.hpp"
#include "WindowControls.hpp"
#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    /**
     * @class TitleBar
     *
     * An undecorated window's title bar: its window controls and its title, and the area that moves the window
     */
    class TitleBar : public Div {
    public:
        TitleBar();

        WindowControls &controls() const { return *_controls; }

    protected:
        WindowControls *_controls{nullptr}; // A child, which we own
        Label          *_label{nullptr};    // A child, which we own
        void addedToRender() override;
    };
}
