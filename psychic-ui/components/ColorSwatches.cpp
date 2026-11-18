#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>
#include "ColorSwatches.hpp"
#include "psychic-ui/Component.hpp"
#include "psychic-ui/Window.hpp"
#include "psychic-ui/psychic-ui.hpp"
#include "psychic-ui/style/Color.hpp"
#include "psychic-ui/themes/palette.hpp"

namespace psychic_ui {

    const std::vector<Color> &ColorSwatches::pivotalPalette() {
        static const std::vector<Color> palette(palette::pivotalSwatches.begin(), palette::pivotalSwatches.end());
        return palette;
    }

    ColorSwatches::ColorSwatches(std::vector<Color> colors) :
        Component(),
        _colors(std::move(colors)) {
        tag("ColorSwatches");

        _focusEnabled = true; // Tab reaches it; the arrows, Home and End move its focus among the chips

        const auto key = [this](const Key key, const Mod /*modifiers*/) {
            if (!_enabled || _colors.empty()) {
                return;
            }
            const int last = static_cast<int>(_colors.size()) - 1;
            const int from = _focused >= 0 ? _focused : _selected;
            int       to   = -2;
            switch (key) {
                case Key::LEFT:
                case Key::UP:
                    to = from < 0 ? last : std::max(from - 1, 0);
                    break;
                case Key::RIGHT:
                case Key::DOWN:
                    to = from < 0 ? 0 : std::min(from + 1, last);
                    break;
                case Key::HOME:
                    to = 0;
                    break;
                case Key::END:
                    to = last;
                    break;
                case Key::SPACE:
                case Key::ENTER:
                case Key::KP_ENTER:
                    updateSelected(from < 0 ? 0 : from);
                    updateFocused(from < 0 ? 0 : from);
                    return;
                default:
                    return;
            }
            updateFocused(to);
            // The keyboard is in use: the ring shows even after a click focused the group
            if (window() && !window()->focusVisible()) {
                window()->requestFocus(this, true);
            }
        };
        keyPressed.subscribe(key);
        keyRepeated.subscribe(key);

        // Taking the focus, the keyboard starts on the selection, else the first chip
        focused.subscribe(
            [this]() {
                if (!_hadFocus) {
                    _hadFocus = true;
                    updateFocused(_selected >= 0 ? _selected : 0);
                }
            }
        );
        blurred.subscribe([this]() { _hadFocus = false; });

        // The chip pressed is the one a click picks
        onMousePress(
            [this](const MouseEvent &event) {
                if (event.button != MouseButton::LEFT || !_skin) {
                    return;
                }
                int x = 0;
                int y = 0;
                _skin->globalToLocal(x, y, static_cast<int>(event.windowX), static_cast<int>(event.windowY));
                _pressed = _skin->swatchAt(x, y);
            }
        );

        clicked.subscribe(
            [this]() {
                if (_enabled && _pressed >= 0) {
                    updateSelected(_pressed);
                    updateFocused(_pressed);
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
        _focused  = std::min(_focused, static_cast<int>(_colors.size()) - 1);
        if (_skin) {
            _skin->setColors(_colors);
            _skin->setFocused(_focused);
        }
        if (had) {
            const auto found = std::ranges::find(_colors, previous);
            if (found != _colors.end()) {
                _selected = static_cast<int>(std::distance(_colors.begin(), found));
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
        const int selected = index >= 0 && std::cmp_less(index, _colors.size()) ? index : -1;
        if (selected != _selected) {
            _selected = selected;
            if (_skin) {
                _skin->setSelected(_selected);
            }
            changed.emit(value());
        }
    }

    void ColorSwatches::updateFocused(const int index) {
        const int focused = index >= 0 && std::cmp_less(index, _colors.size()) ? index : -1;
        if (focused != _focused) {
            _focused = focused;
            if (_skin) {
                _skin->setFocused(_focused);
            }
        }
    }

    Color ColorSwatches::value() const {
        return _selected >= 0 ? _colors[static_cast<std::size_t>(_selected)] : Color{};
    }

    void ColorSwatches::updateValue(const Color color) {
        const auto found = std::ranges::find(_colors, color);
        updateSelected(found != _colors.end() ? static_cast<int>(std::distance(_colors.begin(), found)) : -1);
    }

    void ColorSwatches::skinChanged() {
        if (_skin) {
            _skin->setColors(_colors);
            _skin->setSelected(_selected);
            _skin->setFocused(_focused);
        }
    }

}
