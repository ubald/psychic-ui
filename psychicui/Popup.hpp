#pragma once

#include "psychicui.hpp"
#include "Window.hpp"

namespace psychicui {
    class Popup : public Window {
    public:
        enum Side {
            Left = 0, Right
        };
        Popup(Widget *parent, Window *parentWindow);
        void setAnchorPosition(const Vector2i &anchorPosition);
        const Vector2i &anchorPosition() const;
        void setAnchorHeight(int anchorHeight);
        int anchorHeight() const;
        void setSide(Side popupSide);
        Side side() const;
        Window *parentWindow();
        const Window *parentWindow() const;
        virtual void performLayout(NVGcontext *ctx) override;
        virtual void draw(NVGcontext *ctx) override;
        void refreshRelativePlacement() override;

    protected:

    protected:
        Window   *_parentWindow;
        Vector2i _anchorPosition;
        int      _anchorHeight;
        Side     _side;
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
}
