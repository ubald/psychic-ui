#pragma once

#include <memory>
#include <utility>
#include <vector>
#include "Signal.hpp"
#include "Slot.hpp"

namespace psychic_ui {

    /**
     * Extend from Observer and use `subscribeTo` in order to remove subscriptions automatically.
     * Useful in cases when the subclass' life is shorter than that of the signal. When the signal dies first, the
     * subscription simply ends; disconnecting it later is a no-op.
     */
    class Observer {
    public:
        Observer() = default;
        Observer(const Observer &) = delete;
        Observer &operator=(const Observer &) = delete;
        Observer(Observer &&) = delete;
        Observer &operator=(Observer &&) = delete;

        ~Observer() {
            // Disconnecting can release callbacks, whose captures could call back into us: the slots move to a local
            // list first, which disconnects them as it dies
            auto toDisconnect = std::move(slots);
        }

    protected:
        std::vector<ScopedSlot> slots{};

        /**
         * Subscribe this observer to a signal, disconnected when the observer is destroyed
         * The returned slot can disconnect it earlier.
         */
        template<class... T, class F>
        std::shared_ptr<Slot<T...>> subscribeTo(Signal<T...> &signal, F &&callback) {
            // Forget the slots that ended on their own (signal destroyed, or disconnected directly)
            std::erase_if(slots, [](const ScopedSlot &slot) { return !slot.connected(); });
            auto slot = signal.subscribe(std::forward<F>(callback));
            slots.emplace_back(slot);
            return slot;
        }
    };

}
