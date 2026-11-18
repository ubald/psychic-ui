#pragma once

#include <chrono>
#include <optional>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>
#include "include/core/SkRect.h"
#include "Div.hpp"
#include "Popup.hpp"

namespace psychic_ui {

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
         * Open an existing popup (on top again if it was open; back from where it is if it was leaving)
         */
        Popup &open(const std::shared_ptr<Popup> &popup);

        /**
         * The open popups, bottom first (not those leaving)
         */
        std::vector<Popup *> popups() const;

        /**
         * Every popup opened in it, bottom first, those leaving too: its children, typed
         */
        const std::vector<std::shared_ptr<Popup>> &allPopups() const { return _popups; }

        /**
         * The topmost open popup that is a T, or nullptr
         */
        template<class T>
        T *top() const {
            for (const auto &popup: std::views::reverse(_popups)) {
                if (auto *found = dynamic_cast<T *>(popup.get()); found && !found->exiting()) {
                    return found;
                }
            }
            return nullptr;
        }

        /**
         * Close every popup that is a T
         */
        template<class T>
        void closeAll() {
            const auto popups = _popups; // Closing one closes those it owns, and frees them
            for (const auto &popup: popups) {
                if (dynamic_cast<T *>(popup.get()) && popup->isOpen()) {
                    popup->close();
                }
            }
        }

        /**
         * Place the anchored popups beside their anchors (closing those whose anchor left the window; those leaving
         * stay where they are), then call `laidOut` on those laid out for the first time
         * @return Whether one moved, so that the layout must run again
         */
        bool place();

        /**
         * A press at this point (window coordinates) is about to be dispatched: close the popups it is outside of
         * that a press outside dismisses, topmost first, until one it is inside of, or a modal one that stays
         * @param contextPress A right press: closing a popup that `passesContextPress` doesn't stop it
         * @return Whether it closed a modal popup without landing in another popup: the press goes no further
         */
        bool pressOutside(int x, int y, bool contextPress = false);

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

        /**
         * The soonest of the popups' `nextUpdate`
         */
        std::optional<std::chrono::nanoseconds> nextUpdate() const;

        MouseEventStatus handleMouseMove(const MouseEvent &event, bool handled) override;
        MouseEventStatus handleMouseButtonChange(const MouseEvent &event) override;
        MouseEventStatus handleMousePress(const MouseEvent &event) override;
        MouseEventStatus handleClick(const MouseEvent &event) override;
        MouseEventStatus handleDoubleClick(const MouseEvent &event) override;
        MouseEventStatus handleMouseScroll(const MouseEvent &event) override;
        MouseEventStatus handleContextMenuRequest(const MouseEvent &event) override;

    protected:
        friend class Popup;

        /**
         * The popups opened in it, in its children's order, those leaving too: a popup is in it from `open` until
         * it leaves the overlay (`Popup::removed`)
         */
        std::vector<std::shared_ptr<Popup>> _popups{};

        void close(Popup &popup);

        /**
         * A popup left the overlay
         */
        void forget(const Popup &popup);

        /**
         * Put an open popup on top without taking it out (it keeps its style and transitions)
         */
        void raise(Popup &popup);

        /**
         * Whether a point (window coordinates) is on a popup's surface
         */
        static bool surfaceContains(const Popup &popup, int x, int y);

        /**
         * Send a mouse event to the popups, topmost first, until one takes it; under a modal popup, to its
         * pass-through divs and no further
         */
        template<class F>
        MouseEventStatus dispatch(const MouseEvent &event, const F &handler);
    };
}
