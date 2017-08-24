#pragma once

#include <vector>
#include "psychicui.hpp"
#include "Layout.hpp"
#include "Style.hpp"

namespace psychicui {
    class Widget {
    public:
        Widget(Widget *parent);

        Widget *parent();
        const Widget *parent() const;
        void setParent(Widget *parent);

        int childCount() const;
        const std::vector<Widget *> &children() const;

        virtual void addChild(int index, Widget *widget);
        void addChild(Widget *widget);
        void removeChild(int index);
        void removeChild(const Widget *widget);
        const Widget *childAt(int index) const;
        Widget *childAt(int index);
        int childIndex(Widget *widget) const;

        template<typename WidgetClass, typename... Args>
        WidgetClass *add(const Args &... args) {
            return new WidgetClass(this, args...);
        }

        bool visible() const;
        bool visibleRecursive() const;
        bool focused() const;
        void setFocused(bool focused);
        void requestFocus();

        virtual void setVisible(bool visible);

        const Vector2i &position() const;
        void setPosition(const Vector2i &position);
        const Vector2i absolutePosition() const;
        int x() const;
        void setX(int x);
        int y() const;
        void setY(int y);

        const Vector2i &size() const;
        void setSize(const Vector2i &size);
        int width() const;
        void setWidth(int width);
        int height() const;
        void setHeight(int height);

        void setFixedSize(const Vector2i &fixedSize);
        const Vector2i &fixedSize() const;
        int fixedWidth() const;
        int fixedHeight() const;
        void setFixedWidth(int width);
        void setFixedHeight(int height);

        Cursor cursor() const;
        void setCursor(Cursor cursor);

        Widget *findWidget(const Vector2i &p);
        bool contains(const Vector2i &p) const;

        Style *style();
        const Style *style() const;
        void setStyle(Style *style);

        Layout *layout();
        const Layout *layout() const;
        void setLayout(Layout *layout);

        virtual Vector2i preferredSize(NVGcontext *ctx) const;
        virtual void performLayout(NVGcontext *ctx);
        virtual void draw(NVGcontext *ctx);

        // Events
        virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);
        virtual bool focusEvent(bool focused);
        virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);
        virtual bool keyboardCharacterEvent(unsigned int codepoint);


    protected:
        virtual ~Widget();
        bool                  _enabled{true};
        bool                  _visible{true};
        Vector2i              _position{0, 0};
        Vector2i              _size{0, 0};
        Vector2i              _fixedSize{0, 0};
        bool                  _focused{false};
        bool                  _mouseOver{false};
        Style                 *_style{nullptr};
        Cursor                _cursor{Cursor::Arrow};
        Layout                *_layout{nullptr};
        Widget                *_parent{nullptr};
        std::vector<Widget *> _children;

        void mouseMovedPropagation(const Vector2i &p, int button, int modifiers);
        bool mouseScrolledPropagation(const Vector2i &p, const Vector2f &rel);
        bool mouseButtonPropagation(const Vector2i &p, int button, bool down, int modifiers);
        virtual void mouseMoved(const Vector2i &p, int button, int modifiers);
        virtual void mouseOver();
        virtual void mouseOut();
        virtual void mouseButton(const Vector2i &p, int button, bool down, int modifiers);
        virtual void mouseScrolled(const Vector2i &p, const Vector2f &rel);
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
}

