
#include "Skin.hpp"

namespace psychic_ui::internal {

    namespace {
        using P = Property;

        // TODO: This is getting out of hand, maybe just an exclusion list would be better
        constexpr PropertyMask skinInheritedProperties = mask({
            // Colors
            P::color, P::backgroundColor, P::borderColor, P::borderHorizontalColor, P::borderLeftColor,
            P::borderRightColor, P::borderVerticalColor, P::borderTopColor, P::borderBottomColor,
            P::selectionColor, P::selectionBackgroundColor, P::caretColor, P::contentBackgroundColor,
            // Text
            P::fontFamily, P::fontSize, P::letterSpacing, P::lineHeight, P::textAlign, P::fontSmoothing,
            P::iconSize, P::iconStrokeWidth,
            // Content layout
            P::textDirection, P::direction, P::justify, P::alignContent, P::alignItems, P::wrap, P::overflow,
            P::orientation, P::gap, P::rowGap, P::columnGap,
            P::padding, P::paddingHorizontal, P::paddingLeft, P::paddingRight,
            P::paddingVertical, P::paddingTop, P::paddingBottom,
            P::border, P::borderHorizontal, P::borderLeft, P::borderRight, P::borderVertical, P::borderTop, P::borderBottom,
            // Painting
            P::antiAlias, P::cursor, // Not opacity: the component's layer already fades its skin
            P::borderRadius, P::borderRadiusTop, P::borderRadiusBottom, P::borderRadiusLeft, P::borderRadiusRight,
            P::borderRadiusTopLeft, P::borderRadiusTopRight, P::borderRadiusBottomLeft, P::borderRadiusBottomRight
        });
    }

    SkinBase::SkinBase() :
        Div() {
        tag("Skin");
        _inlineStyle.shrink(1)
            .grow(1);
    }

    const PropertyMask &SkinBase::inheritedProperties() const {
        return skinInheritedProperties;
    }
}
