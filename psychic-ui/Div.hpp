#pragma once

#include <iostream>

#include <vector>
#include <unordered_set>
#include <yoga/Yoga.h>
#include <unicode/unistr.h>
#include <SkCanvas.h>
#include <SkRRect.h>
#include "psychic-ui.hpp"
#include "psychic-ui/style/Style.hpp"
#include "psychic-ui/style/StyleManager.hpp"
#include "psychic-ui/signals/Signal.hpp"
#include "psychic-ui/signals/Observer.hpp"

namespace psychic_ui {

    class Window;

    class Panel;

    /**
     * Return status of the mouse event methods
     */
    enum MouseEventStatus {
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
    using MouseCallback = std::function<void(const int mouseX, const int mouseY, int button, int modifiers)>;

    /**
     * Shortcut type for mouse button signal slot
     */
    using MouseButtonCallback = std::function<void(const int mouseX, const int mouseY, int button, bool down, int modifiers)>;

    /**
     * Shortcut type for mouse scroll signal slot
     */
    using MouseScrollCallback = std::function<void(const int mouseX, const int mouseY, const double scrollX, const double scrollY)>;

    /**
     * @class Div
     *
     * The Div class is the main object used to display content in Psychic UI.
     * It has a similar role as a `div` in html: its purpose is to offer a rectangular container
     * than can be used in a layout and styled using a stylesheet.
     */
    class Div : public Observer, public std::enable_shared_from_this<Div> {
        friend class StyleManager;

        friend class Modal;

    public:
        Div();

        /**
         * Just as a safety for now, delete the copy constructor
         * because I don't see a reason why we would need it.
         */
        Div(const Div &) = delete;

        ~Div(); // override;

        /**
         * Just as a safety for now, delete the copy constructor
         * because I don't see a reason why we would need it.
         */
        Div &operator=(const Div &) = delete;

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
         * Get the children of this Div
         * @return Vector of child Divs
         */
        const std::vector<std::shared_ptr<Div>> children() const;

        /**
         * Add a child at index
         * @param index Index where to add the child
         * @param child Child to add
         * @return Pointer to this, used for chaining multiple calls
         */
        std::shared_ptr<Div> add(unsigned int index, std::shared_ptr<Div> child);

        /**
         * Add a child
         * @param child Child to add
         * @return Pointer to this, used for chaining multiple calls
         */
        std::shared_ptr<Div> add(std::shared_ptr<Div> child);

        /**
         * Construct a child from type T and add it at the same time
         * @tparam T Type of the child to add
         * @tparam Args
         * @param args Arguments to pass to the child's constructor
         * @return
         */
        template<typename T, typename... Args>
        std::shared_ptr<T> add(Args &&... args) {
            return std::static_pointer_cast<T>(add(std::make_shared<T>(std::forward<Args>(args)...)));
        }

        /**
         * Remove a child by index
         * @param index Index of the child to remove
         */
        void remove(unsigned int index);

        /**
         * Remove a child by shared pointer
         * @param child Child to remove
         */
        void remove(std::shared_ptr<Div> child);

        /**
         * Remove all children
         */
        void removeAll();

        /**
         * Get the child at index
         * @param index Index of the child to retrieve
         * @return Const pointer to the Div at index
         */
        const Div *at(unsigned int index) const;

        /**
         * Get the child at index
         * @param index Index of the child to retrieve
         * @return Pointer to the Div at index
         */
        Div *at(unsigned int index);

        /**
         * Get the index of a child by shared pointer
         * @param child Shared pointer to a child
         * @return Index of the child
         */
        int childIndex(std::shared_ptr<Div> child) const;

        /**
         * Get the index of a child by pointer
         * @param child Pointer to a child
         * @return Index of the child
         */
        int childIndex(const Div *child) const;

        // endregion

        // region Visibility, Focus & State

        bool getVisible() const;
        virtual void setVisible(bool value);
        bool enabled() const;
        virtual Div *setEnabled(bool enabled);
        virtual bool active() const;
        bool focusEnabled() const;
        Div *setFocusEnabled(bool focusEnabled);
        bool focused() const;
        void setFocused(bool focused);

        // endregion

        // region Position

