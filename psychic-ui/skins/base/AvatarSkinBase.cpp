#include "AvatarSkinBase.hpp"

namespace psychic_ui {
    AvatarSkinBase::AvatarSkinBase() :
        AvatarSkin() {
        tag("AvatarSkinBase");
        _initials = &add<Label>();
        _initials->addClassName("initials");
        _status = &add<StatusDot>();
        _status->addClassName("status")
               .position(Position::Absolute)
               .display(Display::None);
    }

    void AvatarSkinBase::setInitials(const std::string &initials) {
        _initials->text(initials);
    }

    void AvatarSkinBase::setStatus(const std::optional<Tone> status) {
        if (status) {
            _status->tone(*status)
                   .display(Display::Flex);
        } else {
            _status->display(Display::None);
        }
    }

    void AvatarSkinBase::draw(SkCanvas *canvas) {
        drawChrome(canvas, paint::Box::of(*this), SkinState::of(component()));
    }

    void AvatarSkinBase::drawChrome(SkCanvas *canvas, const paint::Box &box, SkinState /*state*/) {
        const Style &style = computedStyle();
        paint::fill(canvas, box.outer, style.backgroundColor(), style.antiAlias());
        if (box.border > 0) {
            paint::stroke(canvas, box.strokeLine(), style.borderColor(), box.border, style.antiAlias());
        }
    }
}
