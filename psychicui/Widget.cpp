#include <iostream>
#include "Widget.hpp"
#include "Window.hpp"

namespace psychicui {

    Widget::Widget() :
        _yogaNode(YGNodeNew()) {
        YGNodeSetContext(_yogaNode, this);
        // We don't like these from the web default
        YGNodeStyleSetFlexDirection(_yogaNode, YGFlexDirectionColumn);
    }

    Widget::~Widget() {
        YGNodeFree(_yogaNode);
    }

    // region Hierarchy

    Widget *Widget::parent() {
        return _parent;
    }

    const Widget *Widget::parent() const {
        return _parent;
    }

    void Widget::setParent(Widget *parent) {
        _parent = parent;
    }

    std::shared_ptr<Panel> Widget::panel() {
        return _parent ? _parent->panel() : nullptr;
    }

    std::vector<std::shared_ptr<Widget>> Widget::path() {
        std::vector<std::shared_ptr<Widget>> path;
        if (_parent) {
            path = _parent->path();
        }
        path.insert(path.begin(), shared_from_this());
        return path;
    }

    // endregion

    // region Children

    unsigned int Widget::childCount() const {
        return (unsigned int) _children.size();
    }

    const std::vector<std::shared_ptr<Widget>> Widget::children() const {
        return _children;
    }

    void Widget::addChild(unsigned int index, std::shared_ptr<Widget> widget) {
        assert(index <= childCount());
        assert(widget != nullptr);
        _children.insert(_children.begin() + index, widget);
        YGNodeInsertChild(_yogaNode, widget->yogaNode(), index);
        widget->setParent(this);
    }

    void Widget::addChild(std::shared_ptr<Widget> widget) {
        assert(widget != nullptr);
        addChild(childCount(), widget);
    }

    void Widget::removeChild(const std::shared_ptr<Widget> widget) {
        assert(widget != nullptr);
        _children.erase(std::remove(_children.begin(), _children.end(), widget), _children.end());
        YGNodeRemoveChild(_yogaNode, widget->yogaNode());
    }

    void Widget::removeChild(unsigned int index) {
        assert(index <= childCount());
        std::shared_ptr<Widget> widget = _children[index];
        _children.erase(_children.begin() + index);
        YGNodeRemoveChild(_yogaNode, widget->yogaNode());
    }

    int Widget::childIndex(std::shared_ptr<Widget> widget) const {
        assert(widget != nullptr);
        auto it = std::find(_children.begin(), _children.end(), widget);
        if (it == _children.end()) {
            return -1;
        }
        return it - _children.begin();
    }

    const std::shared_ptr<Widget> Widget::childAt(unsigned int index) const {
        assert(index <= childCount());
        return _children[index];
    }

    std::shared_ptr<Widget> Widget::childAt(unsigned int index) {
        assert(index <= childCount());
        return _children[index];
    }

    // endregion

    // region Visibility

    bool Widget::visible() const {
        return _visible;
    }

    void Widget::setVisible(bool value) {
        _visible = value;
    }

    bool Widget::visibleRecursive() const {
        bool visible = _visible;
        if (_parent) {
            visible &= _parent->visibleRecursive();
        }
        return visible;
    }

    // endregion

    // region Focus

    bool Widget::focused() const {
        return _focused;
    }

    void Widget::focused(bool focused) {
        _focused = focused;
    }

    void Widget::requestFocus() {
        requestFocus(this);
    }

    void Widget::requestFocus(Widget *widget) {
        if (_parent) {
            _parent->requestFocus(widget);
        }
    }

    // endregion

    // region Cursor

    Cursor Widget::cursor() const {
        return _cursor;
    }

    void Widget::setCursor(Cursor cursor) {
        _cursor = cursor;
    }

    // endregion

    // region Hit Tests

    bool Widget::contains(const int &x, const int &y) const {
        int lx = x - _x, ly = y - _y;
        return lx >= 0 && lx < _width && ly >= 0 && ly < _height;
    }

    std::shared_ptr<Widget> Widget::findWidget(const int &x, const int &y) {
        int             lx = x - _x, ly = y - _y;
        for (const auto &child: _children) {
            if (child->visible() && child->contains(lx, ly)) {
                return child->findWidget(lx, ly);
            }
        }
        return contains(lx, ly) ? shared_from_this() : nullptr;
    }

    // endregion

    // region Position

//    const Vector2i &Widget::setPosition() const {
//        return _position;
//    }

    void Widget::setPosition(int x, int y) {
        _x = x;
        _y = y;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, _x >= 0 ? _x : YGUndefined);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, _y >= 0 ? _y : YGUndefined);
    }

