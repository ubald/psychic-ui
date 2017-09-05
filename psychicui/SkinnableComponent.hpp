#pragma once

#include <memory>
#include "Skin.hpp"
#include "Component.hpp"

namespace psychicui {

    template<class T>
    class SkinnableComponent : public Component {
    protected:
        SkinnableComponent() = default;

    public:
        const std::shared_ptr<T> skin() const {
            return _skin;
        };

        SkinnableComponent<T> *setSkin(std::shared_ptr<T> skin) {
            if (skin != _skin) {
                if (_skin) {
                    this->remove(_skin);
                }
                _skin = skin;
                if (_skin) {
                    this->add(_skin);
                }
                skinChanged();
            }
            return this;
        }

    protected:
        void draw(SkCanvas *canvas) override {
            // No drawing, skin manages that
        }

        virtual void skinChanged() {}

        void styleUpdated() override {
            Component::styleUpdated();
            std::string skinName = _computedStyle->get(StringProperty::skin);
            if (skinName != _skinName) {
                _skinName = skinName;
                setSkin(std::dynamic_pointer_cast<T>(styleManager()->skin(_skinName)));
            }
        }

        std::string        _skinName{};
        std::shared_ptr<T> _skin{nullptr};
    };
}