#pragma once

#include "Div.hpp"

namespace psychic_ui {

    namespace internal {
        class SkinBase : public Div {
        public:
            virtual void addedToComponent() {};
            virtual void removedFromComponent() {};
        protected:
            SkinBase();

            /**
             * A skin draws its component, so it takes almost all of the component's style:
             * everything but what places the component among its siblings (margins, size, position, flex item)
             */
            const PropertyMask &inheritedProperties() const override;
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
