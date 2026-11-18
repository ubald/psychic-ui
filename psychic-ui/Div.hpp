#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <utility>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include "psychic-ui/Measure.hpp"
#include <unicode/unistr.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "psychic-ui.hpp"
#include "psychic-ui/style/Style.hpp"
#include "psychic-ui/style/StyleManager.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "psychic-ui/signals/Observer.hpp"
#include "psychic-ui/animation/Animation.hpp"

// Yoga stays out of the public headers: its node is only named here
struct YGNode;

namespace psychic_ui {

    class Window;

    namespace detail {
        class StyleTransitions;
    }

    class Panel;

    class MenuItem;

    /**
     * Return status of the mouse event methods
     */
    enum class MouseEventStatus : std::uint8_t {
        /**
         * Mouse was outside the component
         */
            Out,

        /**
         * Mouse was over the component but no event was handled
         */
            Over,

        /**
         * Mouse is over the component and an event was handled
         */
            Handled
    };

    /**
     * Shortcut type for click signal slot
     */
    using ClickCallback = std::function<void()>;

    /**
     * Shortcut type for mouse signal slot
     */
    using MouseCallback = std::function<void(const MouseEvent &event)>;

    /**
     * @class Div
     *
     * The Div class is the main object used to display content in Psychic UI.
     * It has a similar role as a `div` in html: its purpose is to offer a rectangular container
     * than can be used in a layout and styled using a stylesheet.
     */
    class Div : public Observer, public std::enable_shared_from_this<Div> {
        friend class StyleManager;

    public:
        Div();

        /**
         * Just as a safety for now, delete the copy constructor
         * because I don't see a reason why we would need it.
         */
        Div(const Div &) = delete;

        virtual ~Div();

        /**
         * Just as a safety for now, delete the copy constructor
         * because I don't see a reason why we would need it.
         */
        Div &operator=(const Div &) = delete;

        Div(Div &&) = delete;
        Div &operator=(Div &&) = delete;

        /**
         * Get a string representing this Div for debug purposes
         * @return String used to identify a component in a log
         */
        std::string toString() const;

        // region Hierarchy

        /**
         * If the div is included in a window, returns a pointer to that window
         * @return Pointer to owning window, if any
         */
        virtual Window *window();
        virtual const Window *window() const;

        /**
         * If the div is included in a parent, returns a pointer to that parent
         * @return Pointer to parent Div, if any
         */
        Div *parent();

        /**
         * If the div is included in a parent, returns a const pointer to that parent
         * @return Const pointer to parent Div, if any
         */
        const Div *parent() const;

        /**
         * Get the depth of this Div in the display hiarchy.
         * @return Depth of the Div
         */
        int depth() const;

        // endregion

        // region Children

        /**
         * Get the Div's number of children
         * @return NUmber of children
         */
        unsigned int childCount() const;

        /**
         * The children, in document order. Don't add or remove children while iterating over it.
         */
        const std::vector<std::shared_ptr<Div>> &children() const;

        /**
         * Construct a child of type T and add it last. The parent owns the child:
         *
         *     auto &row = add<Div>().row().gap(12);
         *     row.add<Button>("Save").onClick([this] { save(); });
         *
         * @return The child, valid while this div holds it
         */
        template<class T, class... Args>
        requires (std::derived_from<T, Div> && std::constructible_from<T, Args...>)
        T &add(Args &&... args) {
            auto child = std::make_shared<T>(std::forward<Args>(args)...);
            T    &ref  = *child;
            insertChild(childCount(), std::move(child));
            return ref;
        }

        /**
         * Add an existing child at index (in document order)
         * @return The child
         */
        template<class T>
        requires std::derived_from<T, Div>
        T &add(unsigned int index, std::shared_ptr<T> child) {
            T &ref = *child;
            insertChild(index, std::move(child));
            return ref;
        }

        /**
         * Add an existing child last
         * @return The child
         */
        template<class T>
        requires std::derived_from<T, Div>
        T &add(std::shared_ptr<T> child) {
            return add(childCount(), std::move(child));
        }

        /**
         * Remove a child by index
         * @param index Index of the child to remove, must be < childCount()
         */
        void remove(unsigned int index);

        /**
         * Remove a child, does nothing if it isn't one
         */
        void remove(const std::shared_ptr<Div> &child);
        void remove(const Div &child);

        /**
         * Remove all children
         */
        void removeAll();

