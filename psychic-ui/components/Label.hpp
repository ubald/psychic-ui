#pragma once

#include <string>
#include "psychic-ui/TextBase.hpp"

namespace psychic_ui {
    /**
     * @class Label
     *
     * Displays single lines of non-selectable text.
     * For more control and options see the Text class.
     */
    class Label : public TextBase {
    public:
        explicit Label(const std::string &text = "");
        const std::string &text() const;
        void setText(const std::string &text) override;
        void styleUpdated() override;
        YGSize measure(float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) override;
        void layoutUpdated() override;
        void draw(SkCanvas *canvas) override;
    private:
        std::string _text;
        float       _yOffset{0.0f};
        std::string _drawText{};
    };
}
