#pragma once

#include "Widget.hpp"

namespace psychicui {
    class Panel : public Widget {
    public:
        Panel(const std::string &title = "Untitled");

        const std::string &title() const { return _title; }
        void setTitle(const std::string &title) { _title = title; }

        std::shared_ptr<Panel> panel() override;

        bool modal() const { return _modal; }
        void setModal(bool modal) { _modal = modal; }

        Widget *buttonPanel();

        void dispose();
        void center();

        virtual void draw(NVGcontext *ctx) override;
        virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
        virtual void mouseButton(const Vector2i &p, int button, bool down, int modifiers) override;
        virtual void refreshRelativePlacement();

    protected:
        std::string _title;
        Widget      *_buttonPanel;
        bool        _modal;
        bool        _drag;
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
}
