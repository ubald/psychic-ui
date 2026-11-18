#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace psychic_ui::showcase {
    /**
     * One frame of an image: unpremultiplied RGBA, 8 bits a channel, rows `width * 4` bytes apart
     */
    struct PngFrame {
        int                       width{0};
        int                       height{0};
        std::vector<std::uint8_t> rgba{};
    };

    /**
     * A PNG encoder with no dependency, so that the captures need nothing Skia (built without its codecs) or the
     * system lacks: each row gets the PNG filter that leaves the smallest residuals, then deflate with LZ77 matches
     * and the fixed Huffman codes (no dynamic tables: UI captures are mostly long matches, which fixed codes handle
     * well). Opaque images are written as RGB. The same pixels always give the same bytes.
     *
     * Several frames make an animated PNG (APNG), each shown for its delay, looping; browsers that don't know APNG show
     * the first.
     */
    class PngWriter {
    public:
        static std::vector<std::uint8_t> encode(const PngFrame &frame);
        static std::vector<std::uint8_t> encode(std::span<const PngFrame> frames, std::span<const int> delaysMilliseconds);

        /**
         * Encode into a file; false when it can't be written
         */
        static bool write(const std::string &path, const std::vector<std::uint8_t> &png);
    };
}