        /**
         * Get the child at index, in document order (0 is the first child, drawn at the back)
         * @param index Index of the child to retrieve, must be < childCount()
         * @return Const pointer to the Div at index
         */
        const Div *at(unsigned int index) const;

        /**
         * Get the child at index, in document order (0 is the first child, drawn at the back)
         * @param index Index of the child to retrieve, must be < childCount()
         * @return Pointer to the Div at index
         */
        Div *at(unsigned int index);

        /**
         * Get the index of a child by shared pointer, in document order
         * @param child Shared pointer to a child
         * @return Index of the child, -1 if it isn't a child
         */
        int childIndex(const std::shared_ptr<Div> &child) const;

        /**
         * Get the index of a child by pointer, in document order
         * @param child Pointer to a child
         * @return Index of the child, -1 if it isn't a child
         */
        int childIndex(const Div *child) const;

        // endregion

        // region Style properties

        // One chainable setter per style property, generated from PSYCHIC_UI_STYLE_PROPERTIES (style/Properties.hpp).
        // They write the inline style, which overrides the stylesheets, and return the most derived type, so that
        // base and derived setters chain in any order:
        //
        //     add<Button>("Toggle").enabled(false).toggle(true).padding(24).direction(Direction::Row);
        //
        // Read values back from computedStyle(), as of the last style pass. They return `Self &`, written
        // `decltype(auto)` (of `self`, a `Self &`) so that the macro's text has no operator outside parentheses.
        #define PSYCHIC_UI_X(name, Type, Argument, defaultValue, inherited, layout, animatable) \
            template<class Self> \
            decltype(auto) name(this Self &self, Argument value) { \
                static_cast<Div &>(self)._inlineStyle.name(value); \
                return self; \
            }
        PSYCHIC_UI_STYLE_PROPERTIES(PSYCHIC_UI_X)
        #undef PSYCHIC_UI_X

        /**
         * Add to the inline transition: these properties blend to their new values with this timing (see `Transition`)
         *
         *     div.transition(Property::backgroundColor, Timing{100ms});
         */
        template<class Self>
        Self &transition(this Self &self, Property property, const Timing &timing) {
            static_cast<Div &>(self)._inlineStyle.transition(property, timing);
            return self;
        }

        template<class Self>
        Self &transition(this Self &self, std::initializer_list<Property> properties, const Timing &timing) {
            static_cast<Div &>(self)._inlineStyle.transition(properties, timing);
            return self;
        }

        /**
         * Lay the children out in a row, shortcut for `direction(Direction::Row)`
         */
        template<class Self>
        Self &row(this Self &self) { return self.direction(Direction::Row); }

        /**
         * Lay the children out in a column, shortcut for `direction(Direction::Column)`
         */
        template<class Self>
        Self &column(this Self &self) { return self.direction(Direction::Column); }

        /**
         * Shortcut for `width(width).height(height)`
         */
        template<class Self>
        Self &size(this Self &self, Length width, Length height) { return self.width(width).height(height); }

        /**
         * Overlay a whole style onto the inline style, for reusing a set of values:
         *
         *     const Style card = Style{}.padding(12).borderRadius(4).backgroundColor(rgb(0x202020));
         *     add<Div>().style(card).grow(1);
         */
        template<class Self>
        Self &style(this Self &self, const Style &style) {
            static_cast<Div &>(self)._inlineStyle.overlay(style);
            return self;
        }

        /**
         * The inline style, as set through the setters above
         */
        const Style &style() const;

        /**
         * The computed style: what the stylesheets, the parent and the inline and default styles give this div,
         * as of the last style pass, with its running transitions' values: what it paints with and pushes to Yoga.
         * While a property transitions it holds the blend, updated every frame (and `styleUpdated` runs again);
         * `targetStyle()` has where it goes.
         */
        const Style &computedStyle() const;

        /**
         * The computed style with every running transition at its end: what `computedStyle()` will be once they are
         * done. A copy.
         */
        Style targetStyle() const;

        /**
         * Whether a property of the computed style is blending to a new value
         */
        bool transitioning() const;
        bool transitioning(Property property) const;

        // endregion

        // region Animation

        /**
         * Move a number from `from` to `to` with a timing, calling `update` with each value (in
         * `Window::updateAnimations`, once per frame, the last time with `to`). Fire and forget: it ends with this
         * div, or when it gets there. At once when the div isn't in a window or the window reduces motion.
         *
         *     div.animate(0.0f, 1.0f, Timing{150ms, Ease::Out}, [this](float t) { _reveal = t; });
         */
        void animate(float from, float to, const Timing &timing, std::function<void(float)> update);

