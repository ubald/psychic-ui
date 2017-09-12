#include <cmath>
#include <iostream>
#include <SkPaint.h>
#include <SkDashPathEffect.h>
#include "utils/YogaUtils.hpp"
#include "Div.hpp"
#include "Window.hpp"


namespace psychicui {

    const InheritableValues Div::_inheritableValues{
        {color},
        {fontFamily},
        {fontSize, letterSpacing, lineHeight},
        {cursor},
        {}
    };

    #ifdef DEBUG_LAYOUT
    bool Div::debugLayout{false};
    #endif

    Div::Div() :
        Observer(),
        _defaultStyle(std::make_unique<Style>([this]() { invalidateStyle(); })),
        _inlineStyle(std::make_unique<Style>([this]() { invalidateStyle(); })),
        _computedStyle(std::make_unique<Style>()),
        _yogaNode(YGNodeNew()) {
        setTag("div");

        YGNodeSetContext(_yogaNode, this);
        // We don't like these from the web default
        YGNodeStyleSetFlexDirection(_yogaNode, YGFlexDirectionColumn);
    }

    Div::~Div() {
        YGNodeFree(_yogaNode);
    }

    std::string Div::toString() const {
        std::string str = "";
        if (_parent) {
            str = _parent->toString();
        }
        if (!_tags.empty()) {
            str += " " + _tags.back();
        } else {
            str += " #ERROR_NO_TAG#";
        }
        if (!_id.empty()) {
            str += "#" + _id;
        }
        for (auto className: _classNames) {
            str += "." + className;
        }
        return str;
    }

    // region Lifecycle

    // endregion

    // region Hierarchy

    Window *Div::window() {
        return _parent ? _parent->window() : nullptr;
    }

    Div *Div::parent() {
        return _parent;
    }

    const Div *Div::parent() const {
        return _parent;
    }

    void Div::setParent(Div *parent) {
        if (_parent != parent) {
            if (_parent) {
                removedFromRenderRecursive();
                removed();
            }
            _parent = parent;
            _depth  = _parent ? _parent->depth() + 1 : 0;
            if (_parent) {
                // Here we force update the style instead of relying on the invalidation
                // because styles control the layout, and we don't want the component
                // doing a style-less render pass on its first frame alive.
                updateStyleRecursive();
                added();
                if (window()) {
                    addedToRenderRecursive();
                }
            }
        }
    }

    const int Div::depth() const {
        return _depth;
    }

    void Div::added() {}

    void Div::removed() {}

    void Div::addedToRenderRecursive() {
        addedToRender();
        for (auto &child: _children) {
            child->addedToRenderRecursive();
        }
    }

    void Div::removedFromRenderRecursive() {
        removedFromRender();
        for (auto &child: _children) {
            child->removedFromRenderRecursive();
        }
    }

    void Div::addedToRender() {}

    void Div::removedFromRender() {}

    std::shared_ptr<Panel> Div::panel() {
        return _parent ? _parent->panel() : nullptr;
    }

    // endregion

    // region Children

    unsigned int Div::childCount() const {
        return (unsigned int) _children.size();
    }

    const std::vector<std::shared_ptr<Div>> Div::children() const {
        return _children;
    }

    std::shared_ptr<Div> Div::add(unsigned int index, std::shared_ptr<Div> child) {
        assert(child != nullptr);
        assert(index >= 0 && index <= childCount());
        child->setParent(this);
        // Insert in "reverse" so that we can iterate front-to-back without using a reverse_iterator
        _children.insert(_children.end() - index, child);
        YGNodeInsertChild(_yogaNode, child->_yogaNode, index);
        return child;
    }

    std::shared_ptr<Div> Div::add(std::shared_ptr<Div> child) {
        assert(child != nullptr);
        return add(childCount(), child);
    }

    void Div::remove(const std::shared_ptr<Div> child) {
        assert(child != nullptr);
        _children.erase(std::remove(_children.begin(), _children.end(), child), _children.end());
        YGNodeRemoveChild(_yogaNode, child->_yogaNode);
        child->setParent(nullptr);
    }

    void Div::remove(unsigned int index) {
        assert(index <= childCount());
        std::shared_ptr<Div> child = _children[index];
        _children.erase(_children.end() - index);
        YGNodeRemoveChild(_yogaNode, child->_yogaNode);
        child->setParent(nullptr);
    }

    void Div::removeAll() {
        for (auto &child: _children) {
            child->setParent(nullptr);
            YGNodeRemoveChild(_yogaNode, child->_yogaNode);
        }
        _children.clear();
    }

    int Div::childIndex(const std::shared_ptr<Div> component) const {
        assert(component != nullptr);
        auto index = std::distance(
            _children.begin(),
            std::find(_children.begin(), _children.end(), component)
        );
        return (index >= _children.size()) ? -1 : (int) index;
    }

    int Div::childIndex(const Div *component) const {
        assert(component != nullptr);
        auto index = std::distance(
            _children.begin(),
            std::find_if(
                _children.begin(),
                _children.end(),
                [component](const auto &child) {
                    return child.get() == component;
                }
            ));
        return (index >= _children.size()) ? -1 : (int) index;
    }

