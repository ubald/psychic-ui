#include "Popup.hpp"

namespace psychicui {

    Popup::Popup(std::shared_ptr<Panel> parentPanel) :
        Panel("Untitled"),
        _parentPanel(parentPanel),
        _anchorHeight(30),
        _side(Side::Right) {
    }

    void Popup::setAnchorPosition(const int &anchorX, const int &anchorY) {
        _anchorX = anchorX;
        _anchorY = anchorY;
    }

    void Popup::setAnchorHeight(int anchorHeight) {
        _anchorHeight = anchorHeight;
    }

    int Popup::anchorHeight() const {
        return _anchorHeight;
    }

    void Popup::setSide(Side popupSide) {
        _side = popupSide;
    }

    Popup::Side Popup::side() const {
        return _side;
    }

    std::shared_ptr<Panel> Popup::parentPanel() {
        return _parentPanel.lock();
    }

    const std::shared_ptr<Panel> Popup::parentPanel() const {
        return _parentPanel.lock();
    }

    void Popup::refreshRelativePlacement() {
//        if (auto pp = _parentPanel.lock()) {
//            pp->refreshRelativePlacement();
//            _visible &= pp->visibleRecursive();
//            _position = pp->position() + _anchorPosition - Vector2i(0, _anchorHeight);
//        }
    }

    void Popup::draw(SkCanvas *canvas) {

    }
//    void Popup::draw(NVGcontext *ctx) {
//        refreshRelativePlacement();
//
//        if (!_visible) {
//            return;
//        }
//
//        int ds = style()->panelDropShadowSize;
//        int cr = style()->panelCornerRadius;
//
//        nvgSave(ctx);
//        nvgResetScissor(ctx);
//
//        /* Draw a drop shadow */
//        NVGpaint shadowPaint = nvgBoxGradient(
//            ctx, _position.x(), _position.y(), _size.x(), _size.y(), cr * 2, ds * 2,
//            style()->panelDropShadowColor, style()->transparent
//        );
//
//        nvgBeginPath(ctx);
//        nvgRect(ctx, _position.x() - ds, _position.y() - ds, _size.x() + 2 * ds, _size.y() + 2 * ds);
//        nvgRoundedRect(ctx, _position.x(), _position.y(), _size.x(), _size.y(), cr);
//        nvgPathWinding(ctx, NVG_HOLE);
//        nvgFillPaint(ctx, shadowPaint);
//        nvgFill(ctx);
//
//        /* Draw panel */
//        nvgBeginPath(ctx);
//        nvgRoundedRect(ctx, _position.x(), _position.y(), _size.x(), _size.y(), cr);
//
//        Vector2i base = _position + Vector2i(0, _anchorHeight);
//        int      sign = -1;
//        if (_side == Side::Left) {
//            base.x() += _size.x();
//            sign = 1;
//        }
//
//        nvgMoveTo(ctx, base.x() + 15 * sign, base.y());
//        nvgLineTo(ctx, base.x() - 1 * sign, base.y() - 15);
//        nvgLineTo(ctx, base.x() - 1 * sign, base.y() + 15);
//
//        nvgFillColor(ctx, style()->popupBackgroundColor);
//        nvgFill(ctx);
//        nvgRestore(ctx);
//
//        Component::draw(ctx);
//    }

}