#pragma once

namespace psychic_ui {
    class StyleManager;

    /**
     * Rules, declared in `load` with `manager->style(selector)` from the values of `manager->theme()`.
     * The style manager keeps a loaded stylesheet and loads it again when the theme changes, so `load` must only
     * depend on the theme.
     */
    class StyleSheet {
    public:
        StyleSheet() = default;
        StyleSheet(const StyleSheet &) = delete;
        StyleSheet &operator=(const StyleSheet &) = delete;
        StyleSheet(StyleSheet &&) = delete;
        StyleSheet &operator=(StyleSheet &&) = delete;
        virtual ~StyleSheet() = default;
        virtual void load(StyleManager *manager) = 0;
    };
}