        void setPosition(int x, int y);
        void setPosition(int left, int top, int right, int bottom);
        void localToGlobal(int &gx, int &gy, int x = 0, int y = 0) const;
        void globalToLocal(int &lx, int &ly, int x = 0, int y = 0) const;
        int x() const;
        void setX(int x);
        int y() const;
        void setY(int y);
        int getLeft() const;
        void setLeft(int left);
        int getRight() const;
        void setRight(int right);
        int getTop() const;
        void setTop(int top);
        int getBottom() const;
        void setBottom(int bottom);
        float getLeftPercent() const;
        void setLeftPercent(float leftPercent);
        float getRightPercent() const;
        void setRightPercent(float rightPercent);
        float getTopPercent() const;
        void setTopPercent(float topPercent);
        float getBottomPercent() const;
        void setBottomPercent(float bottomPercent);

        // endregion

        // region Dimensions

        void setSize(int width, int height);
        int getWidth() const;
        void setWidth(int width);
        int getHeight() const;
        void setHeight(int height);
        float getWidthPercent() const;
        void setWidthPercent(float widthPercent);
        float getHeightPercent() const;
        void setHeightPercent(float heightPercent);

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

        Div *setScrollX(const int &scrollX) {
            if (scrollX != _scrollX) {
                _scrollX = scrollX;
                onScrolled(_scrollX, _scrollY);
            }
            return this;
        }

        int scrollY() const {
            return _scrollY;
        }

        Div *setScrollY(const int &scrollY) {
            if (scrollY != _scrollY) {
                _scrollY = scrollY;
                onScrolled(_scrollX, _scrollY);
            }
            return this;
        }

        void scroll(double scrollX, double scrollY);
        Signal<int, int> onScrolled{};

        // endregion

        // region Layout

        Signal<int, int> onResized{};

        #ifdef DEBUG_LAYOUT
        static bool debugLayout;
        bool        dashed{false};
        #endif

        // endregion

        // region Style

        /**
         * Get the inline style
         * @return Style*
         */
        Style *style() const;

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
         * Compute style for this component
         */
        void updateStyle();
        void updateStyleRecursive();

        /**
         * Get the computed style
         * @return
         */
        const Style *computedStyle() const;

        const std::vector<std::string> &tags() const;
        const std::string internalId() const;
        const std::string id() const;
        Div *setId(std::string id);
        const std::unordered_set<std::string> &classNames() const;
        Div *setClassNames(std::unordered_set<std::string> additionalClassNames);
        Div *addClassName(std::string className);
        Div *removeClassName(std::string className);
        virtual const InheritableValues inheritableValues() const;

        // endregion

        // region Rendering



        // endregion

        // region Mouse

        bool mouseChildren() const;
        Div *setMouseChildren(bool mouseChildren);
        bool mouseEnabled() const;
        Div *setMouseEnabled(bool mouseEnabled);
        bool mouseOver() const;
        void setMouseOver(bool over); // Mostly for testing
        bool getMouseDown() const;
        void setMouseDown(bool down); // Mostly for testing

        using MouseButtonSlot = std::shared_ptr<Slot<const int, const int, const MouseButton, const bool, const Mod>>;
        using MouseMoveSlot = std::shared_ptr<Slot<const int, const int, const int, const Mod>>;
        using MouseSlot = std::shared_ptr<Slot<const int, const int, const MouseButton, const Mod>>;
        using MouseTriggerSlot = std::shared_ptr<Slot<>>;
        using MouseDoubleClickSlot = std::shared_ptr<Slot<const unsigned int>>;
        using MouseScrollSlot = std::shared_ptr<Slot<const int, const int, const double, const double>>;

        Signal<const int, const int, const MouseButton, const bool, const Mod> onMouseButton{};
        Signal<const int, const int, const MouseButton, const Mod>             onMouseDown{};
        Signal<const int, const int, const MouseButton, const Mod>             onMouseUp{};
        Signal<const int, const int, const MouseButton, const Mod>             onMouseUpOutside{};
        Signal<const int, const int, const int, const Mod>                     onMouseMove{};
        Signal<>                                                               onMouseOver{};
        Signal<>                                                               onMouseOut{};
        Signal<const int, const int, const double, const double>               onMouseScroll{};
        Signal<>                                                               onClick{};
        Signal<const unsigned int>                                             onDoubleClick{};