        /**
         * A class it has for its first style in a window only (its first style after it is added), lost before its
         * first paint, so that its transitions start from what that class's rules give it, as CSS's
         * `@starting-style`: a popup's `entering`. Its descendants' first styles see it too. Until the window lets it
         * go (`Window::updateStylesAndLayout`, after the layout and the popups' placement) nothing of it or its
         * descendants transitions: it was never painted. Nothing starts that way with reduced motion.
         */
        template<class Self>
        Self &startingClass(this Self &self, detail::StringArg className) {
            static_cast<Div &>(self)._startingClass = std::string{className.value};
            return self;
        }

        const std::string &startingClass() const { return _startingClass; }

        /**
         * Whether it, or an ancestor, still has its starting class
         */
        bool starting() const;

        /**
         * Leave with transitions: give it `className`, whose rules say where its transitions go (a popup's
         * `exiting`), take the mouse from it and its descendants, and call `done` (which removes it) once neither it
         * nor a descendant transitions any more, after the style pass that follows (`Window::updateStylesAndLayout`).
         * At once when it isn't in a window, was never painted (`starting()`) or the window reduces motion. A no-op
         * while it exits. Leaving the window ends it without calling `done`.
         */
        void exit(const std::string &className, std::function<void()> done);

        /**
         * Whether it is leaving (`exit`)
         */
        bool exiting() const { return _exit != nullptr; }

        /**
         * Stay after all: lose the exit's class (its transitions head back) and give the mouse back; `done` won't run
         */
        void cancelExit();

        /**
         * Whether it or a descendant transitions
         */
        bool subtreeTransitioning() const;

        /**
         * Where it is painted from where its layout puts it, as it slides there: a layout transition, its parent's
         * `layoutTransition` (FLIP). After a layout that moved it in the window, scrolling aside, it is painted from
         * where it was (from where it was painted, when it was sliding already) and the offset goes to 0 with that
         * timing; Yoga runs once. Composed with its `translateX`/`translateY`, painting only (hit testing finds it at
         * its layout place). Nothing slides on its first layout in that container (an added child appears in place,
         * and so does one shown again), on a window resize, with new rules (a theme, a density, a skin set) or with
         * reduced motion. Its size doesn't animate.
         */
        float layoutOffsetX() const;
        float layoutOffsetY() const;

        /**
         * Whether it slides to its layout place (`layoutOffsetX/Y` aren't 0). It has the `sliding` class meanwhile
         * (styled before the paint that starts it; its rules should only paint, a row's opaque background), and is
         * painted after its siblings that don't slide.
         */
        bool layoutSliding() const;

        // endregion

        // region Visibility, Focus & State

        /**
         * Whether the div is painted and hit, from its computed `visible` style. Hidden divs are still laid out.
         */
        bool visible() const;

        bool enabled() const;

        /**
         * Enable or disable the div (`:disabled` in stylesheets). Subclasses react in `enabledChanged()`.
         */
        template<class Self>
        Self &enabled(this Self &self, detail::BoolArg enabled) {
            static_cast<Div &>(self).updateEnabled(enabled.value);
            return self;
        }

        virtual bool active() const;
        bool focusEnabled() const;

        /**
         * Whether the div can take the focus (`:focus` in stylesheets)
         */
        template<class Self>
        Self &focusEnabled(this Self &self, detail::BoolArg focusEnabled) {
            static_cast<Div &>(self)._focusEnabled = focusEnabled.value;
            return self;
        }

        bool hasFocus() const;

        /**
         * Whether it has a use for Escape now, while it is on the focus path: then Escape goes to the keys' path
         * instead of closing the topmost popup first (a search field with text clears it, a number stepper being
         * edited reverts). None by default.
         */
        virtual bool takesEscape() const { return false; }

        /**
         * Used by the window, which manages the focus path
         */
        void setFocused(bool focused);

        // endregion

        // region Tooltip

        /**
         * A short text the window shows in a tooltip above the div (below when there's no room) after the mouse rests
         * on it, or after a touch long press that opens nothing else; "" for none
         */
        const std::string &tooltip() const { return _tooltip; }

        template<class Self>
        Self &tooltip(this Self &self, const std::string &text) {
            static_cast<Div &>(self)._tooltip = text;
            return self;
        }

        // endregion

        // region Context menu

