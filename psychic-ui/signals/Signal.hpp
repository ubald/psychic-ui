#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include "Slot.hpp"

namespace psychic_ui {

    /**
     * Signal
     * Create a signal with template arguments matching the arguments required for `emit` and parameters that will be
     * used in the callback/lambda. Callbacks receive the arguments as `const T &`, or take no parameters to ignore them.
     *
     * Handlers may connect, disconnect (themselves or others) and destroy the signal while it is emitting:
     * - a slot connected during an emission is first called by the next emission;
     * - a slot disconnected during an emission is not called anymore, even by the running one;
     * - when a handler destroys the signal, the emission stops after that handler.
     *
     * Signals can't be copied or moved: their slots and the handlers' captures refer to them.
     */
    template<class... T>
    class Signal {
    public:
        using SlotPtr = std::shared_ptr<Slot<T...>>;

        Signal() = default;
        Signal(const Signal &) = delete;
        Signal &operator=(const Signal &) = delete;

        ~Signal() {
            if (_core) {
                _core->alive = false;
            }
        }

        /**
         * Check if the signal has any subscriptions
         */
        bool hasSubscriptions() const {
            return _core && std::any_of(_core->slots.cbegin(), _core->slots.cend(), [](const SlotPtr &slot) { return slot->_connected; });
        }

        /**
         * Get the number of subscriptions
         */
        std::size_t subscriptionCount() const {
            return _core ? std::count_if(_core->slots.cbegin(), _core->slots.cend(), [](const SlotPtr &slot) { return slot->_connected; }) : 0;
        }

        /**
         * Subscribe to this signal
         * The signal keeps the slot: keep the returned pointer only to disconnect later. When the callback refers to
         * something that can die before the signal, extend from Observer and use `subscribeTo`, or keep the slot in a
         * ScopedSlot.
         */
        template<class F>
        requires std::invocable<F &, const T &...> || std::invocable<F &>
        SlotPtr subscribe(F &&callback) {
            if (!_core) {
                _core = std::make_shared<detail::SignalCore<T...>>();
            }
            SlotPtr slot;
            if constexpr (std::invocable<F &, const T &...>) {
                slot = std::make_shared<Slot<T...>>(_core, std::forward<F>(callback));
            } else {
                // A callback that takes nothing ignores the arguments: `onClick([] { ... })`
                slot = std::make_shared<Slot<T...>>(
                    _core, [callback = std::forward<F>(callback)](const T &...) mutable { callback(); }
                );
            }
            _core->slots.push_back(slot);
            return slot;
        }

        /**
         * Subscription () operator, same as `subscribe`
         */
        template<class F>
        requires std::invocable<F &, const T &...> || std::invocable<F &>
        SlotPtr operator()(F &&callback) {
            return subscribe(std::forward<F>(callback));
        }

        /**
         * Unsubscribe a slot from this signal, same as `slot->disconnect()`
         */
        void unsubscribe(const SlotPtr &slot) {
            if (slot && _core && slot->_core.lock() == _core) {
                slot->disconnect();
            }
        }

        /**
         * Emit a signal
         * @param args Arguments matching the types used as template arguments, converted once for all the slots
         */
        void emit(const T &... args) {
            if (!_core) {
                return;
            }
            // From here on, only use locals: a handler can destroy this signal
            std::shared_ptr<detail::SignalCore<T...>> core = _core;
            Emitting emitting{*core};
            // By index over a snapshot of the size: slots connected by a handler wait for the next emission
            for (std::size_t i = 0, n = core->slots.size(); i < n; ++i) {
                // The core keeps the slot, and only compacts once no emission runs, so the callback stays valid even
                // if the slot disconnects itself
                Slot<T...> &slot = *core->slots[i];
                if (!slot._connected) {
                    continue;
                }
                slot._callback(args...);
                if (!core->alive) {
                    return;
                }
            }
        }

        /**
         * Emit a signal () operator, same as `emit`
         */
        void operator()(const T &... args) {
            emit(args...);
        }

    private:
        struct Emitting {
            detail::SignalCore<T...> &core;

            explicit Emitting(detail::SignalCore<T...> &core) : core(core) {
                ++core.emitting;
            }

            ~Emitting() {
                --core.emitting;
                core.compact();
            }
        };

        /**
         * Created on the first subscription, most signals never get one
         */
        std::shared_ptr<detail::SignalCore<T...>> _core{};
    };

}
