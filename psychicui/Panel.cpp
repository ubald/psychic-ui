#include "Window.hpp"

namespace psychicui {


    Panel::Panel(const std::string &title) :
        Widget(),
        _title(title),
        _buttonPanel(nullptr),
        _modal(false),
        _drag(false) {
            YGNodeStyleSetPositionType(_yogaNode, YGPositionTypeAbsolute);
    }

    std::shared_ptr<Panel> Panel::panel() {
        return std::dynamic_pointer_cast<Panel>(shared_from_this());
    }

    void Panel::draw(NVGcontext *ctx) {
//        int ds = mTheme->mPanelDropShadowSize, cr = mTheme->mPanelCornerRadius;
//        int hh = mTheme->mPanelHeaderHeight;
//
//        /* Draw panel */
//        nvgSave(ctx);
//        nvgBeginPath(ctx);
//        nvgRoundedRect(ctx, _position.x(), _position.y(), mSize.x(), mSize.y(), cr);
//
//        nvgFillColor(
//            ctx, mMouseFocus ? mTheme->mPanelFillFocused
//                             : mTheme->mPanelFillUnfocused
//        );
//        nvgFill(ctx);
//
//
//        /* Draw a drop shadow */
//        NVGpaint shadowPaint = nvgBoxGradient(
//            ctx, _position.x(), _position.y(), mSize.x(), mSize.y(), cr * 2, ds * 2,
//            mTheme->mDropShadow, mTheme->mTransparent
//        );
//
//        nvgSave(ctx);
//        nvgResetScissor(ctx);
//        nvgBeginPath(ctx);
//        nvgRect(ctx, _position.x() - ds, _position.y() - ds, mSize.x() + 2 * ds, mSize.y() + 2 * ds);
//        nvgRoundedRect(ctx, _position.x(), _position.y(), mSize.x(), mSize.y(), cr);
//        nvgPathWinding(ctx, NVG_HOLE);
//        nvgFillPaint(ctx, shadowPaint);
//        nvgFill(ctx);
//        nvgRestore(ctx);
//
//        if (!_title.empty()) {
//            /* Draw header */
//            NVGpaint headerPaint = nvgLinearGradient(
//                ctx, _position.x(), _position.y(), _position.x(),
//                _position.y() + hh,
//                mTheme->mPanelHeaderGradientTop,
//                mTheme->mPanelHeaderGradientBot
//            );
//
//            nvgBeginPath(ctx);
//            nvgRoundedRect(ctx, _position.x(), _position.y(), mSize.x(), hh, cr);
//
//            nvgFillPaint(ctx, headerPaint);
//            nvgFill(ctx);
//
//            nvgBeginPath(ctx);
//            nvgRoundedRect(ctx, _position.x(), _position.y(), mSize.x(), hh, cr);
//            nvgStrokeColor(ctx, mTheme->mPanelHeaderSepTop);
//
//            nvgSave(ctx);
//            nvgIntersectScissor(ctx, _position.x(), _position.y(), mSize.x(), 0.5f);
//            nvgStroke(ctx);
//            nvgRestore(ctx);
//
//            nvgBeginPath(ctx);
//            nvgMoveTo(ctx, _position.x() + 0.5f, _position.y() + hh - 1.5f);
//            nvgLineTo(ctx, _position.x() + mSize.x() - 0.5f, _position.y() + hh - 1.5);
//            nvgStrokeColor(ctx, mTheme->mPanelHeaderSepBot);
//            nvgStroke(ctx);
//
//            nvgFontSize(ctx, 18.0f);
//            nvgFontFace(ctx, "sans-bold");
//            nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
//
//            nvgFontBlur(ctx, 2);
//            nvgFillColor(ctx, mTheme->mDropShadow);
//            nvgText(
//                ctx, _position.x() + mSize.x() / 2,
//                _position.y() + hh / 2, _title.c_str(), nullptr
//            );
//
//            nvgFontBlur(ctx, 0);
//            nvgFillColor(
//                ctx, mFocused ? mTheme->mPanelTitleFocused
//                              : mTheme->mPanelTitleUnfocused
//            );
//            nvgText(
//                ctx, _position.x() + mSize.x() / 2, _position.y() + hh / 2 - 1,
//                _title.c_str(), nullptr
//            );
//        }
//
//        nvgRestore(ctx);
        Widget::draw(ctx);
    }

    void Panel::dispose() {
//        Widget *widget = this;
//        while (widget->parent()) {
//            widget = widget->parent();
//        }
//        ((Window *) widget)->disposePanel(this);
    }

    void Panel::center() {
//        Widget *widget = this;
//        while (widget->parent()) {
//            widget = widget->parent();
//        }
//        ((Window *) widget)->centerPanel(this);
    }

    bool Panel::mouseDragEvent(const Vector2i &, const Vector2i &rel, int button, int /* modifiers */) {
        if (_drag && (button & (1 << GLFW_MOUSE_BUTTON_1)) != 0) {
            _position += rel;
            _position = _position.cwiseMax(Vector2i::Zero());
            _position = _position.cwiseMin(parent()->size() - _size);
            return true;
        }
        return false;
    }

    void Panel::mouseButton(const Vector2i &p, int button, bool down, int modifiers) {
        Widget::mouseButton(p, button, down, modifiers);
        if (button == GLFW_MOUSE_BUTTON_1) {
            _drag = down && (p.y() - _position.y()) < style()->titleBarHeight;
        }
    }

    void Panel::refreshRelativePlacement() {

    }

}