        /**
         * The menu the div opens where a context menu is asked for on it (`contextMenuRequested`: a right press, a
         * touch long press, the Menu key or Shift+F10 while it or a descendant has the focus), unless a descendant
         * opens its own; empty for none. From the keyboard, the menu starts at its first item.
         */
        const std::vector<std::shared_ptr<MenuItem>> &contextMenu() const { return _contextMenu; }

        template<class Self>
        Self &contextMenu(this Self &self, std::vector<std::shared_ptr<MenuItem>> items) {
            static_cast<Div &>(self).updateContextMenu(std::move(items));
            return self;
        }

        // endregion

        // region Position & Dimensions

        // The setters are style properties (`left`, `top`, `width`, `height`, ...), which go through the computed
        // style like any other: it is the only input of the layout. The getters below read the layout's output,
        // so they are only up to date after the window's layout pass.

        void localToGlobal(int &gx, int &gy, int x = 0, int y = 0) const;
        void globalToLocal(int &lx, int &ly, int x = 0, int y = 0) const;
        int x() const;
        int y() const;
        int getWidth() const;
        int getHeight() const;

        // endregion

        // region Bounds

        int boundsLeft() const {
            return _boundsLeft;
        }

        int boundsTop() const {
            return _boundsTop;
        }

        int boundsRight() const {
            return _boundsRight;
        }

        int boundsBottom() const {
            return _boundsBottom;
        }

        int contentWidth() const {
            return _boundsRight - _boundsLeft;
        }

        int contentHeight() const {
            return _boundsBottom - _boundsTop;
        }

        // endregion

        // region Scroll

        int scrollX() const {
            return _scrollX;
        }

        template<class Self>
        Self &scrollX(this Self &self, int scrollX) {
            static_cast<Div &>(self).scrollTo(scrollX, static_cast<Div &>(self)._scrollY);
            return self;
        }

        int scrollY() const {
            return _scrollY;
        }

        template<class Self>
        Self &scrollY(this Self &self, int scrollY) {
            static_cast<Div &>(self).scrollTo(static_cast<Div &>(self)._scrollX, scrollY);
            return self;
        }

        void scroll(double scrollX, double scrollY);

        /**
         * Scroll by a number of window units, within the content (touch panning)
         * @return Whether the scroll position changed
         */
        bool scrollBy(int deltaX, int deltaY);

        /**
         * Scroll the nearest scrolling ancestor (`overflow: scroll`) vertically, just enough for a rectangle of this
         * div to show; to its start when only what is above the rectangle there would be left out (a border, a
         * header, a padding)
         * @param rect In this div's coordinates (from its top-left corner, plus its scroll offset, as `localToGlobal`)
         */
        void scrollIntoView(const SkRect &rect);

        Signal<int, int> scrolled{};

        // endregion

        // region Layout

        Signal<int, int> resized{};

        #ifdef DEBUG_LAYOUT
        static bool debugLayout;
        bool        dashed{false};
        #endif

        // endregion

        // region Style

        /**
         * Get the current style manager
         * In order of importance, the local override, the parent's (until window) or the singleton
         * @return StyleManager *
         */
        StyleManager *styleManager() const;

        /**
         * Set the local style manager override
         */
        void setStyleManager(std::shared_ptr<StyleManager> styleManager);

        /**
         * Compute style for this component and push it to its Yoga node
         */
        void updateStyle();

        /**
         * Compute style for this component and all its descendants, visible or not
         */
        void updateStyleRecursive();

        /**
         * Compute style only where it was invalidated: nodes whose style is dirty and their descendants,
         * reached through the `_descendantStyleDirty` flags, visible or not (hidden nodes still take part in layout).
         * One pass: styles invalidated by a `styleUpdated` callback during the pass may be left dirty,
         * check `styleDirty()` and run it again (`Window::updateStylesAndLayout` does).
         */
        void updateDirtyStyles();

        /**
         * Whether this node or one of its descendants needs its style computed
         */
        bool styleDirty() const;

        const std::vector<std::string> &tags() const;
        const std::string &internalId() const;
        const std::string &id() const;

        /**
         * Set the id, matched by `#id` selectors (case insensitive)
         */
        template<class Self>
        Self &id(this Self &self, std::string id) {
            static_cast<Div &>(self).updateId(std::move(id));
            return self;
        }

        const std::unordered_set<std::string> &classNames() const;

        /**
         * Replace the class names, matched by `.class` selectors (case insensitive)
         */
        template<class Self>
        Self &classNames(this Self &self, const std::unordered_set<std::string> &classNames) {
            static_cast<Div &>(self).updateClassNames(classNames);
            return self;
        }

