#pragma once

#include <algorithm>
#include <cstdint>

namespace psychic_ui {

    /**
     * A measured size, in pixels
     */
    struct Size {
        float width{0.0f};
        float height{0.0f};

        constexpr bool operator==(const Size &) const = default;
    };

    /**
     * How a measured dimension must fit the space the layout offers
     */
    enum class Fit : std::uint8_t {
        /**
         * The dimension is decided: report the size given
         */
        Exact,
        /**
         * Report what the content needs, up to the size given
         */
        AtMost,
        /**
         * Report what the content needs, the size given means nothing
         */
        Unbounded
    };

    /**
     * What the layout offers a measured div in one dimension
     */
    struct Constraint {
        float size{0.0f};
        Fit   fit{Fit::Unbounded};

        /**
         * The size to report for content that needs `content`
         */
        [[nodiscard]] constexpr float apply(float content) const {
            switch (fit) {
                case Fit::Exact:
                    return size;
                case Fit::AtMost:
                    return std::min(content, size);
                default:
                    return content;
            }
        }
    };
}
