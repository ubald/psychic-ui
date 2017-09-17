#include <algorithm>
#include <iostream>
#include "Style.hpp"
#include "../Div.hpp"

namespace psychic_ui {

    std::unique_ptr<Style> Style::dummyStyle{std::make_unique<Style>()};
    const float Style::Auto = nanf("auto");

    Style::Style() {
    }

    Style::Style(const Style *fromStyle) {
        overlay(fromStyle);
    }

    Style::Style(const std::function<void()> &onChanged) :
        _onChanged(onChanged) {}

    Style *Style::overlay(const Style *style) {
        if (style) {
            doOverlay(style->_colorValues, _colorValues);
            doOverlay(style->_stringValues, _stringValues);
            doOverlay(style->_floatValues, _floatValues);
            doOverlay(style->_intValues, _intValues);
            doOverlay(style->_boolValues, _boolValues);

            // Just assume something changed
            if (_onChanged) {
                _onChanged();
            }
        }

        return this;
    }

    Style *Style::overlayInheritable(const Style *style, const Div *div) {
        if (style) {
            doInheritance(style->_colorValues, _colorValues, div->inheritableValues().colorInheritable);
            doInheritance(style->_stringValues, _stringValues, div->inheritableValues().stringInheritable);
            doInheritance(style->_floatValues, _floatValues, div->inheritableValues().floatInheritable);
            doInheritance(style->_intValues, _intValues, div->inheritableValues().intInheritable);
            doInheritance(style->_boolValues, _boolValues, div->inheritableValues().boolInheritable);

            // Just assume something changed
            if (_onChanged) {
                _onChanged();
            }
        }

        return this;
    }

    Style *Style::defaults(const Style *style) {
        if (style) {
            doDefaults(style->_colorValues, _colorValues);
            doDefaults(style->_stringValues, _stringValues);
            doDefaults(style->_floatValues, _floatValues);
            doDefaults(style->_intValues, _intValues);
            doDefaults(style->_boolValues, _boolValues);

            // Just assume something changed
            if (_onChanged) {
                _onChanged();
            }
        }

        return this;
    }

    void Style::trace() const {
        #ifdef DEBUG_STYLES
        for (const auto &declaration: declarations) {
            std::cout << declaration << std::endl;
        }
        #endif

        std::cout << "{" << std::endl;
        for (auto const &kv : _colorValues) {
            std::cout << "    " << kv.first << ": " << kv.second << std::endl;
        }

        for (auto const &kv : _stringValues) {
            std::cout << "    " << kv.first << ": \"" << kv.second << "\"" << std::endl;
        }

        for (auto const &kv : _floatValues) {
            std::cout << "    " << kv.first << ": " << kv.second << std::endl;
        }

        for (auto const &kv : _intValues) {
            std::cout << "    " << kv.first << ": " << kv.second << std::endl;
        }

        for (auto const &kv : _boolValues) {
            std::cout << "    " << kv.first << ": " << (kv.second ? "true" : "false") << std::endl;
        }
        std::cout << "}" << std::endl << std::endl;
    }

    bool Style::operator==(const Style &other) const {
        return _colorValues == other._colorValues
               && _stringValues == other._stringValues
               && _floatValues == other._floatValues
               && _intValues == other._intValues
               && _boolValues == other._boolValues;
    }

    bool Style::operator!=(const Style &other) const {
        return !(*this == other);
    }
}
