#include <array>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/effects/SkDashPathEffect.h"
#include "yoga/YGConfig.h"
#include "yoga/YGEnums.h"
#include "yoga/YGNode.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"
#include "yoga/YGValue.h"
#include "Div.hpp"
#include "Window.hpp"
#include "components/Menu.hpp"
#include "animation/StyleTransitions.hpp"
#include "Measure.hpp"
#include "psychic-ui.hpp"
#include "signals/Observer.hpp"
#include "style/Color.hpp"
#include "style/Length.hpp"
#include "style/Properties.hpp"
#include "style/Style.hpp"
#include "style/StyleManager.hpp"
#include "style/Values.hpp"


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

            YogaConfig(const YogaConfig &) = delete;
            YogaConfig &operator=(const YogaConfig &) = delete;
            YogaConfig(YogaConfig &&) = delete;
            YogaConfig &operator=(YogaConfig &&) = delete;
        };

        YGConfigRef yogaConfig() {
            static const YogaConfig yogaConfig;
            return yogaConfig.config;
        }

        // Class names are matched in lower case
        std::string lowercase(std::string string) {
            std::ranges::transform(string, string.begin(), ::tolower);
            return string;
        }

        // A side's border color: its own, else its axis', else the border's; none when nothing sets one
        enum class BorderSide : std::uint8_t { Left, Top, Right, Bottom };

        std::optional<Color> sideBorderColor(const Style &style, const BorderSide side) {
            using P         = Property;
            const auto pick = [&style](P own, Color ownColor, P axis, Color axisColor) -> std::optional<Color> {
                if (style.has(own)) {
                    return ownColor;
                }
                if (style.has(axis)) {
                    return axisColor;
                }
                if (style.has(P::borderColor)) {
                    return style.borderColor();
                }
                return std::nullopt;
            };
            switch (side) {
                case BorderSide::Left:
                    return pick(P::borderLeftColor, style.borderLeftColor(), P::borderHorizontalColor, style.borderHorizontalColor());
                case BorderSide::Right:
                    return pick(P::borderRightColor, style.borderRightColor(), P::borderHorizontalColor, style.borderHorizontalColor());
                case BorderSide::Top:
                    return pick(P::borderTopColor, style.borderTopColor(), P::borderVerticalColor, style.borderVerticalColor());
                case BorderSide::Bottom:
                    return pick(P::borderBottomColor, style.borderBottomColor(), P::borderVerticalColor, style.borderVerticalColor());
            }
            return std::nullopt;
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
        tag("div");

        YGNodeSetContext(_yogaNode, this);
    }

    Div::~Div() {
        if (_transitions) {
            _transitions->owner = nullptr; // The window may still hold it for a moment
        }
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
        for (const auto &className: _classNames) {
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

    const Window *Div::window() const {
        return _parent ? std::as_const(*_parent).window() : nullptr;
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
                _parent->childRemoved(*this);
                if (_focused) {
                    window()->requestFocus(_parent);
                }
            }
            _parent = parent;
            // Temporary, will be updated once we're on the render list
            _depth  = _parent ? _parent->depth() + 1 : 0;
            if (_parent) {
                added();
                _parent->childAdded(*this);
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

    void Div::childAdded(Div &/*child*/) {}

    void Div::childRemoved(Div &/*child*/) {}

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
        // Transitions belong to the window: whatever the div's next window, its first style there doesn't blend
        cancelTransitions();
        _styledIn = nullptr;
        // So do its starting class (its next first style takes it again) and its exit (`done` won't run)
        if (_startingApplied) {
            _startingApplied = false;
            _classNames.erase(lowercase(_startingClass));
        }
        if (_exit) {
            const auto exit = std::move(_exit);
            _mouseEnabled  = exit->mouseEnabled;
            _mouseChildren = exit->mouseChildren;
            _classNames.erase(exit->className);
        }
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

    const std::vector<std::shared_ptr<Div>> &Div::children() const {
        return _children;
    }

    void Div::insertChild(unsigned int index, std::shared_ptr<Div> child) {
        if (!child) {
            throw std::invalid_argument("Div: adding no child");
        }
        Div *const former = child->_parent;
        if (index > childCount() - (former == this ? 1 : 0)) {
            throw std::out_of_range("Div: adding a child past the end of the children");
        }
        if (former) {
            former->remove(*child); // Moved: it leaves its parent first (we hold it meanwhile)
        }
        // The current first/last children may lose their :first-child/:last-child state
        invalidateEndChildrenStyle();
        // Insert before setting the parent so that the child's first style pass sees its index
        Div *added = child.get();
        _children.insert(_children.cbegin() + index, std::move(child));
        YGNodeInsertChild(_yogaNode, added->_yogaNode, index);
        added->setParent(this);
    }

    void Div::remove(const std::shared_ptr<Div> &child) {
        if (child) {
            remove(*child);
        }
    }

    void Div::remove(const Div &child) {
        const int index = childIndex(&child);
        if (index >= 0) {
            remove((unsigned int) index);
        }
    }

    void Div::remove(unsigned int index) {
        const std::shared_ptr<Div> child = _children.at(index);
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

    int Div::childIndex(const std::shared_ptr<Div> &child) const {
        return childIndex(child.get());
    }

    int Div::childIndex(const Div *child) const {
        auto it = std::ranges::find_if(
            _children,
            [child](const auto &candidate) {
                return candidate.get() == child;
            }
        );
        return it == _children.cend() ? -1 : (int) std::distance(_children.cbegin(), it);
    }

    const Div *Div::at(unsigned int index) const {
        return _children.at(index).get();
    }

    Div *Div::at(unsigned int index) {
        return _children.at(index).get();
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

    bool Div::visible() const {
        return _visible;
    }

    bool Div::enabled() const {
        return _enabled;
    }

    void Div::updateEnabled(bool enabled) {
        if (_enabled != enabled) {
            _enabled = enabled;
            invalidateStyle();
            enabledChanged();
        }
    }

    bool Div::pressed() const {
        return _enabled && _mouseDown;
    }

    bool Div::active() const {
        return pressed();
    }

    // endregion

    // region Focus

    bool Div::focusEnabled() const {
        return _focusEnabled;
    }

    bool Div::hasFocus() const {
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

    bool Div::contains(const float x, const float y) const {
        return SkRect::Make(SkIRect::MakeXYWH(_x, _y, _width, _height)).contains(x, y);
    }

    bool Div::boundsContains(const float x, const float y) const {
        if (_computedStyle.overflow() == Overflow::Visible) {
            return SkRect::Make(SkIRect::MakeLTRB(_boundsLeft, _boundsTop, _boundsRight, _boundsBottom)).contains(x, y);
        }
        return contains(x, y);
    }

    // endregion

    // region Position & Dimensions

    void Div::localToGlobal(int &globalX, int &globalY, const int x, const int y) const {
        int lgx = _x + x;
        int lgy = _y + y;
        if (_parent) {
            _parent->localToGlobal(lgx, lgy, lgx, lgy);
        }
        globalX = lgx + _scrollX;
        globalY = lgy + _scrollY;
    }

    void Div::globalToLocal(int &localX, int &localY, const int x, const int y) const {
        int globalX = 0;
        int globalY = 0;
        localToGlobal(globalX, globalY);
        localX = x - globalX;
        localY = y - globalY;
    }

    int Div::x() const {
        return _x;
    }

    int Div::y() const {
        return _y;
    }

    int Div::getWidth() const {
        return _width;
    }

    int Div::getHeight() const {
        return _height;
    }

    // endregion

    // region Style

    // region Properties

    StyleManager *Div::styleManager() const {
        if (_styleManager) {
            return _styleManager.get();
        }
        if (_parent) {
            return _parent->styleManager();
        }
        return nullptr;
    }

    void Div::setStyleManager(std::shared_ptr<StyleManager> styleManager) {
        _styleManager = std::move(styleManager);
        invalidateStyle();
    }

    const Style &Div::style() const {
        return _inlineStyle;
    }

    const Style &Div::computedStyle() const {
        return _computedStyle;
    }

    void Div::addTag(std::string tag) {
        std::ranges::transform(tag, tag.begin(), ::tolower);
        _tags.push_back(tag);
        invalidateStyle();
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

    void Div::updateId(std::string id) {
        std::ranges::transform(id, id.begin(), ::tolower);
        if (id != _id) {
            _id = std::move(id);
            invalidateStyle();
        }
    }

    const std::unordered_set<std::string> &Div::classNames() const {
        return _classNames;
    }

    void Div::updateClassNames(const std::unordered_set<std::string> &classNames) {
        std::unordered_set<std::string> lcClassNames;
        for (std::string className: classNames) {
            std::ranges::transform(className, className.begin(), ::tolower);
            lcClassNames.insert(std::move(className));
        }
        if (lcClassNames != _classNames) {
            _classNames = std::move(lcClassNames);
            invalidateStyle();
        }
    }

    void Div::updateClassName(std::string className, bool present) {
        std::ranges::transform(className, className.begin(), ::tolower);
        const bool changed = present ? _classNames.insert(std::move(className)).second : _classNames.erase(className) > 0;
        if (changed) {
            invalidateStyle();
        }
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
        if (auto *manager = styleManager()) {
            // Its first style in a window: with its starting class, until the window lets it go
            Window *w = window();
            if (w && !_startingClass.empty() && !_startingApplied && _styledIn != w && !w->reducedMotion()) {
                if (_classNames.insert(lowercase(_startingClass)).second) {
                    _startingApplied = true;
                    w->_startingDivs.push_back(weak_from_this());
                }
            }
            Style computed = manager->computeStyle(this);
            // The transitions' current values replace what they blend to
            updateTransitions(computed);
            _styledIn         = window();
            _styledGeneration = manager->rulesGeneration();
            // Only what changed reaches Yoga, everything the first time
            const PropertyMask changed = _layoutInitialized ? _computedStyle.diff(computed) : PropertyMask{}.set();
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

        // A Gaussian reaches about 3 sigmas, 1.5 blur radii
        const Style &paint = _computedStyle;
        const auto shadowReach = [](Color color, float blur, float x, float y, float spread) {
            return color.alpha() > 0 ? std::max(0.0f, (blur * 1.5f) + std::max(std::abs(x), std::abs(y)) + spread) : 0.0f;
        };
        _paintOutset = std::max({
            paint.outlineWidth() > 0.0f ? std::max(0.0f, paint.outlineOffset() + paint.outlineWidth()) : 0.0f,
            shadowReach(paint.shadowColor(), paint.shadowBlur(), paint.shadowOffsetX(), paint.shadowOffsetY(), paint.shadowSpread()),
            shadowReach(paint.shadow2Color(), paint.shadow2Blur(), paint.shadow2OffsetX(), paint.shadow2OffsetY(), paint.shadow2Spread())
        });

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

        _drawComplexRoundRect = _radiusTopLeft != _radiusTopRight
                                || _radiusTopRight != _radiusBottomLeft
                                || _radiusBottomLeft != _radiusBottomRight;

        // The layout may not change, but the rounded rect must follow the radii
        _roundRect.setRectRadii(_rect, _radii.data());
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

    namespace {
        Constraint toConstraint(float size, YGMeasureMode mode) {
            switch (mode) {
                case YGMeasureModeExactly:
                    return {.size = size, .fit = Fit::Exact};
                case YGMeasureModeAtMost:
                    return {.size = size, .fit = Fit::AtMost};
                default:
                    return {.size = size, .fit = Fit::Unbounded};
            }
        }
    }

    void Div::setMeasurable() {
        YGNodeSetMeasureFunc(
            _yogaNode,
            [](YGNodeConstRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
                auto *div = static_cast<Div *>(YGNodeGetContext(node));
                if (!div) {
                    std::cerr << "Could not find div to measure\n";
                    return YGSize{};
                }
                const Size size = div->measure(toConstraint(width, widthMode), toConstraint(height, heightMode));
                return YGSize{size.width, size.height};
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

        /**
         * A gap: unset is undefined, so that the row and column gaps fall back to `gap`
         */
        void setGap(YGNodeRef node, const Style &style, Property property, YGGutter gutter, Length value) {
            if (!style.has(property) || value.isAuto()) {
                YGNodeStyleSetGap(node, gutter, YGUndefined);
            } else if (value.isPercent()) {
                YGNodeStyleSetGapPercent(node, gutter, value.value);
            } else {
                YGNodeStyleSetGap(node, gutter, value.value);
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
        // A component lays out its skin alone, filling it: how content is laid out goes to the skin, which inherits it
        const bool content = !_ignoreInternalLayoutContraints;
        if (content && has(P::direction)) { YGNodeStyleSetFlexDirection(n, toYoga(s.direction())); }
        if (content && has(P::justify)) { YGNodeStyleSetJustifyContent(n, toYoga(s.justify())); }
        if (content && has(P::alignContent)) { YGNodeStyleSetAlignContent(n, toYoga(s.alignContent())); }
        if (content && has(P::alignItems)) { YGNodeStyleSetAlignItems(n, toYoga(s.alignItems())); }
        if (has(P::alignSelf)) { YGNodeStyleSetAlignSelf(n, toYoga(s.alignSelf())); }
        if (has(P::position)) { YGNodeStyleSetPositionType(n, toYoga(s.position())); }
        if (content && has(P::wrap)) { YGNodeStyleSetFlexWrap(n, toYoga(s.wrap())); }
        if (has(P::overflow)) { YGNodeStyleSetOverflow(n, toYoga(s.overflow())); }
        if (has(P::display)) { YGNodeStyleSetDisplay(n, toYoga(s.display())); }
        if (has(P::boxSizing)) { YGNodeStyleSetBoxSizing(n, toYoga(s.boxSizing())); }

        if (has(P::flex)) { setFloat(n, s, P::flex, s.flex(), YGNodeStyleSetFlex); }
        // Never negative, which a spring overshooting 0 would make them
        if (has(P::grow)) { setFloat(n, s, P::grow, std::max(0.0f, s.grow()), YGNodeStyleSetFlexGrow); }
        if (has(P::shrink)) { setFloat(n, s, P::shrink, std::max(0.0f, s.shrink()), YGNodeStyleSetFlexShrink); }
        if (has(P::basis)) { setLength(n, s, P::basis, s.basis(), YGNodeStyleSetFlexBasis, YGNodeStyleSetFlexBasisPercent, YGNodeStyleSetFlexBasisAuto); }
        if (has(P::aspectRatio)) { setFloat(n, s, P::aspectRatio, s.aspectRatio(), YGNodeStyleSetAspectRatio); }

        if (content && has(P::gap)) { setGap(n, s, P::gap, YGGutterAll, s.gap()); }
        if (content && has(P::rowGap)) { setGap(n, s, P::rowGap, YGGutterRow, s.rowGap()); }
        if (content && has(P::columnGap)) { setGap(n, s, P::columnGap, YGGutterColumn, s.columnGap()); }

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

        const int previousWidth  = _width;
        const int previousHeight = _height;

        _width  = (int) std::ceil(YGNodeLayoutGetWidth(_yogaNode));
        _height = (int) std::ceil(YGNodeLayoutGetHeight(_yogaNode));

        _rect = SkRect::Make(SkIRect::MakeXYWH(_x, _y, _width, _height));
        _roundRect.setRectRadii(_rect, _radii.data());

        _marginLeft   = YGNodeLayoutGetMargin(_yogaNode, YGEdgeLeft);
        _marginTop    = YGNodeLayoutGetMargin(_yogaNode, YGEdgeTop);
        _marginRight  = YGNodeLayoutGetMargin(_yogaNode, YGEdgeRight);
        _marginBottom = YGNodeLayoutGetMargin(_yogaNode, YGEdgeBottom);

        _marginRect.setLTRB(
            _rect.left() - _marginLeft,
            _rect.top() - _marginTop,
            _rect.right() + _marginRight,
            _rect.bottom() + _marginBottom
        );

        _borderLeft   = YGNodeLayoutGetBorder(_yogaNode, YGEdgeLeft);
        _borderTop    = YGNodeLayoutGetBorder(_yogaNode, YGEdgeTop);
        _borderRight  = YGNodeLayoutGetBorder(_yogaNode, YGEdgeRight);
        _borderBottom = YGNodeLayoutGetBorder(_yogaNode, YGEdgeBottom);

        _paddedRect.setLTRB(
            _rect.left() + YGNodeLayoutGetPadding(_yogaNode, YGEdgeLeft) + _borderLeft,
            _rect.top() + YGNodeLayoutGetPadding(_yogaNode, YGEdgeTop) + _borderTop,
            _rect.right() - (YGNodeLayoutGetPadding(_yogaNode, YGEdgeRight) + _borderRight),
            _rect.bottom() - (YGNodeLayoutGetPadding(_yogaNode, YGEdgeBottom) + _borderBottom)
        );

        _drawBorder = _borderLeft != 0
                      || _borderRight != 0
                      || _borderTop != 0
                      || _borderBottom != 0;

        _drawComplexBorders = _borderLeft != _borderRight
                              || _borderRight != _borderTop
                              || _borderTop != _borderBottom;


        // Children should also update
        _boundsLeft   = _x;
        _boundsTop    = _y;
        _boundsRight  = _x + _width;
        _boundsBottom = _y + _height;
        // A scrolling div's content ends with its end padding, as CSS's scrollable overflow area does, so that the last
        // child doesn't touch its edge once scrolled to the end
        const bool scrolls     = _computedStyle.overflow() == Overflow::Scroll;
        const int  endPaddingX = scrolls ? static_cast<int>(YGNodeLayoutGetPadding(_yogaNode, YGEdgeRight)) : 0;
        const int  endPaddingY = scrolls ? static_cast<int>(YGNodeLayoutGetPadding(_yogaNode, YGEdgeBottom)) : 0;
        for (auto &child: _children) {
            child->layoutUpdated();

            _boundsLeft   = std::min(_boundsLeft, _x + child->boundsLeft());
            _boundsTop    = std::min(_boundsTop, _y + child->boundsTop());
            _boundsRight  = std::max(_boundsRight, _x + child->boundsRight() + endPaddingX);
            _boundsBottom = std::max(_boundsBottom, _y + child->boundsBottom() + endPaddingY);
        }
        const SkRect previousBoundsRect = _boundsRect;
        _boundsRect = SkRect::Make(SkIRect::MakeLTRB(_boundsLeft, _boundsTop, _boundsRight, _boundsBottom));

        layoutReady = true;

        if (previousWidth != _width || previousHeight != _height || previousBoundsRect != _boundsRect) {
            resized.emit(_width, _height);
        }
    }

    // endregion

    // region Draw

    Size Div::measure(Constraint width, Constraint height) {
        return Size{.width = width.size, .height = height.size};
    }

    void Div::render(SkCanvas *canvas) {
        // Paint only: styles and layout were updated by the window before rendering
        // Skia rejects empty rects, but an empty div still renders (a data container builds its
        // children when it renders): outset, only what's off-screen is rejected, as with Skia m73.
        const Style &computed = _computedStyle;
        const float opacity = computed.opacity();
        const float scaleX  = computed.scale() * computed.scaleX();
        const float scaleY  = computed.scale() * computed.scaleY();
        if (!layoutReady || !_visible || opacity <= 0.0f || scaleX == 0.0f || scaleY == 0.0f) {
            return;
        }

        // The paint transform, around everything below (CSS's translate, then scale about the origin): painting only,
        // the layout and hit testing don't see it
        // A layout transition's offset (`layoutTransition`) adds to its translation
        const float translateX = computed.translateX().resolve(_rect.width()) + layoutOffsetX();
        const float translateY = computed.translateY().resolve(_rect.height()) + layoutOffsetY();
        const bool  transformed = translateX != 0.0f || translateY != 0.0f || scaleX != 1.0f || scaleY != 1.0f;
        const SkAutoCanvasRestore transform(canvas, transformed);
        if (transformed) {
            const float originX = _rect.left() + computed.transformOriginX().resolve(_rect.width());
            const float originY = _rect.top() + computed.transformOriginY().resolve(_rect.height());
            canvas->translate(translateX + originX, translateY + originY);
            canvas->scale(scaleX, scaleY);
            canvas->translate(-originX, -originY);
        }

        const float outset = 1.0f + _paintOutset;
        if (canvas->quickReject(_rect.makeOutset(outset, outset))) {
            return;
        }

        if (opacity < 1.0f) {
            // CSS opacity: the div and its children are drawn into a layer, which is then blended as one group, so
            // overlapping parts don't show through each other and nested opacities multiply
            const SkRect layer = _boundsRect.makeOutset(_paintOutset, _paintOutset);
            canvas->saveLayerAlpha(&layer, static_cast<unsigned>(std::lround(opacity * 255.0f)));
        } else {
            canvas->save();
        }

        drawShadow(canvas);

        draw(canvas);

        // The children are clipped, the outline after them isn't
        canvas->save();

        clip(canvas);

        canvas->translate(static_cast<float>(_scrollX), static_cast<float>(_scrollY));

        // region Debug
        #ifdef DEBUG_LAYOUT
        if (debugLayout) {
            SkPaint paint;
            paint.setBlendMode(SkBlendMode::kPlus);
            paint.setStyle(SkPaint::kStroke_Style);
            if (dashed) {
                constexpr std::array intervals{1.0f, 1.0f};
                paint.setPathEffect(SkDashPathEffect::Make(intervals, 0));
            }
            paint.setStrokeWidth(SkIntToScalar(1));

            // Each box in its color, on the pixels' centers
            constexpr float   halfPixel      = 0.5f;
            constexpr SkColor marginColor    = 0x7FFF0000;
            constexpr SkColor borderBoxColor = 0x7F00FF00;
            constexpr SkColor paddingColor   = 0x7F0000FF;
            constexpr SkColor boundsColor    = 0x30FFFF00;
            constexpr SkColor borderColor    = 0x7F00FFFF;
            constexpr SkColor pressedColor   = 0x30FFFFFF;
            constexpr SkColor hoveredColor   = 0x10FFFFFF;

            paint.setColor(marginColor);
            const SkRect mRect = _marginRect.makeInset(halfPixel, halfPixel);
            canvas->drawRect(mRect, paint);

            paint.setColor(borderBoxColor);
            const SkRect dRect = _rect.makeInset(halfPixel, halfPixel);
            canvas->drawRect(dRect, paint);

            paint.setColor(paddingColor);
            const SkRect pRect = _paddedRect.makeInset(halfPixel, halfPixel);
            canvas->drawRect(pRect, paint);

            paint.setColor(boundsColor);
            SkRect bRect = SkRect::MakeLTRB((float) _boundsLeft, (float) _boundsTop, (float) _boundsRight, (float) _boundsBottom);
            bRect.inset(halfPixel, halfPixel);
            canvas->drawRect(bRect, paint);

            if (_borderLeft > 0 || _borderTop > 0 || _borderRight > 0 || _borderBottom > 0) {
                paint.setColor(borderColor);
                canvas->drawRect(
                    SkRect::MakeLTRB(
                        _rect.left() + _borderLeft + halfPixel,
                        _rect.top() + _borderTop + halfPixel,
                        _rect.right() - _borderRight - halfPixel,
                        _rect.bottom() - _borderBottom - halfPixel
                    ), paint
                );
            }

            if (_mouseDown) {
                paint.setColor(pressedColor);
                paint.setStrokeWidth(2);
                canvas->drawRect(_rect.makeInset(1.0f, 1.0f), paint);
            } else if (_mouseOver) {
                paint.setColor(hoveredColor);
                paint.setStrokeWidth(2);
                canvas->drawRect(_rect.makeInset(1.0f, 1.0f), paint);
            }
        }
        #endif
        // endregion

        canvas->translate(static_cast<float>(_x), static_cast<float>(_y));

        // Back-to-front: children are in document order, the last one is on top, except that those sliding to their
        // layout places go over those that don't (rows crossing as they are sorted, or passing over children fading in)
        bool sliding = false;
        for (auto &child: _children) {
            if (child->layoutSliding()) {
                sliding = true;
            } else {
                child->render(canvas);
            }
        }
        if (sliding) {
            for (auto &child: _children) {
                if (child->layoutSliding()) {
                    child->render(canvas);
                }
            }
        }

        canvas->restore();

        drawOutline(canvas);

        canvas->restore();
    }

    void Div::drawShadow(SkCanvas *canvas) {
        const Style &style = _computedStyle;
        if (style.shadowColor().alpha() == 0 && style.shadow2Color().alpha() == 0) {
            return;
        }
        const auto layer = [&](Color color, float blur, float x, float y, float spread) {
            if (color.alpha() == 0) {
                return;
            }
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(color.argb());
            if (blur > 0.0f) {
                paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, blur / 2));
            }
            SkRRect shape = _roundRect.makeOffset(x, y);
            if (spread != 0.0f) {
                shape.outset(spread, spread); // Insets when negative; the radii follow, as in CSS
            }
            canvas->drawRRect(shape, paint);
        };
        // Only outside the box, as in CSS: a translucent background doesn't show it
        canvas->save();
        canvas->clipRRect(_roundRect, SkClipOp::kDifference, true);
        // The second layer under the first
        layer(style.shadow2Color(), style.shadow2Blur(), style.shadow2OffsetX(), style.shadow2OffsetY(), style.shadow2Spread());
        layer(style.shadowColor(), style.shadowBlur(), style.shadowOffsetX(), style.shadowOffsetY(), style.shadowSpread());
        canvas->restore();
    }

    void Div::drawDotGrid(SkCanvas *canvas) {
        const Style &style   = _computedStyle;
        const float  spacing = style.dotGridSpacing();
        const float  radius  = style.dotGridRadius();
        if (style.dotGridColor().alpha() == 0 || spacing < 1.0f || radius <= 0.0f) {
            return;
        }
        // Only the dots that can show: the box within the clip, on the grid anchored at the box's corner
        SkRect visible = _rect;
        if (!visible.intersect(canvas->getLocalClipBounds())) {
            return;
        }
        const float half   = spacing / 2.0f;
        const float firstX = _rect.left() + half + (std::floor((visible.left() - _rect.left()) / spacing) * spacing);
        const float firstY = _rect.top() + half + (std::floor((visible.top() - _rect.top()) / spacing) * spacing);
        const int columns = std::max(0, static_cast<int>(std::ceil((visible.right() + radius - firstX) / spacing)));
        const int rows    = std::max(0, static_cast<int>(std::ceil((visible.bottom() + radius - firstY) / spacing)));
        std::vector<SkPoint> dots{};
        dots.reserve(static_cast<size_t>(columns) * rows);
        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                dots.push_back({firstX + (static_cast<float>(column) * spacing), firstY + (static_cast<float>(row) * spacing)});
            }
        }
        // Round points are discs of the stroke's width, in one call
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(style.dotGridColor().argb());
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setStrokeWidth(radius * 2);
        canvas->save();
        canvas->clipRRect(_roundRect, true);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, dots, paint);
        canvas->restore();
    }

    void Div::drawOutline(SkCanvas *canvas) {
        const Style &style = _computedStyle;
        const float width  = style.outlineWidth();
        if (width <= 0.0f || style.outlineColor().alpha() == 0) {
            return;
        }
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(width);
        paint.setColor(style.outlineColor().argb());
        // The stroke's center line, half its width outside the offset; corners stay concentric with the border's
        SkRRect line;
        const float outset = style.outlineOffset() + (width / 2.0f);
        _roundRect.outset(outset, outset, &line);
        canvas->drawRRect(line, paint);
    }

    void Div::clip(SkCanvas *canvas) {
        const bool clip = _computedStyle.overflow() != Overflow::Visible;
        if (!clip) {
            return;
        }

        const bool antiAlias = _computedStyle.antiAlias();
        if (_drawRoundRect && _drawBorder) {
            SkRRect roundRect;
            // TODO: Offset for unequal borders
            const float halfBorder = (_drawComplexBorders ? _computedStyle.border() : _borderLeft) / 2;
            _roundRect.inset(halfBorder, halfBorder, &roundRect);
            canvas->clipRRect(roundRect, antiAlias);
        } else if (_drawRoundRect) {
            canvas->clipRRect(_roundRect, antiAlias);
        } else {
            canvas->clipRect(_rect, antiAlias);
        }
    }

    void Div::draw(SkCanvas *canvas) {
        // region Background & Borders
        if (_drawBackground || _drawBorder) {
            SkPaint paint;
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(_computedStyle.backgroundColor().argb());
            paint.setAntiAlias(_computedStyle.antiAlias());

            // Equal sides: the layout's width, whether `border` or the sides set it
            const float width      = _borderLeft;
            const float halfBorder = width / 2;
            if (_drawRoundRect && _drawComplexBorders) {
                // Unequal sides: the background under the whole border box, the sides over it along the rounded outline
                if (_drawBackground) {
                    canvas->drawRRect(_roundRect, paint);
                }
                drawRoundedSides(canvas);

            } else if (_drawComplexRoundRect) {
                SkRRect roundRect = _roundRect;
                if (_drawBackground && _drawBorder) {
                    roundRect.inset(halfBorder, halfBorder);
                }

                if (_drawBackground) {
                    canvas->drawRRect(roundRect, paint);
                }

                if (_drawBorder) {
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setColor(_computedStyle.borderColor().argb());
                    paint.setStrokeWidth(width);
                    canvas->drawRRect(roundRect, paint);
                }

            } else if (_drawRoundRect) {
                if (_drawBackground && !_drawBorder) {
                    canvas->drawRoundRect(_rect, _radiusTopLeft, _radiusTopLeft, paint);
                } else if (_drawBorder) {
                    const SkRect inset = _rect.makeInset(halfBorder, halfBorder);
                    if (_drawBackground) {
                        canvas->drawRoundRect(inset, _radiusTopLeft, _radiusTopLeft, paint);
                    }
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setColor(_computedStyle.borderColor().argb());
                    paint.setStrokeWidth(width);
                    canvas->drawRoundRect(inset, _radiusTopLeft, _radiusTopLeft, paint);
                }

            } else {
                if (_drawBackground) {
                    canvas->drawRect(_rect, paint);
                }
                if (_drawBorder && !_drawComplexBorders) {
                    const SkRect inset = _rect.makeInset(halfBorder, halfBorder);
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setColor(_computedStyle.borderColor().argb());
                    paint.setStrokeWidth(width);
                    canvas->drawRect(inset, paint);
                }
            }
        }
        // endregion

        drawDotGrid(canvas);

        // In the parent's coordinates, like _rect
        if (_drawComplexBorders && !_drawRoundRect) {
            drawSideLines(canvas);
        }
    }

    void Div::drawSideLines(SkCanvas *canvas) const {
        // Each side a line, past the corners by half a pixel so that they meet without anti-aliasing
        constexpr float overhang = 0.5f;
        SkPaint         paint;
        paint.setAntiAlias(false);
        paint.setStyle(SkPaint::kStroke_Style);
        const auto line = [&](float sideWidth, BorderSide side, SkPoint start, SkPoint finish) {
            const std::optional<Color> color = sideBorderColor(_computedStyle, side);
            if (sideWidth <= 0 || !color) {
                return;
            }
            paint.setStrokeWidth(sideWidth);
            paint.setColor(color->argb());
            canvas->drawLine(start, finish, paint);
        };
        const float left   = _rect.left() + (_borderLeft / 2.0f);
        const float right  = _rect.right() - (_borderRight / 2.0f);
        const float top    = _rect.top() + (_borderTop / 2.0f);
        const float bottom = _rect.bottom() - (_borderBottom / 2.0f);
        line(_borderLeft, BorderSide::Left, SkPoint::Make(left, _rect.top() - overhang), SkPoint::Make(left, _rect.bottom() + overhang));
        line(_borderRight, BorderSide::Right, SkPoint::Make(right, _rect.top() - overhang), SkPoint::Make(right, _rect.bottom() + overhang));
        line(_borderTop, BorderSide::Top, SkPoint::Make(_rect.left() - overhang, top), SkPoint::Make(_rect.right() + overhang, top));
        line(_borderBottom, BorderSide::Bottom, SkPoint::Make(_rect.left() - overhang, bottom), SkPoint::Make(_rect.right() + overhang, bottom));
    }

    void Div::drawRoundedSides(SkCanvas *canvas) const {
        // As CSS: the border is the band between the rounded border box and the padding box, whose corners' radii are
        // the border box's less the two sides' widths (elliptical when they differ), so a side thins out along a corner
        const SkRect inner = SkRect::MakeLTRB(
            _rect.left() + _borderLeft,
            _rect.top() + _borderTop,
            _rect.right() - _borderRight,
            _rect.bottom() - _borderBottom
        );
        const auto radius = [](float outer, float horizontal, float vertical) {
            return SkVector{std::max(0.0f, outer - horizontal), std::max(0.0f, outer - vertical)};
        };
        const std::array<SkVector, 4> innerRadii{
            radius(_radiusTopLeft, _borderLeft, _borderTop),
            radius(_radiusTopRight, _borderRight, _borderTop),
            radius(_radiusBottomRight, _borderRight, _borderBottom),
            radius(_radiusBottomLeft, _borderLeft, _borderBottom)
        };
        SkRRect innerRect;
        innerRect.setRectRadii(inner, innerRadii.data());

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(_computedStyle.antiAlias());
        const auto band = [&](const Color color) {
            paint.setColor(color.argb());
            if (inner.isEmpty()) {
                canvas->drawRRect(_roundRect, paint); // The sides meet: all border
            } else {
                canvas->drawDRRect(_roundRect, innerRect, paint);
            }
        };

        // Left, top, right, bottom
        const std::array<float, 4>                widths{_borderLeft, _borderTop, _borderRight, _borderBottom};
        const std::array<std::optional<Color>, 4> colors{
            sideBorderColor(_computedStyle, BorderSide::Left),
            sideBorderColor(_computedStyle, BorderSide::Top),
            sideBorderColor(_computedStyle, BorderSide::Right),
            sideBorderColor(_computedStyle, BorderSide::Bottom)
        };

        // One color for every side that has a width: the whole band at once
        std::optional<Color> shared;
        bool                 single = true;
        for (std::size_t i = 0; i < widths.size() && single; ++i) {
            if (widths.at(i) <= 0) {
                continue;
            }
            const std::optional<Color> &color = colors.at(i);
            if (!color || (shared && !(*shared == *color))) {
                single = false;
            } else {
                shared = color;
            }
        }
        if (single) {
            if (shared) {
                band(*shared);
            }
            return;
        }

        // Each side in its color, clipped to its part of the band: two sides with widths meet on the line from the
        // outer corner to the inner one, as CSS's; beside a side without one, the corner is the side's whole
        const auto end = [](float sideWidth, float neighbourWidth, SkPoint outer, SkPoint innerCorner, SkVector along, float cornerRadius) {
            if (neighbourWidth > 0) {
                return innerCorner;
            }
            return outer + (along * std::max(cornerRadius, sideWidth)); // Along the neighbour's edge, past the curve
        };
        const SkPoint topLeft = SkPoint::Make(_rect.left(), _rect.top());
        const SkPoint topRight = SkPoint::Make(_rect.right(), _rect.top());
        const SkPoint bottomRight = SkPoint::Make(_rect.right(), _rect.bottom());
        const SkPoint bottomLeft = SkPoint::Make(_rect.left(), _rect.bottom());
        const SkPoint innerTopLeft = SkPoint::Make(inner.left(), inner.top());
        const SkPoint innerTopRight = SkPoint::Make(inner.right(), inner.top());
        const SkPoint innerBottomRight = SkPoint::Make(inner.right(), inner.bottom());
        const SkPoint innerBottomLeft = SkPoint::Make(inner.left(), inner.bottom());
        const std::array<std::array<SkPoint, 4>, 4> parts{{
            {topLeft,
             end(_borderLeft, _borderTop, topLeft, innerTopLeft, {1, 0}, _radiusTopLeft),
             end(_borderLeft, _borderBottom, bottomLeft, innerBottomLeft, {1, 0}, _radiusBottomLeft),
             bottomLeft},
            {topLeft,
             end(_borderTop, _borderLeft, topLeft, innerTopLeft, {0, 1}, _radiusTopLeft),
             end(_borderTop, _borderRight, topRight, innerTopRight, {0, 1}, _radiusTopRight),
             topRight},
            {topRight,
             end(_borderRight, _borderTop, topRight, innerTopRight, {-1, 0}, _radiusTopRight),
             end(_borderRight, _borderBottom, bottomRight, innerBottomRight, {-1, 0}, _radiusBottomRight),
             bottomRight},
            {bottomLeft,
             end(_borderBottom, _borderLeft, bottomLeft, innerBottomLeft, {0, -1}, _radiusBottomLeft),
             end(_borderBottom, _borderRight, bottomRight, innerBottomRight, {0, -1}, _radiusBottomRight),
             bottomRight}
        }};
        for (std::size_t i = 0; i < parts.size(); ++i) {
            const std::optional<Color> &color = colors.at(i);
            if (widths.at(i) <= 0 || !color) {
                continue;
            }
            canvas->save();
            canvas->clipPath(SkPath::Polygon(parts.at(i), true), _computedStyle.antiAlias());
            band(*color);
            canvas->restore();
        }
    }

    // endregion

    // region Interaction

    // region Properties

    bool Div::mouseEnabled() const {
        return _mouseEnabled;
    }

    bool Div::mouseChildren() const {
        return _mouseChildren;
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

    MouseEvent Div::localMouseEvent(const MouseEvent &event) const {
        MouseEvent local = event;
        local.x = event.x - static_cast<float>(_x + _scrollX);
        local.y = event.y - static_cast<float>(_y + _scrollY);
        return local;
    }

    void Div::updateContextMenu(std::vector<std::shared_ptr<MenuItem>> items) {
        _contextMenu = std::move(items);
        if (_contextMenu.empty()) {
            _contextMenuSlot.disconnect();
            return;
        }
        if (!_contextMenuSlot) {
            // Ours, torn down with us: the window opens the menu where it was asked for
            _contextMenuSlot = contextMenuRequested.subscribe(
                [this](const MouseEvent &event) {
                    if (Window *w = window(); w && !_contextMenu.empty()) {
                        w->openContextMenu(_contextMenu, event.windowX, event.windowY, event.fromKeyboard);
                    }
                }
            );
        }
    }

    MouseEvent Div::mouseEventAtCenter() {
        int windowX = 0;
        int windowY = 0;
        localToGlobal(windowX, windowY);
        MouseEvent event{};
        // The center of the border box, in content coordinates
        event.x       = (static_cast<float>(_width) / 2) - static_cast<float>(_scrollX);
        event.y       = (static_cast<float>(_height) / 2) - static_cast<float>(_scrollY);
        event.windowX = static_cast<float>(windowX) + event.x;
        event.windowY = static_cast<float>(windowY) + event.y;
        event.fromKeyboard = true;
        if (const Window *w = window()) {
            event.source = w->inputMode();
        }
        return event;
    }

    MouseEventStatus Div::handleMouseButtonChange(const MouseEvent &event) {
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
        if (!_visible || !boundsContains(event.x, event.y)) {
            return MouseEventStatus::Out;
        }

        const MouseEvent local = localMouseEvent(event);
        MouseEventStatus ret   = MouseEventStatus::Out;

        if (_mouseChildren) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->handleMouseButtonChange(local);
                if (res != MouseEventStatus::Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled && ret != MouseEventStatus::Handled && mouseButtonChanged.hasSubscriptions()) {
            mouseButtonChanged.emit(local);
            ret = MouseEventStatus::Handled;
        }

        return ret == MouseEventStatus::Out ? MouseEventStatus::Over : ret;
    }

    MouseEventStatus Div::handleMousePress(const MouseEvent &event) {
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
        if (!_visible || !boundsContains(event.x, event.y)) {
            return MouseEventStatus::Out;
        }

        setMouseDown(true);

        const MouseEvent local = localMouseEvent(event);
        MouseEventStatus ret   = MouseEventStatus::Out;

        if (_mouseChildren) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->handleMousePress(local);
                if (res != MouseEventStatus::Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (ret == MouseEventStatus::Out) {
            // We're the deepest we're gonna get, do a focus request
            window()->requestFocus(this);
        }

        if (_mouseEnabled && ret != MouseEventStatus::Handled && mousePressed.hasSubscriptions()) {
            mousePressed.emit(local);
            ret = MouseEventStatus::Handled;
        }

        // onClick swallows left button presses, otherwise a parent might hijack them
        return ret == MouseEventStatus::Handled || (event.button == MouseButton::LEFT && clicked.hasSubscriptions()) ? MouseEventStatus::Handled : MouseEventStatus::Over;
    }

    MouseEventStatus Div::handleMouseRelease(const MouseEvent &event) {
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
        if (!_visible) {
            return MouseEventStatus::Out;
        }

        const bool isOver  = boundsContains(event.x, event.y);
        const bool wasDown = _mouseDown;

        if (isOver || wasDown) {
            setMouseDown(false);
        }

        const MouseEvent local = localMouseEvent(event);
        MouseEventStatus ret   = MouseEventStatus::Out;

        if (_mouseChildren && (isOver || wasDown)) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->handleMouseRelease(local);
                if (res == MouseEventStatus::Over) {
                    ret = res;
                }
            }
        }

        if (!_mouseEnabled) {
            if (!isOver) {
                return MouseEventStatus::Out;
            }
            return ret == MouseEventStatus::Out ? MouseEventStatus::Over : ret;
        }

        // A press that started here ends here, wherever it is released (a drag stops even outside): mouseReleased
        // once, and mouseReleasedOutside first when released outside. A release over us without a press here is ours
        // unless a child handled it.
        const bool hadSubscriptions = mouseReleased.hasSubscriptions();
        if (wasDown && !isOver) {
            mouseReleasedOutside.emit(local);
        }
        if (wasDown || (isOver && ret != MouseEventStatus::Handled)) {
            mouseReleased.emit(local);
            if (isOver && hadSubscriptions) {
                ret = MouseEventStatus::Handled;
            }
        }

        if (!isOver) {
            return MouseEventStatus::Out;
        }

        return ret == MouseEventStatus::Out ? MouseEventStatus::Over : ret;
    }

    MouseEventStatus Div::handleClick(const MouseEvent &event) {
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
        if (!_visible || !boundsContains(event.x, event.y)) {
            return MouseEventStatus::Out;
        }

        // Only the left button clicks: a right click (or a touch long press, which is one) is for mousePressed
        if (!_mouseDown || event.button != MouseButton::LEFT) {
            return MouseEventStatus::Over;
        }

        const MouseEvent local = localMouseEvent(event);
        MouseEventStatus ret   = MouseEventStatus::Out;

        if (_mouseChildren) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->handleClick(local);
                if (res != MouseEventStatus::Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled && ret != MouseEventStatus::Handled && clicked.hasSubscriptions()) {
            clicked.emit(local);
            ret = MouseEventStatus::Handled;
        }

        return ret == MouseEventStatus::Out ? MouseEventStatus::Over : ret;
    }

    MouseEventStatus Div::handleDoubleClick(const MouseEvent &event) {
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
        if (!_visible || !boundsContains(event.x, event.y)) {
            return MouseEventStatus::Out;
        }

        if (!_mouseDown) {
            return MouseEventStatus::Over;
        }

        const MouseEvent local = localMouseEvent(event);
        MouseEventStatus ret   = MouseEventStatus::Out;

        if (_mouseChildren) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->handleDoubleClick(local);
                if (res != MouseEventStatus::Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled && ret != MouseEventStatus::Handled && doubleClicked.hasSubscriptions()) {
            doubleClicked.emit(local);
            ret = MouseEventStatus::Handled;
        }

        return ret == MouseEventStatus::Out ? MouseEventStatus::Over : ret;
    }

    MouseEventStatus Div::handleContextMenuRequest(const MouseEvent &event) {
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
        if (!_visible || !boundsContains(event.x, event.y)) {
            return MouseEventStatus::Out;
        }

        const MouseEvent local = localMouseEvent(event);
        MouseEventStatus ret   = MouseEventStatus::Out;

        if (_mouseChildren) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->handleContextMenuRequest(local);
                if (res != MouseEventStatus::Out) {
                    ret = res;
                    break;
                }
            }
        }

        // The deepest div that asks for it gets it, once
        if (_mouseEnabled && ret != MouseEventStatus::Handled && contextMenuRequested.hasSubscriptions()) {
            contextMenuRequested.emit(local);
            ret = MouseEventStatus::Handled;
        }

        return ret == MouseEventStatus::Out ? MouseEventStatus::Over : ret;
    }

    // endregion

    // region Move

    bool Div::handleMouseExit(const MouseEvent &event) {
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
        if (!_mouseOver) {
            return false;
        }

        setMouseOver(false);

        const MouseEvent local = localMouseEvent(event);
        if (_mouseChildren) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                if (child->handleMouseExit(local)) {
                    break;
                }
            }
        }

        mouseExited.emit(local);

        return true;
    }

    MouseEventStatus Div::handleMouseMove(const MouseEvent &event, bool handled) {
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
        if (!_visible) {
            return MouseEventStatus::Out;
        }

        const bool isOver  = boundsContains(event.x, event.y);
        const bool wasOver = _mouseOver;

        if (!isOver && wasOver) {
            handleMouseExit(event);
        }

        const bool allowOutsideMovement = this == window();

        if (!isOver && !allowOutsideMovement) {
            return MouseEventStatus::Out;
        }

        // From now on we're over the div, unless we"re on window, which allow for the mouse to move outside

        const MouseEvent local = localMouseEvent(event);
        MouseEventStatus ret   = MouseEventStatus::Out;

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
                    auto res = child->handleMouseMove(local, handled);
                    if (res != MouseEventStatus::Out) {
                        handled = true;
                        if (ret != MouseEventStatus::Handled) {
                            ret = res;
                        }
                    }
                }
            }
        }

        if (_mouseEnabled) {
            if (!handled && isOver && !wasOver) {
                mouseEntered.emit(local);
            }
            mouseMoved.emit(local);
            if (!handled && isOver && ret != MouseEventStatus::Handled) {
                window()->setCursor(_computedStyle.cursor());
                ret = MouseEventStatus::Handled;
            } else if (isOver) {
                ret = MouseEventStatus::Over;
            }
        }

        return ret;
    }

    // endregion

    // region Scroll

    void Div::scrollTo(int scrollX, int scrollY) {
        if (scrollX != _scrollX || scrollY != _scrollY) {
            _scrollX = scrollX;
            _scrollY = scrollY;
            if (Window *w = window()) {
                w->requestFrame(); // Only painting changes: no style or layout tells the window
            }
            scrolled.emit(_scrollX, _scrollY);
        }
    }

    void Div::scroll(const double scrollX, const double scrollY) {
        bool moved = false;

        if (_width < _boundsRight - _boundsLeft) {
            const int clampedX = std::min(
                0, std::max(
                    _width - (_boundsRight - _boundsLeft),
                    _scrollX + ((int) std::ceil(scrollX) * 2)
                ));

            moved |= _scrollX != clampedX;
            _scrollX = clampedX;
        }

        if (_height < _boundsBottom - _boundsTop) {
            const int clampedY = std::min(
                0, std::max(
                    _height - (_boundsBottom - _boundsTop),
                    _scrollY + ((int) std::ceil(scrollY) * 2)
                ));

            moved |= _scrollY != clampedY;
            _scrollY = clampedY;
        }

        if (moved) {
            scrolled.emit(_scrollX, _scrollY);
        }
    }

    void Div::scrollIntoView(const SkRect &rect) {
        Div *viewport = _parent;
        while (viewport && viewport->computedStyle().overflow() != Overflow::Scroll) {
            viewport = viewport->parent();
        }
        if (!viewport) {
            return;
        }
        // Where the rectangle is in the viewport, whose own scroll position is left out
        int dummy = 0;
        int rectY = 0;
        localToGlobal(dummy, rectY, 0, static_cast<int>(std::floor(rect.fTop)));
        int viewportY = 0;
        viewport->localToGlobal(dummy, viewportY);
        const int height = static_cast<int>(std::ceil(rect.fBottom) - std::floor(rect.fTop));
        const int top    = rectY - (viewportY - viewport->scrollY());
        const int bottom = top + height;
        if (top < 0) {
            const int fromStart = top - viewport->scrollY();
            viewport->scrollY(fromStart < height ? 0 : viewport->scrollY() - top);
        } else if (bottom > viewport->getHeight()) {
            viewport->scrollY(viewport->scrollY() - (bottom - viewport->getHeight()));
        }
    }

    bool Div::scrollBy(const int deltaX, const int deltaY) {
        int targetX = _scrollX;
        int targetY = _scrollY;
        if (_width < contentWidth()) {
            targetX = std::clamp(_scrollX + deltaX, _width - contentWidth(), 0);
        }
        if (_height < contentHeight()) {
            targetY = std::clamp(_scrollY + deltaY, _height - contentHeight(), 0);
        }
        if (targetX == _scrollX && targetY == _scrollY) {
            return false;
        }
        scrollTo(targetX, targetY);
        return true;
    }

    MouseEventStatus Div::handleMouseScroll(const MouseEvent &event) {
        const auto keepAlive = weak_from_this().lock(); // A handler may remove this div from its parent
        if (!_visible || !boundsContains(event.x, event.y)) {
            return MouseEventStatus::Out;
        }

        const MouseEvent local = localMouseEvent(event);
        MouseEventStatus ret   = MouseEventStatus::Out;

        if (_mouseChildren) {
            // Topmost first, over a copy since handlers can add or remove children
            for (auto &child: childrenTopmostFirst()) {
                if (child->_parent != this) {
                    continue; // Removed by an earlier handler
                }
                auto res = child->handleMouseScroll(local);
                if (res != MouseEventStatus::Out) {
                    ret = res;
                    break;
                }
            }
        }

        if (_mouseEnabled && ret != MouseEventStatus::Handled && mouseScrolled.hasSubscriptions()) {
            mouseScrolled.emit(local);
            ret = MouseEventStatus::Handled;
        }

        if (ret != MouseEventStatus::Handled && _computedStyle.overflow() == Overflow::Scroll) {
            // TODO: Cancel if already handled, we can't scroll two divs at the same time
            scroll(event.scrollX, event.scrollY);
        }

        return ret == MouseEventStatus::Out ? MouseEventStatus::Over : ret;
    }

    // endregion

    // endregion

}
