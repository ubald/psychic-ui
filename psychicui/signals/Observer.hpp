#pragma once

#include <functional>
#include <vector>
#include "Signal.hpp"
#include "Slot.hpp"

namespace psychicui {

    /**
     * Used to stop type deduction in observer::subscribeTo's callback
     * It can be deduced from the signal but was chocking on the callback lambda
     */
    template<typename... T>
    struct Identity {
        typedef std::function<void(T...)> type;
    };

    /**
     * Extend from Observer and use `subscribeTo` in order to remove subscriptions automatically.
     * Useful in cases when the subclass' life is shorter than that of the signal.
     */
    class Observer {
    public:
        Observer() = default;

        ~Observer() {
            for (auto &slot: slots) {
                slot->disconnect();
            }
            slots.clear();
        }

    protected:
        std::vector<std::shared_ptr<SlotBase>> slots{};

        /**
         * Subscribe this observer to a signal
         * Use this method when you know the the signal will outlive you.
         */
        template<class... T>
        void subscribeTo(Signal<T...> &signal, typename Identity<T...>::type &&callback) {
            slots.push_back(signal.subscribe(std::forward<std::function<void(T...)>>(callback)));
        }
    };

}