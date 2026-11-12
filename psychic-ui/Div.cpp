#include <cassert>
#include <cmath>
#include <iostream>
#include <SkPaint.h>
#include <SkDashPathEffect.h>
#include "utils/YogaUtils.hpp"
#include "yoga/Yoga.h"
#include "Div.hpp"
#include "Window.hpp"


namespace psychic_ui {

    namespace {
        // Shared by every Yoga node: built when the first Div is, so as a static it is destroyed after every static Div
        struct YogaConfig {
            YGConfigRef config{YGConfigNew()};

            YogaConfig() {
                YGConfigSetUseWebDefaults(config, true);
                YGConfigSetExperimentalFeatureEnabled(config, YGExperimentalFeatureWebFlexBasis, true);
                YGConfigSetPointScaleFactor(config, 0.0f); // We'll round the values ourselves, rounding is bugged
            }

            ~YogaConfig() {
                YGConfigFree(config);
            }
        };

        YGConfigRef yogaConfig() {
            static YogaConfig yogaConfig;
            return yogaConfig.config;
        }
    }

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

    int Div::idCounter = 0;

    Div::Div() :
        Observer(),
        _internalId(std::to_string(idCounter++)),
        _defaultStyle(std::make_unique<Style>([this]() { invalidateStyle(); })),
        _inlineStyle(std::make_unique<Style>([this]() { invalidateStyle(); })),
        _computedStyle(std::make_unique<Style>()),
        _yogaNode(YGNodeNewWithConfig(yogaConfig())) {
        setTag("div");

        YGNodeSetContext(_yogaNode, this);
    }

    Div::~Div() {
        YGNodeFree(_yogaNode);
    }

