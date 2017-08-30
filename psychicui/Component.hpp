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
        virtual std::shared_ptr<Panel> panel();
        std::vector<std::shared_ptr<Component>> path();

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

        void setPosition(int x, int y);
        int x() const;
        void setX(int x);
        int y() const;
        void setY(int y);

        // endregion

        // region Dimensions

        void setSize(int width, int height);
        int width() const;
        void setWidth(int width);
        int height() const;
        void setHeight(int height);

//        void setMinSize(float minWidth, float minHeight);
//        float minWidth() const;
//        void setMinWidth(float minWidth);
//        float minHeight() const;
//        void setMinHeight(float minHeight);
//
//        void setMaxSize(float maxWidth, float maxHeight);
//        float maxWidth() const;
//        void setMaxWidth(float maxWidth);
//        float maxHeight() const;
//        void setMaxHeight(float maxHeight);

        // endregion

        // region Constraints

//        void setPadding(int value = 0);
//        void setPadding(int horizontal, int vertical);
//        void setPadding(int left, int right, int top, int bottom);
//        int paddingLeft() const;
//        void setPaddingLeft(int paddingLeft = 0);
//        int paddingRight() const;
//        void setPaddingRight(int paddingRight = 0);
//        int paddingTop() const;
//        void setPaddingTop(int paddingTop = 0);
//        int paddingBottom() const;
//        void setPaddingBottom(int paddingBottom = 0);

        // endregion

        // region Layout

        bool wrap() const;
        void setWrap(bool wrap);

        float flex() const;
        void setFlex(float flex);

        float flexShrink() const;
        void setFlexShrink(float flexShrink);

        float flexGrow() const;
        void setFlexGrow(float flexGrow);

        float flexBasis() const;
        void setFlexBasis(float flexBasis);

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
        bool mouseDown() const;

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
        std::unique_ptr<Style>                  _style{nullptr};

        /**
         * Computed Style
         */
        std::unique_ptr<Style>                  _computedStyle{nullptr};

        /**
         * Compute style for this component
         */
        void updateStyle();

        /**
         * Callback for when styles were updated
         */
        virtual void styleUpdated();

        // endregion

        // region Rendering

        SkPaint paint;

        // endregion

        bool   _enabled{true};
        bool   _visible{true};
        int    _x{0};
        int    _y{0};
        int    _width{0};
        int    _height{0};
//        int    _paddingLeft{0};
//        int    _paddingRight{0};
//        int    _paddingTop{0};
//        int    _paddingBottom{0};
        bool   _wrap{false};
        SkRect _rect;

        bool                                    _focused{false};
        bool                                    _mouseOver{false};
        bool                                    _mouseDown{false};



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

