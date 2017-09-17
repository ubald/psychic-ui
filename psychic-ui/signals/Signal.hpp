#pragma once

#include <algorithm>
#include <functional>
#include <vector>
#include "Slot.hpp"

namespace psychic_ui {

    /**
     * Signal
     * Create a signal with template arguments matching the arguments required for `emit` and parameters that will be
     * used in the callback/lambda.
     */

    template<class... T>
    class Signal {
    public:

        /**
         * Check if the signal has any subscriptions
         */
        bool hasSubscriptions() {
            return !slots.empty();
        }

        /**
         * Get the number of subscriptions
         */
        std::size_t subscriptionCount() {
            return slots.size();
        }

        /**
         * Subscribe to this signal
         * Use this method when you know that you will outlive the signal, otherwise, you have to keep the returned
         * shared pointer and unsubscribe manually. For automatic cleanup, extend from Observer and use its methods.
         */
        std::shared_ptr<Slot<T...>> subscribe(std::function<void(T...)> &&callback);

        /**
         * Unsubscribe from this signal
         */
        void unsubscribe(std::shared_ptr<Slot<T...>> slot);
        void unsubscribe(Slot<T...> *slot);

        /**
         * Emit a signal
         * @param args Arguments matching the types used as template arguments
         */
        void emit(T &... args);

        /**
         * Emit a signal () operator
         * @param args Arguments matching the types used as template arguments
         */
        void operator()(T &... args) {
//            emit(std::forward<T...>(args)...);
            emit(args...);
        }

        /**
         * Subscription () operator
         * Use this method when you know that you will outlive the signal, otherwise, you have to keep the returned
         * shared pointer and unsubscribe manually. For automatic cleanup, extend from Observer and use its methods.
         */
        std::shared_ptr<Slot<T...>> operator()(std::function<void(T...)> &&callback) {
            return subscribe(std::forward<std::function<void(T...)>>(callback));
        }

    protected:
        std::vector<std::shared_ptr<Slot<T...>>> slots{};
    };

    template<class... T>
    std::shared_ptr<Slot<T...>> Signal<T...>::subscribe(std::function<void(T...)> &&callback) {
        auto slot = std::make_shared<Slot<T...>>(this, std::forward<std::function<void(T...)>>(callback));
        slots.push_back(slot);
        return slot;
    }

    template<class... T>
    void Signal<T...>::unsubscribe(std::shared_ptr<Slot<T...>> slot) {
        slots.erase(std::remove(slots.begin(), slots.end(), slot), slots.end());
    }

    template<class... T>
    void Signal<T...>::unsubscribe(Slot<T...> *slot) {
        slots.erase(
            std::remove_if(
                slots.begin(),
                slots.end(),
                [slot](const auto &s) {
                    return s.get() == slot;
                }
            ),
            slots.end()
        );
    }

    template<class... T>
    void Signal<T...>::emit(T &... args) {
        try {
            for (auto &slot: slots) {
//                slot->notify(std::forward<T...>(args)...);
                slot->notify(args...);
            }
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in signal subscriber: " << e.what() << std::endl;
        }
    }

}
