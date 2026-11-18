#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
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
     *   (`dismissOnEscape`, the default); a press that dismisses a modal popup goes no further, except a right
     *   press outside a popup that `passesContextPress` (a menu), which goes on, so that another context menu opens
     *   there, as on macOS and Windows.
     * - `trapFocus(true)`: Tab stays among its divs. When it closes with the focus inside it, the focus goes back to
     *   what had it when it opened (`restoreFocus`, the default).
     * - A popup opened from another (`owner(popup)`, a submenu) closes with it.
     * - It enters and leaves with transitions: it has the `entering` class for its first style
     *   (`Div::startingClass`), so that its rules' transitions start from there, and its placed side as a class
     *   (`side-bottom`, `side-top`, `side-right`, `side-left`) with its transform origin at its anchor once anchored.
     *   `close()` gives it the `exiting` class (`Div::exit`): it is closed at once (not `isOpen`, not modal, no mouse,
     *   no focus, `closed` emitted) but stays in the overlay until its and its descendants' transitions end; opening
     *   it again meanwhile brings it back from where it is. With reduced motion it is removed at once.
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

        /**
         * Whether it takes the mouse from what is under it: while it is open, not while it leaves
         */
        bool modal() const { return _modal && !exiting(); }

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

        /**
         * Whether a right press outside it that closes it goes on to what is under it (false), so that a context
         * menu opens there: a menu's (true)
         */
        bool passesContextPress() const { return _passesContextPress; }

        template<class Self>
        Self &passesContextPress(this Self &self, detail::BoolArg passes) {
            static_cast<Popup &>(self)._passesContextPress = passes.value;
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
        Div &surface() {
            if (_surface) {
                return *_surface;
            }
            return *this;
        }

        const Div &surface() const {
            if (_surface) {
                return *_surface;
            }
            return *this;
        }

        // endregion

        /**
         * Whether it is in an overlay and not leaving it
         */
        bool isOpen() const;

        /**
         * Close it, the popups it owns first; a no-op when it is closed (or leaving). It leaves the overlay once its
         * exit transitions end (at once with reduced motion): keep a shared_ptr to reuse it.
         */
        void close();

        /**
         * When it closed (and the focus went back), as its exit starts: it may stay in the overlay a moment longer,
         * fading out (its `removed()` is when it leaves)
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
        bool _passesContextPress{false};
        bool _trapFocus{false};
        bool _restoreFocus{true};

        std::vector<std::weak_ptr<Div>> _passThrough{};
        Overlay                         *_overlay{nullptr}; // The overlay it was opened in, while it is there
        Popup                           *_owner{nullptr};
        Div                             *_surface{nullptr}; // A descendant, or nullptr for itself
        std::weak_ptr<Div>              _focusBefore{};     // What had the focus when it opened
        bool                            _laidOut{false};    // `laidOut` was called since it opened

        void updateAnchor(Div *anchor, int x, int y);

        /**
         * It leaves its overlay (closed and gone, or taken out by other means): the overlay forgets it. A subclass
         * that overrides it calls it.
         */
        void removed() override;

        /**
         * It went on this side: `placedSide()`, and its class (`side-bottom`...)
         */
        void placed(Side side);

        /**
         * Called by the overlay once it is in it, after the focus it had was noted: take the focus (`opened`)
         */
        virtual void opened() {}

        /**
         * Called once, after the first layout since it opened (`Overlay::place`), when what was added to it after
         * `open` is styled and laid out: a dialog focuses its first field
         */
        virtual void laidOut() {}

        /**
         * Advance what goes on with time (a toast's timer), every frame from `Window::updatePopups`
         */
        virtual void update(std::chrono::nanoseconds /*now*/) {}

        /**
         * When `update` has something to do next (a toast's timeout), on the same clock; nullopt when nothing waits
         */
        virtual std::optional<std::chrono::nanoseconds> nextUpdate() const { return std::nullopt; }
    };
}