        template<class Self>
        Self &addClassName(this Self &self, std::string className) {
            static_cast<Div &>(self).updateClassName(std::move(className), true);
            return self;
        }

        template<class Self>
        Self &removeClassName(this Self &self, std::string className) {
            static_cast<Div &>(self).updateClassName(std::move(className), false);
            return self;
        }

        /**
         * The properties this div takes from its parent's computed style when no rule sets them
         */
        virtual const PropertyMask &inheritedProperties() const;

        // endregion

        // endregion

        // region Rendering



        // endregion

        // region Mouse

        bool mouseChildren() const;

        /**
         * Whether the children get mouse events (this div still does)
         */
        template<class Self>
        Self &mouseChildren(this Self &self, detail::BoolArg mouseChildren) {
            static_cast<Div &>(self)._mouseChildren = mouseChildren.value;
            return self;
        }

        bool mouseEnabled() const;

        /**
         * Whether this div gets mouse events; they pass through it when not (see mouseChildren for its children)
         */
        template<class Self>
        Self &mouseEnabled(this Self &self, detail::BoolArg mouseEnabled) {
            static_cast<Div &>(self)._mouseEnabled = mouseEnabled.value;
            return self;
        }
        bool mouseOver() const;
        void setMouseOver(bool over); // Mostly for testing
        bool getMouseDown() const;
        void setMouseDown(bool down); // Mostly for testing

        // Every mouse signal passes a MouseEvent, in this div's content coordinates (see MouseEvent)
        Signal<MouseEvent> mouseButtonChanged{}; // Press or release, `down` tells which
        Signal<MouseEvent> mousePressed{};
        Signal<MouseEvent> mouseReleased{};
        Signal<MouseEvent> mouseReleasedOutside{};
        Signal<MouseEvent> mouseMoved{};
        Signal<MouseEvent> mouseEntered{}; // The move that entered, so a touch's can be told from the mouse's
        Signal<MouseEvent> mouseExited{};
        Signal<MouseEvent> mouseScrolled{};
        // Left button only (a touch tap is one); the mouse button signals above report every button
        Signal<MouseEvent> clicked{};
        Signal<MouseEvent> doubleClicked{}; // On the release of the second press in a row, and the third...
        /**
         * Asks for a context menu here: a right button press (a touch long press is one), or the Menu key or
         * Shift+F10 on the focused div (its center, `clickCount` 0). Goes to the deepest div with a subscriber.
         */
        Signal<MouseEvent> contextMenuRequested{};

        // endregion

        bool contains(float x, float y) const;
        bool boundsContains(float x, float y) const;

        // region External Event Triggers

        // Each takes the event in the parent's content coordinates (the window's for the window)
        virtual MouseEventStatus handleMouseMove(const MouseEvent &event, bool handled);
        virtual bool handleMouseExit(const MouseEvent &event);
        virtual MouseEventStatus handleMouseButtonChange(const MouseEvent &event);
        virtual MouseEventStatus handleMousePress(const MouseEvent &event);
        virtual MouseEventStatus handleMouseRelease(const MouseEvent &event);
        virtual MouseEventStatus handleClick(const MouseEvent &event);
        virtual MouseEventStatus handleDoubleClick(const MouseEvent &event);
        virtual MouseEventStatus handleMouseScroll(const MouseEvent &event);
        virtual MouseEventStatus handleContextMenuRequest(const MouseEvent &event);

        /**
         * An event at this div's center, for what the keyboard does in place of the mouse (a click by Enter, a
         * context menu by the Menu key): no button (set the one it stands for), `clickCount` 0, the window's input
         * mode as source
         */
        MouseEvent mouseEventAtCenter();

        /**
         * The event moved from the parent's content coordinates into this div's
         */
        MouseEvent localMouseEvent(const MouseEvent &event) const;

        Signal<Key, Mod>           keyPressed{};
        Signal<Key, Mod>           keyRepeated{};
        Signal<Key, Mod>           keyReleased{};
        Signal<icu::UnicodeString> typed{};
        Signal<>                   focused{};
        Signal<>                   blurred{};

        // endregion

        // region Events

