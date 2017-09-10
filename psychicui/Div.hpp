#pragma once

#include <iostream>

#include <vector>
#include <unordered_set>
#include <yoga/Yoga.h>
#include <SkCanvas.h>
#include <SkRRect.h>
#include "psychicui.hpp"
#include "psychicui/style/Style.hpp"
#include "psychicui/style/StyleManager.hpp"
#include "psychicui/signals/Signal.hpp"
#include "psychicui/signals/Observer.hpp"

namespace psychicui {

    class Window;

    class Panel;

    enum MouseEventStatus {
        Out,
        Over,
        Handled
    };

    using ClickCallback = std::function<void()>;
    using MouseCallback = std::function<void(const int mouseX, const int mouseY, int button, int modifiers)>;
    using MouseButtonCallback = std::function<void(const int mouseX, const int mouseY, int button, bool down, int modifiers)>;
    using MouseScrollCallback = std::function<void(const int mouseX, const int mouseY, const double scrollX, const double scrollY)>;

    class Div : public Observer, public std::enable_shared_from_this<Div> {
        friend class StyleManager;

        friend class Modal;

    public:
        Div();
        ~Div();

        std::string toString() const;

        // region Hierarchy

        virtual Window *window();
        Div *parent();
        const Div *parent() const;
        void setParent(Div *parent);
        const int depth() const;

//        std::vector<std::shared_ptr<Div>> path();

        virtual std::shared_ptr<Panel> panel();

        // endregion

        // region Children

        unsigned int childCount() const;
        const std::vector<std::shared_ptr<Div>> children() const;
        std::shared_ptr<Div> add(unsigned int index, std::shared_ptr<Div> component);
        std::shared_ptr<Div> add(std::shared_ptr<Div> child);

        template<typename T, typename... Args>
        std::shared_ptr<T> add(const Args &... args) {
            return std::static_pointer_cast<T>(add(std::make_shared<T>(args...)));
        }

        void remove(unsigned int index);
        void remove(std::shared_ptr<Div> child);
        void removeAll();

        const Div *at(unsigned int index) const;
        Div *at(unsigned int index);
        int childIndex(std::shared_ptr<Div> component) const;
        int childIndex(const Div *component) const;

        // endregion

        // region Visibility, Focus & State

        bool getVisible() const;
        virtual void setVisible(bool value);
        bool enabled() const;
        virtual void setEnabled(bool value);
        virtual const bool active() const;
        bool focused() const;
        void requestFocus();
        virtual void requestFocus(Div *component);

        // endregion

        // region Position

        void setPosition(int x, int y);
        void setPosition(int left, int top, int right, int bottom);
        void getLocalToGlobal(int &gx, int &gy, const int x = 0, const int y = 0) const;
        void getGlobalToLocal(int &lx, int &ly, const int x = 0, const int y = 0) const;
        const int x() const;
        void setX(int x);
        const int y() const;
        void setY(int y);
        const int getLeft() const;
        void setLeft(int left);
        const int getRight() const;
        void setRight(int right);
        const int getTop() const;
        void setTop(int top);
        const int getBottom() const;
        void setBottom(int bottom);
        const float getLeftPercent() const;
        void setLeftPercent(float leftPercent);
        const float getRightPercent() const;
        void setRightPercent(float rightPercent);
        const float getTopPercent() const;
        void setTopPercent(float topPercent);
        const float getBottomPercent() const;
        void setBottomPercent(float bottomPercent);

        // endregion

        // region Dimensions

        void setSize(int width, int height);
        const int getWidth() const;
        void setWidth(int width);
        const int getHeight() const;
        void setHeight(int height);
        const float getWidthPercent() const;
        void setWidthPercent(float widthPercent);
        const float getHeightPercent() const;
        void setHeightPercent(float heightPercent);

        // endregion

        // region Bounds

        const int boundsLeft() const {
            return _boundsLeft;
        }

        const int boundsTop() const {
            return _boundsTop;
        }

        const int boundsRight() const {
            return _boundsRight;
        }

