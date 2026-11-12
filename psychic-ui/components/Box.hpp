#pragma once

#include "psychic-ui/Div.hpp"

namespace psychic_ui {

    /**
     * A Div preset to lay its children out along one axis, spaced by `gap`.
     * Nothing more than defaults that stylesheets and setters override:
     * `add<HBox>(12)` is `add<Div>().row().gap(12)`, with the `HBox` tag for stylesheets.
     */
    class Box : public Div {
    protected:
        Box(Direction direction, Length gap);
    };

    /**
     * Children in a row
     */
    class HBox : public Box {
    public:
        explicit HBox(Length gap = 0);
    };

    /**
     * Children in a column
     */
    class VBox : public Box {
    public:
        explicit VBox(Length gap = 0);
    };
}
