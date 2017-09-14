#pragma once

#include <memory>
#include <functional>

template<class T>
class Hatcher {
    public:
        static std::shared_ptr<Hatcher<T>> make(std::function<T()> callback) {
            return std::make_shared<Hatcher<T>>(callback);
        }

        explicit Hatcher(std::function<T()> callback) : _callback(callback) {}

        T hatch() {
            return _callback ? _callback() : nullptr;
        }

    protected:
        std::function<T()> _callback{nullptr};
    };