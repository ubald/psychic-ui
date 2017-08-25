#pragma once

#include "psychicui.hpp"
#include "Panel.hpp"

namespace psychicui {
    class Popup : public Panel {
    public:
        enum Side {
            Left = 0, Right
        };
        Popup(std::shared_ptr<Panel> parentPanel);
        void setAnchorPosition(const Vector2i &anchorPosition);
        const Vector2i &anchorPosition() const;
        void setAnchorHeight(int anchorHeight);
        int anchorHeight() const;
        void setSide(Side popupSide);
        Side side() const;
        std::shared_ptr<Panel> parentPanel();
        const std::shared_ptr<Panel> parentPanel() const;
        virtual void draw(NVGcontext *ctx) override;
        void refreshRelativePlacement() override;

    protected:

    protected:
        std::weak_ptr<Panel> _parentPanel;
        Vector2i             _anchorPosition;
        int                  _anchorHeight;
        Side                 _side;
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
}
