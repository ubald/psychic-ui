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
        setGap(_computedStyle->get(gap));
    }

    void Box::addedToRender() {
        Div::addedToRender();
        updateRules();
    }

    // HBOX

    HBox::HBox() :
        HBox(0) {}

    HBox::HBox(int gap) :
        Box(gap) {
        setTag("HBox");

        _inlineStyle
            ->set(flexDirection, "row");
    }

    void HBox::updateRules() {
        // Once we have access to a style manager add a custom rule targeting us by internal id
        if (auto sm = styleManager()) {
            sm->style("#" + _internalId + " > div")
              ->set(marginRight, _gap); // This is our gap, we'll need to cancel it for the last child
            sm->style("#" + _internalId + " > div:last-child")
              ->set(marginRight, 0); // Last child gap cancellation
        }
    }



    // VBOX

    VBox::VBox() :
        VBox(0) {}

    VBox::VBox(int gap) :
        Box(gap) {
        setTag("VBox");

        _inlineStyle
            ->set(flexDirection, "column");
    }

    void VBox::updateRules() {
        // Once we have access to a style manager add a custom rule targeting us by internal id
        if (auto sm = styleManager()) {
            sm->style("#" + _internalId + " > div")
              ->set(marginBottom, 36); // This is our gap, we'll need to cancel it for the last child
            sm->style("#" + _internalId + " > div:last-child")
              ->set(marginBottom, 0); // Last child gap cancellation
        }
    }
}
