#include "PngWriter.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace psychic_ui::showcase {
    namespace {
        // region Checksums

        std::uint32_t crc32(const std::span<const std::uint8_t> data, std::uint32_t crc = 0) {
            static const std::array<std::uint32_t, 256> table = [] {
                std::array<std::uint32_t, 256> t{};
                std::uint32_t                  n = 0;
                for (std::uint32_t &entry: t) {
                    std::uint32_t c = n++;
                    for (int k = 0; k < 8; ++k) {
                        c = c & 1 ? 0xEDB88320u ^ (c >> 1) : c >> 1;
                    }
                    entry = c;
                }
                return t;
            }();
            crc = ~crc;
            for (const std::uint8_t byte: data) {
                crc = table.at((crc ^ byte) & 0xFF) ^ (crc >> 8);
            }
            return ~crc;
        }

        std::uint32_t adler32(const std::vector<std::uint8_t> &data) {
            std::uint32_t a = 1;
            std::uint32_t b = 0;
            for (const std::uint8_t byte: data) {
                a = (a + byte) % 65521;
                b = (b + a) % 65521;
            }
            return b << 16 | a;
        }

        // endregion

        // region Deflate

        /**
         * Bits in deflate's order: values from their least significant bit, Huffman codes from their most significant
         */
        class BitWriter {
        public:
            explicit BitWriter(std::vector<std::uint8_t> &out) : _out(&out) {}

            void bits(std::uint32_t value, int count) {
                _bits |= value << _count;
                _count += count;
                while (_count >= 8) {
                    _out->push_back(static_cast<std::uint8_t>(_bits & 0xFF));
                    _bits >>= 8;
                    _count -= 8;
                }
            }

            void code(std::uint32_t code, int length) {
                std::uint32_t reversed = 0;
                for (int i = 0; i < length; ++i) {
                    reversed = reversed << 1 | (code >> i & 1);
                }
                bits(reversed, length);
            }

            void flush() {
                if (_count > 0) {
                    _out->push_back(static_cast<std::uint8_t>(_bits & 0xFF));
                }
                _bits  = 0;
                _count = 0;
            }

        private:
            std::vector<std::uint8_t> *_out; // Not owned: the stream it writes to outlives it
            std::uint32_t             _bits{0};
            int                       _count{0};
        };

        constexpr std::array<int, 29> lengthBase{3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
        constexpr std::array<int, 29> lengthExtra{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
        constexpr std::array<int, 30> distanceBase{1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
        constexpr std::array<int, 30> distanceExtra{0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

        constexpr int windowSize = 32768;
        constexpr int minMatch   = 3;
        constexpr int maxMatch   = 258;
        constexpr int maxChain   = 64;
        constexpr int hashBits   = 15;

        // Deflate's fixed Huffman codes
        void literal(BitWriter &out, const int symbol) {
            if (symbol <= 143) {
                out.code(0x30 + symbol, 8);
            } else if (symbol <= 255) {
                out.code(0x190 + symbol - 144, 9);
            } else if (symbol <= 279) {
                out.code(symbol - 256, 7);
            } else {
                out.code(0xC0 + symbol - 280, 8);
            }
        }

        /**
         * The index of the last base not past a value: its code
         */
        template<std::size_t N>
        std::size_t lastNotPast(const std::array<int, N> &bases, const int value) {
            return static_cast<std::size_t>(std::ranges::upper_bound(bases, value) - bases.begin()) - 1;
        }

        void match(BitWriter &out, const int length, const int distance) {
            const std::size_t l = lastNotPast(lengthBase, length);
            literal(out, 257 + static_cast<int>(l));
            out.bits(length - lengthBase.at(l), lengthExtra.at(l));

            const std::size_t d = lastNotPast(distanceBase, distance);
            out.code(static_cast<std::uint32_t>(d), 5);
            out.bits(distance - distanceBase.at(d), distanceExtra.at(d));
        }

        /**
         * A zlib stream of one fixed Huffman block, with greedy LZ77 matches over hash chains
         */
        std::vector<std::uint8_t> zlib(const std::vector<std::uint8_t> &data) {
            std::vector<std::uint8_t> out{0x78, 0x01};
            BitWriter                 writer(out);
            writer.bits(1, 1); // The last block
            writer.bits(1, 2); // Fixed codes

            const int        size = static_cast<int>(data.size());
            std::vector<int> head(1 << hashBits, -1);
            std::vector<int> previous(windowSize, -1);
            const auto       hash = [&](const int i) {
                return (data[i] << 10 ^ data[i + 1] << 5 ^ data[i + 2]) & ((1 << hashBits) - 1);
            };
            const auto insert = [&](const int i) {
                if (i + minMatch <= size) {
                    const int h               = hash(i);
                    previous[i % windowSize] = head[h];
                    head[h]                  = i;
                }
            };

            int i = 0;
            while (i < size) {
                int bestLength   = 0;
                int bestDistance = 0;
                if (i + minMatch <= size) {
                    const int limit     = std::min(maxMatch, size - i);
                    int       candidate = head[hash(i)];
                    for (int chain = 0; candidate >= 0 && i - candidate <= windowSize && chain < maxChain; ++chain) {
                        int length = 0;
                        while (length < limit && data[candidate + length] == data[i + length]) {
                            ++length;
                        }
                        if (length > bestLength) {
                            bestLength   = length;
                            bestDistance = i - candidate;
                            if (length == limit) {
                                break;
                            }
                        }
                        const int next = previous[candidate % windowSize];
                        if (next >= candidate) {
                            break; // Overwritten by a newer position
                        }
                        candidate = next;
                    }
                }
                if (bestLength >= minMatch) {
                    match(writer, bestLength, bestDistance);
                    for (int k = 0; k < bestLength; ++k) {
                        insert(i + k);
                    }
                    i += bestLength;
                } else {
                    literal(writer, data[i]);
                    insert(i);
                    ++i;
                }
            }
            literal(writer, 256);
            writer.flush();

            const std::uint32_t adler = adler32(data);
            for (int shift = 24; shift >= 0; shift -= 8) {
                out.push_back(static_cast<std::uint8_t>(adler >> shift & 0xFF));
            }
            return out;
        }

        // endregion

        // region PNG

        int paeth(const int a, const int b, const int c) {
            const int p  = a + b - c;
            const int pa = std::abs(p - a);
            const int pb = std::abs(p - b);
            const int pc = std::abs(p - c);
            if (pa <= pb && pa <= pc) {
                return a;
            }
            return pb <= pc ? b : c;
        }

        /**
         * What a PNG filter predicts a byte from: the byte to its left (a), above (b) and above left (c)
         */
        int predict(const int filter, const int a, const int b, const int c) {
            switch (filter) {
                case 0: return 0;
                case 1: return a;
                case 2: return b;
                case 3: return (a + b) / 2;
                default: return paeth(a, b, c);
            }
        }

        /**
         * The image's rows, each after the filter byte that leaves the smallest residuals
         */
        std::vector<std::uint8_t> filtered(const PngFrame &frame, const int channels) {
            const std::size_t         stride = static_cast<std::size_t>(frame.width) * channels;
            std::vector<std::uint8_t> previous(stride, 0);
            std::vector<std::uint8_t> row(stride);
            std::vector<std::uint8_t> out{};
            out.reserve((stride + 1) * frame.height);
            std::array<std::vector<std::uint8_t>, 5> candidates{};
            for (auto &candidate: candidates) {
                candidate.resize(stride);
            }

            const std::size_t sourceStride = static_cast<std::size_t>(frame.width) * 4;
            const auto        pixels       = std::span{frame.rgba};
            const auto        width        = static_cast<std::size_t>(frame.width);
            const auto        bytes        = static_cast<std::size_t>(channels);
            for (int y = 0; y < frame.height; ++y) {
                const auto source = pixels.subspan(static_cast<std::size_t>(y) * sourceStride, sourceStride);
                for (std::size_t x = 0; x < width; ++x) {
                    for (std::size_t c = 0; c < bytes; ++c) {
                        row[(x * bytes) + c] = source[(x * 4) + c];
                    }
                }
                std::size_t bestSum = SIZE_MAX;
                int         best    = 0;
                int filter = 0;
                for (auto &candidate: candidates) {
                    std::size_t sum = 0;
                    for (std::size_t i = 0; i < stride; ++i) {
                        const bool left  = i >= bytes;
                        const int  a     = left ? row[i - bytes] : 0;
                        const int  b     = previous[i];
                        const int  c     = left ? previous[i - bytes] : 0;
                        const auto value = static_cast<std::uint8_t>(row[i] - predict(filter, a, b, c));
                        candidate[i] = value;
                        sum += static_cast<std::size_t>(std::abs(static_cast<std::int8_t>(value)));
                    }
                    if (sum < bestSum) {
                        bestSum = sum;
                        best    = filter;
                    }
                    ++filter;
                }
                const auto &chosen = candidates.at(static_cast<std::size_t>(best));
                out.push_back(static_cast<std::uint8_t>(best));
                out.insert(out.end(), chosen.begin(), chosen.end());
                previous.swap(row);
            }
            return out;
        }

        void put32(std::vector<std::uint8_t> &out, const std::uint32_t value) {
            for (int shift = 24; shift >= 0; shift -= 8) {
                out.push_back(static_cast<std::uint8_t>(value >> shift & 0xFF));
            }
        }

        void put16(std::vector<std::uint8_t> &out, const std::uint16_t value) {
            out.push_back(static_cast<std::uint8_t>(value >> 8));
            out.push_back(static_cast<std::uint8_t>(value & 0xFF));
        }

        void chunk(std::vector<std::uint8_t> &out, const std::string_view type, const std::vector<std::uint8_t> &data) {
            put32(out, static_cast<std::uint32_t>(data.size()));
            const std::size_t start = out.size();
            out.insert(out.end(), type.begin(), type.end());
            out.insert(out.end(), data.begin(), data.end());
            put32(out, crc32(std::span{out}.subspan(start)));
        }

        bool opaque(std::span<const PngFrame> frames) {
            for (const PngFrame &frame: frames) {
                for (std::size_t i = 3; i < frame.rgba.size(); i += 4) {
                    if (frame.rgba[i] != 0xFF) {
                        return false;
                    }
                }
            }
            return true;
        }

        struct Region {
            int x{0};
            int y{0};
            int width{0};
            int height{0};
        };

        /**
         * The smallest rectangle holding every pixel that differs between two frames of the same size (one pixel when
         * none does: a frame can't be empty)
         */
        Region changed(const PngFrame &before, const PngFrame &after) {
            int        left   = after.width;
            int        top    = after.height;
            int        right  = -1;
            int        bottom = -1;
            const auto from   = std::span{before.rgba};
            const auto to     = std::span{after.rgba};
            for (int y = 0; y < after.height; ++y) {
                const std::size_t row = static_cast<std::size_t>(y) * after.width * 4;
                for (int x = 0; x < after.width; ++x) {
                    const std::size_t i = row + (static_cast<std::size_t>(x) * 4);
                    if (std::ranges::equal(from.subspan(i, 4), to.subspan(i, 4))) {
                        continue;
                    }
                    left   = std::min(left, x);
                    right  = std::max(right, x);
                    top    = std::min(top, y);
                    bottom = std::max(bottom, y);
                }
            }
            if (right < 0) {
                return {0, 0, 1, 1};
            }
            return {left, top, right - left + 1, bottom - top + 1};
        }

        PngFrame crop(const PngFrame &frame, const Region &region) {
            if (region.x == 0 && region.y == 0 && region.width == frame.width && region.height == frame.height) {
                return frame;
            }
            PngFrame part{region.width, region.height, {}};
            part.rgba.reserve(static_cast<std::size_t>(region.width) * region.height * 4);
            for (int y = region.y; y < region.y + region.height; ++y) {
                const auto begin = frame.rgba.begin() + (static_cast<std::ptrdiff_t>(y) * frame.width + region.x) * 4;
                part.rgba.insert(part.rgba.end(), begin, begin + static_cast<std::ptrdiff_t>(region.width) * 4);
            }
            return part;
        }

        // endregion
    }

    std::vector<std::uint8_t> PngWriter::encode(const PngFrame &frame) {
        return encode(std::span{&frame, 1}, {});
    }

    std::vector<std::uint8_t> PngWriter::encode(std::span<const PngFrame> frames, std::span<const int> delaysMilliseconds) {
        if (frames.empty()) {
            return {};
        }
        const int  width    = frames.front().width;
        const int  height   = frames.front().height;
        const bool rgb      = opaque(frames);
        const int  channels = rgb ? 3 : 4;
        const bool animated = frames.size() > 1;

        std::vector<std::uint8_t> out{0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};

        std::vector<std::uint8_t> header{};
        put32(header, static_cast<std::uint32_t>(width));
        put32(header, static_cast<std::uint32_t>(height));
        header.push_back(8);              // Bits per channel
        header.push_back(rgb ? 2 : 6);    // RGB or RGBA
        header.insert(header.end(), {0, 0, 0}); // Deflate, adaptive filtering, not interlaced
        chunk(out, "IHDR", header);

        if (animated) {
            std::vector<std::uint8_t> control{};
            put32(control, static_cast<std::uint32_t>(frames.size()));
            put32(control, 0); // Loops forever
            chunk(out, "acTL", control);
        }

        std::uint32_t sequence = 0;
        for (std::size_t f = 0; f < frames.size(); ++f) {
            // After the first, a frame only holds what changed since the one before, which stays under it
            const Region   region = f == 0 ? Region{0, 0, width, height} : changed(frames[f - 1], frames[f]);
            const PngFrame part   = crop(frames[f], region);
            if (animated) {
                std::vector<std::uint8_t> control{};
                put32(control, sequence++);
                put32(control, static_cast<std::uint32_t>(region.width));
                put32(control, static_cast<std::uint32_t>(region.height));
                put32(control, static_cast<std::uint32_t>(region.x));
                put32(control, static_cast<std::uint32_t>(region.y));
                put16(control, static_cast<std::uint16_t>(f < delaysMilliseconds.size() ? delaysMilliseconds[f] : 100));
                put16(control, 1000);
                control.push_back(0); // Leave the frame as it is for the next
                control.push_back(0); // Replace, don't blend
                chunk(out, "fcTL", control);
            }
            std::vector<std::uint8_t> data = zlib(filtered(part, channels));
            if (f == 0) {
                chunk(out, "IDAT", data);
            } else {
                std::vector<std::uint8_t> frameData{};
                put32(frameData, sequence++);
                frameData.insert(frameData.end(), data.begin(), data.end());
                chunk(out, "fdAT", frameData);
            }
        }

        chunk(out, "IEND", {});
        return out;
    }

    bool PngWriter::write(const std::string &path, const std::vector<std::uint8_t> &png) {
        std::ofstream file(path, std::ios::binary);
        std::ranges::copy(png, std::ostreambuf_iterator<char>(file));
        return static_cast<bool>(file);
    }
}
