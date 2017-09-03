#include <cmath>
#include <iostream>
#include <utility>
#include "utils/YogaUtils.hpp"
#include "Component.hpp"
#include "Window.hpp"


namespace psychicui {

    Component::Component() :
        _style(std::make_unique<Style>()),
        _computedStyle(std::make_unique<Style>()),
        _yogaNode(YGNodeNew()) {
        setTag("Component");

        YGNodeSetContext(_yogaNode, this);
        // We don't like these from the web default
        YGNodeStyleSetFlexDirection(_yogaNode, YGFlexDirectionColumn);
    }

    Component::~Component() {
        YGNodeFree(_yogaNode);
    }

    // region Lifecycle

    // endregion

    // region Hierarchy

    Window *Component::window() {
        return _parent ? _parent->window() : nullptr;
    }

    Component *Component::parent() {
        return _parent;
    }

    const Component *Component::parent() const {
        return _parent;
    }

    void Component::setParent(Component *parent) {
        if (_parent != parent) {
            _parent = parent;
            _depth  = _parent ? _parent->depth() + 1 : 0;
            if (_parent) {
                // Here we force update the style instead of relying on the invalidation
                // because styles control the layout, and we don't want the component
                // doing a style-less render pass on its first frame alive.
                updateStyleRecursive();
            }
        }
    }