        // endregion

        inline bool contains(int x, int y) const;
        inline bool boundsContains(int x, int y) const;

        // region External Event Triggers

        virtual MouseEventStatus mouseMoved(int mouseX, int mouseY, int buttons, Mod modifiers, bool handled);
        virtual bool mouseExited(int mouseX, int mouseY, int buttons, Mod modifiers);
        virtual MouseEventStatus mouseButton(int mouseX, int mouseY, MouseButton button, bool down, Mod modifiers);
        virtual MouseEventStatus mouseDown(int mouseX, int mouseY, MouseButton button, Mod modifiers);
        virtual MouseEventStatus mouseUp(int mouseX, int mouseY, MouseButton button, Mod modifiers);
        virtual MouseEventStatus click(int mouseX, int mouseY, MouseButton button, Mod modifiers);
        virtual MouseEventStatus doubleClick(int mouseX, int mouseY, const unsigned int clickCount, Mod modifiers);
        virtual MouseEventStatus mouseScrolled(int mouseX, int mouseY, double scrollX, double scrollY);

        using KeySlot = std::shared_ptr<Slot<const Key, const Mod>>;
        using CharSlot = std::shared_ptr<Slot<const UnicodeString &>>;
        using FocusSlot = std::shared_ptr<Slot<>>;

        Signal<const Key, const Mod>  onKeyDown{};
        Signal<const Key, const Mod>  onKeyRepeat{};
        Signal<const Key, const Mod>  onKeyUp{};
        Signal<const UnicodeString &> onCharacter{};
        Signal<>                      onFocus{};
        Signal<>                      onBlur{};

        // endregion

    protected:
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

        int                               _depth{0};
        Div                               *_parent{nullptr};
        std::vector<std::shared_ptr<Div>> _children{};

        // endregion

        // region Style

        /**
         * Set the component tag, it wil be appended to _componentTag in order to get a type hierarchy for styling.
         * Obviously this should be static and/or use Reflection but in order to progress in other parts of the library
         * it will stay local for the time being, until a better technique is implemented.
         * @param componentName
         */
        Div *setTag(std::string componentName);

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
        std::unique_ptr<Style> _defaultStyle{nullptr};

        /**
         * Inline Style
         * Use the inline styles to set values that will override any stylesheet
         */
        std::unique_ptr<Style> _inlineStyle{nullptr};

        /**
         * Computed Style
         * Computed style values, use this to obtain the final style to be applied
         */
        std::unique_ptr<Style> _computedStyle{nullptr};

        /**
         * Dirty style flag
         */
        bool _styleDirty{true};


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
        YGNodeRef _yogaNode{nullptr};

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
         * Ignore internal layout constraints (padding, border)
         * Used by component to forward those to its skin
         */
        bool _ignoreInternalLayoutContraints{false};

        /**
         * Sets the component as measurable
         * Registeres the measure callback
         */
        void setMeasurable();

        /**
         * Updates the layout from the computed style
         * No style validation will occur
         * The layoutUpdated() method will be called on the next frame, after yoga has computed the new layout
         */
        void updateLayout();

        /**
         * Callback for when layout was updated
         */
        virtual void layoutUpdated();

        bool layoutReady{false};

        // endregion

        // region Rendering

        void invalidate();
        virtual YGSize measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode);
        virtual void render(SkCanvas *canvas);
        void clip(SkCanvas *canvas);
        virtual void draw(SkCanvas *canvas);

        bool _drawBackground{false};
        bool _drawBorder{false};
        bool _drawComplexBorders{false};
        bool _drawRoundRect{false};
        bool _drawComplexRoundRect{false};

        float    _radiusTopLeft{0.0f};
        float    _radiusTopRight{0.0f};
        float    _radiusBottomLeft{0.0f};
        float    _radiusBottomRight{0.0f};
        SkVector _radii[4]{
            {0.0f, 0.0f},
            {0.0f, 0.0f},
            {0.0f, 0.0f},
            {0.0f, 0.0f}
        };

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
        static const InheritableValues _inheritableValues;
        void addedToRenderRecursive();
        void removedFromRenderRecursive();
    };
}

