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


    template<typename T> std::shared_ptr<T> component() {
        return std::make_shared<T>();
    }

    using ComponentCreator = std::function<std::shared_ptr<Component>()>;


    class Component : public std::enable_shared_from_this<Component> {
        friend class StyleManager;

    public:
        Component();
        virtual ~Component();

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

        template<typename WidgetClass, typename... Args>
        std::shared_ptr<WidgetClass> add(const Args&... args) {
            return std::static_pointer_cast<WidgetClass>(add(std::make_shared<WidgetClass>(args...)));
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
        virtual bool active() const;

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

        // region Layout

        void setMeasurable();

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
        const std::vector<std::string> &classNames() const;
        Component *setClassNames(std::vector<std::string> additionalClassNames);

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

//        std::shared_ptr<Component> findComponent(const int &x, const int &y);
        inline bool contains(const int &x, const int &y) const;


        // Events
        virtual bool mouseDragEvent(const int &mouseX, const int &mouseY, const int &dragX, const int &dragY, int button, int modifiers);
        virtual bool focusEvent(bool focused);
        virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);
        virtual bool keyboardCharacterEvent(unsigned int codepoint);


    protected:

        // region Lifecycle

        // endregion

        // region Hierarchy

        int                                     _depth{0};
        Component                               *_parent{nullptr};
        std::vector<std::shared_ptr<Component>> _children;

        // endregion

        // region Style

        /**
         * Set the component tag, it wil be appended to _componentTag in order to get a type hierarchy for styling.
         * Obviously this should be static and/or use Reflection but in order to progress in other parts of the library
         * it will stay local for the time being, until a better technique is implemented.
         * @param componentName
         */
        void setTag(std::string componentName);
        std::vector<std::string>                _tags{};

        /**
         * Pseudo CSS class names
         */
        std::vector<std::string> _classNames{};

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

        // region Rendering

        bool    _drawBackground{false};
        SkPaint _backgroundPaint;
        bool    _drawBorder{false};
        SkPaint _borderPaint;

        // endregion

        bool _enabled{true};
        bool _visible{true};
        int  _x{0};
        int  _y{0};
        int  _width{0};
        int  _height{0};
        bool _wrap{false};

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

        /**
         * Component's border rect
         */
        SkRect _borderValues;

        // endregion

        bool _focused{false};

        // region Mouse

        Cursor _cursor{Cursor::Arrow};
        bool   _mouseOver{false};
        bool   _mouseDown{false};

        Cursor mouseMoved(const int &mouseX, const int &mouseY, int button, int modifiers);
        bool mouseButton(const int &mouseX, const int &mouseY, int button, bool down, int modifiers);
        bool mouseScrolled(const int &mouseX, const int &mouseY, const int &scrollX, const int &scrollY);

        std::function<void(const int &mouseX, const int &mouseY, int button, int modifiers)> _onMouseMoved{nullptr};

        std::function<void()> _onMouseOver{nullptr};
        std::function<void()>_onMouseOut{nullptr};
        std::function<void(const int &mouseX, const int &mouseY, int button, bool down, int modifiers)> _onMouseButton{nullptr};
        std::function<void()> _onMouseDown{nullptr};
        std::function<void()> _onClick{nullptr};
        std::function<void()>_onMouseUp{nullptr};
        std::function<void()> _onMouseUpOutside{nullptr};
        std::function<void(const int &mouseX, const int &mouseY, const int &scrollX, const int &scrollY)> _onMouseScrolled{nullptr};


        // endregion




        void focused(bool focused);


    };
}

