#pragma once

#include <iostream>

#include <vector>
#include <unordered_set>
#include <yoga/Yoga.h>
#include <SkCanvas.h>
#include <SkPaint.h>
#include <SkRRect.h>
#include "psychicui.hpp"
#include "psychicui/style/Style.hpp"
#include "psychicui/style/StyleManager.hpp"

namespace psychicui {

    class Window;

    class Panel;


    class Hatcher {
    public:
        explicit Hatcher(std::function<std::shared_ptr<Component>()> callback) : _callback(callback) {}

        std::shared_ptr<Component> hatch() {
            return _callback ? _callback() : nullptr;
        }

    protected:
        std::function<std::shared_ptr<Component>()> _callback{nullptr};
    };

//    template<typename T> std::shared_ptr<T> component() {
//        return std::make_shared<T>();
//    }
//
//    using ComponentCreator = std::function<std::shared_ptr<Component>()>;

    using ClickCallback = std::function<void()>;
    using MouseCallback = std::function<void(const int mouseX, const int mouseY, int button, int modifiers)>;
    using MouseButtonCallback = std::function<void(const int mouseX, const int mouseY, int button, bool down, int modifiers)>;
    using MouseScrollCallback = std::function<void(const int mouseX, const int mouseY, const int scrollX, const int scrollY)>;


    class Component : public std::enable_shared_from_this<Component> {
        friend class StyleManager;

    public:
        Component();
        ~Component();

        // region Hierarchy

        virtual Window *window();
        Component *parent();
        const Component *parent() const;
        void setParent(Component *parent);
        const int depth() const;
//        std::vector<std::shared_ptr<Component>> path();

        virtual std::shared_ptr<Panel> panel();

        // endregion

        // region Children

        unsigned int childCount() const;
        const std::vector<std::shared_ptr<Component>> children() const;
        std::shared_ptr<Component> add(unsigned int index, std::shared_ptr<Component> component);
        std::shared_ptr<Component> add(std::shared_ptr<Component> component);

        template<typename T, typename... Args>
        std::shared_ptr<T> add(const Args &... args) {
            return std::static_pointer_cast<T>(add(std::make_shared<T>(args...)));
        }

        void remove(unsigned int index);
        void remove(const std::shared_ptr<Component> component);
        const Component *at(unsigned int index) const;
        Component *at(unsigned int index);
        int childIndex(std::shared_ptr<Component> component) const;

        // endregion

        // region Visibility, Focus & State

        bool visible() const;
        virtual void setVisible(bool value);
        bool focused() const;
        void requestFocus();
        virtual void requestFocus(Component *component);
        virtual const bool active() const;

        // endregion

        // region Position

        void setPosition(int x, int y);
        void setPosition(int left, int top, int right, int bottom);
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

        void setMeasurable();

        #ifdef DEBUG_LAYOUT
        static bool debugLayout;
        #endif

        // endregion

        // region Style

        YGNodeRef yogaNode();

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
        Component *setClassNames(std::unordered_set<std::string> additionalClassNames);
        Component *addClassName(std::string className);
        Component *removeClassName(std::string className);

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
        bool mouseOver() const;
        void setMouseOver(bool over); // Mostly for testing
        bool mouseDown() const;
        void setMouseDown(bool down); // Mostly for testing

        Component *onMouseDown(MouseCallback mouseDown) {
            _onMouseDown = mouseDown;
            return this;
        }

        Component *onMouseUp(MouseCallback mouseUp) {
            _onMouseUp = mouseUp;
            return this;
        }

        Component *onMouseUpOutside(MouseCallback mouseUpOutside) {
            _onMouseUpOutside = mouseUpOutside;
            return this;
        }

        Component *onClick(ClickCallback click) {
            _onClick = click;
            return this;
        }

        Component *onMouseMove(MouseCallback mouseMove) {
            _onMouseMove = mouseMove;
            return this;
        }

        Component *onMouseOver(MouseCallback mouseOver) {
            _onMouseOver = mouseOver;
            return this;
        }

        Component *onMouseOut(MouseCallback mouseOut) {
            _onMouseOut = mouseOut;
            return this;
        }

        Component *onMouseScrolled(MouseScrollCallback mouseScrolled) {
            _onMouseScroll = mouseScrolled;
            return this;
        }

        Component *onMouseButton(MouseButtonCallback mouseButton) {
            _onMouseButton = mouseButton;
            return this;
        }

        // endregion

//        std::shared_ptr<Component> findComponent(const int x, const int y);
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

        int                                     _depth{0};
        Component                               *_parent{nullptr};
        std::vector<std::shared_ptr<Component>> _children{};

        // endregion

        // region Style

        /**
         * Set the component tag, it wil be appended to _componentTag in order to get a type hierarchy for styling.
         * Obviously this should be static and/or use Reflection but in order to progress in other parts of the library
         * it will stay local for the time being, until a better technique is implemented.
         * @param componentName
         */
        Component *setTag(std::string componentName);
        std::vector<std::string>                _tags{};

        /**
         * Pseudo CSS class names
         */
        std::unordered_set<std::string> _classNames{};

        /**
         * Default Style
         * Use this instead of the inline style when a component needs a specific default for it to work correctly
         * For example, text components should not grow/shrink as they are inline, but yoga doesn't let us do that.
         */
        std::unique_ptr<Style> _defaults{nullptr};

        /**
         * Inline Style
         */
        std::unique_ptr<Style> _style{nullptr};

        /**
         * Dirty style flag
         */
        bool _styleDirty{true};

        /**
         * Style Manager Override
         */
        std::shared_ptr<StyleManager> _styleManager{nullptr};

        /**
         * Computed Style
         */
        std::unique_ptr<Style> _computedStyle{nullptr};

        /**
         * Invalidate the style
         */
        void invalidateStyle();

        /**
         * Callback for when styles were updated
         */
        virtual void styleUpdated();

        /**
         * Callback for when layout was updated
         */
        virtual void layoutUpdated();

        // endregion

        // region Layout

        YGNodeRef _yogaNode{nullptr};

        /**
         * Component's rect
         */
        SkRect _rect;

        /**
         * Component's padded rect
         */
        SkRect _paddedRect;

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




        bool _focused{false};

        // region Mouse

        Cursor _cursor{Cursor::Arrow};
        bool   _mouseOver{false};
        bool   _mouseDown{false};

        Cursor mouseMoved(int mouseX, int mouseY, int button, int modifiers);
        bool mouseButton(int mouseX, int mouseY, int button, bool down, int modifiers);
        bool mouseScrolled(int mouseX, int mouseY, int scrollX, int scrollY);

        virtual void onMouseButton(int mouseX, int mouseY, int button, bool down, int modifiers);
        virtual void onMouseUp(int mouseX, int mouseY, int button, int modifiers);
        virtual void onMouseUpOutside(int mouseX, int mouseY, int button, int modifiers);
        virtual void onMouseDown(int mouseX, int mouseY, int button, int modifiers);
        virtual void onClick();

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




        void focused(bool focused);


    };
}

