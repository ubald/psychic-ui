#pragma once

#include <array>
#include <cstddef>
#include <new>
#include <utility>

namespace psychic_ui {

    /**
     * Holds a T constructed in place in storage of its own and never destroyed, like Chromium's
     * base::NoDestructor: for a function-local static that must outlive every static object that may
     * still use it.
     */
    template<class T>
    class NoDestructor {
    public:
        template<class... Args>
        explicit NoDestructor(Args &&...args) :
            _value(::new (_storage.data()) T(std::forward<Args>(args)...)) {}

        // Leaves the T alone: never destroyed is the point
        ~NoDestructor() = default;

        NoDestructor(const NoDestructor &)            = delete;
        NoDestructor &operator=(const NoDestructor &) = delete;
        NoDestructor(NoDestructor &&)                 = delete;
        NoDestructor &operator=(NoDestructor &&)      = delete;

        T &operator*() { return *_value; }
        T *operator->() { return _value; }

    private:
        alignas(T) std::array<std::byte, sizeof(T)> _storage{};
        T *_value;
    };
}
