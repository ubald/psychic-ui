#pragma once

#include <algorithm>
#include <any>
#include <format>
#include <optional>
#include <string>
#include <vector>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/DragEvent.hpp>
#include <psychic-ui/DragPayload.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/style/Color.hpp>

namespace psychic_ui {

    /**
     * Drag and drop, and zoom: colour chips dragged (by the mouse, a pen, or a finger's long press) onto two boxes,
     * one taking every colour and one only the warm ones, and a canvas zoomed with Ctrl (Cmd on macOS) and the
     * wheel, a trackpad's pinch or two fingers, showing its scale
     */
    class DragAndDropPage : public Div {
    public:
        DragAndDropPage();

        /**
         * What a chip carries
         */
        struct Chip {
            std::string name;
            Color       color;
            bool        warm;
        };

        static constexpr const char *chipType = "psychic-ui-demo/color";

        static constexpr float minimumZoom = 0.25f;
        static constexpr float maximumZoom = 4.0f;

        /**
         * The canvas's scale now
         */
        float zoom() const { return _zoom; }

    private:
        Label *_event{nullptr};
        Div   *_zoomContent{nullptr};
        Label *_scale{nullptr};
        float  _zoom{1.0f};

        /**
         * A drop box with its title, keeping the chips dropped on it
         */
        Div &dropBox(Div &parent, const std::string &title, bool warmOnly);

        void zoomBy(float scale);
    };

    inline DragAndDropPage::DragAndDropPage() : Div() {
        tag("DragAndDropPage")
            .classNames({"demo-panel"});

        add<Label>("Drag and Drop").addClassName("h4");
        add<Label>("Drag a chip onto a box: with the mouse or a pen past a few pixels, or with a finger by a long press. Escape cancels.")
            .addClassName("text-tertiary");
        _event = &add<Label>("—").addClassName("event").addClassName("text-tertiary");

        auto &stage = add<Div>().addClassName("stage");

        // The chips, each dragging its colour
        auto &chips = stage.add<Div>().addClassName("chips");
        for (const Chip &chip: std::vector<Chip>{
                 {"Red", rgb(0xE5484D), true}, {"Orange", rgb(0xF76B15), true}, {"Amber", rgb(0xFFC53D), true},
                 {"Green", rgb(0x30A46C), false}, {"Teal", rgb(0x12A594), false}, {"Blue", rgb(0x0090FF), false},
                 {"Violet", rgb(0x8E4EC6), false},
             }) {
            auto &item = chips.add<Div>().addClassName("chip");
            item.add<Div>().addClassName("swatch").backgroundColor(chip.color);
            item.add<Label>(chip.name);
            item.dragSource([chip](const MouseEvent &) -> std::optional<DragPayload> {
                return DragPayload{.type = chipType, .data = std::any(chip), .label = chip.name};
            });
            item.onDragEnd([this, name = chip.name](const DragEvent &event) {
                if (!event.accepted) {
                    _event->text(name + ": not dropped");
                }
            });
        }

        auto &boxes = stage.add<Div>().addClassName("boxes");
        dropBox(boxes, "Every colour", false);
        dropBox(boxes, "Warm colours only", true);

        // The zoomed canvas: a grid scaled about its middle
        add<Label>("Zoom").addClassName("h5");
        add<Label>("Ctrl (Cmd on macOS) and the wheel, a trackpad's pinch, or two fingers on a touch screen.")
            .addClassName("text-tertiary");
        auto &canvas = add<Div>().addClassName("zoom-canvas");
        _zoomContent = &canvas.add<Div>().addClassName("zoom-content");
        for (int i = 0; i < 16; ++i) {
            _zoomContent->add<Div>().addClassName("cell");
        }
        _scale = &canvas.add<Label>("100 %").addClassName("scale");
        canvas.onZoom([this](const ZoomEvent &event) { zoomBy(event.scale); });
    }

    inline Div &DragAndDropPage::dropBox(Div &parent, const std::string &title, const bool warmOnly) {
        auto &box = parent.add<Div>().addClassName("drop-box");
        box.add<Label>(title).addClassName("h5");
        Div *dropped = &box.add<Div>().addClassName("dropped");
        box.dropTarget([warmOnly](const DragPayload &payload) {
            if (payload.type != chipType) {
                return false;
            }
            const auto *chip = std::any_cast<Chip>(&payload.data);
            return chip && (!warmOnly || chip->warm);
        });
        box.onDrop([this, dropped, title](const DragEvent &event) {
            const auto &chip = std::any_cast<const Chip &>(event.payload->data);
            dropped->add<Div>().addClassName("swatch").backgroundColor(chip.color);
            _event->text(std::format("{} dropped on {}", chip.name, title));
        });
        return box;
    }

    inline void DragAndDropPage::zoomBy(const float scale) {
        _zoom = std::clamp(_zoom * scale, minimumZoom, maximumZoom);
        _zoomContent->scale(_zoom);
        _scale->text(std::format("{:.0f} %", _zoom * 100.0f));
    }
}
