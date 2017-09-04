#pragma once

#include "Component.hpp"

namespace psychicui {

    namespace internal {
        class SkinBase : public Component {
        };
    }

    template<class T>
    class Skin : public internal::SkinBase {
//        static_assert(std::is_base_of<SkinnableComponent<U, T>, T>::value, "Skin is not for a valid component");

    public:
        T *component() const {
            return _component;
        }

        void setComponent(T *component) {
            if (component == _component) {
                return;
            }
            _component = component;
            componentChanged();
        }

        virtual void componentChanged() {

        }

    protected:
        T *_component{nullptr};
    };
}

