#pragma once

#include <SkCanvas.h>
#include "psychicui.hpp"
#include "Panel.hpp"

namespace psychicui {
    class Popup : public Panel {
    public:
        enum Side {
            Left = 0, Right
        };
        Popup(std::shared_ptr<Panel> parentPanel);
        void setAnchorPosition(const int &anchorX, const int &anchorY);
        void setAnchorHeight(int anchorHeight);
        int anchorHeight() const;
        void setSide(Side popupSide);
        Side side() const;
        std::shared_ptr<Panel> parentPanel();
        const std::shared_ptr<Panel> parentPanel() const;
//        virtual void draw(NVGcontext *ctx) override;
        virtual void draw(SkCanvas *canvas) override;
        void refreshRelativePlacement() override;

    protected:

    protected:
        std::weak_ptr<Panel> _parentPanel;
        int                  _anchorX{0};
        int                  _anchorY{0};
        int                  _anchorHeight;
        Side                 _side;
    };
}
