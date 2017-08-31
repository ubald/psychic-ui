#pragma once

#include <vector>
#include <yoga/Yoga.h>
#include <SkCanvas.h>
#include <SkPaint.h>
#include "psychicui.hpp"
#include "psychicui/style/Style.hpp"
#include "psychicui/style/StyleManager.hpp"


namespace psychicui {
    class Window;

    class Panel;

    class Component : public std::enable_shared_from_this<Component> {
        friend class StyleManager;

    public:
        Component();
        virtual ~Component();

        // region Hierarchy

        virtual StyleManager *styleManager() const;
        virtual const Window *window() const;
        Component *parent();
        const Component *parent() const;
        void setParent(Component *parent);
        const int depth() const;
        std::vector<std::shared_ptr<Component>> path();

        virtual std::shared_ptr<Panel> panel();

        // endregion

        // region Children

        unsigned int childCount() const;
        const std::vector<std::shared_ptr<Component>> children() const;
        void addChild(unsigned int index, std::shared_ptr<Component> component);
        void addChild(std::shared_ptr<Component> component);
        void removeChild(unsigned int index);
        void removeChild(const std::shared_ptr<Component> component);
        const std::shared_ptr<Component> childAt(unsigned int index) const;
        std::shared_ptr<Component> childAt(unsigned int index);
        int childIndex(std::shared_ptr<Component> component) const;

        // endregion

        template<typename ComponentClass, typename... Args>
        std::shared_ptr<ComponentClass> add(const Args &... args) {
            return std::make_shared<ComponentClass>(this, args...);
        }

        // region Visibility & Focus

        bool visible() const;
        virtual void setVisible(bool value);
        bool visibleRecursive() const;
        bool focused() const;
        void requestFocus();
        virtual void requestFocus(Component *component);

        // endregion

        // region Position

        void setPosition(float x, float y);
        void setPosition(float left, float top, float right, float bottom);
        float x() const;
        void setX(float x);
        float y() const;
        void setY(float y);
        float left() const;
        void setLeft(float left);
        float right() const;
        void setRight(float right);
        float top() const;
        void setTop(float top);
        float bottom() const;
        void setBottom(float bottom);

        // endregion

        // region Dimensions

        void setSize(float width, float height);
        float width() const;
        void setWidth(float width);
        float height() const;
        void setHeight(float height);
        float percentWidth() const;
        void setPercentWidth(float percentWidth);
        float percentHeight() const;
        void setPercentHeight(float percentHeight);

        // endregion

        // region Style

        YGNodeRef yogaNode();

        /**
         * Get the inline style
         * @return Style*
         */
        Style *style() const;

        /**
         * Get the computed style
         * @return
         */
        const Style *computedStyle() const;

        const std::vector<std::string> &tags() const;
        const std::vector<std::string> &classNames() const;
        void setClassNames(std::vector<std::string> additionalClassNames);


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

        // endregion

        std::shared_ptr<Component> findComponent(const int &x, const int &y);
        bool contains(const int &x, const int &y) const;

        void setMeasurable();

        // Events
        virtual bool mouseDragEvent(const int &mouseX, const int &mouseY, const int &dragX, const int &dragY, int button, int modifiers);
        virtual bool focusEvent(bool focused);
        virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);
        virtual bool keyboardCharacterEvent(unsigned int codepoint);


    protected:

        // region Hierarchy

        int                      _depth{0};

        // endregion

        // region Style

        /**
         * Set the component tag, it wil be appended to _componentTag in order to get a type hierarchy for styling.
         * Obviously this should be static and/or use Reflection but in order to progress in other parts of the library
         * it will stay local for the time being, until a better technique is implemented.
         * @param componentName
         */
        void setTag(std::string componentName);
        std::vector<std::string> _tags{};

        /**
         * Pseudo CSS class names
         */
        std::vector<std::string> _classNames{};

        /**
         * Inline Style
         */
        std::unique_ptr<Style> _style{nullptr};

        /**
         * Computed Style
         */
        std::unique_ptr<Style> _computedStyle{nullptr};

        /**
         * Compute style for this component
         */
        void updateStyle();

        /**
         * Callback for when styles were updated
         */
        virtual void styleUpdated();

        /**
         * Callback for when layout was updated
         */
        virtual void layoutUpdated();

        // endregion

        // region Rendering

        bool    _drawBackground{false};
        SkPaint _backgroundPaint;
        bool    _drawBorder{false};
        SkPaint _borderPaint;

        // endregion

        bool  _enabled{true};
        bool  _visible{true};
        float _x{0.0f};
        float _y{0.0f};
        float _width{0.0f};
        float _height{0.0f};
        bool  _wrap{false};

        /**
         * Component's rect
         */
        SkRect _rect;

        /**
         * Component's padded rect
         */
        SkRect _paddedRect;

        /**
         * Component's border rect
         */
        SkRect _borderValues;

        bool _focused{false};
        bool _mouseOver{false};
        bool _mouseDown{false};


        Cursor                                  _cursor{Cursor::Arrow};
        Component                               *_parent{nullptr};
        std::vector<std::shared_ptr<Component>> _children;

        YGNodeRef _yogaNode{nullptr};

        void focused(bool focused);
        void mouseMovedPropagation(const int &mouseX, const int &mouseY, int button, int modifiers);
        bool mouseScrolledPropagation(const int &mouseX, const int &mouseY, const int &scrollX, const int &scrollY);
        bool mouseButtonPropagation(const int &mouseX, const int &mouseY, int button, bool down, int modifiers);
        virtual void mouseMoved(const int &mouseX, const int &mouseY, int button, int modifiers);
        virtual void onMouseOver();
        virtual void onMouseOut();
        virtual void onMouseButton(const int &mouseX, const int &mouseY, int button, bool down, int modifiers);
        virtual void onMouseDown();
        virtual void onMouseUp();
        virtual void onMouseScrolled(const int &mouseX, const int &mouseY, const int &scrollX, const int &scrollY);
    };
}

