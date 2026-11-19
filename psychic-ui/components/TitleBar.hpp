#pragma once

#include <memory>
#include "Label.hpp"
#include "WindowControls.hpp"
#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    /**
     * @class TitleBar
     *
     * An undecorated window's title bar: its window controls and its title, and the area that moves the window. The
     * controls sit where their style puts them, before the title for macOS's and after it for Windows's and Linux's.
     */
    class TitleBar : public Div {
    public:
        TitleBar();

        WindowControls &controls() const { return *_controls; }

    protected:
        WindowControls *_controls{nullptr}; // A child: our child list owns it
        Label          *_label{nullptr};    // A child: our child list owns it
        void addedToRender() override;
        void placeControls();
    };
}