        // Each event is a chainable connector, `on` + a verb in the present tense (`onClick`, `onMousePress`,
        // `onFocus`), that subscribes to a Signal member named with the same verb in the past tense (`clicked`,
        // `mousePressed`, `focused`; components add `onChange`/`changed` and others), and returns this div:
        //
        //     add<Button>("Save").onClick([this] { save(); });
        //
        // The signal belongs to the div, so the callback lives as long as the div. Capture `this` (the div that
        // creates the child, which outlives it) or raw pointers, never a shared_ptr to the div the signal belongs
        // to: the div would own the callback that owns the div, a cycle that never frees it.
        // To stop listening before the div dies, subscribe to the signal itself and keep the slot
        // (`ScopedSlot slot = button.clicked.subscribe(...)`). To listen to a signal of something that may die
        // first, use `subscribeTo` (Observer), which ends the subscription with the subscriber.
        #define PSYCHIC_UI_EVENT(connector, signal) \
            template<class Self, class F> \
            Self &connector(this Self &self, F &&callback) { \
                self.signal.subscribe(std::forward<F>(callback)); \
                return self; \
            }

        PSYCHIC_UI_EVENT(onMouseButtonChange, mouseButtonChanged)
        PSYCHIC_UI_EVENT(onMousePress, mousePressed)
        PSYCHIC_UI_EVENT(onMouseRelease, mouseReleased)
        PSYCHIC_UI_EVENT(onMouseReleaseOutside, mouseReleasedOutside)
        PSYCHIC_UI_EVENT(onMouseMove, mouseMoved)
        PSYCHIC_UI_EVENT(onMouseEnter, mouseEntered)
        PSYCHIC_UI_EVENT(onMouseExit, mouseExited)
        PSYCHIC_UI_EVENT(onMouseScroll, mouseScrolled)
        PSYCHIC_UI_EVENT(onClick, clicked)
        PSYCHIC_UI_EVENT(onDoubleClick, doubleClicked)
        PSYCHIC_UI_EVENT(onContextMenuRequest, contextMenuRequested)
        PSYCHIC_UI_EVENT(onKeyPress, keyPressed)
        PSYCHIC_UI_EVENT(onKeyRepeat, keyRepeated)
        PSYCHIC_UI_EVENT(onKeyRelease, keyReleased)
        PSYCHIC_UI_EVENT(onType, typed)
        PSYCHIC_UI_EVENT(onFocus, focused)
        PSYCHIC_UI_EVENT(onBlur, blurred)
        PSYCHIC_UI_EVENT(onScroll, scrolled)
        PSYCHIC_UI_EVENT(onResize, resized)

        // endregion

    protected:
        std::string _tooltip{};
        std::vector<std::shared_ptr<MenuItem>> _contextMenu{};
        ScopedSlot _contextMenuSlot{}; // Our subscription to contextMenuRequested while there is a context menu
        void updateContextMenu(std::vector<std::shared_ptr<MenuItem>> items);
        bool _enabled{true};
        bool _visible{true};
        int  _x{0};
        int  _y{0};
        int  _width{0};
        int  _height{0};
        // Bounds, includes absolute positioned children
        int  _boundsLeft{0};
        int  _boundsTop{0};
        int  _boundsRight{0};
        int  _boundsBottom{0};

        // region Focus

        bool _focusEnabled{false};
        bool _focused{false};

        // endregion

        // region Hierarchy

        /**
         * Set the parent of that div
         * Used internally when adding/removing a child to/from a parent
         * @param Div* parent
         */
        void setParent(Div *parent);

        virtual void added();
        virtual void removed();
        virtual void addedToRender();
        virtual void removedFromRender();

        /**
         * Copy of the children, topmost (last) first, for dispatching mouse events.
         * A copy so that handlers can add or remove children while we iterate.
         */
        std::vector<std::shared_ptr<Div>> childrenTopmostFirst() const;

        /**
         * Invalidate the style of the first and last children, whose :first-child/:last-child state
         * changes when children are added or removed
         */
        void invalidateEndChildrenStyle();

        /**
         * Insert a child in both the children and the Yoga node, then parent it (which styles it)
         */
        void insertChild(unsigned int index, std::shared_ptr<Div> child);

        // region Implementation of the chainable setters

        void updateEnabled(bool enabled);
        void updateId(std::string id);
        void updateClassNames(const std::unordered_set<std::string> &classNames);
        void updateClassName(std::string className, bool present);
        void scrollTo(int scrollX, int scrollY);

        /**
         * Called when `enabled(bool)` changes the enabled state
         */
        virtual void enabledChanged() {}

        // endregion

        int                               _depth{0};
        Div                               *_parent{nullptr};
        /**
         * Children in document order (same as Yoga): first is at the back, last is on top
         */
        std::vector<std::shared_ptr<Div>> _children{};

        // endregion

        // region Style

