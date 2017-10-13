#pragma once

#include <functional>

namespace psychic_ui {

    template<class... T>
    class Signal;

    /**
     * Base Subscription, just so that we can store it in the Observer
     */
    class SlotBase {
    public:
        /**
         * Disconnects this slot, removing it from the signal
         */
        virtual void disconnect() = 0;
    };

    /**
     * Subscription for a signal
     * This is used in the Signal class to keep a list of callbacks to execute
     * when the signal is emitted and also to hold as a shared_ptr in an Observer
     * derived class in order to automatically clean subscriptions in scenarios where
     * the Observer has a shorter life span than the Signal source. When a Signal
     * source has a shorter life, there is no need to hold a pointer to the slot as
     * the Signal will be destroyed first, thus destroying the slots and their callbacks.
     *
     * When keeping a pointer to a slot for manual disconnection, type the Slot template
     * with the same type parameters as the Signal. For example, a `Signal<float, int>` will
     * return a `shared_ptr<Slot<float, int>>` when subscribing.
     */
    template<class... T>
    class Slot : public SlotBase {
    public:
        /**
         * Creates a slot from a signal and callback
         * The pointer to the signal is used in order to be able to disconnect
         * the slot from its `disconnect` method.
         *
         * You should not have to create slots manually, this is called
         * by the Signal class when subscribing.
         *
         * @param signal Signal that this slot is attached to
         * @param callback Callback to execute when the signal is emited
         */
        explicit Slot(Signal<T...> *signal, std::function<void(T...)> callback) :
            notify(callback),
            _signal(signal) {}

        /**
         * Disconnect the slot
         * This will unsubscribe from the signal, effectively removing this
         * slot from the vector in the Signal instance.
         */
        void disconnect() override {
            _signal->unsubscribe(this);
            _signal = nullptr;
        }

        /**
         * Function to ba called when the signal is emitted.
         * This should not be called manually, it is called by
         * the Signal class
         */
        std::function<void(T...)> notify{nullptr};

    private:
        /**
         * Signal that owns this slot
         * This pointer is used to allow disconnecting from the slot.
         */
        Signal<T...> *_signal;
    };

}
