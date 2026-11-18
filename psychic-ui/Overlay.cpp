#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <ranges>
#include <vector>
#include "include/core/SkRect.h"
#include "yoga/YGNode.h"
#include "Div.hpp"
#include "Overlay.hpp"
#include "Popup.hpp"
#include "Window.hpp"
#include "psychic-ui.hpp"
#include "style/Length.hpp"
#include "style/Values.hpp"

namespace psychic_ui {

    // region Placement

    namespace {
        bool vertical(const Side side) {
            return side == Side::Bottom || side == Side::Top;
        }

        Side opposite(const Side side) {
            switch (side) {
                case Side::Bottom: return Side::Top;
                case Side::Top: return Side::Bottom;
                case Side::Right: return Side::Left;
                case Side::Left: return Side::Right;
            }
            return side;
        }

        /**
         * Keep a span of this size inside [start, end): at the start when it is larger
         */
        float shift(const float position, const float size, const float start, const float end) {
            return std::max(start, std::min(position, end - size));
        }
    }

    Placement placePopup(const SkRect &anchor, const float width, const float height, const SkRect &bounds,
                         const Side side, const PopupAlign align, const float sideOffset, const float alignOffset) {
        // Where it goes on a side, along the axis away from the anchor, and the room that side has
        const auto mainPosition = [&](const Side s) {
            switch (s) {
                case Side::Bottom: return anchor.bottom() + sideOffset;
                case Side::Top: return anchor.top() - sideOffset - height;
                case Side::Right: return anchor.right() + sideOffset;
                case Side::Left: return anchor.left() - sideOffset - width;
            }
            return 0.0f;
        };
        const auto room = [&](const Side s) {
            switch (s) {
                case Side::Bottom: return bounds.bottom() - anchor.bottom() - sideOffset;
                case Side::Top: return anchor.top() - sideOffset - bounds.top();
                case Side::Right: return bounds.right() - anchor.right() - sideOffset;
                case Side::Left: return anchor.left() - sideOffset - bounds.left();
            }
            return 0.0f;
        };
        const auto fits = [&](const Side s) {
            return room(s) >= (vertical(s) ? height : width);
        };

        Placement placement{};
        placement.side = side;
        if (!fits(side) && (fits(opposite(side)) || room(opposite(side)) > room(side))) {
            placement.side = opposite(side);
        }

        const float main = mainPosition(placement.side);
        if (vertical(placement.side)) {
            float x = 0;
            switch (align) {
                case PopupAlign::Start: x = anchor.left() + alignOffset; break;
                case PopupAlign::Center: x = anchor.centerX() - (width / 2.0f) + alignOffset; break;
                case PopupAlign::End: x = anchor.right() - width - alignOffset; break;
            }
            placement.x = shift(x, width, bounds.left(), bounds.right());
            placement.y = shift(main, height, bounds.top(), bounds.bottom());
        } else {
            float y = 0;
            switch (align) {
                case PopupAlign::Start: y = anchor.top() + alignOffset; break;
                case PopupAlign::Center: y = anchor.centerY() - (height / 2.0f) + alignOffset; break;
                case PopupAlign::End: y = anchor.bottom() - height - alignOffset; break;
            }
            placement.x = shift(main, width, bounds.left(), bounds.right());
            placement.y = shift(y, height, bounds.top(), bounds.bottom());
        }
        return placement;
    }

    // endregion

    // region Overlay

    Overlay::Overlay() :
        Div() {
        tag("Overlay");
        _defaultStyle.position(Position::Absolute)
            .left(0)
            .top(0)
            .width(100_pct)
            .height(100_pct);
    }

    Popup &Overlay::open(const std::shared_ptr<Popup> &popup) {
        if (popup->_parent == this && popup->exiting()) {
            // Back before it left: it stays, on top, and its transitions head back from where they are
            popup->cancelExit();
            raise(*popup);
            const Window *w = window();
            Div    *focused = w ? w->focusedDiv() : nullptr;
            popup->_focusBefore = focused ? focused->weak_from_this() : std::weak_ptr<Div>{};
            popup->opened();
            return *popup;
        }
        if (popup->_parent == this) {
            remove(*popup); // On top again
        } else if (popup->isOpen()) {
            popup->close();
        }
        const Window *w = window();
        Div    *focused = w ? w->focusedDiv() : nullptr;
        popup->_focusBefore = focused ? focused->weak_from_this() : std::weak_ptr<Div>{};
        add(popup); // Tracked by childAdded
        popup->_laidOut = false;
        popup->opened();
        return *popup;
    }