    const Div *Div::at(unsigned int index) const {
        assert(index <= childCount());
        return _children[index].get();
    }

    Div *Div::at(unsigned int index) {
        assert(index <= childCount());
        return _children[index].get();
    }

    // endregion

    // region State

    bool Div::getVisible() const {
        return _visible;
    }

    void Div::setVisible(bool value) {
        _visible = value;
        style()->set(visible, value);
    }

    bool Div::enabled() const {
        return _enabled;
    }

    void Div::setEnabled(bool value) {
        _enabled = value;
        invalidateStyle();
    }

    // endregion

    // region Focus

    bool Div::focused() const {
        return _focused;
    }

    void Div::focused(bool focused) {
        _focused = focused;
        invalidateStyle();
    }

    void Div::requestFocus() {
        requestFocus(this);
    }

    void Div::requestFocus(Div *component) {
        if (_parent) {
            _parent->requestFocus(component);
        }
    }

    // endregion

    // region State

    const bool Div::active() const {
        return _mouseDown;
    };

    // endregion

    // region Hit Tests

    bool Div::contains(const int x, const int y) const {
        return x >= _x && x < _x + _width && y >= _y && y < _y + _height;
    }

    bool Div::boundsContains(const int x, const int y) const {
        if (_computedStyle->get(overflow) == "visible") {
            return x >= _boundsLeft && x < _boundsRight && y >= _boundsTop && y < _boundsBottom;
        } else {
            // TODO: Keep right and bottom cached
            return x >= _x && x < _x + _width && y >= _y && y < _y + _height;
        }
    }

//    std::shared_ptr<Div> Div::findComponent(const int x, const int y) {
//        int             lx = x - _x, ly = y - _y;
//        for (const auto &child: _children) {
//            if (child->visible() && child->boundsContains(lx, ly)) {
//                return child->findComponent(lx, ly);
//            }
//        }
//        return boundsContains(lx, ly) ? shared_from_this() : nullptr;
//    }

    // endregion

    // region Position

    void Div::localToGlobal(int &gx, int &gy, const int x, const int y) const {
        int lgx = _x + x;
        int lgy = _y + y;
        if (_parent) {
            _parent->localToGlobal(lgx, lgy, lgx, lgy);
        }
        gx = lgx + _scrollX;
        gy = lgy + _scrollY;
    }

    void Div::globalToLocal(int &lx, int &ly, const int x, const int y) const {
        int gx = 0;
        int gy = 0;
        localToGlobal(gx, gy);
        lx = x - gx;
        ly = y - gy;
    }

