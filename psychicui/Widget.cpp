#include "Widget.hpp"
#include "Window.hpp"

namespace psychicui {

    Widget::Widget() {
        YGNodeSetContext(_yogaNode, this);
    }

    Widget::~Widget() {
        YGNodeFree(_yogaNode);
    }

    // HIERARCHY

    Widget * Widget::parent() {
        return _parent;
    }

    const Widget * Widget::parent() const {
        return _parent;
    }

    void Widget::setParent(Widget * parent) {
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

    // CHILDREN

    int Widget::childCount() const {
        return (int) _children.size();
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
    }

    void Widget::removeChild(int index) {
        assert(index <= childCount());
        std::shared_ptr<Widget> widget = _children[index];
        _children.erase(_children.begin() + index);
    }

    int Widget::childIndex(std::shared_ptr<Widget> widget) const {
        assert(widget != nullptr);
        auto it = std::find(_children.begin(), _children.end(), widget);
        if (it == _children.end()) {
            return -1;
        }
        return (int) (it - _children.begin());
    }

    const std::shared_ptr<Widget> Widget::childAt(int index) const {
        assert(index <= childCount());
        return _children[index];
    }

    std::shared_ptr<Widget> Widget::childAt(int index) {
        assert(index <= childCount());
        return _children[index];
    }

    // VISIBILITY

    bool Widget::visible() const {
        return _visible;
    }

    void Widget::setVisible(bool visible) {
        _visible = visible;
    }

    bool Widget::visibleRecursive() const {
        bool visible = _visible;
        if (_parent) {
            visible &= _parent->visibleRecursive();
        }
        return visible;
    }

    // FOCUS

    bool Widget::focused() const {
        return _focused;
    }

    void Widget::setFocused(bool focused) {
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

    // CURSOR

    Cursor Widget::cursor() const {
        return _cursor;
    }

    void Widget::setCursor(Cursor cursor) {
        _cursor = cursor;
    }

    // HIT TESTS

    bool Widget::contains(const Vector2i &p) const {
        auto d = (p - _position).array();
        return (d >= 0).all() && (d < _size.array()).all();
    }

    std::shared_ptr<Widget> Widget::findWidget(const Vector2i &p) {
        for (auto child: _children) {
            if (child->visible() && child->contains(p - _position)) {
                return child->findWidget(p - _position);
            }
        }
        return contains(p) ? shared_from_this() : nullptr;
    }

    // POSITION

    const Vector2i &Widget::position() const {
        return _position;
    }

    void Widget::setPosition(const Vector2i &position) {
        _position = position;
    }

    const Vector2i Widget::absolutePosition() const {
        return _parent ? _parent->absolutePosition() + _position : _position;
    }

    int Widget::x() const {
        return _position.x();
    }

    void Widget::setX(int x) {
        _position[0] = x;
    }

    int Widget::y() const {
        return _position.y();
    }

    void Widget::setY(int y) {
        _position[1] = y;
    }

    // SIZE

    const Vector2i &Widget::size() const {
        return _size;
    }

    void Widget::setSize(const Vector2i &size) {
        _size = size;
    }

    int Widget::width() const {
        return _size.x();
    }

    void Widget::setWidth(int width) {
        _size[0] = width;
    }

    int Widget::height() const {
        return _size.y();
    }

    void Widget::setHeight(int height) {
        _size[1] = height;
    }

    void Widget::setFixedSize(const Vector2i &fixedSize) {
        _fixedSize = fixedSize;
    }

    const Vector2i &Widget::fixedSize() const {
        return _fixedSize;
    }

    int Widget::fixedWidth() const {
        return _fixedSize.x();
    }

    void Widget::setFixedWidth(int width) {
        _fixedSize[0] = width;
    }

    int Widget::fixedHeight() const {
        return _fixedSize.y();
    }

    void Widget::setFixedHeight(int height) {
        _fixedSize[1] = height;
    }

    // STYLE

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

    // LAYOUT

    YGNodeRef Widget::yogaNode() {
        return _yogaNode;
    }

    // DRAW

    void Widget::draw(NVGcontext *ctx) {
        #if NANOGUI_SHOW_WIDGET_BOUNDS
        nvgStrokeWidth(ctx, 1.0f);
        nvgBeginPath(ctx);
        nvgRect(ctx, _position.x() - 0.5f, _position.y() - 0.5f, _size.x() + 1, _size.y() + 1);
        nvgStrokeColor(ctx, nvgRGBA(255, 0, 0, 255));
        nvgStroke(ctx);
        #endif

        if (_children.empty()) {
            return;
        }

        nvgSave(ctx);
        nvgTranslate(ctx, _position.x(), _position.y());
        for (auto child : _children) {
            if (child->visible()) {
                nvgSave(ctx);
                nvgIntersectScissor(
                    ctx,
                    child->_position.x(),
                    child->_position.y(),
                    child->_size.x(),
                    child->_size.y());
                child->draw(ctx);
                nvgRestore(ctx);
            }
        }
        nvgRestore(ctx);
    }

    // INTERACTION

    void Widget::mouseButton(const Vector2i &p, int button, bool down, int modifiers) {

    }

    bool Widget::mouseButtonPropagation(const Vector2i &p, int button, bool down, int modifiers) {
        if (!_visible || !contains(p)) {
            return false;
        }

        mouseButton(p, button, down, modifiers);

        for (auto child: _children) {
            if (child->mouseButtonPropagation(p - _position, button, down, modifiers)) {
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

    void Widget::mouseMoved(const Vector2i &p, int button, int modifiers) {

    }

    void Widget::mouseMovedPropagation(const Vector2i &p, int button, int modifiers) {
        if (!_visible) {
            return;
        }

        bool over       = contains(p);
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
            mouseMoved(p, button, modifiers);
            for (auto child: _children) {
                child->mouseMovedPropagation(p - _position, button, modifiers);
            }
        }
    }

    void Widget::mouseScrolled(const Vector2i &p, const Vector2f &rel) {

    }

    bool Widget::mouseScrolledPropagation(const Vector2i &p, const Vector2f &rel) {
        if (!_visible || !contains(p)) {
            return false;
        }

        mouseScrolled(p, rel);

        for (auto child: _children) {
            if (child->mouseScrolledPropagation(p - _position, rel)) {
                break;
            }
        }

        return true;
    }

    bool Widget::mouseDragEvent(const Vector2i &, const Vector2i &, int, int) {
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

}