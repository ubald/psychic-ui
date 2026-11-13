#pragma once

#include <string>
#include <string_view>
#include "psychic-ui/Div.hpp"

namespace psychic_ui {

    /**
     * @class Icon
     *
     * A registered icon (IconRegistry), drawn in the `color` style: `add<Icon>("search").size(20)`.
     * It measures `iconSize` square (plus its padding and border), and draws the icon at that size, centered in its
     * content box, stroked `iconStrokeWidth` wide (see paint::icon). Both styles are inherited, so a container can
     * size the icons inside it. An unknown name draws nothing.
     */
    class Icon : public Div {
    public:
        explicit Icon(std::string_view name = "");

        const std::string &name() const { return _name; }

        /**
         * The icon to draw, by its registered name
         */
        template<class Self>
        Self &name(this Self &self, detail::StringArg name) {
            static_cast<Icon &>(self).updateName(name.value);
            return self;
        }

        /**
         * The icon's size, a shorthand for `iconSize`
         */
        template<class Self>
        Self &size(this Self &self, detail::FloatArg size) { return self.iconSize(size); }

        using Div::size;

    protected:
        void updateName(std::string_view name);
        void styleUpdated() override;
        Size measure(Constraint width, Constraint height) override;
        void draw(SkCanvas *canvas) override;

    private:
        std::string _name{};
        float       _measuredSize{-1.0f}; // The iconSize the layout last measured
    };
}
