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

namespace psychicui {

    class Window;

    class Panel;

    using ClickCallback = std::function<void()>;
    using MouseCallback = std::function<void(const int mouseX, const int mouseY, int button, int modifiers)>;
    using MouseButtonCallback = std::function<void(const int mouseX, const int mouseY, int button, bool down, int modifiers)>;
    using MouseScrollCallback = std::function<void(const int mouseX, const int mouseY, const double scrollX, const double scrollY)>;

    class Div : public std::enable_shared_from_this<Div> {
        friend class StyleManager;

    public:
        Div();
        ~Div();

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
        void getGlobalPosition(int &x, int &y) const;
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

        // region Layout

        #ifdef DEBUG_LAYOUT
        static bool debugLayout;
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
        virtual void draw(SkCanvas *canvas);

        // endregion

        // region Mouse & Cursor

        Cursor cursor() const;
        void setCursor(Cursor cursor);
        bool mouseChildren() const;
        Div *setMouseChildren(bool mouseChildren);
        bool mouseEnabled() const;
        Div *setMouseEnabled(bool mouseEnabled);
        bool mouseOver() const;
        void setMouseOver(bool over); // Mostly for testing
        bool getMouseDown() const;
        void setMouseDown(bool down); // Mostly for testing

        Div *onMouseDown(MouseCallback mouseDown) {
            _onMouseDown = mouseDown;
            return this;
        }

        Div *onMouseUp(MouseCallback mouseUp) {
            _onMouseUp = mouseUp;
            return this;
        }

        Div *onMouseUpOutside(MouseCallback mouseUpOutside) {
            _onMouseUpOutside = mouseUpOutside;
            return this;
        }

        Div *onClick(ClickCallback click) {
            _onClick = click;
            return this;
        }

        Div *onMouseMove(MouseCallback mouseMove) {
            _onMouseMove = mouseMove;
            return this;
        }

        Div *onMouseOver(MouseCallback mouseOver) {
            _onMouseOver = mouseOver;
            return this;
        }

        Div *onMouseOut(MouseCallback mouseOut) {
            _onMouseOut = mouseOut;
            return this;
        }

        Div *onMouseScrolled(MouseScrollCallback mouseScrolled) {
            _onMouseScroll = mouseScrolled;
            return this;
        }

        Div *onMouseButton(MouseButtonCallback mouseButton) {
            _onMouseButton = mouseButton;
            return this;
        }

        // endregion

//        std::shared_ptr<Div> findComponent(const int x, const int y);
        inline bool contains(const int x, const int y) const;


        // Events
        virtual bool mouseDragEvent(const int mouseX, const int mouseY, const int dragX, const int dragY, int button, int modifiers);
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

        // endregion

        // region Mouse

        Cursor _cursor{Cursor::Arrow};
        bool   _mouseEnabled{true};
        bool   _mouseChildren{true};
        bool   _mouseOver{false};
        bool   _mouseDown{false};

        Cursor mouseMoved(int mouseX, int mouseY, int button, int modifiers);
        bool mouseButton(int mouseX, int mouseY, int button, bool down, int modifiers);
        bool mouseDown(int mouseX, int mouseY, int button, int modifiers);
        bool mouseUp(int mouseX, int mouseY, int button, int modifiers);
        bool mouseScrolled(int mouseX, int mouseY, double scrollX, double scrollY);

        virtual void onMouseButtonEvent(int mouseX, int mouseY, int button, bool down, int modifiers);
        virtual void onMouseUpEvent(int mouseX, int mouseY, int button, int modifiers);
        virtual void onMouseUpOutsideEvent(int mouseX, int mouseY, int button, int modifiers);
        virtual void onMouseDownEvent(int mouseX, int mouseY, int button, int modifiers);
        virtual void onClickEvent();
        virtual void onMouseMoveEvent(int mouseX, int mouseY, int button, int modifiers);
        virtual void onMouseScrollEvent(int mouseX, int mouseY, double scrollX, double scrollY);

        ClickCallback       _onClick{nullptr};
        MouseCallback       _onMouseMove{nullptr};
        MouseCallback       _onMouseOver{nullptr};
        MouseCallback       _onMouseOut{nullptr};
        MouseButtonCallback _onMouseButton{nullptr};
        MouseCallback       _onMouseDown{nullptr};
        MouseCallback       _onMouseUp{nullptr};
        MouseCallback       _onMouseUpOutside{nullptr};
        MouseScrollCallback _onMouseScroll{nullptr};


        // endregion



        bool _focused{false};
        void focused(bool focused);

    private:
        static const InheritableValues _inheritableValues;
        void addedToRenderRecursive();
        void removedFromRenderRecursive();
    };
}

