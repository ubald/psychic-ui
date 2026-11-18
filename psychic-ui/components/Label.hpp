#pragma once

#include <string>
#include "psychic-ui/components/Text.hpp"

namespace psychic_ui {
    /**
     * @class Label
     *
     * A Text on one line, tagged `Label`: line returns lay out as spaces, and the text
     * is kept as it was set.
     */
    class Label : public Text {
    public:
        explicit Label(const std::string &text = "");
    };
}
