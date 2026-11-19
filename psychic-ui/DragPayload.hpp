#pragma once

#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include "psychic-ui.hpp"

namespace psychic_ui {

    class Div;

    /**
     * What a drag carries, from its source to the target it's dropped on (`DragAndDrop`)
     */
    struct DragPayload {
        /**
         * What it is, which drop targets test to accept it: "psychic-lights/fixture"
         */
        std::string          type{};
        /**
         * The app's value
         */
        std::any             data{};
        /**
         * Shown under the pointer by the default preview
         */
        std::string          label{};
        /**
         * Shown under the pointer instead of the label, when set
         */
        std::shared_ptr<Div> preview{};
    };

    /**
     * What a drag source gives when a drag starts from it, from the press that began it (in the source's
     * coordinates): the payload, or none to refuse the drag
     */
    using DragSourceFn = std::function<std::optional<DragPayload>(const MouseEvent &press)>;

    /**
     * Whether a drop target takes what a drag carries (by its type, by its data)
     */
    using DropAcceptFn = std::function<bool(const DragPayload &payload)>;

}
