#include "Screen.hpp"
#include "Window.hpp"

namespace psychicui {


    Window::Window(Widget *parent, const std::string &title) : 
        Widget(parent), 
        _title(title), 
        _buttonPanel(nullptr), 
        _modal(false), 
        _drag(false) {
        }

    Vector2i Window::preferredSize(NVGcontext *ctx) const {
        if (_buttonPanel) {
            _buttonPanel->setVisible(false);
        }
        Vector2i result = Widget::preferredSize(ctx);
        if (_buttonPanel) {
            _buttonPanel->setVisible(true);
        }

        nvgFontSize(ctx, 18.0f);
        nvgFontFace(ctx, "sans-bold");
        float bounds[4];
        nvgTextBounds(ctx, 0, 0, _title.c_str(), nullptr, bounds);

        return result.cwiseMax(Vector2i(bounds[2] - bounds[0] + 20, bounds[3] - bounds[1]));
    }

    Widget *Window::buttonPanel() {
        if (!_buttonPanel) {
            _buttonPanel = new Widget(this);
//            _buttonPanel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 4));
        }
        return _buttonPanel;
    }

    void Window::performLayout(NVGcontext *ctx) {
        if (!_buttonPanel) {
            Widget::performLayout(ctx);
        } else {
            _buttonPanel->setVisible(false);
            Widget::performLayout(ctx);
            for (auto w : _buttonPanel->children()) {
                w->setFixedSize(Vector2i(22, 22));
            }
            _buttonPanel->setVisible(true);
            _buttonPanel->setSize(Vector2i(width(), 22));
            _buttonPanel->setPosition(Vector2i(width() - (_buttonPanel->preferredSize(ctx).x() + 5), 3));
            _buttonPanel->performLayout(ctx);
        }
    }

    void Window::draw(NVGcontext *ctx) {
//        int ds = mTheme->mWindowDropShadowSize, cr = mTheme->mWindowCornerRadius;
//        int hh = mTheme->mWindowHeaderHeight;
//
//        /* Draw window */
//        nvgSave(ctx);
//        nvgBeginPath(ctx);
//        nvgRoundedRect(ctx, _position.x(), _position.y(), mSize.x(), mSize.y(), cr);
//
//        nvgFillColor(
//            ctx, mMouseFocus ? mTheme->mWindowFillFocused
//                             : mTheme->mWindowFillUnfocused
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
//                mTheme->mWindowHeaderGradientTop,
//                mTheme->mWindowHeaderGradientBot
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
//            nvgStrokeColor(ctx, mTheme->mWindowHeaderSepTop);
//
//            nvgSave(ctx);
//            nvgIntersectScissor(ctx, _position.x(), _position.y(), mSize.x(), 0.5f);
//            nvgStroke(ctx);
//            nvgRestore(ctx);
//
//            nvgBeginPath(ctx);
//            nvgMoveTo(ctx, _position.x() + 0.5f, _position.y() + hh - 1.5f);
//            nvgLineTo(ctx, _position.x() + mSize.x() - 0.5f, _position.y() + hh - 1.5);
//            nvgStrokeColor(ctx, mTheme->mWindowHeaderSepBot);
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
//                ctx, mFocused ? mTheme->mWindowTitleFocused
//                              : mTheme->mWindowTitleUnfocused
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

    void Window::dispose() {
        Widget *widget = this;
        while (widget->parent()) {
            widget = widget->parent();
        }
        ((Screen *) widget)->disposeWindow(this);
    }

    void Window::center() {
        Widget *widget = this;
        while (widget->parent()) {
            widget = widget->parent();
        }
        ((Screen *) widget)->centerWindow(this);
    }

    bool Window::mouseDragEvent(const Vector2i &, const Vector2i &rel, int button, int /* modifiers */) {
        if (_drag && (button & (1 << GLFW_MOUSE_BUTTON_1)) != 0) {
            _position += rel;
            _position = _position.cwiseMax(Vector2i::Zero());
            _position = _position.cwiseMin(parent()->size() - _size);
            return true;
        }
        return false;
    }

    void Window::mouseButton(const Vector2i &p, int button, bool down, int modifiers) {
        Widget::mouseButton(p, button, down, modifiers);
        if (button == GLFW_MOUSE_BUTTON_1) {
            _drag = down && (p.y() - _position.y()) < style()->titleBarHeight;
        }
    }

    void Window::refreshRelativePlacement() {

    }

}