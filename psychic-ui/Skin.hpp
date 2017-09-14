#pragma once

#include "Div.hpp"

namespace psychic_ui {

    namespace internal {
        class SkinBase : public Div {
        protected:
            SkinBase();
            const InheritableValues inheritableValues() const override;
        private:
            static const InheritableValues _inheritableValues;
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

