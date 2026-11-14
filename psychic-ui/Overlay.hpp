#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "include/core/SkRect.h"
#include "Div.hpp"
#include "signals/Signal.hpp"

namespace psychic_ui {

    class Overlay;

    /**
     * The side of its anchor a popup goes on
     */
    enum class Side : std::uint8_t { Bottom, Top, Right, Left };

    /**
     * Where a popup lines up along its anchor's side: its start (left or top), center or end
     */
    enum class PopupAlign : std::uint8_t { Start, Center, End };

    /**
     * Where a popup goes, and on which side it ended up
     */
    struct Placement {
        float x{0};
        float y{0};
        Side  side{Side::Bottom};
    };

    /**
     * Place a popup of this size beside an anchor (both in the same coordinates), inside `bounds`: on `side`, unless it
     * doesn't fit there and the opposite side fits or has more room (it flips), `sideOffset` away from the anchor;
     * along the side, aligned by `align` (`alignOffset` further along it from the start or toward the start from the
     * end), then shifted to stay inside the bounds. A popup larger than the bounds starts at their start.
     */
    Placement placePopup(const SkRect &anchor, float width, float height, const SkRect &bounds,
                         Side side, PopupAlign align, float sideOffset = 0, float alignOffset = 0);

    /**
     * @class Popup
     *
     * A layer of the window's overlay (`Window::overlay()`), drawn above the content: a menu, a select's list, a
     * tooltip, a dialog, the toasts. Open one with `overlay.open<T>(args...)`; it stays until `close()`.
     *
     * - Placed beside an anchor (`anchor(div)`, or `anchor(x, y)` for a point in window coordinates) on a side
     *   (`side`, `alignment`, `sideOffset`, `alignOffset`), flipping to the other side and shifting along it to stay
     *   `collisionPadding` inside the window, every frame (it follows its anchor); `placedSide()` is where it went.
     *   Without an anchor it stays where its style puts it in the window-sized overlay.
     * - `modal(true)`: the content under it (and the popups under it) get no mouse input, except its pass-through
     *   divs (`passThrough(div)`, like a menu bar while one of its menus is open).
     * - Dismissed by a press outside its surface (`dismissOnOutsidePress`, the default) and by Escape
     *   (`dismissOnEscape`, the default); a press that dismisses a modal popup goes no further.
     * - `trapFocus(true)`: Tab stays among its divs. When it closes with the focus inside it, the focus goes back to
     *   what had it when it opened (`restoreFocus`, the default).
     * - A popup opened from another (`owner(popup)`, a submenu) closes with it.
     *
     * Its surface (`surface()`, itself by default) is the part that counts as inside: a dialog's panel, not its
     * backdrop.
     */
    class Popup : public Div {
    public:
        Popup();

        // region Placement

        /**
         * Place it beside a div (held weakly: the popup closes when the div leaves the window)
         */
        template<class Self>
        Self &anchor(this Self &self, Div &anchor) {
            static_cast<Popup &>(self).updateAnchor(&anchor, 0, 0);
            return self;
        }

        /**
         * Place it at a point, in window coordinates (a context menu)
         */
        template<class Self>
        Self &anchor(this Self &self, int x, int y) {
            static_cast<Popup &>(self).updateAnchor(nullptr, x, y);
            return self;
        }

        Div *anchorDiv() const { return _anchor.lock().get(); }
        bool anchored() const { return _anchored; }

        Side side() const { return _side; }

        template<class Self>
        Self &side(this Self &self, Side side) {
            static_cast<Popup &>(self)._side = side;
            return self;
        }

        PopupAlign alignment() const { return _align; }

        template<class Self>
        Self &alignment(this Self &self, PopupAlign align) {
            static_cast<Popup &>(self)._align = align;
            return self;
        }

        /**
         * How far from the anchor, away from it
         */
        template<class Self>
        Self &sideOffset(this Self &self, detail::FloatArg offset) {
            static_cast<Popup &>(self)._sideOffset = offset.value;
            return self;
        }

