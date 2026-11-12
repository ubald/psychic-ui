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

        ~Observer() {
            // Disconnecting can release callbacks, whose captures could call back into us: work on a local list
            auto toDisconnect = std::move(slots);
            for (auto &slot: toDisconnect) {
                slot->disconnect();
            }
        }

    protected:
        std::vector<std::shared_ptr<SlotBase>> slots{};

        /**
         * Subscribe this observer to a signal, disconnected when the observer is destroyed
         */
        template<class... T, class F>
        std::shared_ptr<Slot<T...>> subscribeTo(Signal<T...> &signal, F &&callback) {
            // Forget the slots that ended on their own (signal destroyed, or disconnected directly)
            std::erase_if(slots, [](const std::shared_ptr<SlotBase> &slot) { return !slot->connected(); });
            auto slot = signal.subscribe(std::forward<F>(callback));
            slots.push_back(slot);
            return slot;
        }

        /**
         * Disconnect a slot obtained from `subscribeTo`, ignores null
         */
        void unsubscribeFrom(const std::shared_ptr<SlotBase> &slot) {
            if (!slot) {
                return;
            }
            slot->disconnect();
            std::erase(slots, slot);
        }
    };

}
