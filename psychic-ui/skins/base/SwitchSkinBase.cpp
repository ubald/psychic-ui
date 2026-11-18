#include <algorithm>
#include "include/core/SkCanvas.h"
#include "SkinState.hpp"
#include "SwitchSkinBase.hpp"
#include "psychic-ui/Div.hpp"
#include "psychic-ui/components/Label.hpp"
#include "psychic-ui/components/Switch.hpp"
#include "psychic-ui/paint/Box.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {
    SwitchSkinBase::SwitchSkinBase() :
        SwitchSkin() {
        tag("SwitchSkinBase");
        _track = &add<Div>();
        _track->addClassName("track")
              .mouseEnabled(false)
              .direction(Direction::Row)
              .alignItems(Align::Center)
              .shrink(0);

        // The free space around the thumb, shared by the position: no measuring, the layout places it. Their
        // `grow` is the position, so a transition on it (`Switch .thumb-space`) moves the thumb.
        _before = &_track->add<Div>();
        _before->addClassName("thumb-space")
               .mouseEnabled(false).grow(0).basis(0);

        _thumb = &_track->add<Div>();
        _thumb->addClassName("thumb")
              .mouseEnabled(false)
              .shrink(0);

        _after = &_track->add<Div>();
        _after->addClassName("thumb-space")
              .mouseEnabled(false).grow(1).basis(0);

        _label = &add<Label>();
        _label->addClassName("label")
              .mouseEnabled(false);
    }

    void SwitchSkinBase::setLabel(const std::string &label) {
        _label->text(label);
        _label->display(label.empty() ? Display::None : Display::Flex);
    }

    void SwitchSkinBase::setThumbPosition(const float position) {
        const float clamped = std::clamp(position, 0.0f, 1.0f);
        _before->grow(clamped);
        _after->grow(1.0f - clamped);
    }

    float SwitchSkinBase::positionAt(const int x, const int /*y*/) const {
        // The track is our child, so its position is in our coordinates; the space before the thumb starts its
        // content box, and the spaces on either side share the thumb's travel
        const float start  = static_cast<float>(_track->x() + _before->x()) + (static_cast<float>(_thumb->getWidth()) / 2.0f);
        const auto  travel = static_cast<float>(_before->getWidth() + _after->getWidth());
        return travel > 0 ? (static_cast<float>(x) - start) / travel : 0.0f;
    }

    void SwitchSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void SwitchSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &/*box*/, SkinState /*state*/) {
        Div::draw(canvas);
    }
}
