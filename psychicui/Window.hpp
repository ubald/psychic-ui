#pragma once

#include "Widget.hpp"

namespace psychicui {
    class Window : public Widget {
    public:
        Window(Widget *parent, const std::string &title = "Untitled");
        const std::string &title() const { return _title; }
        void setTitle(const std::string &title) { _title = title; }
        bool modal() const { return _modal; }
        void setModal(bool modal) { _modal = modal; }
        Widget *buttonPanel();
        void dispose();
        void center();
        virtual void draw(NVGcontext *ctx) override;
        virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
        virtual void mouseButton(const Vector2i &p, int button, bool down, int modifiers) override;
        virtual Vector2i preferredSize(NVGcontext *ctx) const override;
        virtual void performLayout(NVGcontext *ctx) override;
        virtual void refreshRelativePlacement();
    protected:
    protected:
        std::string _title;
        Widget      *_buttonPanel;
        bool        _modal;
        bool        _drag;
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
}