        /**
         * How far along the anchor's side from where `alignment` puts it (negative: before the anchor's start)
         */
        template<class Self>
        Self &alignOffset(this Self &self, detail::FloatArg offset) {
            static_cast<Popup &>(self)._alignOffset = offset.value;
            return self;
        }

        /**
         * How far from the window's edges it stays
         */
        template<class Self>
        Self &collisionPadding(this Self &self, detail::FloatArg padding) {
            static_cast<Popup &>(self)._collisionPadding = padding.value;
            return self;
        }

        /**
         * The side it went on at its last placement, after flipping
         */
        Side placedSide() const { return _placedSide; }

        /**
         * Where its anchor was at its last placement, in the overlay's coordinates
         */
        const SkRect &anchorRect() const { return _anchorRect; }

        // endregion

        // region Behavior

        bool modal() const { return _modal; }

        template<class Self>
        Self &modal(this Self &self, detail::BoolArg modal) {
            static_cast<Popup &>(self)._modal = modal.value;
            return self;
        }

        bool dismissOnOutsidePress() const { return _dismissOnOutsidePress; }

        template<class Self>
        Self &dismissOnOutsidePress(this Self &self, detail::BoolArg dismiss) {
            static_cast<Popup &>(self)._dismissOnOutsidePress = dismiss.value;
            return self;
        }

        bool dismissOnEscape() const { return _dismissOnEscape; }

        template<class Self>
        Self &dismissOnEscape(this Self &self, detail::BoolArg dismiss) {
            static_cast<Popup &>(self)._dismissOnEscape = dismiss.value;
            return self;
        }

        bool trapFocus() const { return _trapFocus; }

        template<class Self>
        Self &trapFocus(this Self &self, detail::BoolArg trap) {
            static_cast<Popup &>(self)._trapFocus = trap.value;
            return self;
        }

        bool restoreFocus() const { return _restoreFocus; }

        template<class Self>
        Self &restoreFocus(this Self &self, detail::BoolArg restore) {
            static_cast<Popup &>(self)._restoreFocus = restore.value;
            return self;
        }

        /**
         * A div under it that still gets the mouse while it is open (held weakly)
         */
        template<class Self>
        Self &passThrough(this Self &self, Div &div) {
            static_cast<Popup &>(self)._passThrough.push_back(div.weak_from_this());
            return self;
        }

        /**
         * The popup it was opened from, which closes it when it closes
         */
        Popup *owner() const { return _owner; }

        template<class Self>
        Self &owner(this Self &self, Popup &owner) {
            static_cast<Popup &>(self)._owner = &owner;
            return self;
        }

        /**
         * The part that counts as inside it (itself unless a subclass says otherwise)
         */
        Div &surface() const { return _surface ? *_surface : const_cast<Popup &>(*this); }

        // endregion

        /**
         * Whether it is in an overlay
         */
        bool isOpen() const;

        /**
         * Close it, the popups it owns first; a no-op when it is closed. It is removed from the overlay: keep a
         * shared_ptr to reuse it.
         */
        void close();

        /**
         * After it closed (and the focus went back)
         */
        Signal<> closed{};

        PSYCHIC_UI_EVENT(onClose, closed)

    protected:
        friend class Overlay;

        std::weak_ptr<Div> _anchor{};
        bool               _anchored{false};
        bool               _anchorIsDiv{false};
        int                _anchorX{0};
        int                _anchorY{0};
        Side               _side{Side::Bottom};
        PopupAlign         _align{PopupAlign::Start};
        float              _sideOffset{0};
        float              _alignOffset{0};
        float              _collisionPadding{12};
        Side               _placedSide{Side::Bottom};
        SkRect             _anchorRect{};

        bool _modal{false};
        bool _dismissOnOutsidePress{true};
        bool _dismissOnEscape{true};
        bool _trapFocus{false};
        bool _restoreFocus{true};

        std::vector<std::weak_ptr<Div>> _passThrough{};
        Popup                           *_owner{nullptr};
        Div                             *_surface{nullptr}; // A descendant, or nullptr for itself
        std::weak_ptr<Div>              _focusBefore{};     // What had the focus when it opened

