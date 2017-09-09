#pragma once

#include <functional>

namespace psychicui {

    template<class... T>
    class Signal;

    /**
     * Base Subscription, just so that we can store it in the Observer
     */
    class SlotBase {
    public:
        virtual void disconnect() = 0;
    };

    /**
     * Subscription for a signal, this is used to wrap a callback in a shared/weak pointer.
     */
    template<class... T>
    class Slot : public SlotBase {
    public:
        explicit Slot(Signal<T...> *signal, std::function<void(T...)> callback) :
            _signal(signal),
            notify(callback) {}

        void disconnect() override {
            _signal->unsubscribe(this);
        }

        std::function<void(T...)> notify;
    private:
        Signal<T...> *_signal;
    };

}