    void Overlay::raise(Popup &popup) {
        const int index = childIndex(&popup);
        if (index < 0 || index == static_cast<int>(_children.size()) - 1) {
            return;
        }
        invalidateEndChildrenStyle();
        auto child = _children[static_cast<std::size_t>(index)];
        _children.erase(_children.begin() + index);
        _children.push_back(child);
        YGNodeRemoveChild(_yogaNode, popup._yogaNode);
        YGNodeInsertChild(_yogaNode, popup._yogaNode, YGNodeGetChildCount(_yogaNode));
        invalidateEndChildrenStyle();
        if (const auto it = std::ranges::find(_popups, &popup, &std::shared_ptr<Popup>::get); it != _popups.end()) {
            std::ranges::rotate(it, it + 1, _popups.end());
        }
    }

    Overlay::~Overlay() {
        for (const auto &popup: _popups) {
            popup->_overlay = nullptr; // One held elsewhere outlives it
        }
    }

    void Overlay::childAdded(Div &child) {
        Div::childAdded(child);
        if (Popup *popup = trackChild(_popups, child)) {
            popup->_overlay = this;
        }
    }

    void Overlay::childRemoved(Div &child) {
        if (Popup *popup = untrackChild(_popups, child)) {
            popup->_overlay = nullptr;
        }
        Div::childRemoved(child);
    }

    std::vector<Popup *> Overlay::popups() const {
        std::vector<Popup *> list{};
        list.reserve(_popups.size());
        for (const auto &popup: _popups) {
            if (!popup->exiting()) {
                list.push_back(popup.get());
            }
        }
        return list;
    }

    void Overlay::close(Popup &popup) {
        const auto keepAlive = popup.shared_from_this();

        // What it opened first, topmost first
        const auto popups = _popups;
        for (const auto &other: std::views::reverse(popups)) {
            if (other->_owner == &popup && other->isOpen()) {
                other->close();
            }
        }
        if (popup._parent != this || popup.exiting()) {
            return; // Closed meanwhile
        }

        // The focus leaves it now, while it is still there to fade out: back to what had it before, or to the
        // overlay, as a press there would
        Window *w = window();
        if (w && popup.hasFocus()) {
            auto       before = popup._focusBefore.lock();
            const auto inside = [&popup](const Div *div) {
                for (; div; div = div->parent()) {
                    if (div == &popup) {
                        return true;
                    }
                }
                return false;
            };
            if (popup._restoreFocus && before && before->window() == w && !inside(before.get())) {
                w->requestFocus(before.get(), w->focusVisible());
            } else {
                w->requestFocus(this);
            }
        }

        // It leaves once its transitions are over (at once with reduced motion)
        popup.exit("exiting", [this, &popup]() {
            for (const auto &other: _popups) {
                if (other->_owner == &popup) {
                    other->_owner = nullptr; // Its owner goes
                }
            }
            if (popup._parent == this) {
                remove(popup);
            }
        });
        popup.closed.emit();
    }

    bool Overlay::surfaceContains(const Popup &popup, const int x, const int y) {
        const Div &surface = popup.surface();
        int       localX   = 0;
        int       localY   = 0;
        surface.globalToLocal(localX, localY, x, y);
        // globalToLocal counts the surface's own scroll, which a hit test doesn't
        localX += surface.scrollX();
        localY += surface.scrollY();
        return localX >= 0 && localY >= 0 && localX < surface.getWidth() && localY < surface.getHeight();
    }

    namespace {
        bool onPassThrough(const std::vector<std::weak_ptr<Div>> &divs, const int x, const int y) {
            for (const auto &weak: divs) {
                if (auto div = weak.lock(); div && div->visible()) {
                    int localX = 0;
                    int localY = 0;
                    div->globalToLocal(localX, localY, x, y);
                    localX += div->scrollX();
                    localY += div->scrollY();
                    if (localX >= 0 && localY >= 0 && localX < div->getWidth() && localY < div->getHeight()) {
                        return true;
                    }
                }
            }
            return false;
        }
    }