//    const Vector2i Widget::absolutePosition() const {
//        return _parent ? _parent->absolutePosition() + _position : _position;
//    }

    int Widget::x() const {
        return _x;
    }

    void Widget::setX(int x) {
        _x = x;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, _x >= 0 ? _x : YGUndefined);
    }

    int Widget::y() const {
        return _y;
    }

    void Widget::setY(int y) {
        _y = y;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, _y >= 0 ? _y : YGUndefined);
    }

    // endregion

    // region Size

//    const Vector2i &Widget::setSize() const {
//        return _size;
//    }

    void Widget::setSize(int width, int height) {
        _width  = width;
        _height = height;
        YGNodeStyleSetWidth(_yogaNode, _width >= 0 ? _width : YGUndefined);
        YGNodeStyleSetHeight(_yogaNode, _height >= 0 ? _height : YGUndefined);
    }

    int Widget::width() const {
        return _width;
    }

    void Widget::setWidth(int width) {
        _width = width;
        YGNodeStyleSetWidth(_yogaNode, _width >= 0 ? _width : YGUndefined);
    }

    int Widget::height() const {
        return _height;
    }

    void Widget::setHeight(int height) {
        _height = height;
        YGNodeStyleSetHeight(_yogaNode, _height >= 0 ? _height : YGUndefined);
    }

    void Widget::setMinSize(float minWidth, float minHeight) {
        YGNodeStyleSetMinWidth(_yogaNode, minWidth >= 0 ? minWidth : YGUndefined);
        YGNodeStyleSetMinHeight(_yogaNode, minHeight >= 0 ? minHeight : YGUndefined);
    }

    float Widget::minWidth() const {
        return YGNodeStyleGetMinWidth(_yogaNode).value;
    }

    void Widget::setMinWidth(float minWidth) {
        YGNodeStyleSetMinWidth(_yogaNode, minWidth >= 0 ? minWidth : YGUndefined);
    }

    float Widget::minHeight() const {
        return YGNodeStyleGetMinHeight(_yogaNode).value;
    }

    void Widget::setMinHeight(float minHeight) {
        YGNodeStyleSetMinHeight(_yogaNode, minHeight >= 0 ? minHeight : YGUndefined);
    }
    
    void Widget::setMaxSize(float maxWidth, float maxHeight) {
        YGNodeStyleSetMaxWidth(_yogaNode, maxWidth >= 0 ? maxWidth : YGUndefined);
        YGNodeStyleSetMaxHeight(_yogaNode, maxHeight >= 0 ? maxHeight : YGUndefined);
    }

    float Widget::maxWidth() const {
        return YGNodeStyleGetMaxWidth(_yogaNode).value;
    }

    void Widget::setMaxWidth(float maxWidth) {
        YGNodeStyleSetMaxWidth(_yogaNode, maxWidth >= 0 ? maxWidth : YGUndefined);
    }

    float Widget::maxHeight() const {
        return YGNodeStyleGetMaxHeight(_yogaNode).value;
    }

    void Widget::setMaxHeight(float maxHeight) {
        YGNodeStyleSetMaxHeight(_yogaNode, maxHeight >= 0 ? maxHeight : YGUndefined);
    }

    // endregion

    // region Constraints

    void Widget::setPadding(int value) {
        setPaddingLeft(value);
        setPaddingRight(value);
        setPaddingTop(value);
        setPaddingBottom(value);
    }

    void Widget::setPadding(int horizontal, int vertical) {
        setPaddingLeft(horizontal);
        setPaddingRight(horizontal);
        setPaddingTop(vertical);
        setPaddingBottom(vertical);
    }

    void Widget::setPadding(int left, int right, int top, int bottom) {
        setPaddingLeft(left);
        setPaddingRight(right);
        setPaddingTop(top);
        setPaddingBottom(bottom);
    }

    int Widget::paddingLeft() const {
        return _paddingLeft;
    }

    void Widget::setPaddingLeft(int paddingLeft) {
        _paddingLeft = paddingLeft;
        YGNodeStyleSetPadding(_yogaNode, YGEdgeLeft, _paddingLeft);
    }

    int Widget::paddingRight() const {
        return _paddingRight;
    }

    void Widget::setPaddingRight(int paddingRight) {
        _paddingRight = paddingRight;
        YGNodeStyleSetPadding(_yogaNode, YGEdgeRight, _paddingRight);
    }

    int Widget::paddingTop() const {
        return _paddingTop;
    }

    void Widget::setPaddingTop(int paddingTop) {
        _paddingTop = paddingTop;
        YGNodeStyleSetPadding(_yogaNode, YGEdgeTop, _paddingTop);
    }

    int Widget::paddingBottom() const {
        return _paddingBottom;
    }

    void Widget::setPaddingBottom(int paddingBottom) {
        _paddingBottom = paddingBottom;
        YGNodeStyleSetPadding(_yogaNode, YGEdgeBottom, _paddingBottom);
    }

    bool Widget::wrap() const {
        return _wrap;
    }

    void Widget::setWrap(bool wrap) {
        _wrap = wrap;
        YGNodeStyleSetFlexWrap(_yogaNode, _wrap ? YGWrapWrap : YGWrapNoWrap);
    }

    float Widget::flex() const {
        return YGNodeStyleGetFlex(_yogaNode);
    }

    void Widget::setFlex(float flex) {
        YGNodeStyleSetFlex(_yogaNode, flex);
    }

    float Widget::flexShrink() const {
        return YGNodeStyleGetFlexShrink(_yogaNode);
    }

    void Widget::setFlexShrink(float flexShrink) {
        YGNodeStyleSetFlexShrink(_yogaNode, flexShrink);
    }

    float Widget::flexGrow() const {
        return YGNodeStyleGetFlexGrow(_yogaNode);
    }

    void Widget::setFlexGrow(float flexGrow) {
        YGNodeStyleSetFlexGrow(_yogaNode, flexGrow);
    }

    float Widget::flexBasis() const {
        return YGNodeStyleGetFlexBasis(_yogaNode).value;
    }

    void Widget::setFlexBasis(float flexBasis) {
        YGNodeStyleSetFlexBasis(_yogaNode, flexBasis);
    }

    Layout Widget::layout() const {
        return _layout;
    }

    void Widget::setLayout(Layout layout) {
        _layout = layout;
        switch (_layout) {
            case Horizontal:
                YGNodeStyleSetFlexDirection(_yogaNode, YGFlexDirectionRow);
                break;
            case Vertical:
                YGNodeStyleSetFlexDirection(_yogaNode, YGFlexDirectionColumn);
                break;
        }
    }

    // endregion

    // region Style

    std::shared_ptr<Style> Widget::style() {
        if (_style) {
            return _style;
        } else if (_parent) {
            return _parent->style();
        } else {
            return Style::defaultStyle;
        }
    }

    const std::shared_ptr<Style> Widget::style() const {
        if (_style) {
            return _style;
        } else if (_parent) {
            return _parent->style();
        } else {
            return Style::defaultStyle;
        }
    }

    void Widget::setStyle(std::shared_ptr<Style> style) {
        _style = style;
    }

    // endregion

    // region Layout

    YGNodeRef Widget::yogaNode() {
        return _yogaNode;
    }

    void Widget::invalidate() {
        YGNodeMarkDirty(_yogaNode);
    }

    void Widget::setMeasurable() {
        YGNodeSetMeasureFunc(
            _yogaNode,
            [](YGNodeRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
                YGSize size{};
                auto   widget = static_cast<Widget *>(YGNodeGetContext(node));
                if (!widget) {
                    std::cerr << "Could not find widget to measure" << std::endl;
                    return size;
                }
                size = widget->measure(width, widthMode, height, heightMode);
                return size;
            }
        );
    }

    // endregion

    // region Draw

    YGSize Widget::measure(float width, YGMeasureMode /*widthMode*/, float height, YGMeasureMode /*heightMode*/) {
        return YGSize{width, height};
    }

    void Widget::render(SkCanvas *canvas) {
        float left   = YGNodeLayoutGetLeft(_yogaNode);
        float top    = YGNodeLayoutGetTop(_yogaNode);
//        float right  = YGNodeLayoutGetRight(_yogaNode);
//        float bottom = YGNodeLayoutGetBottom(_yogaNode);
        float width  = YGNodeLayoutGetWidth(_yogaNode);
        float height = YGNodeLayoutGetHeight(_yogaNode);

        _x      = (int) left;
        _y      = (int) top;
        _width  = (int) width;
        _height = (int) height;
        _rect.set(_x, _y, _x + _width, _y + _height);

        canvas->save();
        canvas->translate(_x, _y);

        draw(canvas);

        for (const auto &child : _children) {
            if (child->visible()) {
                canvas->save();
                child->render(canvas);
                canvas->restore();
            }
        }

//        #if PSYCHICUI_SHOW_WIDGET_BOUNDS
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(1));
        paint.setColor(0x7FFF0000);
        canvas->drawRect({0.5f, 0.5f, _width - 0.5f, _height - 0.5f}, paint);

        paint.setColor(0x7F0000FF);
        canvas->drawRect(
            {
                _paddingLeft + 0.5f,
                _paddingTop + 0.5f,
                _width - _paddingRight - 0.5f,
                _height - _paddingBottom - 0.5f
            },
            paint
        );

        if (_mouseDown) {
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(0x30FFFFFF);
            canvas->drawRect({0.5f, 0.5f, _width - 0.5f, _height - 0.5f}, paint);
        } else if (_mouseOver) {
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(0x10FFFFFF);
            canvas->drawRect({0.5f, 0.5f, _width - 0.5f, _height - 0.5f}, paint);
        }