        /**
         * Add a tag to the chain read by `tags()`: each class in the hierarchy adds its own, so selectors can match
         * any of them. Obviously this should be static and/or use Reflection but in order to progress in other parts
         * of the library it will stay local for the time being, until a better technique is implemented.
         */
        template<class Self>
        Self &tag(this Self &self, std::string tag) {
            static_cast<Div &>(self).addTag(std::move(tag));
            return self;
        }

        void addTag(std::string tag);

        /**
         * Chain of tags from the inheritance chain
         */
        std::vector<std::string> _tags{};

        /**
         * Internal Id
         */
        static int  idCounter;
        std::string _internalId{};

        /**
         * Id
         */
        std::string _id{};

        /**
         * Pseudo CSS class names
         */
        std::unordered_set<std::string> _classNames{};

        /**
         * Style Manager Override
         */
        std::shared_ptr<StyleManager> _styleManager{nullptr};

        /**
         * Default Style
         * Use this instead of the inline style when a component needs a specific default for it to work correctly
         * For example, text components should not grow/shrink as they are inline, but yoga doesn't let us do that.
         */
        Style _defaultStyle;

        /**
         * Inline Style
         * Use the inline styles to set values that will override any stylesheet
         */
        Style _inlineStyle;

        /**
         * Computed Style
         * Computed style values, use this to obtain the final style to be applied
         */
        Style _computedStyle{};

        /**
         * Whether the computed style was pushed to the Yoga node once: until then every layout property is
         */
        bool _layoutInitialized{false};

        /**
         * Dirty style flag
         */
        bool _styleDirty{true};

        /**
         * A descendant has a dirty style, so the style pass has to go through this node
         */
        bool _descendantStyleDirty{false};

        /**
         * The window, and the style manager's rules generation, the style was last computed in: transitions only
         * start from a style computed in the same window with the same rules, so neither a div's first style in a
         * window nor a theme switch transitions
         */
        Window      *_styledIn{nullptr};
        std::size_t _styledGeneration{0};

        /**
         * The running style transitions, which the window ticks (made with the first one)
         */
        std::shared_ptr<detail::StyleTransitions> _transitions{};

        /**
         * `animate`'s, until they end
         */
        std::vector<std::shared_ptr<Animation>> _animations{};

        /**
         * `startingClass`, and whether it has it now (from its first style in a window until the window lets it go)
         */
        std::string _startingClass{};
        bool        _startingApplied{false};

        /**
         * A running `exit`: its class, what to call at the end, the mouse flags it took
         */
        struct Exit {
            std::string           className;
            std::function<void()> done;
            bool                  mouseEnabled;
            bool                  mouseChildren;
        };
        std::unique_ptr<Exit> _exit{};

        /**
         * Its layout transition, made the first time it is laid out in a container that has one: where its last layout
         * put it in the window (scrolling aside), in which of the window's slide passes and which container, and its
         * paint offset sliding to 0 (two channels)
         */
        struct LayoutSlide {
            std::shared_ptr<detail::TweenAnimation> offset{std::make_shared<detail::TweenAnimation>()};
            float                                   x{0.0f};
            float                                   y{0.0f};
            std::uint64_t                           pass{0};
            const Div                               *container{nullptr};
        };
        std::unique_ptr<LayoutSlide> _layoutSlide{};

        /**
         * A slide pass's walk down the tree: what a div needs from its ancestors
         */
        struct SlideWalk {
            std::uint64_t pass{0};
            bool          snap{false};      // Everything goes where it is laid out
            float         originX{0.0f};    // The parent's place in the window, scrolling aside
            float         originY{0.0f};
            float         paintedX{0.0f};   // The ancestors' offsets as painted until now
            float         paintedY{0.0f};
            float         offsetX{0.0f};    // Their offsets from now on
            float         offsetY{0.0f};
            const Timing  *timing{nullptr}; // The parent's layout transition, nullptr when it has none
        };

        /**
         * After a layout (`Window::updateStylesAndLayout`), down the tree: start, retarget or drop the layout slides
         */
        void updateLayoutSlides(const SlideWalk &walk);

        /**
         * The `sliding` class while `layoutSliding()`
         */
        void updateSlidingClass();

        /**
         * Lose the starting class (the window lets it go): its next style transitions from it
         */
        void releaseStartingClass();

        /**
         * The exit is over (nothing of it transitions): undo it and call `done`
         */
        void finishExit();

        friend class Window;

        /**
         * Start, retarget or drop transitions for a new computed style, and put their current values in it
         */
        void updateTransitions(Style &computed);

