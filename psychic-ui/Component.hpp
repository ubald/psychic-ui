#pragma once

#include <memory>
#include "Skin.hpp"
#include "Div.hpp"

namespace psychic_ui {

    template<class T>
    class Component : public Div {
    protected:
        Component();

    public:

        /**
         * The skin drawing this component, hatched from the `skin` style property
         */
        std::shared_ptr<T> skin() const { return _skin; };

        // The `skin` style property's setter (by name), which the getter above would hide
        using Div::skin;

        /**
         * Replace the skin with one built by hand, until the `skin` style property changes
         */
        template<class Self>
        Self &skin(this Self &self, std::shared_ptr<T> skin) {
            static_cast<Component &>(self).updateSkin(std::move(skin));
            return self;
        }

    protected:
        void updateSkin(std::shared_ptr<T> skin);

        void draw(SkCanvas */*canvas*/) override {/*Skin Manages drawing*/}

        virtual void skinChanged() {}

        void styleUpdated() override;

        Atom               _skinName{};
        std::shared_ptr<T> _skin{nullptr};

        // Make some of div's stuff protected since we manage our content
        using Div::add;
        using Div::remove;
        using Div::removeAll;
    };

    template<class T>
    Component<T>::Component():
        Div() {
        tag("Component");
        // Our skin will inherit all of our styles but the external ones (margin)
        // But we should do our part and ignore any internal layout styling
        _ignoreInternalLayoutContraints = true;
    }

    template<class T>
    void Component<T>::updateSkin(std::shared_ptr<T> skin) {
        if (skin != _skin) {
            if (_skin) {
                _skin->removedFromComponent();
                this->remove(_skin);
            }
            _skin = skin;
            if (_skin) {
                // Skin is always in the back so the component can add more
                // children if need be (ie: TextInput overlays its text)
                this->add(0, _skin);
                _skin->addedToComponent();
            }
            skinChanged();
        }
    }

    template<class T>
    void Component<T>::styleUpdated() {
        Div::styleUpdated();
        const Atom &skinName = _computedStyle.skin();
        if (skinName != _skinName) {
            _skinName = skinName;
            updateSkin(std::dynamic_pointer_cast<T>(styleManager()->skin(_skinName.str())));
        }
    }
}