//        #endif

        canvas->restore();
    }

    void Widget::draw(SkCanvas *canvas) {

    }

//    void Widget::draw(NVGcontext *ctx) {
//        #if NANOGUI_SHOW_WIDGET_BOUNDS
//        nvgStrokeWidth(ctx, 1.0f);
//        nvgBeginPath(ctx);
//        nvgRect(ctx, _position.x() - 0.5f, _position.y() - 0.5f, _size.setX() + 1, _size.setY() + 1);
//        nvgStrokeColor(ctx, nvgRGBA(255, 0, 0, 255));
//        nvgStroke(ctx);
//        #endif
//
//        if (_children.empty()) {
//            return;
//        }
//
//        nvgSave(ctx);
//        nvgTranslate(ctx, _position.setX(), _position.setY());
//        for (auto child : _children) {
//            if (child->setVisible()) {
//                nvgSave(ctx);
//                nvgIntersectScissor(
//                    ctx,
//                    child->_position.setX(),
//                    child->_position.setY(),
//                    child->_size.setX(),
//                    child->_size.setY());
//                child->draw(ctx);
//                nvgRestore(ctx);
//            }
//        }
//        nvgRestore(ctx);
//    }

    // endregion

    // region Interaction

    void Widget::mouseButton(const int &mouseX, const int &mouseY, int button, bool down, int modifiers) {

    }

    void Widget::mouseDown() {

    }

    void Widget::mouseUp() {

    }

    bool Widget::mouseButtonPropagation(const int &mouseX, const int &mouseY, int button, bool down, int modifiers) {
        if (!_visible || !contains(mouseX, mouseY)) {
            return false;
        }

        mouseButton(mouseX, mouseY, button, down, modifiers);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (down != _mouseDown) {
                _mouseDown = down;
                if (_mouseDown) {
                    mouseDown();
                } else {
                    mouseUp();
                }
            }
        }


        for (auto child: _children) {
            if (child->mouseButtonPropagation(mouseX - _x, mouseY - _y, button, down, modifiers)) {
                break;
            }
        }

        if (button == GLFW_MOUSE_BUTTON_1 && down && !_focused) {
            requestFocus();
        }

        return true;
    }

    void Widget::mouseOver() {

    }

    void Widget::mouseOut() {

    }

    void Widget::mouseMoved(const int &mouseX, const int &mouseY, int button, int modifiers) {

    }

    void Widget::mouseMovedPropagation(const int &mouseX, const int &mouseY, int button, int modifiers) {
        if (!_visible) {
            return;
        }

        bool over       = contains(mouseX, mouseY);
        bool overBefore = _mouseOver;

        if (over != _mouseOver) {
            _mouseOver = over;
            if (_mouseOver) {
                mouseOver();
            } else {
                mouseOut();
            }
        }

        if (over || overBefore) {
            mouseMoved(mouseX, mouseY, button, modifiers);
            for (auto child: _children) {
                child->mouseMovedPropagation(mouseX - _x, mouseY - _y, button, modifiers);
            }
        }
    }

    void Widget::mouseScrolled(const int &mouseX, const int &mouseY, const int &scrollX, const int &scrollY) {

    }

    bool Widget::mouseScrolledPropagation(const int &mouseX, const int &mouseY, const int &scrollX, const int &scrollY) {
        if (!_visible || !contains(mouseX, mouseY)) {
            return false;
        }

        mouseScrolled(mouseX, mouseY, scrollX, scrollY);

        for (auto child: _children) {
            if (child->mouseScrolledPropagation(mouseX - _x, mouseY - _y, scrollX, scrollY)) {
                break;
            }
        }

        return true;
    }

    bool Widget::mouseDragEvent(const int &mouseX, const int &mouseY, const int &dragX, const int &dragY, int button, int modifiers) {
        return false;
    }

    bool Widget::focusEvent(bool focused) {
        _focused = focused;
        return false;
    }

    bool Widget::keyboardEvent(int, int, int, int) {
        return false;
    }

    bool Widget::keyboardCharacterEvent(unsigned int) {
        return false;
    }

    // endregion

}