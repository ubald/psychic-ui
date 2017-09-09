#pragma once

#include <string>
#include <SkCanvas.h>
#include "Div.hpp"

namespace psychicui {
    class Panel : public Div {
    public:
        explicit Panel(const std::string &title = "Untitled");

        const std::string &title() const { return _title; }

        void setTitle(const std::string &title) { _title = title; }

        std::shared_ptr<Panel> panel() override;

        bool modal() const { return _modal; }

        void setModal(bool modal) { _modal = modal; }

        void dispose();
        void center();

        void draw(SkCanvas *canvas) override;
        void refreshRelativePlacement();

    protected:
        std::string _title;
        Div      *_buttonPanel;
        bool        _modal;
        bool        _drag;
    };
}