        const int boundsBottom() const {
            return _boundsBottom;
        }

        // endregion

        // region Layout

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
        const std::string id() const;
        Div *setId(std::string id);
        const std::unordered_set<std::string> &classNames() const;
        Div *setClassNames(std::unordered_set<std::string> additionalClassNames);
        Div *addClassName(std::string className);
        Div *removeClassName(std::string className);
        virtual const InheritableValues inheritableValues() const;

        // endregion

        // region Rendering

        void invalidate();
        virtual YGSize measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode);
        virtual void render(SkCanvas *canvas);
        virtual void clip(SkCanvas *canvas);
        virtual void draw(SkCanvas *canvas);

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


        using MouseButtonSlot = std::shared_ptr<Slot<const int, const int, const int, const bool, const int>>;
        using MouseSlot = std::shared_ptr<Slot<const int, const int, const int, const int>>;
        using MouseTriggerSlot = std::shared_ptr<Slot<>>;
        using MouseScrollSlot = std::shared_ptr<Slot<const int, const int, const double, const double>>;

        Signal<const int, const int, const int, const bool, const int> onMouseButton{};
        Signal<const int, const int, const int, const int>             onMouseDown{};
        Signal<const int, const int, const int, const int>             onMouseUp{};
        Signal<const int, const int, const int, const int>             onMouseUpOutside{};
        Signal<const int, const int, const int, const int>             onMouseMove{};
        Signal<>                                                       onMouseOver{};
        Signal<>                                                       onMouseOut{};
        Signal<const int, const int, const double, const double>       onMouseScroll{};
        Signal<>                                                       onClick{};

        // endregion

//        std::shared_ptr<Div> findComponent(const int x, const int y);
        inline bool contains(const int x, const int y) const;
        inline bool boundsContains(const int x, const int y) const;


        // Events
        virtual bool focusEvent(bool focused);
        virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);
        virtual bool keyboardCharacterEvent(unsigned int codepoint);


    protected:
        bool _enabled{true};
        bool _visible{true};
        int  _x{0};
        int  _y{0};
        int  _width{0};
        int  _height{0};
        // Bounds, includes absolute positioned children
        int _boundsLeft{0};
        int _boundsTop{0};
        int _boundsRight{0};
        int _boundsBottom{0};
        bool _wrap{false};

        // region Lifecycle

        // endregion

        // region Hierarchy

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
        SkRect _rect;

        /**
         * Component's padded rect
         */
        SkRect _paddedRect;



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

        bool _drawBackground{false};
        bool _drawBorder{false};
        bool _drawComplexBorders{false};
        bool _drawRoundRect{false};
        bool _drawComplexRoundRect{false};

        float _radiusTopLeft{0.0f};
        float _radiusTopRight{0.0f};
        float _radiusBottomLeft{0.0f};
        float _radiusBottomRight{0.0f};

        float _borderLeft{0.0f};
        float _borderRight{0.0f};
        float _borderTop{0.0f};
        float _borderBottom{0.0f};

        float _scrollX{0.0f};
        float _scrollY{0.0f};

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

        virtual MouseEventStatus mouseMoved(int mouseX, int mouseY, int button, int modifiers, bool handled);
        virtual bool mouseExited(int mouseX, int mouseY, int button, int modifiers);
        virtual MouseEventStatus mouseButton(int mouseX, int mouseY, int button, bool down, int modifiers);
        virtual MouseEventStatus mouseDown(int mouseX, int mouseY, int button, int modifiers);
        virtual MouseEventStatus mouseUp(int mouseX, int mouseY, int button, int modifiers);
        virtual MouseEventStatus click(int mouseX, int mouseY, int button, int modifiers);
        virtual MouseEventStatus mouseScrolled(int mouseX, int mouseY, double scrollX, double scrollY);

        // endregion



        bool _focused{false};
        void focused(bool focused);

    private:
        static const InheritableValues _inheritableValues;
        void addedToRenderRecursive();
        void removedFromRenderRecursive();
    };
}