    std::string Div::toString() const {
        std::string str;
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
        for (auto &className: _classNames) {
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
                if (_focused) {
                    window()->requestFocus(_parent);
                }
            }
            _parent = parent;
            // Temporary, will be updated once we're on the render list
            _depth  = _parent ? _parent->depth() + 1 : 0;
            if (_parent) {
                added();
                if (window()) {
                    addedToRenderRecursive();
                }
                // Here we force update the style instead of relying on the invalidation
                // because styles control the layout, and we don't want the div
                // doing a style-less render pass on its first frame alive. Only after
                // `addedToRenderRecursive` since it can make calls that would change
                // the styling.
                updateStyleRecursive();
            }
        }
    }

    int Div::depth() const {
        return _depth;
    }

    void Div::added() {}

    void Div::removed() {}

    void Div::addedToRenderRecursive() {
        // Update the depth first so that styles have access to it
        _depth = _parent ? _parent->depth() + 1 : 0;
        createStyles();
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
        assert(index <= childCount());
        // The current first/last children may lose their :first-child/:last-child state
        invalidateEndChildrenStyle();
        // Insert before setting the parent so that the child's first style pass sees its index
        _children.insert(_children.cbegin() + index, child);
        YGNodeInsertChild(_yogaNode, child->_yogaNode, index);
        child->setParent(this);
        return child;
    }

    std::shared_ptr<Div> Div::add(std::shared_ptr<Div> child) {
        assert(child != nullptr);
        return add(childCount(), child);
    }

    void Div::remove(const std::shared_ptr<Div> child) {
        assert(child != nullptr);
        int index = childIndex(child);
        if (index >= 0) {
            remove((unsigned int) index);
        }
    }

    void Div::remove(unsigned int index) {
        assert(index < childCount());
        std::shared_ptr<Div> child = _children[index];
        _children.erase(_children.cbegin() + index);
        YGNodeRemoveChild(_yogaNode, child->_yogaNode);
        // The new first/last children gain their :first-child/:last-child state
        invalidateEndChildrenStyle();
        child->setParent(nullptr);
    }

    void Div::removeAll() {
        if (_children.empty()) {
            return;
        }
        auto children = std::move(_children);
        _children.clear();
        YGNodeRemoveAllChildren(_yogaNode);
        for (auto &child: children) {
            child->setParent(nullptr);
        }
    }

    int Div::childIndex(const std::shared_ptr<Div> child) const {
        assert(child != nullptr);
        return childIndex(child.get());
    }

    int Div::childIndex(const Div *child) const {
        assert(child != nullptr);
        auto it = std::find_if(
            _children.cbegin(),
            _children.cend(),
            [child](const auto &c) {
                return c.get() == child;
            }
        );
        return it == _children.cend() ? -1 : (int) std::distance(_children.cbegin(), it);
    }

    const Div *Div::at(unsigned int index) const {
        assert(index < childCount());
        return _children[index].get();
    }

    Div *Div::at(unsigned int index) {
        assert(index < childCount());
        return _children[index].get();
    }

    std::vector<std::shared_ptr<Div>> Div::childrenTopmostFirst() const {
        return {_children.crbegin(), _children.crend()};
    }

    void Div::invalidateEndChildrenStyle() {
        if (!_children.empty()) {
            _children.front()->invalidateStyle();
            _children.back()->invalidateStyle();
        }
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

    Div *Div::setEnabled(bool enabled) {
        if (_enabled != enabled) {
            _enabled = enabled;
            invalidateStyle();
        }
        return this;
    }

    bool Div::active() const {
        return _enabled && _mouseDown;
    };

    // endregion

    // region Focus

    bool Div::focusEnabled() const {
        return _focusEnabled;
    }

    Div *Div::setFocusEnabled(bool focusEnabled) {
        _focusEnabled = focusEnabled;
        return this;
    }

    bool Div::focused() const {
        return _focused;
    }

    void Div::setFocused(bool focused) {
        if (_focused != focused) {
            _focused = focused;
            invalidateStyle();
        }
    }

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

    // endregion

    // region Position & Dimensions

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
        _inlineStyle
            ->set(left, static_cast<float>(x))
            ->set(top, static_cast<float>(y));
    }

    int Div::x() const {
        return _x;
    }

    void Div::setX(int x) {
        _inlineStyle->set(left, static_cast<float>(x));
    }

    int Div::y() const {
        return _y;
    }

    void Div::setY(int y) {
        _inlineStyle->set(top, static_cast<float>(y));
    }

    void Div::setSize(int width, int height) {
        _inlineStyle
            ->set(psychic_ui::width, static_cast<float>(width))
            ->set(psychic_ui::height, static_cast<float>(height));
    }

    int Div::getWidth() const {
        return _width;
    }

    void Div::setWidth(int width) {
        _inlineStyle->set(psychic_ui::width, static_cast<float>(width));
    }

    int Div::getHeight() const {
        return _height;
    }

    void Div::setHeight(int height) {
        _inlineStyle->set(psychic_ui::height, static_cast<float>(height));
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
            return nullptr;
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

    Div *Div::setTag(std::string tag) {
        std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
        _tags.push_back(tag);
        invalidateStyle();
        return this;
    }

    const std::vector<std::string> &Div::tags() const {
        return _tags;
    }

    const std::string Div::internalId() const {
        return _internalId;
    }

    const std::string Div::id() const {
        return _id;
    }

    Div *Div::setId(std::string id) {
        std::transform(id.begin(), id.end(), id.begin(), ::tolower);
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
        std::unordered_set<std::string> lcClassNames;
        std::transform(
            classNames.begin(), classNames.end(), std::inserter(lcClassNames, std::end(lcClassNames)),
            [](auto className) {
                std::string c = className;
                std::transform(c.begin(), c.end(), c.begin(), ::tolower);
                return c;
            }
        );
        _classNames = std::move(classNames);
        invalidateStyle();
        return this;
    }

    Div *Div::addClassName(std::string className) {
        std::transform(className.begin(), className.end(), className.begin(), ::tolower);
        auto res = _classNames.insert(className);
        if (res.second) {
            invalidateStyle();
        }
        return this;
    }

    Div *Div::removeClassName(std::string className) {
        std::transform(className.begin(), className.end(), className.begin(), ::tolower);
        auto res = _classNames.erase(className);
        if (res) {
            invalidateStyle();
        }
        return this;
    }

    InheritableValues Div::inheritableValues() const {
        return _inheritableValues;
    }

    // endregion

    void Div::updateRuntimeStyles() {
        createStyles();
        for (auto &child: _children) {
            child->updateRuntimeStyles();
        }
    }

    void Div::invalidateStyle() {
        if (!_styleDirty) {
            // Descendants inherit from us and can match on our state, the pass recomputes them with us
            _styleDirty = true;
            for (const auto &child: _children) {
                child->invalidateStyle();
            }
        }
        // Let the style pass find us from the root
        for (Div *ancestor = _parent; ancestor && !ancestor->_descendantStyleDirty; ancestor = ancestor->_parent) {
            ancestor->_descendantStyleDirty = true;
        }
    }

    bool Div::styleDirty() const {
        return _styleDirty || _descendantStyleDirty;
    }

    void Div::updateStyle() {
        if (auto sm = styleManager()) {
            _computedStyle = sm->computeStyle(this);
            updateLayout();
            _styleDirty = false;
            styleUpdated();
        }
    }

    void Div::updateStyleRecursive() {
        // Cleared first so that invalidations made by the callbacks below flag us again
        _descendantStyleDirty = false;
        updateStyle();
        for (auto &child: _children) {
            child->updateStyleRecursive();
        }
    }

    void Div::updateDirtyStyles() {
        if (_styleDirty) {
            updateStyleRecursive();
        } else if (_descendantStyleDirty) {
            _descendantStyleDirty = false;
            for (auto &child: _children) {
                child->updateDirtyStyles();
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
        _drawBackground = _computedStyle->has(backgroundColor)
                          && (_computedStyle->get(backgroundColor) != 0x00000000);
        // endregion
    }

    // endregion

    // region Layout

    void Div::invalidate() {
        YGNodeMarkDirty(_yogaNode);
        //std::cout << "Mark dirty" << std::endl;
    }

    bool Div::isValid() const {
        std::cout << "Is dirty: " << (YGNodeIsDirty(_yogaNode) ? "Yes" : "No") << std::endl;
        return !YGNodeIsDirty(_yogaNode);
    }

    void Div::setMeasurable() {
        YGNodeSetMeasureFunc(
            _yogaNode,
            [](YGNodeConstRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
                YGSize size{};
                auto   div = static_cast<Div *>(YGNodeGetContext(node));
                if (!div) {
                    std::cerr << "Could not find div to measure" << std::endl;
                    return size;
                }
                return div->measure(width, widthMode, height, heightMode);
            }
        );
    }

    // region Yoga Macros

    #define YOGA_STYLE_SET(prop, conv, style, fallback) \
        YGNodeStyleSet##prop(_yogaNode, Yoga##conv##FromString(_computedStyle->get(style), fallback)); \

    #define YOGA_STYLE_SET_FLOAT_UNDEFINED(prop, style) \
        if (_computedStyle->has(style)) { \
            float value = _computedStyle->get(style); \
            YGNodeStyleSet##prop(_yogaNode, std::isnan(value) ? YGUndefined : value); \
        } else if (!YGFloatIsUndefined(YGNodeStyleGet##prop(_yogaNode))) { \
            YGNodeStyleSet##prop(_yogaNode, YGUndefined); \
        } \

    #define YOGA_STYLE_SET_FLOAT(prop, style, fallback) \
        if (_computedStyle->has(style)) { \
            YGNodeStyleSet##prop(_yogaNode, _computedStyle->get(style)); \
        } else if (YGNodeStyleGet##prop(_yogaNode) != (fallback)) { \
            /*Yoga returns the default value when it is set as undefined*/ \
            YGNodeStyleSet##prop(_yogaNode, YGUndefined); \
        } \

    #define YOGA_STYLE_SET_PERCENT(prop, style) \
        if (_computedStyle->has(style) && !std::isnan(_computedStyle->get(style))) { \
            YGNodeStyleSet##prop(_yogaNode, _computedStyle->get(style)); \
        } else if (_computedStyle->has(style##Percent)) { \
            YGNodeStyleSet##prop##Percent(_yogaNode, YogaPercent(_computedStyle->get(style##Percent))); \
        } else if (YGNodeStyleGet##prop(_yogaNode).unit != YGUnitAuto) { \
            YGNodeStyleSet##prop(_yogaNode, YGUndefined); \
        } \

    #define YOGA_STYLE_SET_PERCENT_AUTO(prop, style) \
        if (_computedStyle->has(style)) { \
            float value = _computedStyle->get(style); \
            if (std::isnan(value)) { \
                YGNodeStyleSet##prop##Auto(_yogaNode); \
            } else { \
                YGNodeStyleSet##prop(_yogaNode, value); \
            } \
        } else if (_computedStyle->has(style##Percent)) { \
            YGNodeStyleSet##prop##Percent(_yogaNode, YogaPercent(_computedStyle->get(style##Percent))); \
        } else if (YGNodeStyleGet##prop(_yogaNode).unit != YGUnitAuto) { \
            YGNodeStyleSet##prop##Auto(_yogaNode); \
        } \

    #define YOGA_STYLE_SET_EDGE_FLOAT(prop, edge, style) \
        if (_computedStyle->has(style)) { \
            float value = _computedStyle->get(style); \
            YGNodeStyleSet##prop(_yogaNode, YGEdge##edge, std::isnan(value) ? YGUndefined : value); \
        } else if (!YGFloatIsUndefined(YGNodeStyleGet##prop(_yogaNode, YGEdge##edge))) { \
            YGNodeStyleSet##prop(_yogaNode, YGEdge##edge, YGUndefined); \
        } \

    #define YOGA_STYLE_SET_EDGE_PERCENT(prop, edge, style) \
        if (_computedStyle->has(style) && !std::isnan(_computedStyle->get(style))) { \
            YGNodeStyleSet##prop(_yogaNode, YGEdge##edge, _computedStyle->get(style)); \
        } else if (_computedStyle->has(style##Percent)) { \
            YGNodeStyleSet##prop##Percent(_yogaNode, YGEdge##edge, YogaPercent(_computedStyle->get(style##Percent))); \
        } else if (YGNodeStyleGet##prop(_yogaNode, YGEdge##edge).unit != YGUnitUndefined) { \
            YGNodeStyleSet##prop(_yogaNode, YGEdge##edge, YGUndefined); \
        } \

    #define YOGA_STYLE_SET_EDGE_PERCENT_AUTO(prop, edge, style) \
        if (_computedStyle->has(style)) { \
            float value = _computedStyle->get(style); \
            if (std::isnan(value)) { \
                YGNodeStyleSet##prop##Auto(_yogaNode, YGEdge##edge); \
            } else { \
                YGNodeStyleSet##prop(_yogaNode, YGEdge##edge, value); \
            } \
        } else if (_computedStyle->has(style##Percent)) { \
            YGNodeStyleSet##prop##Percent(_yogaNode, YGEdge##edge, YogaPercent(_computedStyle->get(style##Percent))); \
        } else if (YGNodeStyleGet##prop(_yogaNode, YGEdge##edge).unit != YGUnitUndefined) { \
            YGNodeStyleSet##prop(_yogaNode, YGEdge##edge, YGUndefined); \
        } \

    // endregion

    void Div::updateLayout() {
        YOGA_STYLE_SET(Direction, Direction, direction, YGDirectionInherit)
        YOGA_STYLE_SET(FlexDirection, FlexDirection, flexDirection, YGFlexDirectionColumn)
        YOGA_STYLE_SET(JustifyContent, Justify, justifyContent, YGJustifyFlexStart)
        YOGA_STYLE_SET(AlignContent, Align, alignContent, YGAlignFlexStart)
        YOGA_STYLE_SET(AlignItems, Align, alignItems, YGAlignStretch)
        YOGA_STYLE_SET(AlignSelf, Align, alignSelf, YGAlignAuto)
        YOGA_STYLE_SET(PositionType, Position, position, YGPositionTypeRelative)
        YOGA_STYLE_SET(FlexWrap, Wrap, flexWrap, YGWrapNoWrap)
        YOGA_STYLE_SET(Overflow, Overflow, overflow, YGOverflowVisible)
        YOGA_STYLE_SET(Display, Display, display, YGDisplayFlex)

        YOGA_STYLE_SET_FLOAT_UNDEFINED(Flex, flex)
        YOGA_STYLE_SET_FLOAT(FlexGrow, grow, /*kDefaultFlexGrow*/ 0.0f)
        YOGA_STYLE_SET_FLOAT(FlexShrink, shrink, /*kWebDefaultFlexShrink*/ 1.0f) // NOTE: Not the default actually
        YOGA_STYLE_SET_PERCENT_AUTO(FlexBasis, basis)

        YOGA_STYLE_SET_EDGE_PERCENT(Position, Left, left)
        YOGA_STYLE_SET_EDGE_PERCENT(Position, Top, top)
        YOGA_STYLE_SET_EDGE_PERCENT(Position, Right, right)
        YOGA_STYLE_SET_EDGE_PERCENT(Position, Bottom, bottom)

        YOGA_STYLE_SET_PERCENT_AUTO(Width, width)
        YOGA_STYLE_SET_PERCENT(MinWidth, minWidth)
        YOGA_STYLE_SET_PERCENT(MaxWidth, maxWidth)
        YOGA_STYLE_SET_PERCENT_AUTO(Height, height)
        YOGA_STYLE_SET_PERCENT(MinHeight, minHeight)
        YOGA_STYLE_SET_PERCENT(MaxHeight, maxHeight)
        // TODO AspectRatio

        YOGA_STYLE_SET_EDGE_PERCENT_AUTO(Margin, All, margin)
        YOGA_STYLE_SET_EDGE_PERCENT_AUTO(Margin, Horizontal, marginHorizontal)
        YOGA_STYLE_SET_EDGE_PERCENT_AUTO(Margin, Vertical, marginVertical)
        YOGA_STYLE_SET_EDGE_PERCENT_AUTO(Margin, Left, marginLeft)
        YOGA_STYLE_SET_EDGE_PERCENT_AUTO(Margin, Top, marginTop)
        YOGA_STYLE_SET_EDGE_PERCENT_AUTO(Margin, Right, marginRight)
        YOGA_STYLE_SET_EDGE_PERCENT_AUTO(Margin, Bottom, marginBottom)

        if (!_ignoreInternalLayoutContraints) {
            YOGA_STYLE_SET_EDGE_PERCENT(Padding, All, padding)
            YOGA_STYLE_SET_EDGE_PERCENT(Padding, Horizontal, paddingHorizontal)
            YOGA_STYLE_SET_EDGE_PERCENT(Padding, Vertical, paddingVertical)
            YOGA_STYLE_SET_EDGE_PERCENT(Padding, Left, paddingLeft)
            YOGA_STYLE_SET_EDGE_PERCENT(Padding, Top, paddingTop)
            YOGA_STYLE_SET_EDGE_PERCENT(Padding, Right, paddingRight)
            YOGA_STYLE_SET_EDGE_PERCENT(Padding, Bottom, paddingBottom)

            YOGA_STYLE_SET_EDGE_FLOAT(Border, All, border)
            YOGA_STYLE_SET_EDGE_FLOAT(Border, Horizontal, borderHorizontal)
            YOGA_STYLE_SET_EDGE_FLOAT(Border, Vertical, borderVertical)
            YOGA_STYLE_SET_EDGE_FLOAT(Border, Left, borderLeft)
            YOGA_STYLE_SET_EDGE_FLOAT(Border, Top, borderTop)
            YOGA_STYLE_SET_EDGE_FLOAT(Border, Right, borderRight)
            YOGA_STYLE_SET_EDGE_FLOAT(Border, Bottom, borderBottom)
        }
    }

    void Div::layoutUpdated() {
        if (!YGNodeGetHasNewLayout(_yogaNode)) {
            return;
        }

        YGNodeSetHasNewLayout(_yogaNode, false);

        _x = (int) std::ceil(YGNodeLayoutGetLeft(_yogaNode));
        _y = (int) std::ceil(YGNodeLayoutGetTop(_yogaNode));

        int previousWidth  = _width;
        int previousHeight = _height;

        _width  = (int) std::ceil(YGNodeLayoutGetWidth(_yogaNode));
        _height = (int) std::ceil(YGNodeLayoutGetHeight(_yogaNode));

        _rect.set(_x, _y, _x + _width, _y + _height);
        _roundRect.setRectRadii(_rect, _radii);

        _marginLeft   = YGNodeLayoutGetMargin(_yogaNode, YGEdgeLeft);
        _marginTop    = YGNodeLayoutGetMargin(_yogaNode, YGEdgeTop);
        _marginRight  = YGNodeLayoutGetMargin(_yogaNode, YGEdgeRight);
        _marginBottom = YGNodeLayoutGetMargin(_yogaNode, YGEdgeBottom);

        _marginRect.set(
            _x - _marginLeft,
            _y - _marginTop,
            _x + _width + _marginRight,
            _y + _height + _marginBottom
        );

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
        SkRect previousBoundsRect = _boundsRect;
        _boundsRect.set(_boundsLeft, _boundsTop, _boundsRight, _boundsBottom);

        layoutReady = true;

        if (previousWidth != _width || previousHeight != _height || previousBoundsRect != _boundsRect) {
            onResized(_width, _height);
        }
    }

    // endregion

    // region Draw

    YGSize Div::measure(float width, YGMeasureMode /*widthMode*/, float height, YGMeasureMode /*heightMode*/) {
        return YGSize{width, height};
    }

    void Div::render(SkCanvas *canvas) {
        // Paint only: styles and layout were updated by the window before rendering
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
            SkRect mRect = _marginRect.makeInset(0.5f, 0.5f);
            canvas->drawRect(mRect, paint);

            paint.setColor(0x7F00FF00);
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
                paint.setColor(0x7F00FFFF);
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

        // Back-to-front: children are in document order, the last one is on top
        for (auto &child: _children) {
            child->render(canvas);
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

            } else {
                if (_drawBackground) {
                    canvas->drawRect(_rect, paint);
                }
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
                    float hw = _borderLeft / 2.0f;
                    canvas->drawLine(hw, -0.5f, hw, _rect.bottom() + 0.5f, paint);
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
                    float hw = _borderRight / 2.0f;
                    canvas->drawLine(_rect.right() - hw, -0.5f, _rect.right() - hw, _rect.bottom() + 0.5f, paint);
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
                    float hw = _borderTop / 2.0f;
                    canvas->drawLine(-0.5f, hw, _rect.right() + 0.5f, hw, paint);
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
                    float hw = _borderBottom / 2.0f;
                    canvas->drawLine(-0.5f, _rect.bottom() - hw, _rect.right() + 0.5f, _rect.bottom() - hw, paint);
                }
            }
        }

        // endregion
    }

    // endregion

    // region Interaction

    // region Properties

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
        if (_mouseOver != over) {
            _mouseOver = over;
            invalidateStyle();
        }
    }

    bool Div::getMouseDown() const {
        return _mouseDown;
    }

    void Div::setMouseDown(bool down) {
        if (down != _mouseDown) {
            _mouseDown = down;
            invalidateStyle();
        }
    }

    // endregion

    // region Buttons

    MouseEventStatus Div::mouseButton(const int mouseX, const int mouseY, const MouseButton button, const bool down, const Mod modifiers) {
        if (!_visible || !boundsContains(mouseX, mouseY)) {
            return Out;
        }

        int              localMouseX = mouseX - _x - _scrollX;
        int              localMouseY = mouseY - _y - _scrollY;
        MouseEventStatus ret         = Out;

        if (_mouseChildren) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->mouseButton(localMouseX, localMouseY, button, down, modifiers);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled && ret != Handled && onMouseButton.hasSubscriptions()) {
            onMouseButton(localMouseX, localMouseY, button, down, modifiers);
            ret = Handled;
        }

        return ret == Out ? Over : ret;
    }

    MouseEventStatus Div::mouseDown(const int mouseX, const int mouseY, const MouseButton button, const Mod modifiers) {
        if (!_visible || !boundsContains(mouseX, mouseY)) {
            return Out;
        }

        setMouseDown(true);

        int              localMouseX = mouseX - _x - _scrollX;
        int              localMouseY = mouseY - _y - _scrollY;
        MouseEventStatus ret         = Out;

        if (_mouseChildren) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->mouseDown(localMouseX, localMouseY, button, modifiers);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (ret == Out) {
            // We're the deepest we're gonna get, do a focus request
            window()->requestFocus(this);
        }

        if (_mouseEnabled && ret != Handled && onMouseDown.hasSubscriptions()) {
            onMouseDown(localMouseX, localMouseY, button, modifiers);
            ret = Handled;
        }

        // onClick swallows mouse down events, otherwise a parent might hijack it
        return ret == Handled || onClick.hasSubscriptions() ? Handled : Over;
    }

    MouseEventStatus Div::mouseUp(const int mouseX, const int mouseY, const MouseButton button, const Mod modifiers) {
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
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->mouseUp(localMouseX, localMouseY, button, modifiers);
                if (res == Over) {
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

        if (_mouseEnabled && ret != Handled && onMouseUp.hasSubscriptions()) {
            onMouseUp(localMouseX, localMouseY, button, modifiers);
            ret = Handled;
        }

        return ret == Out ? Over : ret;
    }

    MouseEventStatus Div::click(const int mouseX, const int mouseY, const MouseButton button, const Mod modifiers) {
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
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->click(localMouseX, localMouseY, button, modifiers);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled && ret != Handled && onClick.hasSubscriptions()) {
            onClick();
            ret = Handled;
        }

        return ret == Out ? Over : ret;
    }

    MouseEventStatus Div::doubleClick(const int mouseX, const int mouseY, const unsigned int clickCount, const Mod modifiers) {
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
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->doubleClick(localMouseX, localMouseY, clickCount, modifiers);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled && ret != Handled && onDoubleClick.hasSubscriptions()) {
            onDoubleClick(clickCount);
            ret = Handled;
        }

        return ret == Out ? Over : ret;
    }

    // endregion

    // region Move

    bool Div::mouseExited(const int mouseX, const int mouseY, const int buttons, const Mod modifiers) {
        if (!_mouseOver) {
            return false;
        }

        setMouseOver(false);

        int localMouseX = mouseX - _x - _scrollX;
        int localMouseY = mouseY - _y - _scrollY;
        if (_mouseChildren) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                if (child->mouseExited(localMouseX, localMouseY, buttons, modifiers)) {
                    break;
                }
            }
        }

        onMouseOut();

        return true;
    }

    MouseEventStatus Div::mouseMoved(const int mouseX, const int mouseY, const int buttons, const Mod modifiers, bool handled) {
        if (!_visible) {
            return Out;
        }

        bool isOver  = boundsContains(mouseX, mouseY);
        bool wasOver = _mouseOver;

        if (!isOver && wasOver) {
            mouseExited(mouseX, mouseY, buttons, modifiers);
        }

        bool allowOutsideMovement = this == window();

        if (!isOver && !allowOutsideMovement) {
            return Out;
        }

        // From now on we're over the div, unless we"re on window, which allow for the mouse to move outside

        int              localMouseX = mouseX - _x - _scrollX;
        int              localMouseY = mouseY - _y - _scrollY;
        MouseEventStatus ret         = Out;

        if (isOver) {
            if (!handled) {
                setMouseOver(true);
            }

            if (_mouseChildren) {
                // Topmost first, over a copy since handlers can add or remove children
                for (auto &child: childrenTopmostFirst()) {
                    if (child->_parent != this) {
                        continue; // Removed by an earlier handler
                    }
                    auto res = child->mouseMoved(localMouseX, localMouseY, buttons, modifiers, handled);
                    if (res != Out) {
                        handled = true;
                        if (ret != Handled) {
                            ret = res;
                        }
                    }
                }
            }
        }

        if (_mouseEnabled) {
            if (!handled && isOver && !wasOver) {
                onMouseOver();
            }
            onMouseMove(localMouseX, localMouseY, buttons, modifiers);
            if (!handled && isOver && ret != Handled) {
                window()->setCursor(_computedStyle->get(cursor));
                ret = Handled;
            } else if (isOver) {
                ret = Over;
            }
        }

        return ret;
    }

    // endregion

    // region Scroll

    void Div::scroll(const double scrollX, const double scrollY) {
        bool scrolled = false;

        if (_width < _boundsRight - _boundsLeft) {
            int sx = std::min(
                0, std::max(
                    _width - (_boundsRight - _boundsLeft),
                    _scrollX + (int) std::ceil(scrollX) * 2
                ));

            scrolled = _scrollX != sx;
            _scrollX = sx;
        }

        if (_height < _boundsBottom - _boundsTop) {
            int sy = std::min(
                0, std::max(
                    _height - (_boundsBottom - _boundsTop),
                    _scrollY + (int) std::ceil(scrollY) * 2
                ));

            scrolled = _scrollY != sy;
            _scrollY = sy;
        }

        if (scrolled) {
            onScrolled(_scrollX, _scrollY);
        }
    }

    MouseEventStatus Div::mouseScrolled(const int mouseX, const int mouseY, const double scrollX, const double scrollY) {
        if (!_visible || !boundsContains(mouseX, mouseY)) {
            return Out;
        }

        int              localMouseX = mouseX - _x - _scrollX;
        int              localMouseY = mouseY - _y - _scrollY;
        MouseEventStatus ret         = Out;

        if (_mouseChildren) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->mouseScrolled(localMouseX, localMouseY, scrollX, scrollY);
                if (res != Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled && ret != Handled && onMouseScroll.hasSubscriptions()) {
            onMouseScroll(localMouseX, localMouseY, scrollX, scrollY);
            ret = Handled;
        }

        if (ret != Handled && _computedStyle->get(overflow) == "scroll") {
            // TODO: Cancel if already handled, we can't scroll two divs at the same time
            scroll(scrollX, scrollY);
        }

        return ret == Out ? Over : ret;
    }

    // endregion

    // endregion

}