        void updateAnchor(Div *anchor, int x, int y);

        /**
         * Called by the overlay once it is in it, after the focus it had was noted: take the focus (`opened`)
         */
        virtual void opened() {}

        /**
         * Advance what goes on with time (a toast's timer), every frame from `Window::updatePopups`
         */
        virtual void update(std::chrono::nanoseconds /*now*/) {}
    };

    /**
     * @class Overlay
     *
     * Where the window's popups live: a child of the window above its content, filling it, whose children are the
     * popups (`Popup`), the last on top. It only takes the mouse where a popup is, or everywhere under a modal popup.
     * See `Popup`.
     */
    class Overlay : public Div {
    public:
        Overlay();

        /**
         * Open a popup: construct it, add it on top, then let it take the focus
         */
        template<class T, typename... Args>
        T &open(Args &&... args) {
            static_assert(std::is_base_of_v<Popup, T>, "T must extend Popup");
            return static_cast<T &>(open(std::make_shared<T>(std::forward<Args>(args)...)));
        }

        /**
         * Open an existing popup (on top again if it was open)
         */
        Popup &open(const std::shared_ptr<Popup> &popup);

        /**
         * The open popups, bottom first
         */
        std::vector<Popup *> popups() const;

        /**
         * The topmost popup that is a T, or nullptr
         */
        template<class T>
        T *top() const {
            for (auto it = _children.rbegin(); it != _children.rend(); ++it) {
                if (auto popup = dynamic_cast<T *>(it->get())) {
                    return popup;
                }
            }
            return nullptr;
        }

        /**
         * Close every popup that is a T
         */
        template<class T>
        void closeAll() {
            auto children = _children; // Closing one closes those it owns, and frees them
            for (auto &child: children) {
                auto *popup = static_cast<Popup *>(child.get());
                if (dynamic_cast<T *>(popup) && popup->isOpen()) {
                    popup->close();
                }
            }
        }

        /**
         * Place the anchored popups beside their anchors (closing those whose anchor left the window)
         * @return Whether one moved, so that the layout must run again
         */
        bool place();

        /**
         * A press at this point (window coordinates) is about to be dispatched: close the popups it is outside of
         * that a press outside dismisses, topmost first, until one it is inside of, or a modal one that stays
         * @return Whether it closed a modal popup without landing in another popup: the press goes no further
         */
        bool pressOutside(int x, int y);

        /**
         * Escape was pressed: close the topmost popup Escape dismisses
         * @return Whether one closed
         */
        bool dismissTop();

        /**
         * Whether a modal popup is open, which takes the mouse from what is under it
         */
        bool modal() const;

        /**
         * Where Tab moves the focus: the topmost popup that traps it, or nullptr
         */
        Popup *focusScope() const;

        /**
         * The topmost popup whose surface takes the mouse at this point (window coordinates), or nullptr
         */
        Popup *popupAt(int x, int y) const;

        void update(std::chrono::nanoseconds now);

        MouseEventStatus handleMouseMove(const MouseEvent &event, bool handled) override;
        MouseEventStatus handleMouseButtonChange(const MouseEvent &event) override;
        MouseEventStatus handleMousePress(const MouseEvent &event) override;
        MouseEventStatus handleClick(const MouseEvent &event) override;
        MouseEventStatus handleDoubleClick(const MouseEvent &event) override;
        MouseEventStatus handleMouseScroll(const MouseEvent &event) override;
        MouseEventStatus handleContextMenuRequest(const MouseEvent &event) override;

    protected:
        friend class Popup;

        void close(Popup &popup);

        /**
         * Whether a point (window coordinates) is on a popup's surface
         */
        static bool surfaceContains(const Popup &popup, int x, int y);

        /**
         * Send a mouse event to the popups, topmost first, until one takes it; under a modal popup, to its
         * pass-through divs and no further
         */
        template<class F>
        MouseEventStatus dispatch(const MouseEvent &event, F &&handler);
    };
}
