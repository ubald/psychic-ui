#pragma once

#include <memory>
#include "Skin.hpp"
#include "Component.hpp"

namespace psychicui {
    template<class T, class U>
    class SkinnableComponent : public Component {
        static_assert(std::is_base_of<Skin<U>, T>::value, "Skin type parameter is not a skin for this component");
    public:
        explicit SkinnableComponent(std::shared_ptr<T> skin) :
            Component() {
            setSkin(skin ? skin : std::make_shared<T>());
        }

        const std::shared_ptr<T> skin() const {
            return _skin;
        };

        SkinnableComponent<T, U> *setSkin(std::shared_ptr<T> skin) {
            if (skin != _skin) {
                if (_skin) {
                    this->remove(_skin);
                }
                _skin = skin;
                if (_skin) {
                    _skin->setComponent(static_cast<U *>(this));
                    this->add(_skin);
                }
            }
            return this;
        }

    protected:
        std::shared_ptr<T> _skin{nullptr};
    };
}