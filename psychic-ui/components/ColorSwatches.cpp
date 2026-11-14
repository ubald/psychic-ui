#include <algorithm>
#include "ColorSwatches.hpp"

namespace psychic_ui {

    const std::vector<Color> &ColorSwatches::pivotalPalette() {
        static const std::vector<Color> palette{
            rgb(0xE5484D), rgb(0xF76B3C), rgb(0xF2994A), rgb(0xE8B339), rgb(0xF2C94C), rgb(0xA8C43F),
            rgb(0x6CC26A), rgb(0x4CB782), rgb(0x2BB3A3), rgb(0x26B5CE), rgb(0x3FA5F0), rgb(0x4B8BF5),
            rgb(0x5E6AD2), rgb(0x8E6CEF), rgb(0x8E4EC6), rgb(0xC054D8), rgb(0xD6409F), rgb(0xF0617E),
            rgb(0xB07D56), rgb(0x95A2B3),
        };
        return palette;
    }

    ColorSwatches::ColorSwatches(std::vector<Color> colors) :
        Component(),
        _colors(std::move(colors)) {
        tag("ColorSwatches");

        _focusEnabled = true; // Tab reaches it; the arrows, Home and End move the selection

        const auto key = [this](const Key key, const Mod /*modifiers*/) {
            if (!_enabled || _colors.empty()) {
                return;
            }
            const int last = static_cast<int>(_colors.size()) - 1;
            switch (key) {
                case Key::LEFT:
                case Key::UP:
                    updateSelected(_selected < 0 ? last : std::max(_selected - 1, 0));
                    break;
                case Key::RIGHT:
                case Key::DOWN:
                    updateSelected(_selected < 0 ? 0 : std::min(_selected + 1, last));
                    break;
                case Key::HOME:
                    updateSelected(0);
                    break;
                case Key::END:
                    updateSelected(last);
                    break;
                default:
                    break;
            }
        };
        keyPressed.subscribe(key);
        keyRepeated.subscribe(key);

        // The chip pressed is the one a click picks
        onMousePress(
            [this](const int mouseX, const int mouseY, const MouseButton button, Mod /*modifiers*/) {
                if (button != MouseButton::LEFT || !_skin) {
                    return;
                }
                int windowX = 0;
                int windowY = 0;
                localToGlobal(windowX, windowY, mouseX, mouseY);
                int x = 0;
                int y = 0;
                _skin->globalToLocal(x, y, windowX, windowY);
                _pressed = _skin->swatchAt(x, y);
            }
        );

        clicked.subscribe(
            [this]() {
                if (_enabled && _pressed >= 0) {
                    updateSelected(_pressed);
                }
                _pressed = -1;
            }
        );
    }

    const std::vector<Color> &ColorSwatches::colors() const {
        return _colors;
    }

    void ColorSwatches::updateColors(std::vector<Color> colors) {
        const Color previous = value();
        const bool  had      = _selected >= 0;
        _colors   = std::move(colors);
        _selected = -1;
        if (_skin) {
            _skin->setColors(_colors);
        }
        if (had) {
            const auto found = std::find(_colors.cbegin(), _colors.cend(), previous);
            if (found != _colors.cend()) {
                _selected = static_cast<int>(std::distance(_colors.cbegin(), found));
            } else {
                changed.emit(Color{});
            }
        }
        if (_skin) {
            _skin->setSelected(_selected);
        }
    }

    int ColorSwatches::selected() const {
        return _selected;
    }

    void ColorSwatches::updateSelected(const int index) {
        const int selected = index >= 0 && index < static_cast<int>(_colors.size()) ? index : -1;
        if (selected != _selected) {
            _selected = selected;
            if (_skin) {
                _skin->setSelected(_selected);
            }
            changed.emit(value());
        }
    }

    Color ColorSwatches::value() const {
        return _selected >= 0 ? _colors[static_cast<std::size_t>(_selected)] : Color{};
    }

    void ColorSwatches::updateValue(const Color color) {
        const auto found = std::find(_colors.cbegin(), _colors.cend(), color);
        updateSelected(found != _colors.cend() ? static_cast<int>(std::distance(_colors.cbegin(), found)) : -1);
    }

    void ColorSwatches::skinChanged() {
        if (_skin) {
            _skin->setColors(_colors);
            _skin->setSelected(_selected);
        }
    }

}
