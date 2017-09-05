#pragma once

#include "Component.hpp"

namespace psychicui {

    namespace internal {
        class SkinBase : public Component {
        protected:
            SkinBase() :
                Component() {
                setTag("Skin");
            }
        };
    }

    template<class T>
    class Skin : public internal::SkinBase {
    public:
        T *component() const {
            // TODO: Find a better pattern for that, but for now the show must go on
            return dynamic_cast<T *>(_parent);
        }
    };
}

