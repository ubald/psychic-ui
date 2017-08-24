#include "Popup.hpp"

namespace psychicui {

    Popup::Popup(Widget *parent, Window *parentWindow) :
        Window(parent, ""),
        _parentWindow(parentWindow),
        _anchorPosition(Vector2i::Zero()),
        _anchorHeight(30),
        _side(Side::Right) {
    }

    void Popup::setAnchorPosition(const Vector2i &anchorPosition) {
        _anchorPosition = anchorPosition;
    }

    const Vector2i &Popup::anchorPosition() const {
        return _anchorPosition;
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

    Window *Popup::parentWindow() {
        return _parentWindow;
    }

    const Window *Popup::parentWindow() const {
        return _parentWindow;
    }

    void Popup::performLayout(NVGcontext *ctx) {
        if (_layout || _children.size() != 1) {
            Widget::performLayout(ctx);
        } else {
            _children[0]->setPosition(Vector2i::Zero());
            _children[0]->setSize(_size);
            _children[0]->performLayout(ctx);
        }
        if (_side == Side::Left) {
            _anchorPosition[0] -= size()[0];
        }
    }

    void Popup::refreshRelativePlacement() {
        _parentWindow->refreshRelativePlacement();
        _visible &= _parentWindow->visibleRecursive();
        _position = _parentWindow->position() + _anchorPosition - Vector2i(0, _anchorHeight);
    }

    void Popup::draw(NVGcontext *ctx) {
        refreshRelativePlacement();

        if (!_visible) {
            return;
        }

        int ds = style()->windowDropShadowSize;
        int cr = style()->windowCornerRadius;

        nvgSave(ctx);
        nvgResetScissor(ctx);

        /* Draw a drop shadow */
        NVGpaint shadowPaint = nvgBoxGradient(
            ctx, _position.x(), _position.y(), _size.x(), _size.y(), cr * 2, ds * 2,
            style()->windowDropShadowColor, style()->transparent
        );

        nvgBeginPath(ctx);
        nvgRect(ctx, _position.x() - ds, _position.y() - ds, _size.x() + 2 * ds, _size.y() + 2 * ds);
        nvgRoundedRect(ctx, _position.x(), _position.y(), _size.x(), _size.y(), cr);
        nvgPathWinding(ctx, NVG_HOLE);
        nvgFillPaint(ctx, shadowPaint);
        nvgFill(ctx);

        /* Draw window */
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, _position.x(), _position.y(), _size.x(), _size.y(), cr);

        Vector2i base = _position + Vector2i(0, _anchorHeight);
        int      sign = -1;
        if (_side == Side::Left) {
            base.x() += _size.x();
            sign = 1;
        }

        nvgMoveTo(ctx, base.x() + 15 * sign, base.y());
        nvgLineTo(ctx, base.x() - 1 * sign, base.y() - 15);
        nvgLineTo(ctx, base.x() - 1 * sign, base.y() + 15);

        nvgFillColor(ctx, style()->popupBackgroundColor);
        nvgFill(ctx);
        nvgRestore(ctx);

        Widget::draw(ctx);
    }

}