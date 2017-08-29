#pragma once

#include <string>
#include <SkCanvas.h>
#include "Component.hpp"

namespace psychicui {
    class Panel : public Component {
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
        bool mouseDragEvent(const int &mouseX, const int &mouseY, const int &dragX, const int &dragY, int button, int modifiers) override;
        void mouseButton(const int &mouseX, const int &mouseY, int button, bool down, int modifiers) override;
        void refreshRelativePlacement();

    protected:
        std::string _title;
        Component      *_buttonPanel;
        bool        _modal;
        bool        _drag;
    };
}
