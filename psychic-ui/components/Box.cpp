#include "Box.hpp"

namespace psychic_ui {

    // BOX

    Box::Box(int gap) :
        Div(), _gap(gap) {
        setTag("Box");
    }


    int Box::getGap() const {
        return _gap;
    }

    void Box::setGap(int gap) {
        if (gap != _gap) {
            _gap = gap;
            updateRules();
        }
    }

    void Box::styleUpdated() {
        Div::styleUpdated();
        setGap(static_cast<int>(_computedStyle.gap().resolve(0.0f)));
    }

    void Box::createStyles() {
        Div::createStyles();
        updateRules();
    }

    // HBOX

    HBox::HBox() :
        HBox(0) {}

    HBox::HBox(int gap) :
        Box(gap) {
        setTag("HBox");

        _inlineStyle
            .direction(Direction::Row);
    }

    void HBox::updateRules() {
        // TODO: Once we have access to a style manager add a custom rule targeting us by internal id
        if (auto sm = styleManager()) {
            sm->style("#" + _internalId + " > div")
              .marginRight(_gap); // This is our gap, we'll need to cancel it for the last child
            sm->style("#" + _internalId + " > div:last-child")
              .marginRight(0); // Last child gap cancellation
        }
    }

    // VBOX

    VBox::VBox() :
        VBox(0) {}

    VBox::VBox(int gap) :
        Box(gap) {
        setTag("VBox");

        _inlineStyle
            .direction(Direction::Column);
    }

    void VBox::updateRules() {
        // TODO: Once we have access to a style manager add a custom rule targeting us by internal id
        if (auto sm = styleManager()) {
            sm->style("#" + _internalId + " > div")
              .marginBottom(_gap); // This is our gap, we'll need to cancel it for the last child
            sm->style("#" + _internalId + " > div:last-child")
              .marginBottom(0); // Last child gap cancellation
        }
    }
}
