#include "Buttons.hpp"
#include <psychicui/Button.hpp>

namespace psychicui {

    Buttons::Buttons():Component() {
        setTag("Buttons");
        setClassNames({"demo-panel"});

        style()
        ->set(wrap, "wrap");

        // region No Borders

        // Rectangle
        add(std::make_shared<Button>("Rectangle"))
        ->style()
        ->set(borderRadius, 0);

        // Rounded
        add(std::make_shared<Button>("Rounded 12px"))
        ->style()
        ->set(borderRadius, 12);

        // Rounded Left
        add(std::make_shared<Button>("Rounded Left"))
        ->style()
        ->set(borderRadiusLeft, 12);

        // Rounded Right
        add(std::make_shared<Button>("Rounded Right"))
        ->style()
        ->set(borderRadiusRight, 12);

        // Rounded Top
        add(std::make_shared<Button>("Rounded Top"))
        ->style()
        ->set(borderRadiusTop, 12);

        // Rounded Bottom
        add(std::make_shared<Button>("Rounded Bottom"))
        ->style()
        ->set(borderRadiusBottom, 12);

        // Rounded Top Left
        add(std::make_shared<Button>("Rounded Top Left"))
        ->style()
        ->set(borderRadiusTopLeft, 12);

        // Rounded Top Right
        add(std::make_shared<Button>("Rounded Top Right"))
        ->style()
        ->set(borderRadiusTopRight, 12);

        // Rounded Bottom Left
        add(std::make_shared<Button>("Rounded Bottom Left"))
        ->style()
        ->set(borderRadiusBottomLeft, 12);

        // Rounded Bottom Right
        add(std::make_shared<Button>("Rounded Bottom Right"))
        ->style()
        ->set(borderRadiusBottomRight, 12);

        // endregion

        // region Full Borders

        // Rectangle
        add(std::make_shared<Button>("Rectangle Border"))
        ->style()
        ->set(border, 3)
        ->set(borderRadius, 0);

        // Rounded
        add(std::make_shared<Button>("Rounded 12px Border"))
        ->style()
        ->set(border, 3)
        ->set(borderRadius, 12);

        // Rounded Left
        add(std::make_shared<Button>("Rounded Left Border"))
        ->style()
        ->set(border, 3)
        ->set(borderRadiusLeft, 12);

        // Rounded Right
        add(std::make_shared<Button>("Rounded Right Border"))
        ->style()
        ->set(border, 3)
        ->set(borderRadiusRight, 12);

        // Rounded Top
        add(std::make_shared<Button>("Rounded Top Border"))
        ->style()
        ->set(border, 3)
        ->set(borderRadiusTop, 12);

        // Rounded Bottom
        add(std::make_shared<Button>("Rounded Bottom Border"))
        ->style()
        ->set(border, 3)
        ->set(borderRadiusBottom, 12);

        // Rounded Top Left
        add(std::make_shared<Button>("Rounded Top Left Border"))
        ->style()
        ->set(border, 3)
        ->set(borderRadiusTopLeft, 12);

        // Rounded Top Right
        add(std::make_shared<Button>("Rounded Top Right Border"))
        ->style()
        ->set(border, 3)
        ->set(borderRadiusTopRight, 12);

        // Rounded Bottom Left
        add(std::make_shared<Button>("Rounded Bottom Left Border"))
        ->style()
        ->set(border, 3)
        ->set(borderRadiusBottomLeft, 12);

        // Rounded Bottom Right
        add(std::make_shared<Button>("Rounded Bottom Right Border"))
        ->style()
        ->set(border, 3)
        ->set(borderRadiusBottomRight, 12);

        // endregion
    }

}