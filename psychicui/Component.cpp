#include <iostream>
#include "Component.hpp"
#include "StyleManager.hpp"

namespace psychicui {

    Component::Component() :
        _style(Style::getDefaultStyle()),
        _yogaNode(YGNodeNew()) {
        setComponentType("Component");

        YGNodeSetContext(_yogaNode, this);
        // We don't like these from the web default
        YGNodeStyleSetFlexDirection(_yogaNode, YGFlexDirectionColumn);
    }

    Component::~Component() {
        YGNodeFree(_yogaNode);
    }

    void Component::setComponentType(std::string componentName) {
        _componentType.push_back(componentName);
    }

    // region Hierarchy

    Component *Component::parent() {
        return _parent;
    }

    const Component *Component::parent() const {
        return _parent;
    }

    void Component::setParent(Component *parent) {
        _parent = parent;
    }

    std::shared_ptr<Panel> Component::panel() {
        return _parent ? _parent->panel() : nullptr;
    }

    std::vector<std::shared_ptr<Component>> Component::path() {
        std::vector<std::shared_ptr<Component>> path;
        if (_parent) {
            path = _parent->path();
        }
        path.insert(path.begin(), shared_from_this());
        return path;
    }

    // endregion

    // region Children

    unsigned int Component::childCount() const {
        return (unsigned int) _children.size();
    }

    const std::vector<std::shared_ptr<Component>> Component::children() const {
        return _children;
    }

    void Component::addChild(unsigned int index, std::shared_ptr<Component> component) {
        assert(index <= childCount());
        assert(component != nullptr);
        _children.insert(_children.begin() + index, component);
        YGNodeInsertChild(_yogaNode, component->yogaNode(), index);
        component->setParent(this);
    }

    void Component::addChild(std::shared_ptr<Component> component) {
        assert(component != nullptr);
        addChild(childCount(), component);
    }

    void Component::removeChild(const std::shared_ptr<Component> component) {
        assert(component != nullptr);
        _children.erase(std::remove(_children.begin(), _children.end(), component), _children.end());
        YGNodeRemoveChild(_yogaNode, component->yogaNode());
    }

    void Component::removeChild(unsigned int index) {
        assert(index <= childCount());
        std::shared_ptr<Component> component = _children[index];
        _children.erase(_children.begin() + index);
        YGNodeRemoveChild(_yogaNode, component->yogaNode());
    }

    int Component::childIndex(std::shared_ptr<Component> component) const {
        assert(component != nullptr);
        auto it = std::find(_children.begin(), _children.end(), component);
        if (it == _children.end()) {
            return -1;
        }
        return it - _children.begin();
    }

    const std::shared_ptr<Component> Component::childAt(unsigned int index) const {
        assert(index <= childCount());
        return _children[index];
    }

    std::shared_ptr<Component> Component::childAt(unsigned int index) {
        assert(index <= childCount());
        return _children[index];
    }

    // endregion

    // region Visibility

    bool Component::visible() const {
        return _visible;
    }

    void Component::setVisible(bool value) {
        _visible = value;
    }

    bool Component::visibleRecursive() const {
        bool visible = _visible;
        if (_parent) {
            visible &= _parent->visibleRecursive();
        }
        return visible;
    }

    // endregion

    // region Focus

    bool Component::focused() const {
        return _focused;
    }

    void Component::focused(bool focused) {
        _focused = focused;
    }

    void Component::requestFocus() {
        requestFocus(this);
    }

    void Component::requestFocus(Component *component) {
        if (_parent) {
            _parent->requestFocus(component);
        }
    }

    // endregion

    // region Cursor

    Cursor Component::cursor() const {
        return _cursor;
    }

    void Component::setCursor(Cursor cursor) {
        _cursor = cursor;
    }

    // endregion

    // region Hit Tests

    bool Component::contains(const int &x, const int &y) const {
        int lx = x - _x, ly = y - _y;
        return lx >= 0 && lx < _width && ly >= 0 && ly < _height;
    }

    std::shared_ptr<Component> Component::findComponent(const int &x, const int &y) {
        int             lx = x - _x, ly = y - _y;
        for (const auto &child: _children) {
            if (child->visible() && child->contains(lx, ly)) {
                return child->findComponent(lx, ly);
            }
        }
        return contains(lx, ly) ? shared_from_this() : nullptr;
    }

    // endregion

    // region Position

