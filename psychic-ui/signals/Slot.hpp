#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace psychic_ui {

    template<class... T>
    class Signal;

    template<class... T>
    class Slot;

    namespace detail {
        /**
         * What a Signal shares with its slots and with its running emissions.
         * Slots only hold a weak_ptr to it, so disconnecting after the signal died is a no-op.
         * An emission holds a shared_ptr to it, so the slots it iterates outlive a handler that destroys the signal.
         */
        template<class... T>
        struct SignalCore {
            std::vector<std::shared_ptr<Slot<T...>>> slots{};
            unsigned int                             emitting{0}; // Nested emissions in progress
            bool                                     dirty{false}; // Some slots were disconnected

            /**
             * Drop the disconnected slots, unless an emission is iterating over them.
             */
            void compact() {
                if (emitting > 0 || !dirty) {
                    return;
                }
                dirty = false;
                // Build the new list before releasing anything: releasing a callback runs the destructors of what
                // it captured, which can disconnect other slots of this signal.
                auto removed = std::exchange(slots, {});
                slots.reserve(removed.size());
                for (auto &slot: removed) {
                    if (slot->_connected) {
                        slots.push_back(std::move(slot));
                    }
                }
                for (auto &slot: removed) {
                    if (slot) {
                        slot->_callback = nullptr; // A handle can outlive the slot; don't let it keep the captures
                    }
                }
            }
        };
    }

    /**
     * Type-erased slot, so that a ScopedSlot can hold a slot of any signal.
     */
    class SlotBase {
    public:
        SlotBase() = default;
        SlotBase(const SlotBase &) = delete;
        SlotBase &operator=(const SlotBase &) = delete;
        SlotBase(SlotBase &&) = delete;
        SlotBase &operator=(SlotBase &&) = delete;
        virtual ~SlotBase() = default;

        /**
         * Disconnects this slot from its signal.
         * Safe to call more than once, from inside the callback, and after the signal was destroyed.
         */
        virtual void disconnect() = 0;

        /**
         * Whether the callback will still be called when the signal is emitted
         */
        [[nodiscard]] virtual bool connected() const = 0;
    };

    /**
     * Subscription to a signal, returned by `Signal::subscribe`.
     *
     * The signal keeps its slots alive, so dropping the returned pointer does not disconnect: keep it in a ScopedSlot
     * to disconnect later, or extend from Observer and use `subscribeTo` to disconnect when the observer dies.
     */
    template<class... T>
    class Slot final : public SlotBase {
    public:
        using Callback = std::function<void(const T &...)>;

        Slot(std::weak_ptr<detail::SignalCore<T...>> core, Callback callback) :
            _core(std::move(core)),
            _callback(std::move(callback)) {}

        // NOLINTNEXTLINE(portability-template-virtual-member-function): it compiles for every T, whether instantiated or not
        void disconnect() override {
            if (!_connected) {
                return;
            }
            _connected = false;
            // The callback is released by the signal once no emission runs it: it may be the one calling us
            if (auto core = _core.lock()) {
                core->dirty = true;
                core->compact();
            }
        }

        // NOLINTNEXTLINE(portability-template-virtual-member-function): it compiles for every T, whether instantiated or not
        [[nodiscard]] bool connected() const override {
            return _connected;
        }

    private:
        friend class Signal<T...>;
        friend struct detail::SignalCore<T...>;

        std::weak_ptr<detail::SignalCore<T...>> _core;
        Callback                                _callback;
        bool                                    _connected{true};
    };

    /**
     * Owning slot handle: disconnects when destroyed or when another slot is assigned to it.
     * Use it for a subscription you replace or drop, typically to a signal that outlives you:
     *
     *     ScopedSlot _onMouseMove;
     *     _onMouseMove = window()->mouseMoved.subscribe([this](...) { ... }); // disconnects the previous one first
     */
    class ScopedSlot {
    public:
        ScopedSlot() = default;

        template<class S>
        requires std::derived_from<S, SlotBase>
        ScopedSlot(std::shared_ptr<S> slot) : // NOLINT(google-explicit-constructor): assigned from subscribe()
            _slot(std::move(slot)) {}

        ScopedSlot(const ScopedSlot &) = delete;
        ScopedSlot &operator=(const ScopedSlot &) = delete;

        ScopedSlot(ScopedSlot &&other) noexcept :
            _slot(std::move(other._slot)) {}

        ScopedSlot &operator=(ScopedSlot &&other) noexcept {
            if (this != &other) {
                disconnect();
                _slot = std::move(other._slot);
            }
            return *this;
        }

        ~ScopedSlot() {
            disconnect();
        }

        void disconnect() {
            if (auto slot = std::move(_slot)) {
                slot->disconnect();
            }
        }

        [[nodiscard]] bool connected() const {
            return _slot && _slot->connected();
        }

        explicit operator bool() const {
            return connected();
        }

    private:
        std::shared_ptr<SlotBase> _slot{};
    };

}