    const int Component::depth() const {
        return _depth;
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

    std::shared_ptr<Component> Component::add(unsigned int index, std::shared_ptr<Component> component) {
        assert(index <= childCount());
        assert(component != nullptr);
        component->setParent(this);
        _children.insert(_children.begin() + index, component);
        YGNodeInsertChild(_yogaNode, component->yogaNode(), index);
        return component;
    }

    std::shared_ptr<Component> Component::add(std::shared_ptr<Component> component) {
        assert(component != nullptr);
        add(childCount(), component);
        return component;
    }

    void Component::remove(const std::shared_ptr<Component> component) {
        assert(component != nullptr);
        _children.erase(std::remove(_children.begin(), _children.end(), component), _children.end());
        YGNodeRemoveChild(_yogaNode, component->yogaNode());
        component->setParent(nullptr);
    }

    void Component::remove(unsigned int index) {
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

    const Component *Component::at(unsigned int index) const {
        assert(index <= childCount());
        return _children[index].get();
    }

    Component *Component::at(unsigned int index) {
        assert(index <= childCount());
        return _children[index].get();
    }

    // endregion

    // region Visibility

    bool Component::visible() const {
        return _visible;
    }

    void Component::setVisible(bool value) {
        _visible = value;
        style()->set(BoolProperty::visible, value);
        invalidateStyle();
    }

    // endregion

    // region Focus

    bool Component::focused() const {
        return _focused;
    }

    void Component::focused(bool focused) {
        _focused = focused;
        invalidateStyle();
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

    // region State

    const bool Component::active() const {
        return _mouseDown;
    };

    // endregion

    // region Hit Tests

    bool Component::contains(const int &x, const int &y) const {
        int lx = x - _x, ly = y - _y;
        return lx >= 0 && lx < _width && ly >= 0 && ly < _height;
    }

//    std::shared_ptr<Component> Component::findComponent(const int &x, const int &y) {
//        int             lx = x - _x, ly = y - _y;
//        for (const auto &child: _children) {
//            if (child->visible() && child->contains(lx, ly)) {
//                return child->findComponent(lx, ly);
//            }
//        }
//        return contains(lx, ly) ? shared_from_this() : nullptr;
//    }

    // endregion

    // region Position

    void Component::setPosition(int x, int y) {
        _x = x;
        _y = y;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, _x);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, _y);
    }

    void Component::setPosition(int left, int top, int right, int bottom) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, left);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, top);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeRight, right);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeBottom, bottom);
    }

    const int Component::x() const {
        return _x;
    }

    void Component::setX(int x) {
        _x = x;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, _x >= 0 ? _x : YGUndefined);
    }

    const int Component::y() const {
        return _y;
    }

    void Component::setY(int y) {
        _y = y;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, _y >= 0 ? _y : YGUndefined);
    }

    const int Component::left() const {
        return (int) YGNodeStyleGetPosition(_yogaNode, YGEdgeLeft).value;
    }

    void Component::setLeft(int left) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, left);
    }

    const int Component::right() const {
        return (int) YGNodeStyleGetPosition(_yogaNode, YGEdgeRight).value;
    }

    void Component::setRight(int right) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeRight, right);
    }

    const int Component::top() const {
        return (int) YGNodeStyleGetPosition(_yogaNode, YGEdgeTop).value;
    }

    void Component::setTop(int top) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, top);
    }

    const int Component::bottom() const {
        return (int) YGNodeStyleGetPosition(_yogaNode, YGEdgeBottom).value;
    }

    void Component::setBottom(int bottom) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeBottom, bottom);
    }

    // endregion

    // region Dimensions

    void Component::setSize(int width, int height) {
        _width  = width;
        _height = height;
        YGNodeStyleSetWidth(_yogaNode, _width);
        YGNodeStyleSetHeight(_yogaNode, _height);
    }

    const int Component::width() const {
        return _width;
    }

    void Component::setWidth(int width) {
        _width = width;
        YGNodeStyleSetWidth(_yogaNode, _width);
    }

    const int Component::height() const {
        return _height;
    }

    void Component::setHeight(int height) {
        _height = height;
        YGNodeStyleSetHeight(_yogaNode, _height);
    }

    const float Component::percentWidth() const {
        auto size = YGNodeStyleGetWidth(_yogaNode);
        return size.unit == YGUnitPercent ? size.value : nanf("not a percent");
    }

    void Component::setPercentWidth(float percentWidth) {
        YGNodeStyleSetWidthPercent(_yogaNode, percentWidth);
    }

    const float Component::percentHeight() const {
        auto size = YGNodeStyleGetHeight(_yogaNode);
        return size.unit == YGUnitPercent ? size.value : nanf("not a percent");
    }

    void Component::setPercentHeight(float percentHeight) {
        YGNodeStyleSetHeightPercent(_yogaNode, percentHeight);
    }

    // endregion

    // region Style

    StyleManager *Component::styleManager() const {
        return _styleManager
               ? _styleManager.get()
               : (_parent ? _parent->styleManager() : StyleManager::getInstance().get());
    }

    void Component::setStyleManager(std::shared_ptr<StyleManager> styleManager) {
        _styleManager = std::move(styleManager);
        invalidateStyle();
    }

    Style *Component::style() const {
        return _style.get();
    }

    const Style *Component::computedStyle() const {
        return _computedStyle.get();
    }

    void Component::setTag(std::string componentName) {
        std::transform(componentName.begin(), componentName.end(), componentName.begin(), ::tolower);
        _tags.push_back(componentName);
        invalidateStyle();
    }

    const std::vector<std::string> &Component::tags() const {
        return _tags;
    }

    const std::vector<std::string> &Component::classNames() const {
        return _classNames;
    }

    Component *Component::setClassNames(std::vector<std::string> classNames) {
        _classNames = std::move(classNames);
        invalidateStyle();
        return this;
    }

    void Component::invalidateStyle() {
        if (_styleDirty) {
            return;
        }
        _styleDirty = true;
        for (const auto &child: _children) {
            child->invalidateStyle();
        }
    }

    void Component::updateStyle() {
        _computedStyle = std::move(styleManager()->computeStyle(this));

        // Overlay "inline" style
        _computedStyle->overlay(_style.get());

        // region Update Yoga

        // region Flex

        YGNodeStyleSetFlexDirection(
            _yogaNode,
            YogaDirectionFromString(_computedStyle->get(direction), YGFlexDirectionColumn)
        );
        YGNodeStyleSetJustifyContent(
            _yogaNode,
            YogaJustifyFromString(_computedStyle->get(justifyContent), YGJustifyFlexStart)
        );
        YGNodeStyleSetAlignContent(
            _yogaNode,
            YogaAlignFromString(_computedStyle->get(alignContent), YGAlignFlexStart)
        );
        YGNodeStyleSetAlignItems(
            _yogaNode,
            YogaAlignFromString(_computedStyle->get(alignItems), YGAlignStretch)
        );
        YGNodeStyleSetAlignSelf(
            _yogaNode,
            YogaAlignFromString(_computedStyle->get(alignSelf), YGAlignAuto)
        );
        YGNodeStyleSetPositionType(
            _yogaNode,
            YogaPositionFromString(_computedStyle->get(position), YGPositionTypeRelative)
        );
        YGNodeStyleSetFlexWrap(
            _yogaNode,
            YogaWrapFromString(_computedStyle->get(wrap), YGWrapNoWrap)
        );
        YGNodeStyleSetOverflow(
            _yogaNode,
            YogaOverflowFromString(_computedStyle->get(overflow), YGOverflowVisible)
        );

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

        // endregion

        // TODO: Position

        // region  Dimensions

        YGNodeStyleSetMinWidth(
            _yogaNode,
            !isnan(_computedStyle->get(minWidth)) ? _computedStyle->get(minWidth) : YGUndefined
        );
        YGNodeStyleSetMinHeight(
            _yogaNode,
            !isnan(_computedStyle->get(minHeight)) ? _computedStyle->get(minHeight) : YGUndefined
        );
        YGNodeStyleSetMaxWidth(
            _yogaNode,
            !isnan(_computedStyle->get(maxWidth)) ? _computedStyle->get(maxWidth) : YGUndefined
        );
        YGNodeStyleSetMaxHeight(
            _yogaNode,
            !isnan(_computedStyle->get(maxHeight)) ? _computedStyle->get(maxHeight) : YGUndefined
        );
        // TODO: Percent
        // TODO: Width + Height
        // TODO AspectRatio

        // endregion

        // region Margins

        YGNodeStyleSetMargin(
            _yogaNode,
            YGEdgeAll,
            !isnan(_computedStyle->get(margin)) ? _computedStyle->get(margin) : YGUndefined
        );
        YGNodeStyleSetMargin(
            _yogaNode,
            YGEdgeHorizontal,
            !isnan(_computedStyle->get(marginHorizontal)) ? _computedStyle->get(marginHorizontal) : YGUndefined
        );
        YGNodeStyleSetMargin(
            _yogaNode,
            YGEdgeVertical,
            !isnan(_computedStyle->get(marginVertical)) ? _computedStyle->get(marginVertical) : YGUndefined
        );
        YGNodeStyleSetMargin(
            _yogaNode,
            YGEdgeLeft,
            !isnan(_computedStyle->get(marginLeft)) ? _computedStyle->get(marginLeft) : YGUndefined
        );
        YGNodeStyleSetMargin(
            _yogaNode,
            YGEdgeRight,
            !isnan(_computedStyle->get(marginRight)) ? _computedStyle->get(marginRight) : YGUndefined
        );
        YGNodeStyleSetMargin(
            _yogaNode,
            YGEdgeTop,
            !isnan(_computedStyle->get(marginTop)) ? _computedStyle->get(marginTop) : YGUndefined);
        YGNodeStyleSetMargin(
            _yogaNode,
            YGEdgeBottom,
            !isnan(_computedStyle->get(marginBottom)) ? _computedStyle->get(marginBottom) : YGUndefined
        );
        // TODO: Auto
        // TODO: Percent

        // endregion

        // region Padding

        YGNodeStyleSetPadding(
            _yogaNode,
            YGEdgeAll,
            !isnan(_computedStyle->get(padding)) ? _computedStyle->get(padding) : YGUndefined
        );
        YGNodeStyleSetPadding(
            _yogaNode,
            YGEdgeHorizontal,
            !isnan(_computedStyle->get(paddingHorizontal)) ? _computedStyle->get(paddingHorizontal) : YGUndefined
        );
        YGNodeStyleSetPadding(
            _yogaNode,
            YGEdgeVertical,
            !isnan(_computedStyle->get(paddingVertical)) ? _computedStyle->get(paddingVertical) : YGUndefined
        );
        YGNodeStyleSetPadding(
            _yogaNode,
            YGEdgeLeft,
            !isnan(_computedStyle->get(paddingLeft)) ? _computedStyle->get(paddingLeft) : YGUndefined
        );
        YGNodeStyleSetPadding(
            _yogaNode,
            YGEdgeRight,
            !isnan(_computedStyle->get(paddingRight)) ? _computedStyle->get(paddingRight) : YGUndefined
        );
        YGNodeStyleSetPadding(
            _yogaNode,
            YGEdgeTop,
            !isnan(_computedStyle->get(paddingTop)) ? _computedStyle->get(paddingTop) : YGUndefined
        );
        YGNodeStyleSetPadding(
            _yogaNode,
            YGEdgeBottom,
            !isnan(_computedStyle->get(paddingBottom)) ? _computedStyle->get(paddingBottom) : YGUndefined
        );
        // TODO: Percent

        // endregion

        // region Border

        YGNodeStyleSetBorder(
            _yogaNode,
            YGEdgeAll,
            !isnan(_computedStyle->get(border)) ? _computedStyle->get(border) : YGUndefined
        );
        YGNodeStyleSetBorder(
            _yogaNode,
            YGEdgeHorizontal,
            !isnan(_computedStyle->get(borderHorizontal)) ? _computedStyle->get(borderHorizontal) : YGUndefined
        );
        YGNodeStyleSetBorder(
            _yogaNode,
            YGEdgeVertical,
            !isnan(_computedStyle->get(borderVertical)) ? _computedStyle->get(borderVertical) : YGUndefined
        );
        YGNodeStyleSetBorder(
            _yogaNode,
            YGEdgeLeft,
            !isnan(_computedStyle->get(borderLeft)) ? _computedStyle->get(borderLeft) : YGUndefined
        );
        YGNodeStyleSetBorder(
            _yogaNode,
            YGEdgeRight,
            !isnan(_computedStyle->get(borderRight)) ? _computedStyle->get(borderRight) : YGUndefined
        );
        YGNodeStyleSetBorder(
            _yogaNode,
            YGEdgeTop,
            !isnan(_computedStyle->get(borderTop)) ? _computedStyle->get(borderTop) : YGUndefined
        );
        YGNodeStyleSetBorder(
            _yogaNode,
            YGEdgeBottom,
            !isnan(_computedStyle->get(borderBottom)) ? _computedStyle->get(borderBottom) : YGUndefined
        );

        // endregion

        // endregion

        if (_computedStyle->has(BoolProperty::visible)) {
            _visible = _computedStyle->get(BoolProperty::visible);
        }

        _styleDirty = false;

        styleUpdated();
    }

    void Component::updateStyleRecursive() {
        updateStyle();
        if (_visible) {
            for (auto &child: _children) {
                child->updateStyleRecursive();
            }
        }
    }

    void Component::styleUpdated() {
        float tmp;

        if (_computedStyle->has(borderRadius)) {
            tmp                = _computedStyle->get(borderRadius);
            _radiusTopLeft     = tmp;
            _radiusTopRight    = tmp;
            _radiusBottomLeft  = tmp;
            _radiusBottomRight = tmp;
        }

        if (_computedStyle->has(borderRadiusLeft)) {
            tmp               = _computedStyle->get(borderRadiusLeft);
            _radiusTopLeft    = tmp;
            _radiusBottomLeft = tmp;
        }

        if (_computedStyle->has(borderRadiusRight)) {
            tmp                = _computedStyle->get(borderRadiusRight);
            _radiusTopRight    = tmp;
            _radiusBottomRight = tmp;
        }

        if (_computedStyle->has(borderRadiusTop)) {
            tmp             = _computedStyle->get(borderRadiusTop);
            _radiusTopLeft  = tmp;
            _radiusTopRight = tmp;
        }

        if (_computedStyle->has(borderRadiusBottom)) {
            tmp                = _computedStyle->get(borderRadiusBottom);
            _radiusBottomLeft  = tmp;
            _radiusBottomRight = tmp;
        }

        if (_computedStyle->has(borderRadiusTopLeft)) {
            _radiusTopLeft = _computedStyle->get(borderRadiusTopLeft);;
        }

        if (_computedStyle->has(borderRadiusTopRight)) {
            _radiusTopRight = _computedStyle->get(borderRadiusTopRight);;
        }

        if (_computedStyle->has(borderRadiusBottomLeft)) {
            _radiusBottomLeft = _computedStyle->get(borderRadiusBottomLeft);;
        }

        if (_computedStyle->has(borderRadiusBottomRight)) {
            _radiusBottomRight = _computedStyle->get(borderRadiusBottomRight);;
        }

        _drawRoundRect = _radiusTopLeft != 0
                         || _radiusTopRight != 0
                         || _radiusBottomLeft != 0
                         || _radiusBottomRight != 0;

        _drawComplexRoundRect = !(
            _radiusTopLeft == _radiusTopRight
            && _radiusTopRight == _radiusBottomLeft
            && _radiusBottomLeft == _radiusBottomRight
        );

        _drawBackground = _computedStyle->has(backgroundColor) && (_computedStyle->get(backgroundColor) != 0x00000000);
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

        _borderLeft   = YGNodeLayoutGetBorder(_yogaNode, YGEdgeLeft);
        _borderTop    = YGNodeLayoutGetBorder(_yogaNode, YGEdgeTop);
        _borderRight  = YGNodeLayoutGetBorder(_yogaNode, YGEdgeRight);
        _borderBottom = YGNodeLayoutGetBorder(_yogaNode, YGEdgeBottom);

        _drawBorder         = _borderLeft != 0
                              || _borderRight != 0
                              || _borderTop != 0
                              || _borderBottom != 0;

        _drawComplexBorders = !(
            _borderLeft == _borderRight
            && _borderRight == _borderTop
            && _borderTop == _borderBottom
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
        if (_styleDirty) {
            updateStyle();
        }

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

        if ( _borderLeft > 0 || _borderTop > 0 || _borderRight > 0 || _borderBottom > 0 ) {
            paint.setColor(0x7F00FF00);
            canvas->drawRect(
                SkRect{
                    _borderLeft,
                    _borderTop,
                    _rect.width() - _borderRight,
                    _rect.height() - _borderBottom
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
        SkPaint paint;
        Color   bgColor = _computedStyle->get(backgroundColor);

        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(bgColor);
        paint.setAntiAlias(_drawRoundRect ? true : _computedStyle->get(antiAlias));

        // Draw Background
        if (_drawBackground || _drawBorder) {
            float hb = _computedStyle->get(border) / 2;

            if (_drawComplexRoundRect) {
                SkRRect  rrect;
                SkVector radii[4] = {
                    {_radiusTopLeft,     _radiusTopLeft},
                    {_radiusTopRight,    _radiusTopRight},
                    {_radiusBottomRight, _radiusBottomRight},
                    {_radiusBottomLeft,  _radiusBottomLeft}
                };
                rrect.setRectRadii(_rect, radii);
                if (_drawBackground && _drawBorder) {
                    rrect.inset(hb, hb);
                }

                if (_drawBackground) {
                    canvas->drawRRect(rrect, paint);
                }

                if (_drawBorder) {
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setColor(_computedStyle->get(borderColor));
                    paint.setStrokeWidth(_computedStyle->get(border));
                    canvas->drawRRect(rrect, paint);
                }

            } else if (_drawRoundRect) {
                if (_drawBackground && !_drawBorder) {
                    canvas->drawRoundRect(_rect, _radiusTopLeft, _radiusTopLeft, paint);
                } else if (_drawBorder) {
                    SkRect inset = _rect.makeInset(hb, hb);
                    if (_drawBackground) {
                        canvas->drawRoundRect(inset, _radiusTopLeft, _radiusTopLeft, paint);
                    }
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setColor(_computedStyle->get(borderColor));
                    paint.setStrokeWidth(_computedStyle->get(border));
                    canvas->drawRoundRect(inset, _radiusTopLeft, _radiusTopLeft, paint);
                }

            } else if (_drawBackground) {
                canvas->drawRect(_rect, paint);
                if (!_drawComplexBorders) {
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setColor(_computedStyle->get(borderColor));
                    paint.setStrokeWidth(_computedStyle->get(border));
                    canvas->drawRect(_rect, paint);
                }
            }
        }

        // region Border Color

        // TODO: Optimize
        if (_drawComplexBorders && !_drawRoundRect) {
            paint.setStyle(SkPaint::kStroke_Style);

            if (_borderLeft > 0) {
                bool draw = true;
                paint.setStrokeWidth(_borderLeft);
                if (_computedStyle->has(borderLeftColor)) {
                    paint.setColor(_computedStyle->get(borderLeftColor));
                } else if (_computedStyle->has(borderHorizontalColor)) {
                    paint.setColor(_computedStyle->get(borderHorizontalColor));
                } else if (_computedStyle->has(borderColor)) {
                    paint.setColor(_computedStyle->get(borderColor));
                } else {
                    draw = false;
                }
                if (draw) {
                    float hw = _borderLeft / 2;
                    canvas->drawLine(hw, 0, hw, _rect.bottom(), paint);
                }
            }

            if (_borderRight > 0) {
                bool draw = true;
                paint.setStrokeWidth(_borderRight);
                if (_computedStyle->has(borderRightColor)) {
                    paint.setColor(_computedStyle->get(borderRightColor));
                } else if (_computedStyle->has(borderHorizontalColor)) {
                    paint.setColor(_computedStyle->get(borderHorizontalColor));
                } else if (_computedStyle->has(borderColor)) {
                    paint.setColor(_computedStyle->get(borderColor));
                } else {
                    draw = false;
                }
                if (draw) {
                    float hw = _borderRight / 2;
                    canvas->drawLine(_rect.right() - hw, 0, _rect.right() - hw, _rect.bottom(), paint);
                }
            }

            if (_borderTop > 0) {
                bool draw = true;
                paint.setStrokeWidth(_borderTop);
                if (_computedStyle->has(borderTopColor)) {
                    paint.setColor(_computedStyle->get(borderTopColor));
                } else if (_computedStyle->has(borderVerticalColor)) {
                    paint.setColor(_computedStyle->get(borderVerticalColor));
                } else if (_computedStyle->has(borderColor)) {
                    paint.setColor(_computedStyle->get(borderColor));
                } else {
                    draw = false;
                }
                if (draw) {
                    float hw = _borderTop / 2;
                    canvas->drawLine(0, hw, _rect.right(), hw, paint);
                }
            }

            if (_borderBottom > 0) {
                bool draw = true;
                paint.setStrokeWidth(_borderBottom);
                if (_computedStyle->has(borderBottomColor)) {
                    paint.setColor(_computedStyle->get(borderBottomColor));
                } else if (_computedStyle->has(borderVerticalColor)) {
                    paint.setColor(_computedStyle->get(borderVerticalColor));
                } else if (_computedStyle->has(borderColor)) {
                    paint.setColor(_computedStyle->get(borderColor));
                } else {
                    draw = false;
                }
                if (draw) {
                    float hw = _borderBottom / 2;
                    canvas->drawLine(0, _rect.bottom() - hw, _rect.right(), _rect.bottom() - hw, paint);
                }
            }
        }

        // endregion
    }

    // endregion

    // region Interaction

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
        invalidateStyle();
    }

    bool Component::mouseDown() const {
        return _mouseDown;
    }

    void Component::setMouseDown(bool down) {
        _mouseDown = down;
        invalidateStyle();
    }

    bool Component::mouseButton(const int &mouseX, const int &mouseY, int button, bool down, int modifiers) {
        if (!_visible) {
            return false;
        }

        // First check for released outside
        if (button == GLFW_MOUSE_BUTTON_LEFT && !down && _mouseDown && !contains(mouseX, mouseY)) {
            setMouseDown(false);
            if (_onMouseUpOutside) {
                _onMouseUpOutside();
            }
            if (_onMouseUp) {
                _onMouseUp();
            }
        }

        // Find the deepest child that will handle the request
        // We still have to go through all the children because of the mouseUpOutside handling
        for (const auto &child: _children) {
            if (child->mouseButton(mouseX - _x, mouseY - _y, button, down, modifiers)) {
                return true;
            }
        }

        // Now that we checked all the children its time to bail if the mouse is not on us
        if (!contains(mouseX, mouseY)) {
            return false;
        }

        // Generic mouse button handler
        if (_onMouseButton) {
            _onMouseButton(mouseX, mouseY, button, down, modifiers);
        }

        // TODO: All states should have different trees to check for handling
        //       That or a choice must be made on what can prevent the parent from catching a child click
        bool handled = false;
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (down != _mouseDown) {
                setMouseDown(down);
                if (_mouseDown && _onMouseDown) {
                    _onMouseDown();
                } else if (!_mouseDown) {
                    if (_onClick) {
                        _onClick();
                        handled = true;
                    }
                    if (_onMouseUp) {
                        _onMouseUp();
                    }
                }
            }
        }

        if (button == GLFW_MOUSE_BUTTON_1 && down && !_focused) {
            //TODO: requestFocus();
        }

        return handled;
    }

    Cursor Component::mouseMoved(const int &mouseX, const int &mouseY, int button, int modifiers) {
        if (!_visible) {
            return Cursor::Arrow;
        }

        bool isOver  = contains(mouseX, mouseY);
        bool wasOver = _mouseOver;

        // Notify about the change
        if (isOver != _mouseOver) {
            setMouseOver(isOver);
            if (_mouseOver && _onMouseOver) {
                _onMouseOver();
            } else if (!_mouseOver && _onMouseOut) {
                _onMouseOut();
            }
        }

        // Notify about movement only if over
        if (isOver && _onMouseMoved) {
            _onMouseMoved(mouseX, mouseY, button, modifiers);
        }

        // Bail if not over and not just out, no work is needed anymore
        if (!isOver && !wasOver) {
            return Cursor::Arrow;
        }

        // Propagate to children (including if we just lost the mouse, a child might be interested)
        // We pass mouse movement to every children since is doesn't conflict like clicks with depth.
        Cursor          cursor = _cursor;
        for (const auto &child: _children) {
            Cursor c = child->mouseMoved(mouseX - _x, mouseY - _y, button, modifiers);
            if (c != Cursor::Arrow) {
                cursor = c;
            }
        }

        return cursor;
    }

    bool Component::mouseScrolled(const int &mouseX, const int &mouseY, const int &scrollX, const int &scrollY) {
        if (!_visible || !contains(mouseX, mouseY)) {
            return false;
        }

        if (_onMouseScrolled) {
            _onMouseScrolled(mouseX, mouseY, scrollX, scrollY);
        }

        for (auto child: _children) {
            if (child->mouseScrolled(mouseX - _x, mouseY - _y, scrollX, scrollY)) {
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