#include <iostream>
#include "utils/YogaUtils.hpp"
#include "Component.hpp"
#include "Window.hpp"



namespace psychicui {

    Component::Component() :
        _style(std::make_unique<Style>()),
        _yogaNode(YGNodeNew()) {
        setTag("Component");

        YGNodeSetContext(_yogaNode, this);
        // We don't like these from the web default
        YGNodeStyleSetFlexDirection(_yogaNode, YGFlexDirectionColumn);

        // Initialize a basic style so that we don't segfault ;)
        updateStyle();
    }

    Component::~Component() {
        YGNodeFree(_yogaNode);
    }

    void Component::setTag(std::string componentName) {
        std::transform(componentName.begin(), componentName.end(), componentName.begin(), ::tolower);
        _tags.push_back(componentName);
    }

    // region Hierarchy

    StyleManager *Component::styleManager() const {
        const Window * w = window();
        return w ? w->styleManager() : nullptr;
    }

    const Window *Component::window() const {
        return _parent ? _parent->window() : nullptr;
    }

    Component *Component::parent() {
        return _parent;
    }

    const Component *Component::parent() const {
        return _parent;
    }

    void Component::setParent(Component *parent) {
        _parent = parent;
        _depth = parent->depth() + 1;
        updateStyle();
    }

    const int Component::depth() const {
        return _depth;
    }

    std::vector<std::shared_ptr<Component>> Component::path() {
        std::vector<std::shared_ptr<Component>> path;
        if (_parent) {
            path = _parent->path();
        }
        path.insert(path.begin(), shared_from_this());
        return path;
    }

    std::shared_ptr<Panel> Component::panel() {
        return _parent ? _parent->panel() : nullptr;
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

    // region Mouse & Cursor

    Cursor Component::cursor() const {
        return _cursor;
    }

    void Component::setCursor(Cursor cursor) {
        _cursor = cursor;
    }

    bool Component::mouseOver() const {
        return _mouseOver;
    }

    void Component::setMouseOver(bool over) {
        _mouseOver = over;
    }

    bool Component::mouseDown() const {
        return _mouseDown;
    }

    void Component::setMouseDown(bool down) {
        _mouseDown = down;
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

    void Component::setPosition(float x, float y) {
        _x = x;
        _y = y;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, _x);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, _y);
    }

