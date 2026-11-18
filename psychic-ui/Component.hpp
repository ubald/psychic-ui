#pragma once

#include <memory>
#include <utility>
#include "Skin.hpp"
#include "Div.hpp"

namespace psychic_ui {

    /**
     * A Div drawn by a skin implementing `T`. `Base` is the Div subclass it extends, when the components of a kind
     * share code that doesn't depend on their skin (`TextFieldBase`).
     */
    template<class T, class Base = Div>
    class Component : public Base {
    protected:
        Component();

    public:

        /**
         * The skin drawing this component: the active skin set's, in the variant the `skin` style property
         * names ("" for the default)
         */
        std::shared_ptr<T> skin() const { return _skin; };

        // The `skin` style property's setter (by name), which the getter above would hide
        using Div::skin;

        /**
         * Replace the skin with one built by hand, until the `skin` style property or the skin set changes
         */
        template<class Self>
        Self &skin(this Self &self, std::shared_ptr<T> skin) {
            auto &component = static_cast<Component &>(self);
            component.updateSkin(std::move(skin));
            component._skinByHand = true;
            return self;
        }

    protected:
        void updateSkin(std::shared_ptr<T> skin);

        void draw(SkCanvas */*canvas*/) override {/*Skin Manages drawing*/} // NOLINT(portability-template-virtual-member-function): it compiles for every T, whether instantiated or not

        virtual void skinChanged() {} // NOLINT(portability-template-virtual-member-function): it compiles for every T, whether instantiated or not

        void styleUpdated() override; // NOLINT(portability-template-virtual-member-function): it compiles for every T, whether instantiated or not

        Atom               _skinName{};
        std::size_t        _skinsGeneration{0};
        bool               _skinByHand{false}; // Until the next style update, which records what it was chosen against
        std::shared_ptr<T> _skin{nullptr};

        // Make some of div's stuff protected since we manage our content
        using Div::add;
        using Div::remove;
        using Div::removeAll;
    };

    template<class T, class Base>
    Component<T, Base>::Component():
        Base() {
        this->tag("Component");
        // Our skin will inherit all of our styles but the external ones (margin)
        // But we should do our part and ignore any internal layout styling
        this->_ignoreInternalLayoutContraints = true;
    }

    template<class T, class Base>
    void Component<T, Base>::updateSkin(std::shared_ptr<T> skin) {
        if (skin != _skin) {
            if (_skin) {
                this->remove(_skin);
            }
            _skin = std::move(skin);
            if (_skin) {
                // The skin is always child 0, behind what the component adds itself
                this->add(0, _skin);
            }
            skinChanged();
        }
    }

    template<class T, class Base>
    void Component<T, Base>::styleUpdated() {
        Base::styleUpdated();
        // The `skin` property names a variant of the skin set's skin for T; "" is the default one
        const Atom        &variant    = this->_computedStyle.skin();
        const std::size_t generation = this->styleManager()->skinsGeneration(); // Bumped when the skin set changes
        if (_skinByHand) {
            _skinByHand = false;
        } else if (variant != _skinName || generation != _skinsGeneration) {
            // Typed: the variant from the set or its fallbacks, else the default; never null once a set is in use
            updateSkin(this->styleManager()->template skin<T>(variant.str()));
        }
        _skinName        = variant;
        _skinsGeneration = generation;
    }
}