    void Div::setPosition(int x, int y) {
        _x = x;
        _y = y;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, _x);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, _y);
    }

    void Div::setPosition(int left, int top, int right, int bottom) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, left);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, top);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeRight, right);
        YGNodeStyleSetPosition(_yogaNode, YGEdgeBottom, bottom);
    }

    const int Div::x() const {
        return _x;
    }

    void Div::setX(int x) {
        _x = x;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, _x >= 0 ? _x : YGUndefined);
    }

    const int Div::y() const {
        return _y;
    }

    void Div::setY(int y) {
        _y = y;
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, _y >= 0 ? _y : YGUndefined);
    }

    const int Div::getLeft() const {
        return (int) YGNodeStyleGetPosition(_yogaNode, YGEdgeLeft).value;
    }

    void Div::setLeft(int left) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, left);
    }

    const float Div::getLeftPercent() const {
        auto size = YGNodeStyleGetPosition(_yogaNode, YGEdgeLeft);
        return size.unit == YGUnitPercent ? size.value / 100.f : nanf("not a percent");
    }

    void Div::setLeftPercent(float leftPercent) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, YogaPercent(leftPercent));
    }

    const int Div::getRight() const {
        return (int) YGNodeStyleGetPosition(_yogaNode, YGEdgeRight).value;
    }

    void Div::setRight(int right) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeRight, right);
    }

    const float Div::getRightPercent() const {
        auto size = YGNodeStyleGetPosition(_yogaNode, YGEdgeRight);
        return size.unit == YGUnitPercent ? size.value / 100.f : nanf("not a percent");
    }

    void Div::setRightPercent(float rightPercent) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeRight, YogaPercent(rightPercent));
    }

    const int Div::getTop() const {
        return (int) YGNodeStyleGetPosition(_yogaNode, YGEdgeTop).value;
    }

    void Div::setTop(int top) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, top);
    }

    const float Div::getTopPercent() const {
        auto size = YGNodeStyleGetPosition(_yogaNode, YGEdgeTop);
        return size.unit == YGUnitPercent ? size.value / 100.f : nanf("not a percent");
    }

    void Div::setTopPercent(float topPercent) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, YogaPercent(topPercent));
    }

    const int Div::getBottom() const {
        return (int) YGNodeStyleGetPosition(_yogaNode, YGEdgeBottom).value;
    }

    void Div::setBottom(int bottom) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeBottom, bottom);
    }

    const float Div::getBottomPercent() const {
        auto size = YGNodeStyleGetPosition(_yogaNode, YGEdgeBottom);
        return size.unit == YGUnitPercent ? size.value / 100.f : nanf("not a percent");
    }

    void Div::setBottomPercent(float bottomPercent) {
        YGNodeStyleSetPosition(_yogaNode, YGEdgeBottom, YogaPercent(bottomPercent));
    }

    // endregion

    // region Dimensions

    void Div::setSize(int width, int height) {
        _width  = width;
        _height = height;
        YGNodeStyleSetWidth(_yogaNode, _width);
        YGNodeStyleSetHeight(_yogaNode, _height);
    }

    const int Div::getWidth() const {
        return _width;
    }

    void Div::setWidth(int width) {
        _width = width;
        YGNodeStyleSetWidth(_yogaNode, _width);
    }

    const int Div::getHeight() const {
        return _height;
    }

    void Div::setHeight(int height) {
        _height = height;
        YGNodeStyleSetHeight(_yogaNode, _height);
    }

    const float Div::getWidthPercent() const {
        auto size = YGNodeStyleGetWidth(_yogaNode);
        return size.unit == YGUnitPercent ? size.value / 100.f : nanf("not a percent");
    }

    void Div::setWidthPercent(float widthPercent) {
        YGNodeStyleSetWidthPercent(_yogaNode, YogaPercent(widthPercent));
    }

    const float Div::getHeightPercent() const {
        auto size = YGNodeStyleGetHeight(_yogaNode);
        return size.unit == YGUnitPercent ? size.value / 100.f : nanf("not a percent");
    }

    void Div::setHeightPercent(float heightPercent) {
        YGNodeStyleSetHeightPercent(_yogaNode, YogaPercent(heightPercent));
    }

    // endregion

    // region Style

    // region Properties

    StyleManager *Div::styleManager() const {
        if (_styleManager) {
            return _styleManager.get();
        } else if (_parent) {
            return _parent->styleManager();
        } else {
            return StyleManager::getInstance().get();
        }
    }

    void Div::setStyleManager(std::shared_ptr<StyleManager> styleManager) {
        _styleManager = std::move(styleManager);
        invalidateStyle();
    }

    Style *Div::style() const {
        return _inlineStyle.get();
    }

    const Style *Div::computedStyle() const {
        return _computedStyle.get();
    }

    Div *Div::setTag(std::string componentName) {
        std::transform(componentName.begin(), componentName.end(), componentName.begin(), ::tolower);
        _tags.push_back(componentName);
        invalidateStyle();
        return this;
    }

    const std::vector<std::string> &Div::tags() const {
        return _tags;
    }

    const std::string Div::id() const {
        return _id;
    }

    Div *Div::setId(std::string id) {
        if (id != _id) {
            _id = id;
            invalidateStyle();
        }
        return this;
    }

    const std::unordered_set<std::string> &Div::classNames() const {
        return _classNames;
    }

    Div *Div::setClassNames(std::unordered_set<std::string> classNames) {
        _classNames = std::move(classNames);
        invalidateStyle();
        return this;
    }

    Div *Div::addClassName(const std::string className) {
        auto res = _classNames.insert(className);
        if (res.second) {
            invalidateStyle();
        }
        return this;
    }

    Div *Div::removeClassName(const std::string className) {
        auto res = _classNames.erase(className);
        if (res) {
            invalidateStyle();
        }
        return this;
    }

    const InheritableValues Div::inheritableValues() const {
        return _inheritableValues;
    }

    // endregion

    void Div::invalidateStyle() {
        if (_styleDirty) {
            return;
        }
        _styleDirty = true;
        for (const auto &child: _children) {
            child->invalidateStyle();
        }
    }

    void Div::updateStyle() {
        _computedStyle = styleManager()->computeStyle(this);
        updateLayout();
        _styleDirty = false;
        styleUpdated();
    }

    void Div::updateStyleRecursive() {
        updateStyle();
        if (_visible) {
            for (auto &child: _children) {
                child->updateStyleRecursive();
            }
        }
    }

    void Div::styleUpdated() {
        // region Visibility
        if (_computedStyle->has(visible)) {
            _visible = _computedStyle->get(visible);
        }
        // endregion
        // region Radius
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

        _radii[0].set(_radiusTopLeft, _radiusTopLeft);
        _radii[1].set(_radiusTopRight, _radiusTopRight);
        _radii[2].set(_radiusBottomRight, _radiusBottomRight);
        _radii[3].set(_radiusBottomLeft, _radiusBottomLeft);

        _drawRoundRect = _radiusTopLeft != 0
                         || _radiusTopRight != 0
                         || _radiusBottomLeft != 0
                         || _radiusBottomRight != 0;

        _drawComplexRoundRect = !(
            _radiusTopLeft == _radiusTopRight
            && _radiusTopRight == _radiusBottomLeft
            && _radiusBottomLeft == _radiusBottomRight
        );
        // endregion
        // region Background
        _drawBackground       = _computedStyle->has(backgroundColor)
                                && (_computedStyle->get(backgroundColor) != 0x00000000);
        // endregion
    }

    // endregion

    // region Layout

    void Div::invalidate() {
        YGNodeMarkDirty(_yogaNode);
    }

    void Div::setMeasurable() {
        YGNodeSetMeasureFunc(
            _yogaNode,
            [](YGNodeRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
                YGSize size{};
                auto   component = static_cast<Div *>(YGNodeGetContext(node));
                if (!component) {
                    std::cerr << "Could not find component to measure" << std::endl;
                    return size;
                }
                size = component->measure(width, widthMode, height, heightMode);
                return size;
            }
        );
    }

    void Div::updateLayout() {
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

//        YGNodeStyleSetFlex(_yogaNode, _computedStyle->get(flex, YGNodeStyleGetFlex(_yogaNode)));
        YGNodeStyleSetFlex(_yogaNode, _computedStyle->get(flex));
//        YGNodeStyleSetFlexFlexAuto(_yogaNode, _computedStyle->get(flex));
//        YGNodeStyleSetFlexFlexPercent(_yogaNode, _computedStyle->get(flex));

//        YGNodeStyleSetFlexGrow(_yogaNode, _computedStyle->get(grow, YGNodeStyleGetFlexGrow(_yogaNode)));
        YGNodeStyleSetFlexGrow(_yogaNode, _computedStyle->get(grow));
//        YGNodeStyleSetFlexGrowAuto(_yogaNode, _computedStyle->get(grow));
//        YGNodeStyleSetFlexGrowPercent(_yogaNode, _computedStyle->get(grow));

//        YGNodeStyleSetFlexShrink(_yogaNode, _computedStyle->get(shrink, YGNodeStyleGetFlexShrink(_yogaNode)));
        YGNodeStyleSetFlexShrink(_yogaNode, _computedStyle->get(shrink));
//        YGNodeStyleSetFlexShrinkAuto(_yogaNode, _computedStyle->get(shrink));
//        YGNodeStyleSetFlexShrinkPercent(_yogaNode, _computedStyle->get(shrink));

//        YGNodeStyleSetFlexBasis(_yogaNode, _computedStyle->get(basis, YGNodeStyleGetFlexBasis(_yogaNode).value));
        YGNodeStyleSetFlexBasis(_yogaNode, _computedStyle->get(basis));
//        YGNodeStyleSetFlexBasisAuto(_yogaNode, _computedStyle->get(basis));
//        YGNodeStyleSetFlexBasisPercent(_yogaNode, _computedStyle->get(basis));
        // endregion
        // region Position
        if (_computedStyle->has(left)) {
            YGNodeStyleSetPosition(_yogaNode, YGEdgeLeft, _computedStyle->get(left, YGUndefined));
        } else {
            YGNodeStyleSetPosition(
                _yogaNode,
                YGEdgeLeft,
                YogaPercent(_computedStyle->get(leftPercent, YGUndefined)));
        }
        if (_computedStyle->has(right)) {
            YGNodeStyleSetPosition(_yogaNode, YGEdgeRight, _computedStyle->get(right, YGUndefined));
        } else {
            YGNodeStyleSetPosition(
                _yogaNode,
                YGEdgeRight,
                YogaPercent(_computedStyle->get(rightPercent, YGUndefined)));
        }
        if (_computedStyle->has(top)) {
            YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, _computedStyle->get(top, YGUndefined));
        } else {
            YGNodeStyleSetPosition(_yogaNode, YGEdgeTop, YogaPercent(_computedStyle->get(topPercent, YGUndefined)));
        }
        if (_computedStyle->has(bottom)) {
            YGNodeStyleSetPosition(_yogaNode, YGEdgeBottom, _computedStyle->get(bottom, YGUndefined));
        } else {
            YGNodeStyleSetPosition(
                _yogaNode,
                YGEdgeBottom,
                YogaPercent(_computedStyle->get(bottomPercent, YGUndefined)));
        }
        // endregion
        // region  Dimensions
        if (_computedStyle->has(width)) {
            YGNodeStyleSetWidth(_yogaNode, _computedStyle->get(width, YGUndefined));
        } else {
            YGNodeStyleSetWidthPercent(_yogaNode, YogaPercent(_computedStyle->get(widthPercent, YGUndefined)));
        }
        if (_computedStyle->has(minWidth)) {
            YGNodeStyleSetMinWidth(_yogaNode, _computedStyle->get(minWidth, YGUndefined));
        } else {
            YGNodeStyleSetMinWidthPercent(
                _yogaNode,
                YogaPercent(_computedStyle->get(minWidthPercent, YGUndefined)));
        }
        if (_computedStyle->has(maxWidth)) {
            YGNodeStyleSetMaxWidth(_yogaNode, _computedStyle->get(maxWidth, YGUndefined));
        } else {
            YGNodeStyleSetMaxWidthPercent(
                _yogaNode,
                YogaPercent(_computedStyle->get(maxWidthPercent, YGUndefined)));
        }

        if (_computedStyle->has(height)) {
            YGNodeStyleSetHeight(_yogaNode, _computedStyle->get(height, YGUndefined));
        } else {
            YGNodeStyleSetHeightPercent(_yogaNode, YogaPercent(_computedStyle->get(heightPercent, YGUndefined)));
        }
        if (_computedStyle->has(minHeight)) {
            YGNodeStyleSetMinHeight(_yogaNode, _computedStyle->get(minHeight, YGUndefined));
        } else {
            YGNodeStyleSetMinHeightPercent(
                _yogaNode,
                YogaPercent(_computedStyle->get(minHeightPercent, YGUndefined)));
        }
        if (_computedStyle->has(maxHeight)) {
            YGNodeStyleSetMaxHeight(_yogaNode, _computedStyle->get(maxHeight, YGUndefined));
        } else {
            YGNodeStyleSetMaxHeightPercent(
                _yogaNode,
                YogaPercent(_computedStyle->get(maxHeightPercent, YGUndefined)));
        }
        // TODO AspectRatio
        // endregion
        // region Margins
        YGNodeStyleSetMargin(_yogaNode, YGEdgeAll, _computedStyle->get(margin, YGUndefined));
        YGNodeStyleSetMargin(_yogaNode, YGEdgeHorizontal, _computedStyle->get(marginHorizontal, YGUndefined));
        YGNodeStyleSetMargin(_yogaNode, YGEdgeVertical, _computedStyle->get(marginVertical, YGUndefined));
        YGNodeStyleSetMargin(_yogaNode, YGEdgeLeft, _computedStyle->get(marginLeft, YGUndefined));
        YGNodeStyleSetMargin(_yogaNode, YGEdgeRight, _computedStyle->get(marginRight, YGUndefined));
        YGNodeStyleSetMargin(_yogaNode, YGEdgeTop, _computedStyle->get(marginTop, YGUndefined));
        YGNodeStyleSetMargin(_yogaNode, YGEdgeBottom, _computedStyle->get(marginBottom, YGUndefined));
        // TODO: Auto
        // TODO: Percent
        // endregion
        // region Padding
        if (!_ignoreInternalLayoutContraints) {
            YGNodeStyleSetPadding(_yogaNode, YGEdgeAll, _computedStyle->get(padding, YGUndefined));
            YGNodeStyleSetPadding(_yogaNode, YGEdgeHorizontal, _computedStyle->get(paddingHorizontal, YGUndefined));
            YGNodeStyleSetPadding(_yogaNode, YGEdgeVertical, _computedStyle->get(paddingVertical, YGUndefined));
            YGNodeStyleSetPadding(_yogaNode, YGEdgeLeft, _computedStyle->get(paddingLeft, YGUndefined));
            YGNodeStyleSetPadding(_yogaNode, YGEdgeRight, _computedStyle->get(paddingRight, YGUndefined));
            YGNodeStyleSetPadding(_yogaNode, YGEdgeTop, _computedStyle->get(paddingTop, YGUndefined));
            YGNodeStyleSetPadding(_yogaNode, YGEdgeBottom, _computedStyle->get(paddingBottom, YGUndefined));
        }
        // TODO: Percent
        // endregion
        // region Border
        if (!_ignoreInternalLayoutContraints) {
            YGNodeStyleSetBorder(_yogaNode, YGEdgeAll, _computedStyle->get(border, YGUndefined));
            YGNodeStyleSetBorder(_yogaNode, YGEdgeHorizontal, _computedStyle->get(borderHorizontal, YGUndefined));
            YGNodeStyleSetBorder(_yogaNode, YGEdgeVertical, _computedStyle->get(borderVertical, YGUndefined));
            YGNodeStyleSetBorder(_yogaNode, YGEdgeLeft, _computedStyle->get(borderLeft, YGUndefined));
            YGNodeStyleSetBorder(_yogaNode, YGEdgeRight, _computedStyle->get(borderRight, YGUndefined));
            YGNodeStyleSetBorder(_yogaNode, YGEdgeTop, _computedStyle->get(borderTop, YGUndefined));
            YGNodeStyleSetBorder(_yogaNode, YGEdgeBottom, _computedStyle->get(borderBottom, YGUndefined));
        }
        // endregion
    }

    void Div::layoutUpdated() {
//        int right  = (int) YGNodeLayoutGetRight(_yogaNode);
//        int bottom = (int) YGNodeLayoutGetBottom(_yogaNode);

        _x = (int) YGNodeLayoutGetLeft(_yogaNode);
        _y = (int) YGNodeLayoutGetTop(_yogaNode);

        int previousWidth  = _width;
        int previousHeight = _height;
        _width  = (int) YGNodeLayoutGetWidth(_yogaNode);
        _height = (int) YGNodeLayoutGetHeight(_yogaNode);

        _rect.set(_x, _y, _x + _width, _y + _height);
        _roundRect.setRectRadii(_rect, _radii);

        _borderLeft   = YGNodeLayoutGetBorder(_yogaNode, YGEdgeLeft);
        _borderTop    = YGNodeLayoutGetBorder(_yogaNode, YGEdgeTop);
        _borderRight  = YGNodeLayoutGetBorder(_yogaNode, YGEdgeRight);
        _borderBottom = YGNodeLayoutGetBorder(_yogaNode, YGEdgeBottom);

        _paddedRect.set(
            _x + YGNodeLayoutGetPadding(_yogaNode, YGEdgeLeft) + _borderLeft,
            _y + YGNodeLayoutGetPadding(_yogaNode, YGEdgeTop) + _borderTop,
            _x + _width - (YGNodeLayoutGetPadding(_yogaNode, YGEdgeRight) + _borderRight),
            _y + _height - (YGNodeLayoutGetPadding(_yogaNode, YGEdgeBottom) + _borderBottom)
        );


        _drawBorder = _borderLeft != 0
                      || _borderRight != 0
                      || _borderTop != 0
                      || _borderBottom != 0;

        _drawComplexBorders = !(
            _borderLeft == _borderRight
            && _borderRight == _borderTop
            && _borderTop == _borderBottom
        );

        // Children should also update
        _boundsLeft   = _x;
        _boundsTop    = _y;
        _boundsRight  = _x + _width;
        _boundsBottom = _y + _height;
        for (auto &child: _children) {
            child->layoutUpdated();

            _boundsLeft   = std::min(_boundsLeft, _x + child->boundsLeft());
            _boundsTop    = std::min(_boundsTop, _y + child->boundsTop());
            _boundsRight  = std::max(_boundsRight, _x + child->boundsRight());
            _boundsBottom = std::max(_boundsBottom, _y + child->boundsBottom());
        }
        _boundsRect.set(_boundsLeft, _boundsTop, _boundsRight, _boundsBottom);

        layoutReady = true;

        if (previousWidth != _width || previousHeight != _height) {
            onResized(_width, _height);
        }
    }

    // endregion

    // region Draw

    YGSize Div::measure(float width, YGMeasureMode /*widthMode*/, float height, YGMeasureMode /*heightMode*/) {
        return YGSize{width, height};
    }

    void Div::render(SkCanvas *canvas) {
        // Update styles first since it can have an impact on visibility
        if (_styleDirty) {
            updateStyle();
        }

        if (!layoutReady || !_visible || canvas->quickReject(_rect)) {
            return;
        }

        canvas->save();

        draw(canvas);

        clip(canvas);

        canvas->translate(_scrollX, _scrollY);

        // region Debug
        #ifdef DEBUG_LAYOUT
        if (debugLayout) {
            SkPaint paint;
            paint.setBlendMode(SkBlendMode::kPlus);
            paint.setStyle(SkPaint::kStroke_Style);
            if (dashed) {
                const SkScalar intervals[] = {1.0f, 1.0f};
                paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
            }
            paint.setStrokeWidth(SkIntToScalar(1));

            paint.setColor(0x7FFF0000);
            SkRect dRect = _rect.makeInset(0.5f, 0.5f);
            canvas->drawRect(dRect, paint);

            paint.setColor(0x7F0000FF);
            SkRect pRect = _paddedRect.makeInset(0.5f, 0.5f);
            canvas->drawRect(pRect, paint);

            paint.setColor(0x30FFFF00);
            SkRect bRect{(float) _boundsLeft, (float) _boundsTop, (float) _boundsRight, (float) _boundsBottom};
            bRect.inset(0.5f, 0.5f);
            canvas->drawRect(bRect, paint);

            if (_borderLeft > 0 || _borderTop > 0 || _borderRight > 0 || _borderBottom > 0) {
                paint.setColor(0x7F00FF00);
                canvas->drawRect(
                    SkRect{
                        _x + _borderLeft + 0.5f,
                        _y + _borderTop + 0.5f,
                        _x + _rect.width() - _borderRight - 0.5f,
                        _y + _rect.height() - _borderBottom - 0.5f
                    }, paint
                );
            }

            if (_mouseDown) {
                paint.setColor(0x30FFFFFF);
                paint.setStrokeWidth(2);
                canvas->drawRect({_x + 1.f, _y + 1.f, _x + _width - 1.f, _y + _height - 1.f}, paint);
            } else if (_mouseOver) {
                paint.setColor(0x10FFFFFF);
                paint.setStrokeWidth(2);
                canvas->drawRect({_x + 1.f, _y + 1.f, _x + _width - 1.f, _y + _height - 1.f}, paint);
            }
        }
        #endif
        // endregion

        canvas->translate(_x, _y);

        // This is the only place where we iterate in reverse because we order children in reverse in the vector
        // as to not explode when we modify children in event handlers. We still need to render back-to-front though ;)
        for (auto child = _children.rbegin(); child != _children.rend(); ++child) {
            (*child)->render(canvas);
        }

        canvas->restore();
    }

    void Div::clip(SkCanvas *canvas) {
        bool clip = _computedStyle->get(overflow) != "visible";
        if (!clip) {
            return;
        }

        bool aa = _computedStyle->get(antiAlias);
        if (_drawRoundRect && _drawBorder) {
            SkRRect roundRect;
            // TODO: Offset for unequal borders
            float   hb = _computedStyle->get(border) / 2;
            _roundRect.inset(hb, hb, &roundRect);
            canvas->clipRRect(roundRect, aa);
        } else if (_drawRoundRect) {
            canvas->clipRRect(_roundRect, aa);
        } else {
            canvas->clipRect(_rect, aa);
        }
    }

    void Div::draw(SkCanvas *canvas) {
        // region Background & Borders
        if (_drawBackground || _drawBorder) {
            SkPaint paint;
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(_computedStyle->get(backgroundColor));
            paint.setAntiAlias(_computedStyle->get(antiAlias));
            if (_computedStyle->has(opacity)) {
                paint.setAlpha((unsigned int) (_computedStyle->get(opacity) * 255.f));
            }

            float hb = _computedStyle->get(border) / 2;
            if (_drawComplexRoundRect) {
                SkRRect roundRect = _roundRect;
                if (_drawBackground && _drawBorder) {
                    // TODO: Offset for unequal borders
                    roundRect.inset(hb, hb);
                }

                if (_drawBackground) {
                    canvas->drawRRect(roundRect, paint);
                }

                if (_drawBorder) {
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setColor(_computedStyle->get(borderColor));
                    paint.setStrokeWidth(_computedStyle->get(border));
                    canvas->drawRRect(roundRect, paint);
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
                if (_drawBorder && !_drawComplexBorders) {
                    SkRect inset = _rect.makeInset(hb, hb);
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setColor(_computedStyle->get(borderColor));
                    paint.setStrokeWidth(_computedStyle->get(border));
                    canvas->drawRect(inset, paint);
                }
            }
        }
        // endregion
        // region Individual Borders

        // TODO: Optimize
        if (_drawComplexBorders && !_drawRoundRect) {
            SkPaint paint;
            paint.setAntiAlias(false);
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

    bool Div::mouseEnabled() const {
        return _mouseEnabled;
    }

    Div *Div::setMouseEnabled(bool mouseEnabled) {
        _mouseEnabled = mouseEnabled;
        return this;
    }

    bool Div::mouseChildren() const {
        return _mouseChildren;
    }

    Div *Div::setMouseChildren(bool mouseChildren) {
        _mouseChildren = mouseChildren;
        return this;
    }

    bool Div::mouseOver() const {
        return _mouseOver;
    }

    void Div::setMouseOver(bool over) {
        _mouseOver = over;
        invalidateStyle();
    }

    // region Buttons

    bool Div::getMouseDown() const {
        return _mouseDown;
    }

    void Div::setMouseDown(bool down) {
        if (down != _mouseDown) {
            _mouseDown = down;
            invalidateStyle();
        }
    }

    MouseEventStatus Div::mouseButton(const int mouseX, const int mouseY, const int button, const bool down, const int modifiers) {
        if (!_visible || !boundsContains(mouseX, mouseY)) {
            return Out;
        }

        int              localMouseX = mouseX - _x - _scrollX;
        int              localMouseY = mouseY - _y - _scrollY;
        MouseEventStatus ret         = Out;

        if (_mouseChildren) {
            for (auto &child: _children) {
                auto res = child->mouseButton(localMouseX, localMouseY, button, down, modifiers);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled) {
            if (ret != Handled && onMouseButton.hasSubscriptions()) {
                onMouseButton(localMouseX, localMouseY, button, down, modifiers);
                ret = Handled;
            } else {
                ret = Over;
            }
        }

        return ret;
    }

    MouseEventStatus Div::mouseDown(const int mouseX, const int mouseY, const int button, const int modifiers) {
        if (!_visible || !boundsContains(mouseX, mouseY)) {
            return Out;
        }

        setMouseDown(true);

        int              localMouseX = mouseX - _x - _scrollX;
        int              localMouseY = mouseY - _y - _scrollY;
        MouseEventStatus ret         = Out;

        if (_mouseChildren) {
            for (auto &child: _children) {
                auto res = child->mouseDown(localMouseX, localMouseY, button, modifiers);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled) {
            if (ret == Out && onMouseDown.hasSubscriptions()) {
                onMouseDown(localMouseX, localMouseY, button, modifiers);
                ret = Handled;
            } else {
                ret = Over;
            }
        }

        return ret;
    }

    MouseEventStatus Div::mouseUp(const int mouseX, const int mouseY, const int button, const int modifiers) {
        if (!_visible) {
            return Out;
        }

        bool isOver  = boundsContains(mouseX, mouseY);
        bool wasDown = _mouseDown;

        if (isOver || wasDown) {
            setMouseDown(false);
        }

        int              localMouseX = mouseX - _x - _scrollX;
        int              localMouseY = mouseY - _y - _scrollY;
        MouseEventStatus ret         = Out;

        if (_mouseChildren && (isOver || wasDown)) {
            for (auto &child: _children) {
                auto res = child->mouseUp(localMouseX, localMouseY, button, modifiers);
                if (res != Out && ret != Handled) {
                    ret = res;
                }
            }
        }

        if (_mouseEnabled && wasDown) {
            // Special case for items receiving a mouse up outside
            onMouseUpOutside(localMouseX, localMouseY, button, modifiers);
            onMouseUp(localMouseX, localMouseY, button, modifiers);
        }

        if (!isOver) {
            return Out;
        }

        if (_mouseEnabled) {
            if (ret != Handled && onMouseUp.hasSubscriptions()) {
                onMouseUp(localMouseX, localMouseY, button, modifiers);
                ret = Handled;
            } else {
                ret = Over;
            }
        }

        return ret;
    }

    MouseEventStatus Div::click(const int mouseX, const int mouseY, const int button, const int modifiers) {
        if (!_visible || !boundsContains(mouseX, mouseY)) {
            return Out;
        }

        if (!_mouseDown) {
            return Over;
        }

        int              localMouseX = mouseX - _x - _scrollX;
        int              localMouseY = mouseY - _y - _scrollY;
        MouseEventStatus ret         = Out;

        if (_mouseChildren) {
            for (auto &child: _children) {
                auto res = child->click(localMouseX, localMouseY, button, modifiers);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled) {
            if (ret != Handled && onClick.hasSubscriptions()) {
                onClick();
                ret = Handled;
            } else {
                ret = Over;
            }
        }

        return ret;
    }

    // endregion

    // region Move

    bool Div::mouseExited(const int mouseX, const int mouseY, const int button, const int modifiers) {
        if (!_mouseOver) {
            return false;
        }

        setMouseOver(false);

        int localMouseX = mouseX - _x - _scrollX;
        int localMouseY = mouseY - _y - _scrollY;
        if (_mouseChildren) {
            for (auto &child: _children) {
                if (child->mouseExited(localMouseX, localMouseY, button, modifiers)) {
                    break;
                }
            }
        }

        onMouseOut();

        return true;
    }

    MouseEventStatus Div::mouseMoved(const int mouseX, const int mouseY, const int button, const int modifiers, bool handled) {
        if (!_visible) {
            return Out;
        }

        bool isOver  = boundsContains(mouseX, mouseY);
        bool wasOver = _mouseOver;

        if (!isOver && wasOver) {
            mouseExited(mouseX, mouseY, button, modifiers);
        }

        if (!isOver) {
            return Out;
        }

        // From now on we're over the component

        if (!handled) {
            setMouseOver(true);
        }

        int              localMouseX = mouseX - _x - _scrollX;
        int              localMouseY = mouseY - _y - _scrollY;
        MouseEventStatus ret         = Out;

        if (_mouseChildren) {
            for (auto &child: _children) {
                auto res = child->mouseMoved(localMouseX, localMouseY, button, modifiers, handled);
                if (res != Out) {
                    handled = true;
                    if (ret != Handled) {
                        ret = res;
                    }
                }
            }
        }

        if (_mouseEnabled) {
            if (!handled && !wasOver) {
                onMouseOver();
            }
            onMouseMove(localMouseX, localMouseY, button, modifiers);
            if (!handled && ret != Handled) {
                window()->setCursor(_computedStyle->get(cursor));
                ret = Handled;
            } else {
                ret = Over;
            }
        }

        return ret;
    }

    // endregion

    // region Scroll

    void Div::scroll(const double  scrollX, const double  scrollY) {
        bool scrolled = false;

        if (_width < _boundsRight - _boundsLeft) {
            int previousScrollX = _scrollX;
            _scrollX = std::min(
                0,
                std::max(
                    _width - (_boundsRight - _boundsLeft),
                    _scrollX + (int) std::ceil(scrollX) * 2
                ));
            scrolled = previousScrollX != _scrollX;
        }
        if (_height < _boundsBottom - _boundsTop) {
            int previousScrollY = _scrollY;
            _scrollX += scrollX * 2.0f;
            _scrollY            = std::min(
                0,
                std::max(
                    _height - (_boundsBottom - _boundsTop),
                    _scrollY + (int) std::ceil(scrollY) * 2
                ));
            scrolled            = previousScrollY != _scrollY;
        }

        if (scrolled) {
            onScrolled(_scrollX, _scrollY);
        }
    }

    MouseEventStatus Div::mouseScrolled(const int mouseX, const int mouseY, const double scrollX, const double scrollY) {
        if (!_visible || !boundsContains(mouseX, mouseY)) {
            return Out;
        }

        if (_computedStyle->get(overflow) == "scroll") {
            scroll(scrollX, scrollY);
        }

        int              localMouseX = mouseX - _x - _scrollX;
        int              localMouseY = mouseY - _y - _scrollY;
        MouseEventStatus ret         = Out;

        if (_mouseChildren) {
            for (auto &child: _children) {
                auto res = child->mouseScrolled(localMouseX, localMouseY, scrollX, scrollY);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled) {
            if (ret != Handled && onMouseScroll.hasSubscriptions()) {
                onMouseScroll(localMouseX, localMouseY, scrollX, scrollY);
                ret = Handled;
            } else {
                ret = Over;
            }
        }

        return ret;
    }

// endregion

// STUFF>>>>>>>>>>>>

    bool Div::focusEvent(bool focused) {
        _focused = focused;
        return false;
    }

    bool Div::keyboardEvent(int, int, int, int) {
        return false;
    }

    bool Div::keyboardCharacterEvent(unsigned int) {
        return false;
    }

// endregion

}