    void Component::setPosition(float left, float top, float right, float bottom) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, left);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, top);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeRight, right);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeBottom, bottom);
    }

    float Component::x() const {
        return _x;
    }

    void Component::setX(float x) {
        _x = x;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, _x >= 0 ? _x : YGUndefined);
    }

    float Component::y() const {
        return _y;
    }

    void Component::setY(float y) {
        _y = y;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, _y >= 0 ? _y : YGUndefined);
    }

    float Component::left() const {
        return YGNodeStyleGetPosition(_yogaNode, YGEdgeLeft).value;
    }

    void Component::setLeft(float left) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, left);
    }

    float Component::right() const {
        return YGNodeStyleGetPosition(_yogaNode, YGEdgeRight).value;
    }

    void Component::setRight(float right) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeRight, right);
    }

    float Component::top() const {
        return YGNodeStyleGetPosition(_yogaNode, YGEdgeTop).value;
    }

    void Component::setTop(float top) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, top);
    }

    float Component::bottom() const {
        return YGNodeStyleGetPosition(_yogaNode, YGEdgeBottom).value;
    }

    void Component::setBottom(float bottom) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeBottom, bottom);
    }

    // endregion

    // region Dimensions

    void Component::setSize(float width, float height) {
        _width  = width;
        _height = height;
        YGNodeStyleSetWidth(_yogaNode, _width);
        YGNodeStyleSetHeight(_yogaNode, _height);
    }

    float Component::width() const {
        return _width;
    }

    void Component::setWidth(float width) {
        _width = width;
        YGNodeStyleSetWidth(_yogaNode, _width);
    }

    float Component::height() const {
        return _height;
    }

    void Component::setHeight(float height) {
        _height = height;
        YGNodeStyleSetHeight(_yogaNode, _height);
    }

    float Component::percentWidth() const {
        auto size = YGNodeStyleGetWidth(_yogaNode);
        return size.unit == YGUnitPercent ? size.value : nanf("not a percent");
    }

    void Component::setPercentWidth(float percentWidth) {
        YGNodeStyleSetWidthPercent(_yogaNode, percentWidth);
    }

    float Component::percentHeight() const {
        auto size = YGNodeStyleGetHeight(_yogaNode);
        return size.unit == YGUnitPercent ? size.value : nanf("not a percent");
    }

    void Component::setPercentHeight(float percentHeight) {
        YGNodeStyleSetHeightPercent(_yogaNode, percentHeight);
    }

    // endregion

    // region Style

    Style *Component::style() const {
        return _style.get();
    }

    const Style * Component::computedStyle() const {
        return _computedStyle.get();
    }

    const std::vector<std::string> &Component::tags() const {
        return _tags;
    }

    const std::vector<std::string> &Component::classNames() const {
        return _classNames;
    }

    void Component::setClassNames(std::vector<std::string> classNames) {
        _classNames = std::move(classNames);
    }

    void Component::updateStyle() {
        if (window()) {
            // Compute if we have access to a style manager
            _computedStyle = std::move(window()->styleManager()->computeStyle(this));
        } else {
            // Otherwise start with a blank style
            _computedStyle = std::make_unique<Style>();
        }

        // Overlay "inline" style
        _computedStyle->overlay(_style.get());

        // Update Yoga

        // Flex
        YGNodeStyleSetFlexDirection(_yogaNode, YogaDirectionFromString(_computedStyle->get(direction), YGFlexDirectionColumn));
        YGNodeStyleSetJustifyContent(_yogaNode, YogaJustifyFromString(_computedStyle->get(justifyContent), YGJustifyFlexStart));
        YGNodeStyleSetAlignContent(_yogaNode, YogaAlignFromString(_computedStyle->get(alignContent), YGAlignFlexStart));
        YGNodeStyleSetAlignItems(_yogaNode, YogaAlignFromString(_computedStyle->get(alignItems), YGAlignStretch));
        YGNodeStyleSetAlignSelf(_yogaNode, YogaAlignFromString(_computedStyle->get(alignSelf), YGAlignAuto));
        YGNodeStyleSetPositionType(_yogaNode, YogaPositionFromString(_computedStyle->get(position), YGPositionTypeRelative));
        YGNodeStyleSetFlexWrap(_yogaNode, YogaWrapFromString(_computedStyle->get(wrap), YGWrapNoWrap));
        YGNodeStyleSetOverflow(_yogaNode, YogaOverflowFromString(_computedStyle->get(overflow), YGOverflowVisible));

        YGNodeStyleSetFlex(_yogaNode, _computedStyle->get(flex));
//        YGNodeStyleSetFlexFlexAuto(_yogaNode, _computedStyle->get(flex));
//        YGNodeStyleSetFlexFlexPercent(_yogaNode, _computedStyle->get(flex));
        YGNodeStyleSetFlexGrow(_yogaNode, _computedStyle->get(grow));
//        YGNodeStyleSetFlexGrowAuto(_yogaNode, _computedStyle->get(grow));
//        YGNodeStyleSetFlexGrowPercent(_yogaNode, _computedStyle->get(grow));
        YGNodeStyleSetFlexShrink(_yogaNode, _computedStyle->get(shrink));
//        YGNodeStyleSetFlexShrinkAuto(_yogaNode, _computedStyle->get(shrink));
//        YGNodeStyleSetFlexShrinkPercent(_yogaNode, _computedStyle->get(shrink));
        YGNodeStyleSetFlexBasis(_yogaNode, _computedStyle->get(basis));
//        YGNodeStyleSetFlexBasisAuto(_yogaNode, _computedStyle->get(basis));
//        YGNodeStyleSetFlexBasisPercent(_yogaNode, _computedStyle->get(basis));

        // TODO: Position

        // Dimensions
        YGNodeStyleSetMinWidth(_yogaNode, _computedStyle->get(minWidth) >= 0 ? _computedStyle->get(minWidth) : YGUndefined);
        YGNodeStyleSetMinHeight(_yogaNode, _computedStyle->get(minHeight) >= 0 ? _computedStyle->get(minHeight) : YGUndefined);
        YGNodeStyleSetMaxWidth(_yogaNode, _computedStyle->get(maxWidth) >= 0 ? _computedStyle->get(maxWidth) : YGUndefined);
        YGNodeStyleSetMaxHeight(_yogaNode, _computedStyle->get(maxHeight) >= 0 ? _computedStyle->get(maxHeight) : YGUndefined);
        // TODO: Percent
        // TODO: Width + Height
        // TODO AspectRatio

        // Margins
        YGNodeStyleSetMargin(_yogaNode, YGEdgeAll, _computedStyle->get(margin) >= 0 ? _computedStyle->get(margin) : YGUndefined);
        YGNodeStyleSetMargin(_yogaNode, YGEdgeHorizontal, _computedStyle->get(marginHorizontal) >= 0 ? _computedStyle->get(marginHorizontal) : YGUndefined);
        YGNodeStyleSetMargin(_yogaNode, YGEdgeVertical, _computedStyle->get(marginVertical) >= 0 ? _computedStyle->get(marginVertical) : YGUndefined);
        YGNodeStyleSetMargin(_yogaNode, YGEdgeLeft, _computedStyle->get(marginLeft) >= 0 ? _computedStyle->get(marginLeft) : YGUndefined);
        YGNodeStyleSetMargin(_yogaNode, YGEdgeRight, _computedStyle->get(marginRight) >= 0 ? _computedStyle->get(marginRight) : YGUndefined);
        YGNodeStyleSetMargin(_yogaNode, YGEdgeTop, _computedStyle->get(marginTop) >= 0 ? _computedStyle->get(marginTop) : YGUndefined);
        YGNodeStyleSetMargin(_yogaNode, YGEdgeBottom, _computedStyle->get(marginBottom) >= 0 ? _computedStyle->get(marginBottom) : YGUndefined);
        // TODO: Auto
        // TODO: Percent

        // Padding
        YGNodeStyleSetPadding(_yogaNode, YGEdgeAll, _computedStyle->get(padding) >= 0 ? _computedStyle->get(padding) : YGUndefined);
        YGNodeStyleSetPadding(_yogaNode, YGEdgeHorizontal, _computedStyle->get(paddingHorizontal) >= 0 ? _computedStyle->get(paddingHorizontal) : YGUndefined);
        YGNodeStyleSetPadding(_yogaNode, YGEdgeVertical, _computedStyle->get(paddingVertical) >= 0 ? _computedStyle->get(paddingVertical) : YGUndefined);
        YGNodeStyleSetPadding(_yogaNode, YGEdgeLeft, _computedStyle->get(paddingLeft) >= 0 ? _computedStyle->get(paddingLeft) : YGUndefined);
        YGNodeStyleSetPadding(_yogaNode, YGEdgeRight, _computedStyle->get(paddingRight) >= 0 ? _computedStyle->get(paddingRight) : YGUndefined);
        YGNodeStyleSetPadding(_yogaNode, YGEdgeTop, _computedStyle->get(paddingTop) >= 0 ? _computedStyle->get(paddingTop) : YGUndefined);
        YGNodeStyleSetPadding(_yogaNode, YGEdgeBottom, _computedStyle->get(paddingBottom) >= 0 ? _computedStyle->get(paddingBottom) : YGUndefined);
        // TODO: Percent

        // Border
        YGNodeStyleSetBorder(_yogaNode, YGEdgeAll, _computedStyle->get(border) >= 0 ? _computedStyle->get(border) : YGUndefined);
        YGNodeStyleSetBorder(_yogaNode, YGEdgeHorizontal, _computedStyle->get(borderHorizontal) >= 0 ? _computedStyle->get(borderHorizontal) : YGUndefined);
        YGNodeStyleSetBorder(_yogaNode, YGEdgeVertical, _computedStyle->get(borderVertical) >= 0 ? _computedStyle->get(borderVertical) : YGUndefined);
        YGNodeStyleSetBorder(_yogaNode, YGEdgeLeft, _computedStyle->get(borderLeft) >= 0 ? _computedStyle->get(borderLeft) : YGUndefined);
        YGNodeStyleSetBorder(_yogaNode, YGEdgeRight, _computedStyle->get(borderRight) >= 0 ? _computedStyle->get(borderRight) : YGUndefined);
        YGNodeStyleSetBorder(_yogaNode, YGEdgeTop, _computedStyle->get(borderTop) >= 0 ? _computedStyle->get(borderTop) : YGUndefined);
        YGNodeStyleSetBorder(_yogaNode, YGEdgeBottom, _computedStyle->get(borderBottom) >= 0 ? _computedStyle->get(borderBottom) : YGUndefined);

        // Children should also update
        for (auto &child: _children) {
            child->updateStyle();
        }

        styleUpdated();
    }

    void Component::styleUpdated() {
        // Cache paint stiff for background, might help, even if we draw borders as multiple calls
        _drawBackground = _computedStyle->has(backgroundColor) && (_computedStyle->get(backgroundColor) != 0x00000000);
        if (_drawBackground) {
            _backgroundPaint.setColor(_computedStyle->get(backgroundColor));
            _backgroundPaint.setStyle(SkPaint::kFill_Style);
            if (_computedStyle->get(cornerRadius) > 0) {
                _backgroundPaint.setAntiAlias(true);
            } else {
                _backgroundPaint.setAntiAlias(_computedStyle->get(antiAlias));
            }
        }
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

    void Component::layoutUpdated() {
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

        _rect.set(0, 0, _width, _height);
        _paddedRect.set(
            YGNodeLayoutGetPadding(_yogaNode, YGEdgeLeft),
            YGNodeLayoutGetPadding(_yogaNode, YGEdgeTop),
            _width - YGNodeLayoutGetPadding(_yogaNode, YGEdgeRight),
            _height - YGNodeLayoutGetPadding(_yogaNode, YGEdgeBottom)
        );
        _borderValues.set(
            YGNodeLayoutGetBorder(_yogaNode, YGEdgeLeft),
            YGNodeLayoutGetBorder(_yogaNode, YGEdgeTop),
            YGNodeLayoutGetBorder(_yogaNode, YGEdgeRight),
            YGNodeLayoutGetBorder(_yogaNode, YGEdgeBottom)
        );

        // Children should also update
        for (auto &child: _children) {
            child->layoutUpdated();
        }
    }

    // endregion

    // region Draw

    YGSize Component::measure(float width, YGMeasureMode /*widthMode*/, float height, YGMeasureMode /*heightMode*/) {
        return YGSize{width, height};
    }

    void Component::render(SkCanvas *canvas) {
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

        #ifdef DEBUG_LAYOUT
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(1));
        paint.setColor(0x7FFF0000);
        canvas->drawRect(_rect, paint);

        paint.setColor(0x7F0000FF);
        canvas->drawRect(_paddedRect, paint);

        if ( _borderValues.left() > 0 || _borderValues.top() > 0 || _borderValues.right() > 0 || _borderValues.bottom() > 0 ) {
            paint.setColor(0x7F00FF00);
            canvas->drawRect(
                SkRect{
                    _borderValues.left(),
                    _borderValues.top(),
                    _rect.width() - _borderValues.right(),
                    _rect.height() - _borderValues.bottom()
                }, paint
            );
        }

        if (_mouseDown) {
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(0x30FFFFFF);
            canvas->drawRect({0.5f, 0.5f, _width - 0.5f, _height - 0.5f}, paint);
        } else if (_mouseOver) {
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(0x10FFFFFF);
            canvas->drawRect({0.5f, 0.5f, _width - 0.5f, _height - 0.5f}, paint);
        }
        #endif

        canvas->restore();
    }

    void Component::draw(SkCanvas *canvas) {
        if (_drawBackground) {
            // TODO: Separate border radius
            if (_computedStyle->get(cornerRadius) > 0) {
                canvas->drawRoundRect(
                    _rect,
                    _computedStyle->get(cornerRadius),
                    _computedStyle->get(cornerRadius),
                    _backgroundPaint
                );
            } else {
                canvas->drawRect(_rect, _backgroundPaint);
            }
        }

        // TODO: Only do full borders if we have corner radius
        if (_borderValues.left() > 0) {
            bool draw = true;
            _borderPaint.setStrokeWidth(_borderValues.left());
            if (_computedStyle->has(borderLeftColor)) {
                _borderPaint.setColor(_computedStyle->get(borderLeftColor));
            } else if (_computedStyle->has(borderHorizontalColor)) {
                _borderPaint.setColor(_computedStyle->get(borderHorizontalColor));
            } else if (_computedStyle->has(borderColor)) {
                _borderPaint.setColor(_computedStyle->get(borderColor));
            } else {
                draw = false;
            }
            if (draw) {
                float hw =_borderValues.left() / 2;
                canvas->drawLine(hw, 0, hw, _rect.bottom(), _borderPaint);
            }
        }
        
        if (_borderValues.right() > 0) {
            bool draw = true;
            _borderPaint.setStrokeWidth(_borderValues.right());
            if (_computedStyle->has(borderRightColor)) {
                _borderPaint.setColor(_computedStyle->get(borderRightColor));
            } else if (_computedStyle->has(borderHorizontalColor)) {
                _borderPaint.setColor(_computedStyle->get(borderHorizontalColor));
            } else if (_computedStyle->has(borderColor)) {
                _borderPaint.setColor(_computedStyle->get(borderColor));
            } else {
                draw = false;
            }
            if (draw) {
                float hw =_borderValues.right() / 2;
                canvas->drawLine(_rect.right() - hw, 0, _rect.right() - hw, _rect.bottom(), _borderPaint);
            }
        }
        
        if (_borderValues.top() > 0) {
            bool draw = true;
            _borderPaint.setStrokeWidth(_borderValues.top());
            if (_computedStyle->has(borderTopColor)) {
                _borderPaint.setColor(_computedStyle->get(borderTopColor));
            } else if (_computedStyle->has(borderHorizontalColor)) {
                _borderPaint.setColor(_computedStyle->get(borderVerticalColor));
            } else if (_computedStyle->has(borderColor)) {
                _borderPaint.setColor(_computedStyle->get(borderColor));
            } else {
                draw = false;
            }
            if (draw) {
                float hw =_borderValues.top() / 2;
                canvas->drawLine(0, hw, _rect.right(), hw, _borderPaint);
            }
        }
        
        if (_borderValues.bottom() > 0) {
            bool draw = true;
            _borderPaint.setStrokeWidth(_borderValues.bottom());
            if (_computedStyle->has(borderBottomColor)) {
                _borderPaint.setColor(_computedStyle->get(borderBottomColor));
            } else if (_computedStyle->has(borderHorizontalColor)) {
                _borderPaint.setColor(_computedStyle->get(borderVerticalColor));
            } else if (_computedStyle->has(borderColor)) {
                _borderPaint.setColor(_computedStyle->get(borderColor));
            } else {
                draw = false;
            }
            if (draw) {
                float hw =_borderValues.top() / 2;
                canvas->drawLine(0, _rect.bottom() - hw, _rect.right(), _rect.bottom() - hw, _borderPaint);
            }
        }
    }
    
    // endregion

    // region Interaction

    void Component::onMouseButton(const int &mouseX, const int &mouseY, int button, bool down, int modifiers) {

    }

    void Component::onMouseDown() {
        updateStyle();
    }

    void Component::onMouseUp() {
        updateStyle();
    }

    bool Component::mouseButtonPropagation(const int &mouseX, const int &mouseY, int button, bool down, int modifiers) {
        if (!_visible || !contains(mouseX, mouseY)) {
            return false;
        }

        onMouseButton(mouseX, mouseY, button, down, modifiers);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (down != _mouseDown) {
                _mouseDown = down;
                if (_mouseDown) {
                    onMouseDown();
                } else {
                    onMouseUp();
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

    void Component::onMouseOver() {
        updateStyle();
    }

    void Component::onMouseOut() {
        updateStyle();
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
                onMouseOver();
            } else {
                onMouseOut();
            }
        }

        if (over || overBefore) {
            mouseMoved(mouseX, mouseY, button, modifiers);
            for (auto child: _children) {
                child->mouseMovedPropagation(mouseX - _x, mouseY - _y, button, modifiers);
            }
        }
    }

    void Component::onMouseScrolled(const int &mouseX, const int &mouseY, const int &scrollX, const int &scrollY) {

    }

    bool Component::mouseScrolledPropagation(const int &mouseX, const int &mouseY, const int &scrollX, const int &scrollY) {
        if (!_visible || !contains(mouseX, mouseY)) {
            return false;
        }

        onMouseScrolled(mouseX, mouseY, scrollX, scrollY);

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


    // STUFF>>>>>>>>>>>>

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