    bool Overlay::place() {
        bool moved   = false;
        int  originX = 0;
        int  originY = 0;
        localToGlobal(originX, originY);
        const SkRect bounds = SkRect::MakeWH(static_cast<float>(_width), static_cast<float>(_height));

        auto popups = _popups; // Closing one changes them
        for (const auto &popup: popups) {
            if (!popup->_anchored || popup->_parent != this || popup->exiting()) {
                continue; // Leaving: it stays where it was
            }
            SkRect anchor{};
            if (popup->_anchorIsDiv) {
                auto div = popup->_anchor.lock();
                if (!div || div->window() != window()) {
                    popup->close(); // Its anchor left
                    continue;
                }
                int x = 0;
                int y = 0;
                div->localToGlobal(x, y);
                x -= div->scrollX(); // localToGlobal counts the div's own scroll
                y -= div->scrollY();
                anchor = SkRect::MakeXYWH(
                    static_cast<float>(x - originX), static_cast<float>(y - originY),
                    static_cast<float>(div->getWidth()), static_cast<float>(div->getHeight())
                );
            } else {
                anchor = SkRect::MakeXYWH(static_cast<float>(popup->_anchorX - originX), static_cast<float>(popup->_anchorY - originY), 0, 0);
            }

            const float     padding   = popup->_collisionPadding;
            const Placement placement = placePopup(
                anchor,
                static_cast<float>(popup->getWidth()), static_cast<float>(popup->getHeight()),
                bounds.makeInset(padding, padding),
                popup->_side, popup->_align, popup->_sideOffset, popup->_alignOffset
            );
            popup->placed(placement.side);
            popup->_anchorRect = anchor;
            const auto x = static_cast<int>(std::round(placement.x));
            const auto y = static_cast<int>(std::round(placement.y));
            if (popup->x() != x || popup->y() != y || popup->style().left() != Length{static_cast<float>(x)}
                || popup->style().top() != Length{static_cast<float>(y)}) {
                popup->left(static_cast<float>(x)).top(static_cast<float>(y));
                moved = true;
            }

            // It grows from its anchor (Base UI's --transform-origin): the anchor's middle, clamped to its edge
            // facing the anchor
            const auto  width   = static_cast<float>(popup->getWidth());
            const auto  height  = static_cast<float>(popup->getHeight());
            float       originX = 0;
            float       originY = 0;
            switch (placement.side) {
                case Side::Bottom:
                case Side::Top:
                    originX = std::clamp(anchor.centerX() - static_cast<float>(x), 0.0f, width);
                    originY = placement.side == Side::Bottom ? 0.0f : height;
                    break;
                case Side::Right:
                case Side::Left:
                    originX = placement.side == Side::Right ? 0.0f : width;
                    originY = std::clamp(anchor.centerY() - static_cast<float>(y), 0.0f, height);
                    break;
            }
            if (popup->style().transformOriginX() != Length{originX} || popup->style().transformOriginY() != Length{originY}) {
                popup->transformOriginX(originX).transformOriginY(originY);
            }
        }

        // The popups laid out for the first time since they opened
        popups = _popups;
        for (const auto &popup: popups) {
            if (!popup->_laidOut && popup->_parent == this && !popup->exiting()) {
                popup->_laidOut = true;
                popup->laidOut();
            }
        }
        return moved;
    }

    bool Overlay::pressOutside(const int x, const int y, const bool contextPress) {
        bool closedModal = false;
        const auto popups = _popups;
        for (const auto &popup: std::views::reverse(popups)) {
            if (popup->_parent != this || popup->exiting()) {
                continue; // Closed with its owner, or leaving
            }
            if (popup->visible() && popup->mouseEnabled() && surfaceContains(*popup, x, y)) {
                return false;
            }
            if (onPassThrough(popup->_passThrough, x, y)) {
                return false;
            }
            if (popup->_dismissOnOutsidePress) {
                closedModal = closedModal || (popup->_modal && !(contextPress && popup->_passesContextPress));
                popup->close();
            } else if (popup->_modal) {
                return false;
            }
        }
        return closedModal;
    }

    bool Overlay::dismissTop() {
        const auto popups = _popups;
        for (const auto &popup: std::views::reverse(popups)) {
            if (popup->_parent != this || popup->exiting()) {
                continue;
            }
            if (popup->_dismissOnEscape) {
                popup->close();
                return true;
            }
            if (popup->modal()) {
                return false;
            }
        }
        return false;
    }

    bool Overlay::modal() const {
        return std::ranges::any_of(_popups, [](const auto &popup) { return popup->modal(); });
    }

    Popup *Overlay::focusScope() const {
        for (const auto &popup: std::views::reverse(_popups)) {
            if (popup->_trapFocus && !popup->exiting()) {
                return popup.get();
            }
        }
        return nullptr;
    }

    Popup *Overlay::popupAt(const int x, const int y) const {
        for (const auto &popup: std::views::reverse(_popups)) {
            if (popup->visible() && popup->mouseEnabled() && !popup->exiting() && surfaceContains(*popup, x, y)) {
                return popup.get();
            }
        }
        return nullptr;
    }

    void Overlay::update(const std::chrono::nanoseconds now) {
        const auto popups = _popups; // A toast's timer closes it
        for (const auto &popup: popups) {
            if (popup->parent() == this && !popup->exiting()) {
                popup->update(now);
            }
        }
    }

    std::optional<std::chrono::nanoseconds> Overlay::nextUpdate() const {
        std::optional<std::chrono::nanoseconds> next{};
        for (const auto &popup: _popups) {
            if (popup->exiting()) {
                continue;
            }
            const auto update = popup->nextUpdate();
            if (update && (!next || *update < *next)) {
                next = update;
            }
        }
        return next;
    }