        /**
         * Put the transitions' values at `now` in the computed style
         * @return Whether one still runs
         */
        bool advanceTransitions(std::chrono::nanoseconds now);
        void finishTransitions();

        /**
         * Drop the transitions, leaving the computed style where they were (removed from the window)
         */
        void cancelTransitions();

        /**
         * The transitions changed these computed values: push them to Yoga, run `styleUpdated`, restyle the children
         * that inherit them
         */
        void transitionsChanged(const PropertyMask &changed);

        friend class detail::StyleTransitions;


        /**
         * Invalidate the style
         */
        void invalidateStyle();

        /**
         * Update the runtime style rules
         */
        void updateRuntimeStyles();

        /**
         * Create runtime styles
         * This is called by updateRuntimeStyles and has to be overridden by div's
         * that want to add rules to the stylesheet. It will be called during the app's
         * lifetime when adding to render list or if a stylesheet is loaded.
         */
        virtual void createStyles() {};

        /**
         * Callback for when styles were updated
         */
        virtual void styleUpdated();

        // endregion

        // region Layout

        /**
         * Yoga Node
         */
        YGNode *_yogaNode{nullptr};

        /**
         * Component's rect
         */
        SkRect _rect{};

        /**
         * Component's margin rect
         */
        SkRect _marginRect{};
        float  _marginLeft{0.0f};
        float  _marginRight{0.0f};
        float  _marginTop{0.0f};
        float  _marginBottom{0.0f};

        /**
         * Component's padded rect
         */
        SkRect _paddedRect{};

        /**
         * Round rect
         */
        SkRRect _roundRect{};

        /**
         * Bounds rect
         */
        SkRect _boundsRect{};

        /**
         * How far the shadow and the outline paint outside the border box, for culling and the opacity layer
         */
        float _paintOutset{0.0f};


        /**
         * Ignore internal layout constraints (padding, border)
         * Used by component to forward those to its skin
         */
        bool _ignoreInternalLayoutContraints{false};

        /**
         * Sets the component as measurable
         * Registers the measure callback
         */
        void setMeasurable();

        /**
         * Push the computed style's layout properties to the Yoga node, only those in `changed`.
         * Paint-only properties never reach Yoga, so changing them doesn't dirty the layout.
         * The layoutUpdated() method will be called after Yoga has computed the new layout
         */
        void updateLayout(const PropertyMask &changed);

        /**
         * Callback for when layout was updated
         */
        virtual void layoutUpdated();

        bool layoutReady{false};

        // endregion

        // region Rendering

        void invalidate();
        bool isValid() const;

        /**
         * The size of a measurable div's content (see setMeasurable), within what the layout offers
         * in each dimension. Must be pure: the layout caches it and calls it with various constraints.
         */
        virtual Size measure(Constraint width, Constraint height);
        virtual void render(SkCanvas *canvas);
        void clip(SkCanvas *canvas);
        virtual void draw(SkCanvas *canvas);

        /**
         * The `shadow*` and `outline*` styles, around the border box: every div paints them, whatever its `draw`
         */
        void drawShadow(SkCanvas *canvas);
        void drawDotGrid(SkCanvas *canvas);
        void drawOutline(SkCanvas *canvas);

        bool _drawBackground{false};
        bool _drawBorder{false};
        bool _drawComplexBorders{false};
        bool _drawRoundRect{false};
        bool _drawComplexRoundRect{false};

        float    _radiusTopLeft{0.0f};
        float    _radiusTopRight{0.0f};
        float    _radiusBottomLeft{0.0f};
        float    _radiusBottomRight{0.0f};
        std::array<SkVector, 4> _radii{}; // Top left, top right, bottom right, bottom left, as Skia wants them

        float _borderLeft{0.0f};
        float _borderRight{0.0f};
        float _borderTop{0.0f};
        float _borderBottom{0.0f};

        int _scrollX{0};
        int _scrollY{0};

        // endregion

        // region Mouse

        /**
         * Enable the mouse events for this div.
         * Events will passthrough if disabled
         * See mouseChildren for ignoring the children
         */
        bool _mouseEnabled{true};

        /**
         * Enables the mouse events for this div's children
         * Events will still affect the current div
         * See mouseEnabled for ignoring events on this div
         */
        bool _mouseChildren{true};

        bool _mouseOver{false};
        bool _mouseDown{false};



        // endregion

    private:
        void addedToRenderRecursive();
        void removedFromRenderRecursive();
    };
}

