#include <cassert>
#include <cmath>
#include <iostream>
#include <SkPaint.h>
#include <SkDashPathEffect.h>
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

    #ifdef DEBUG_LAYOUT
    bool Div::debugLayout{false};
    #endif

    int Div::idCounter = 0;

    Div::Div() :
        Observer(),
        _internalId(std::to_string(idCounter++)),
        _defaultStyle([](void *div) { static_cast<Div *>(div)->invalidateStyle(); }, this),
        _inlineStyle([](void *div) { static_cast<Div *>(div)->invalidateStyle(); }, this),
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
        _inlineStyle.visible(value);
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
        if (_computedStyle.overflow() == Overflow::Visible) {
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
            .left(x)
            .top(y);
    }

    int Div::x() const {
        return _x;
    }

    void Div::setX(int x) {
        _inlineStyle.left(x);
    }

    int Div::y() const {
        return _y;
    }

    void Div::setY(int y) {
        _inlineStyle.top(y);
    }

    void Div::setSize(int width, int height) {
        _inlineStyle
            .width(width)
            .height(height);
    }

    int Div::getWidth() const {
        return _width;
    }

    void Div::setWidth(int width) {
        _inlineStyle.width(width);
    }

    int Div::getHeight() const {
        return _height;
    }

    void Div::setHeight(int height) {
        _inlineStyle.height(height);
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

    Style &Div::style() {
        return _inlineStyle;
    }

    const Style &Div::style() const {
        return _inlineStyle;
    }

    const Style &Div::computedStyle() const {
        return _computedStyle;
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

    const std::string &Div::internalId() const {
        return _internalId;
    }

    const std::string &Div::id() const {
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
        _classNames = std::move(lcClassNames);
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

    const PropertyMask &Div::inheritedProperties() const {
        return psychic_ui::inheritedProperties;
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
            Style computed = sm->computeStyle(this);
            // Only what changed reaches Yoga, everything the first time
            PropertyMask changed = _layoutInitialized ? _computedStyle.diff(computed) : PropertyMask{}.set();
            _computedStyle     = computed;
            _layoutInitialized = true;
            updateLayout(changed);
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
        // Derived state is always assigned, from the computed value or its default,
        // so that a rule that stops matching (:hover, a removed class) takes its effect away with it

        _visible = _computedStyle.visible();

        // region Radius
        // The most specific radius set wins: corner, then side, then all
        const Style &s = _computedStyle;
        _radiusTopLeft     = s.borderRadius();
        _radiusTopRight    = s.borderRadius();
        _radiusBottomLeft  = s.borderRadius();
        _radiusBottomRight = s.borderRadius();

        if (s.has(Property::borderRadiusLeft)) {
            _radiusTopLeft    = s.borderRadiusLeft();
            _radiusBottomLeft = s.borderRadiusLeft();
        }
        if (s.has(Property::borderRadiusRight)) {
            _radiusTopRight    = s.borderRadiusRight();
            _radiusBottomRight = s.borderRadiusRight();
        }
        if (s.has(Property::borderRadiusTop)) {
            _radiusTopLeft  = s.borderRadiusTop();
            _radiusTopRight = s.borderRadiusTop();
        }
        if (s.has(Property::borderRadiusBottom)) {
            _radiusBottomLeft  = s.borderRadiusBottom();
            _radiusBottomRight = s.borderRadiusBottom();
        }
        if (s.has(Property::borderRadiusTopLeft)) {
            _radiusTopLeft = s.borderRadiusTopLeft();
        }
        if (s.has(Property::borderRadiusTopRight)) {
            _radiusTopRight = s.borderRadiusTopRight();
        }
        if (s.has(Property::borderRadiusBottomLeft)) {
            _radiusBottomLeft = s.borderRadiusBottomLeft();
        }
        if (s.has(Property::borderRadiusBottomRight)) {
            _radiusBottomRight = s.borderRadiusBottomRight();
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

        // The layout may not change, but the rounded rect must follow the radii
        _roundRect.setRectRadii(_rect, _radii);
        // endregion

        // region Background
        _drawBackground = s.has(Property::backgroundColor) && s.backgroundColor().argb() != 0x00000000;
        // endregion
    }

    // endregion

    // region Layout

    void Div::invalidate() {
        // Yoga only lets leaves with a measure function be marked dirty (it asserts otherwise)
        if (YGNodeHasMeasureFunc(_yogaNode)) {
            YGNodeMarkDirty(_yogaNode);
        }
    }

    bool Div::isValid() const {
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

    // region Style to Yoga

    namespace {
        YGFlexDirection toYoga(Direction direction) {
            switch (direction) {
                case Direction::ColumnReverse:
                    return YGFlexDirectionColumnReverse;
                case Direction::Row:
                    return YGFlexDirectionRow;
                case Direction::RowReverse:
                    return YGFlexDirectionRowReverse;
                default:
                    return YGFlexDirectionColumn;
            }
        }

        YGDirection toYoga(TextDirection direction) {
            switch (direction) {
                case TextDirection::Ltr:
                    return YGDirectionLTR;
                case TextDirection::Rtl:
                    return YGDirectionRTL;
                default:
                    return YGDirectionInherit;
            }
        }

        YGJustify toYoga(Justify justify) {
            switch (justify) {
                case Justify::Center:
                    return YGJustifyCenter;
                case Justify::End:
                    return YGJustifyFlexEnd;
                case Justify::SpaceBetween:
                    return YGJustifySpaceBetween;
                case Justify::SpaceAround:
                    return YGJustifySpaceAround;
                case Justify::SpaceEvenly:
                    return YGJustifySpaceEvenly;
                default:
                    return YGJustifyFlexStart;
            }
        }

        YGAlign toYoga(Align align) {
            switch (align) {
                case Align::Start:
                    return YGAlignFlexStart;
                case Align::Center:
                    return YGAlignCenter;
                case Align::End:
                    return YGAlignFlexEnd;
                case Align::Stretch:
                    return YGAlignStretch;
                case Align::Baseline:
                    return YGAlignBaseline;
                case Align::SpaceBetween:
                    return YGAlignSpaceBetween;
                case Align::SpaceAround:
                    return YGAlignSpaceAround;
                case Align::SpaceEvenly:
                    return YGAlignSpaceEvenly;
                default:
                    return YGAlignAuto;
            }
        }

        YGPositionType toYoga(Position position) {
            switch (position) {
                case Position::Static:
                    return YGPositionTypeStatic;
                case Position::Absolute:
                    return YGPositionTypeAbsolute;
                default:
                    return YGPositionTypeRelative;
            }
        }

        YGWrap toYoga(Wrap wrap) {
            switch (wrap) {
                case Wrap::Wrap:
                    return YGWrapWrap;
                case Wrap::WrapReverse:
                    return YGWrapWrapReverse;
                default:
                    return YGWrapNoWrap;
            }
        }

        YGOverflow toYoga(Overflow overflow) {
            switch (overflow) {
                case Overflow::Hidden:
                    return YGOverflowHidden;
                case Overflow::Scroll:
                    return YGOverflowScroll;
                default:
                    return YGOverflowVisible;
            }
        }

        YGDisplay toYoga(Display display) {
            switch (display) {
                case Display::None:
                    return YGDisplayNone;
                case Display::Contents:
                    return YGDisplayContents;
                default:
                    return YGDisplayFlex;
            }
        }

        YGBoxSizing toYoga(BoxSizing boxSizing) {
            return boxSizing == BoxSizing::ContentBox ? YGBoxSizingContentBox : YGBoxSizingBorderBox;
        }

        using SetFloat = void (*)(YGNodeRef, float);
        using SetAuto = void (*)(YGNodeRef);
        using SetEdge = void (*)(YGNodeRef, YGEdge, float);
        using SetEdgeAuto = void (*)(YGNodeRef, YGEdge);

        /**
         * A float that Yoga treats as undefined when unset (the flex shorthand needs grow/shrink undefined)
         */
        void setFloat(YGNodeRef node, const Style &style, Property property, float value, SetFloat set) {
            set(node, style.has(property) ? value : YGUndefined);
        }

        /**
         * A dimension: unset is `autoValue` when there is one (width, height, basis), undefined otherwise
         */
        void setLength(YGNodeRef node, const Style &style, Property property, Length value, SetFloat point, SetFloat percent, SetAuto autoValue) {
            if (!style.has(property) || value.isAuto()) {
                autoValue ? autoValue(node) : point(node, YGUndefined);
            } else if (value.isPercent()) {
                percent(node, value.value);
            } else {
                point(node, value.value);
            }
        }

        /**
         * An edge: unset is undefined, so that Yoga falls back to the less specific edge (left, horizontal, all)
         */
        void setEdge(YGNodeRef node, const Style &style, Property property, YGEdge edge, Length value, SetEdge point, SetEdge percent, SetEdgeAuto autoValue) {
            if (!style.has(property)) {
                point(node, edge, YGUndefined);
            } else if (value.isAuto()) {
                autoValue ? autoValue(node, edge) : point(node, edge, YGUndefined);
            } else if (value.isPercent()) {
                percent(node, edge, value.value);
            } else {
                point(node, edge, value.value);
            }
        }

        void setBorder(YGNodeRef node, const Style &style, Property property, YGEdge edge, float value) {
            YGNodeStyleSetBorder(node, edge, style.has(property) ? value : YGUndefined);
        }
    }

    // endregion

    void Div::updateLayout(const PropertyMask &changed) {
        if ((changed & layoutProperties).none()) {
            return; // Paint only: Yoga's node stays clean
        }

        const Style &s = _computedStyle;
        YGNodeRef  n   = _yogaNode;
        auto       has = [&changed](Property property) { return changed.test(index(property)); };
        using P = Property;

        // Enumerated values are always pushed, unset is their default
        if (has(P::textDirection)) { YGNodeStyleSetDirection(n, toYoga(s.textDirection())); }
        if (has(P::direction)) { YGNodeStyleSetFlexDirection(n, toYoga(s.direction())); }
        if (has(P::justify)) { YGNodeStyleSetJustifyContent(n, toYoga(s.justify())); }
        if (has(P::alignContent)) { YGNodeStyleSetAlignContent(n, toYoga(s.alignContent())); }
        if (has(P::alignItems)) { YGNodeStyleSetAlignItems(n, toYoga(s.alignItems())); }
        if (has(P::alignSelf)) { YGNodeStyleSetAlignSelf(n, toYoga(s.alignSelf())); }
        if (has(P::position)) { YGNodeStyleSetPositionType(n, toYoga(s.position())); }
        if (has(P::wrap)) { YGNodeStyleSetFlexWrap(n, toYoga(s.wrap())); }
        if (has(P::overflow)) { YGNodeStyleSetOverflow(n, toYoga(s.overflow())); }
        if (has(P::display)) { YGNodeStyleSetDisplay(n, toYoga(s.display())); }
        if (has(P::boxSizing)) { YGNodeStyleSetBoxSizing(n, toYoga(s.boxSizing())); }

        if (has(P::flex)) { setFloat(n, s, P::flex, s.flex(), YGNodeStyleSetFlex); }
        if (has(P::grow)) { setFloat(n, s, P::grow, s.grow(), YGNodeStyleSetFlexGrow); }
        if (has(P::shrink)) { setFloat(n, s, P::shrink, s.shrink(), YGNodeStyleSetFlexShrink); }
        if (has(P::basis)) { setLength(n, s, P::basis, s.basis(), YGNodeStyleSetFlexBasis, YGNodeStyleSetFlexBasisPercent, YGNodeStyleSetFlexBasisAuto); }
        if (has(P::aspectRatio)) { setFloat(n, s, P::aspectRatio, s.aspectRatio(), YGNodeStyleSetAspectRatio); }

        // Insets: auto is the same as none
        if (has(P::left)) { setEdge(n, s, P::left, YGEdgeLeft, s.left(), YGNodeStyleSetPosition, YGNodeStyleSetPositionPercent, nullptr); }
        if (has(P::top)) { setEdge(n, s, P::top, YGEdgeTop, s.top(), YGNodeStyleSetPosition, YGNodeStyleSetPositionPercent, nullptr); }
        if (has(P::right)) { setEdge(n, s, P::right, YGEdgeRight, s.right(), YGNodeStyleSetPosition, YGNodeStyleSetPositionPercent, nullptr); }
        if (has(P::bottom)) { setEdge(n, s, P::bottom, YGEdgeBottom, s.bottom(), YGNodeStyleSetPosition, YGNodeStyleSetPositionPercent, nullptr); }

        if (has(P::width)) { setLength(n, s, P::width, s.width(), YGNodeStyleSetWidth, YGNodeStyleSetWidthPercent, YGNodeStyleSetWidthAuto); }
        if (has(P::height)) { setLength(n, s, P::height, s.height(), YGNodeStyleSetHeight, YGNodeStyleSetHeightPercent, YGNodeStyleSetHeightAuto); }
        if (has(P::minWidth)) { setLength(n, s, P::minWidth, s.minWidth(), YGNodeStyleSetMinWidth, YGNodeStyleSetMinWidthPercent, nullptr); }
        if (has(P::minHeight)) { setLength(n, s, P::minHeight, s.minHeight(), YGNodeStyleSetMinHeight, YGNodeStyleSetMinHeightPercent, nullptr); }
        if (has(P::maxWidth)) { setLength(n, s, P::maxWidth, s.maxWidth(), YGNodeStyleSetMaxWidth, YGNodeStyleSetMaxWidthPercent, nullptr); }
        if (has(P::maxHeight)) { setLength(n, s, P::maxHeight, s.maxHeight(), YGNodeStyleSetMaxHeight, YGNodeStyleSetMaxHeightPercent, nullptr); }

        if (has(P::margin)) { setEdge(n, s, P::margin, YGEdgeAll, s.margin(), YGNodeStyleSetMargin, YGNodeStyleSetMarginPercent, YGNodeStyleSetMarginAuto); }
        if (has(P::marginHorizontal)) { setEdge(n, s, P::marginHorizontal, YGEdgeHorizontal, s.marginHorizontal(), YGNodeStyleSetMargin, YGNodeStyleSetMarginPercent, YGNodeStyleSetMarginAuto); }
        if (has(P::marginVertical)) { setEdge(n, s, P::marginVertical, YGEdgeVertical, s.marginVertical(), YGNodeStyleSetMargin, YGNodeStyleSetMarginPercent, YGNodeStyleSetMarginAuto); }
        if (has(P::marginLeft)) { setEdge(n, s, P::marginLeft, YGEdgeLeft, s.marginLeft(), YGNodeStyleSetMargin, YGNodeStyleSetMarginPercent, YGNodeStyleSetMarginAuto); }
        if (has(P::marginTop)) { setEdge(n, s, P::marginTop, YGEdgeTop, s.marginTop(), YGNodeStyleSetMargin, YGNodeStyleSetMarginPercent, YGNodeStyleSetMarginAuto); }
        if (has(P::marginRight)) { setEdge(n, s, P::marginRight, YGEdgeRight, s.marginRight(), YGNodeStyleSetMargin, YGNodeStyleSetMarginPercent, YGNodeStyleSetMarginAuto); }
        if (has(P::marginBottom)) { setEdge(n, s, P::marginBottom, YGEdgeBottom, s.marginBottom(), YGNodeStyleSetMargin, YGNodeStyleSetMarginPercent, YGNodeStyleSetMarginAuto); }

        // A component forwards its padding and border to its skin
        if (!_ignoreInternalLayoutContraints) {
            if (has(P::padding)) { setEdge(n, s, P::padding, YGEdgeAll, s.padding(), YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent, nullptr); }
            if (has(P::paddingHorizontal)) { setEdge(n, s, P::paddingHorizontal, YGEdgeHorizontal, s.paddingHorizontal(), YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent, nullptr); }
            if (has(P::paddingVertical)) { setEdge(n, s, P::paddingVertical, YGEdgeVertical, s.paddingVertical(), YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent, nullptr); }
            if (has(P::paddingLeft)) { setEdge(n, s, P::paddingLeft, YGEdgeLeft, s.paddingLeft(), YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent, nullptr); }
            if (has(P::paddingTop)) { setEdge(n, s, P::paddingTop, YGEdgeTop, s.paddingTop(), YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent, nullptr); }
            if (has(P::paddingRight)) { setEdge(n, s, P::paddingRight, YGEdgeRight, s.paddingRight(), YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent, nullptr); }
            if (has(P::paddingBottom)) { setEdge(n, s, P::paddingBottom, YGEdgeBottom, s.paddingBottom(), YGNodeStyleSetPadding, YGNodeStyleSetPaddingPercent, nullptr); }

            if (has(P::border)) { setBorder(n, s, P::border, YGEdgeAll, s.border()); }
            if (has(P::borderHorizontal)) { setBorder(n, s, P::borderHorizontal, YGEdgeHorizontal, s.borderHorizontal()); }
            if (has(P::borderVertical)) { setBorder(n, s, P::borderVertical, YGEdgeVertical, s.borderVertical()); }
            if (has(P::borderLeft)) { setBorder(n, s, P::borderLeft, YGEdgeLeft, s.borderLeft()); }
            if (has(P::borderTop)) { setBorder(n, s, P::borderTop, YGEdgeTop, s.borderTop()); }
            if (has(P::borderRight)) { setBorder(n, s, P::borderRight, YGEdgeRight, s.borderRight()); }
            if (has(P::borderBottom)) { setBorder(n, s, P::borderBottom, YGEdgeBottom, s.borderBottom()); }
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
        bool clip = _computedStyle.overflow() != Overflow::Visible;
        if (!clip) {
            return;
        }

        bool aa = _computedStyle.antiAlias();
        if (_drawRoundRect && _drawBorder) {
            SkRRect roundRect;
            // TODO: Offset for unequal borders
            float   hb = _computedStyle.border() / 2;
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
            paint.setColor(_computedStyle.backgroundColor().argb());
            paint.setAntiAlias(_computedStyle.antiAlias());
            if (_computedStyle.opacity() < 1.0f) {
                // Fades the color, keeping its own transparency
                paint.setAlpha((unsigned int) (paint.getAlpha() * _computedStyle.opacity()));
            }

            float hb = _computedStyle.border() / 2;
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
                    paint.setColor(_computedStyle.borderColor().argb());
                    paint.setStrokeWidth(_computedStyle.border());
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
                    paint.setColor(_computedStyle.borderColor().argb());
                    paint.setStrokeWidth(_computedStyle.border());
                    canvas->drawRoundRect(inset, _radiusTopLeft, _radiusTopLeft, paint);
                }

            } else {
                if (_drawBackground) {
                    canvas->drawRect(_rect, paint);
                }
                if (_drawBorder && !_drawComplexBorders) {
                    SkRect inset = _rect.makeInset(hb, hb);
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setColor(_computedStyle.borderColor().argb());
                    paint.setStrokeWidth(_computedStyle.border());
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
                if (_computedStyle.has(Property::borderLeftColor)) {
                    paint.setColor(_computedStyle.borderLeftColor().argb());
                } else if (_computedStyle.has(Property::borderHorizontalColor)) {
                    paint.setColor(_computedStyle.borderHorizontalColor().argb());
                } else if (_computedStyle.has(Property::borderColor)) {
                    paint.setColor(_computedStyle.borderColor().argb());
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
                if (_computedStyle.has(Property::borderRightColor)) {
                    paint.setColor(_computedStyle.borderRightColor().argb());
                } else if (_computedStyle.has(Property::borderHorizontalColor)) {
                    paint.setColor(_computedStyle.borderHorizontalColor().argb());
                } else if (_computedStyle.has(Property::borderColor)) {
                    paint.setColor(_computedStyle.borderColor().argb());
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
                if (_computedStyle.has(Property::borderTopColor)) {
                    paint.setColor(_computedStyle.borderTopColor().argb());
                } else if (_computedStyle.has(Property::borderVerticalColor)) {
                    paint.setColor(_computedStyle.borderVerticalColor().argb());
                } else if (_computedStyle.has(Property::borderColor)) {
                    paint.setColor(_computedStyle.borderColor().argb());
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
                if (_computedStyle.has(Property::borderBottomColor)) {
                    paint.setColor(_computedStyle.borderBottomColor().argb());
                } else if (_computedStyle.has(Property::borderVerticalColor)) {
                    paint.setColor(_computedStyle.borderVerticalColor().argb());
                } else if (_computedStyle.has(Property::borderColor)) {
                    paint.setColor(_computedStyle.borderColor().argb());
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
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
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
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
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
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
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
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
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
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
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
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
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
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
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
                window()->setCursor(_computedStyle.cursor());
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
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
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

        if (ret != Handled && _computedStyle.overflow() == Overflow::Scroll) {
            // TODO: Cancel if already handled, we can't scroll two divs at the same time
            scroll(scrollX, scrollY);
        }

        return ret == Out ? Over : ret;
    }

    // endregion

    // endregion

}