    // region Mouse

    namespace {
        /**
         * The event at this point (in the receiving div's parent's coordinates)
         */
        MouseEvent movedTo(const MouseEvent &event, const float x, const float y) {
            MouseEvent moved = event;
            moved.x          = x;
            moved.y          = y;
            return moved;
        }

        /**
         * The event in the coordinates of a div's parent (its content's, scrolled), from its window coordinates
         */
        MouseEvent inParentOf(const Div &div, const MouseEvent &event) {
            int originX = 0;
            int originY = 0;
            div.parent()->localToGlobal(originX, originY);
            return movedTo(
                event,
                event.windowX - static_cast<float>(originX - div.parent()->scrollX()),
                event.windowY - static_cast<float>(originY - div.parent()->scrollY())
            );
        }
    }

    template<class F>
    MouseEventStatus Overlay::dispatch(const MouseEvent &event, const F &handler) {
        const auto keepAlive = weak_from_this().lock(); // A handler may close popups
        if (!_visible) {
            return MouseEventStatus::Out;
        }
        const MouseEvent local = movedTo(
            event,
            event.x - static_cast<float>(_x + _scrollX),
            event.y - static_cast<float>(_y + _scrollY)
        );
        const auto popups = _popups; // A handler may close popups
        for (const auto &popup: std::views::reverse(popups)) {
            if (popup->parent() != this) {
                continue; // Closed by an earlier handler
            }
            if (popup->mouseEnabled() || popup->mouseChildren()) {
                auto res = handler(*popup, local);
                if (res != MouseEventStatus::Out) {
                    return res;
                }
            }
            if (popup->modal()) {
                for (auto &weak: std::vector(popup->_passThrough)) {
                    auto div = weak.lock();
                    if (!div || !div->parent()) {
                        continue;
                    }
                    auto res = handler(*div, inParentOf(*div, event));
                    if (res != MouseEventStatus::Out) {
                        return res;
                    }
                }
                return MouseEventStatus::Handled; // Nothing under it gets it
            }
        }
        return MouseEventStatus::Out;
    }

    MouseEventStatus Overlay::handleMouseMove(const MouseEvent &event, bool handled) {
        const auto keepAlive = weak_from_this().lock();
        if (!_visible) {
            return MouseEventStatus::Out;
        }
        const MouseEvent local = movedTo(
            event,
            event.x - static_cast<float>(_x + _scrollX),
            event.y - static_cast<float>(_y + _scrollY)
        );
        MouseEventStatus ret     = MouseEventStatus::Out;
        bool             blocked = false;
        const auto       move    = [&](Div &div, const MouseEvent &moved) {
            auto res = div.handleMouseMove(moved, handled);
            if (res != MouseEventStatus::Out) {
                handled = true;
                if (ret != MouseEventStatus::Handled) {
                    ret = res;
                }
            }
        };
        const auto popups = _popups;
        for (const auto &popup: std::views::reverse(popups)) {
            if (popup->parent() != this) {
                continue;
            }
            if (!popup->mouseEnabled() && !popup->mouseChildren()) {
                continue;
            }
            move(*popup, local);
            if (popup->modal() && !blocked) {
                for (auto &weak: std::vector(popup->_passThrough)) {
                    auto div = weak.lock();
                    if (div && div->parent()) {
                        move(*div, inParentOf(*div, event));
                    }
                }
                // What is under it still hears the mouse leave, never enter
                blocked = true;
                handled = true;
            }
        }
        return blocked ? MouseEventStatus::Handled : ret;
    }

    MouseEventStatus Overlay::handleMouseButtonChange(const MouseEvent &event) {
        return dispatch(event, [](Div &div, const MouseEvent &e) { return div.handleMouseButtonChange(e); });
    }

    MouseEventStatus Overlay::handleMousePress(const MouseEvent &event) {
        return dispatch(event, [](Div &div, const MouseEvent &e) { return div.handleMousePress(e); });
    }

    MouseEventStatus Overlay::handleClick(const MouseEvent &event) {
        return dispatch(event, [](Div &div, const MouseEvent &e) { return div.handleClick(e); });
    }

    MouseEventStatus Overlay::handleDoubleClick(const MouseEvent &event) {
        return dispatch(event, [](Div &div, const MouseEvent &e) { return div.handleDoubleClick(e); });
    }

    MouseEventStatus Overlay::handleMouseScroll(const MouseEvent &event) {
        return dispatch(event, [](Div &div, const MouseEvent &e) { return div.handleMouseScroll(e); });
    }

    MouseEventStatus Overlay::handleContextMenuRequest(const MouseEvent &event) {
        return dispatch(event, [](Div &div, const MouseEvent &e) { return div.handleContextMenuRequest(e); });
    }

    // endregion

    // endregion
}