    void Component::setPosition(int x, int y) {
        _x = x;
        _y = y;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, _x >= 0 ? _x : YGUndefined);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, _y >= 0 ? _y : YGUndefined);
    }

    int Component::x() const {
        return _x;
    }

    void Component::setX(int x) {
        _x = x;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, _x >= 0 ? _x : YGUndefined);
    }

    int Component::y() const {
        return _y;
    }

    void Component::setY(int y) {
        _y = y;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, _y >= 0 ? _y : YGUndefined);
    }

    // endregion

    // region Dimensions

    void Component::setSize(int width, int height) {
        _width  = width;
        _height = height;
        YGNodeStyleSetWidth(_yogaNode, _width >= 0 ? _width : YGUndefined);
        YGNodeStyleSetHeight(_yogaNode, _height >= 0 ? _height : YGUndefined);
    }

    int Component::width() const {
        return _width;
    }

    void Component::setWidth(int width) {
        _width = width;
        YGNodeStyleSetWidth(_yogaNode, _width >= 0 ? _width : YGUndefined);
    }

    int Component::height() const {
        return _height;
    }

    void Component::setHeight(int height) {
        _height = height;
        YGNodeStyleSetHeight(_yogaNode, _height >= 0 ? _height : YGUndefined);
    }

    void Component::setMinSize(float minWidth, float minHeight) {
        YGNodeStyleSetMinWidth(_yogaNode, minWidth >= 0 ? minWidth : YGUndefined);
        YGNodeStyleSetMinHeight(_yogaNode, minHeight >= 0 ? minHeight : YGUndefined);
    }

    float Component::minWidth() const {
        return YGNodeStyleGetMinWidth(_yogaNode).value;
    }

    void Component::setMinWidth(float minWidth) {
        YGNodeStyleSetMinWidth(_yogaNode, minWidth >= 0 ? minWidth : YGUndefined);
    }

    float Component::minHeight() const {
        return YGNodeStyleGetMinHeight(_yogaNode).value;
    }

    void Component::setMinHeight(float minHeight) {
        YGNodeStyleSetMinHeight(_yogaNode, minHeight >= 0 ? minHeight : YGUndefined);
    }
    
    void Component::setMaxSize(float maxWidth, float maxHeight) {
        YGNodeStyleSetMaxWidth(_yogaNode, maxWidth >= 0 ? maxWidth : YGUndefined);
        YGNodeStyleSetMaxHeight(_yogaNode, maxHeight >= 0 ? maxHeight : YGUndefined);
    }

    float Component::maxWidth() const {
        return YGNodeStyleGetMaxWidth(_yogaNode).value;
    }

    void Component::setMaxWidth(float maxWidth) {
        YGNodeStyleSetMaxWidth(_yogaNode, maxWidth >= 0 ? maxWidth : YGUndefined);
    }

    float Component::maxHeight() const {
        return YGNodeStyleGetMaxHeight(_yogaNode).value;
    }

    void Component::setMaxHeight(float maxHeight) {
        YGNodeStyleSetMaxHeight(_yogaNode, maxHeight >= 0 ? maxHeight : YGUndefined);
    }

    // endregion

    // region Constraints

    void Component::setPadding(int value) {
        setPaddingLeft(value);
        setPaddingRight(value);
        setPaddingTop(value);
        setPaddingBottom(value);
    }

    void Component::setPadding(int horizontal, int vertical) {
        setPaddingLeft(horizontal);
        setPaddingRight(horizontal);
        setPaddingTop(vertical);
        setPaddingBottom(vertical);
    }

    void Component::setPadding(int left, int right, int top, int bottom) {
        setPaddingLeft(left);
        setPaddingRight(right);
        setPaddingTop(top);
        setPaddingBottom(bottom);
    }

    int Component::paddingLeft() const {
        return _paddingLeft;
    }

    void Component::setPaddingLeft(int paddingLeft) {
        _paddingLeft = paddingLeft;
        YGNodeStyleSetPadding(_yogaNode, YGEdgeLeft, _paddingLeft);
    }

    int Component::paddingRight() const {
        return _paddingRight;
    }

    void Component::setPaddingRight(int paddingRight) {
        _paddingRight = paddingRight;
        YGNodeStyleSetPadding(_yogaNode, YGEdgeRight, _paddingRight);
    }

    int Component::paddingTop() const {
        return _paddingTop;
    }

    void Component::setPaddingTop(int paddingTop) {
        _paddingTop = paddingTop;
        YGNodeStyleSetPadding(_yogaNode, YGEdgeTop, _paddingTop);
    }

    int Component::paddingBottom() const {
        return _paddingBottom;
    }

    void Component::setPaddingBottom(int paddingBottom) {
        _paddingBottom = paddingBottom;
        YGNodeStyleSetPadding(_yogaNode, YGEdgeBottom, _paddingBottom);
    }

    // endregion

    // region Layout

    bool Component::wrap() const {
        return _wrap;
    }

    void Component::setWrap(bool wrap) {
        _wrap = wrap;
        YGNodeStyleSetFlexWrap(_yogaNode, _wrap ? YGWrapWrap : YGWrapNoWrap);
    }

    float Component::flex() const {
        return YGNodeStyleGetFlex(_yogaNode);
    }

    void Component::setFlex(float flex) {
        YGNodeStyleSetFlex(_yogaNode, flex);
    }

    float Component::flexShrink() const {
        return YGNodeStyleGetFlexShrink(_yogaNode);
    }

    void Component::setFlexShrink(float flexShrink) {
        YGNodeStyleSetFlexShrink(_yogaNode, flexShrink);
    }

    float Component::flexGrow() const {
        return YGNodeStyleGetFlexGrow(_yogaNode);
    }

    void Component::setFlexGrow(float flexGrow) {
        YGNodeStyleSetFlexGrow(_yogaNode, flexGrow);
    }

    float Component::flexBasis() const {
        return YGNodeStyleGetFlexBasis(_yogaNode).value;
    }

    void Component::setFlexBasis(float flexBasis) {
        YGNodeStyleSetFlexBasis(_yogaNode, flexBasis);
    }

    Layout Component::layout() const {
        return _layout;
    }

    void Component::setLayout(Layout layout) {
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

    std::shared_ptr<Style> Component::style() {
        return _style;
    }

    void Component::setStyle(std::shared_ptr<Style> style) {
        _style = style;
    }

    std::shared_ptr<Style> Component::getComputedStyle() {
//        return StyleManager::computeStyle(this);
        return std::shared_ptr<Style>(Style::getDefaultStyle());
    }

    std::vector<std::string> Component::classNames() {
        return _classNames;
    }

    void Component::setClassNames(std::vector<std::string> classNames) {
        _classNames = classNames;
    }

    // endregion

    // region Layout

    YGNodeRef Component::yogaNode() {
        return _yogaNode;
    }

    void Component::invalidate() {
        YGNodeMarkDirty(_yogaNode);
    }

    void Component::setMeasurable() {
        YGNodeSetMeasureFunc(
            _yogaNode,
            [](YGNodeRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
                YGSize size{};
                auto   component = static_cast<Component *>(YGNodeGetContext(node));
                if (!component) {
                    std::cerr << "Could not find component to measure" << std::endl;
                    return size;
                }
                size = component->measure(width, widthMode, height, heightMode);
                return size;
            }
        );
    }

    // endregion

    // region Draw

    YGSize Component::measure(float width, YGMeasureMode /*widthMode*/, float height, YGMeasureMode /*heightMode*/) {
        return YGSize{width, height};
    }

    void Component::render(SkCanvas *canvas) {
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

        // TODO: Clip if overflow: hidden

        draw(canvas);

        for (const auto &child : _children) {
            if (child->visible()) {
                canvas->save();
                child->render(canvas);
                canvas->restore();
            }
        }

//        #if PSYCHICUI_SHOW_COMPONENT_BOUNDS
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

    void Component::draw(SkCanvas *canvas) {

    }

//    void Component::draw(NVGcontext *ctx) {
//        #if NANOGUI_SHOW_COMPONENT_BOUNDS
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

    void Component::mouseButton(const int &mouseX, const int &mouseY, int button, bool down, int modifiers) {

    }

    void Component::mouseDown() {

    }

    void Component::mouseUp() {

    }

    bool Component::mouseButtonPropagation(const int &mouseX, const int &mouseY, int button, bool down, int modifiers) {
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

    void Component::mouseOver() {

    }

    void Component::mouseOut() {

    }

    void Component::mouseMoved(const int &mouseX, const int &mouseY, int button, int modifiers) {

    }

    void Component::mouseMovedPropagation(const int &mouseX, const int &mouseY, int button, int modifiers) {
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

    void Component::mouseScrolled(const int &mouseX, const int &mouseY, const int &scrollX, const int &scrollY) {

    }

    bool Component::mouseScrolledPropagation(const int &mouseX, const int &mouseY, const int &scrollX, const int &scrollY) {
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

    bool Component::mouseDragEvent(const int &mouseX, const int &mouseY, const int &dragX, const int &dragY, int button, int modifiers) {
        return false;
    }

    bool Component::focusEvent(bool focused) {
        _focused = focused;
        return false;
    }

    bool Component::keyboardEvent(int, int, int, int) {
        return false;
    }

    bool Component::keyboardCharacterEvent(unsigned int) {
        return false;
    }

    // endregion

}