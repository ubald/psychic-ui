
#include "Skin.hpp"

namespace psychic_ui {
    namespace internal {

        namespace {
            using P = Property;

            // TODO: This is getting out of hand, maybe just an exclusion list would be better
            constexpr PropertyMask skinInheritedProperties = mask({
                // Colors
                P::color, P::backgroundColor, P::borderColor, P::borderHorizontalColor, P::borderLeftColor,
                P::borderRightColor, P::borderVerticalColor, P::borderTopColor, P::borderBottomColor,
                P::selectionColor, P::selectionBackgroundColor, P::contentBackgroundColor,
                // Text
                P::fontFamily, P::fontSize, P::letterSpacing, P::lineHeight, P::textAlign, P::textAntiAlias,
                // Content layout
                P::textDirection, P::direction, P::justify, P::alignContent, P::alignItems, P::wrap, P::overflow,
                P::orientation, P::gap,
                P::padding, P::paddingHorizontal, P::paddingLeft, P::paddingRight,
                P::paddingVertical, P::paddingTop, P::paddingBottom,
                P::border, P::borderHorizontal, P::borderLeft, P::borderRight, P::borderVertical, P::borderTop, P::borderBottom,
                // Painting
                P::opacity, P::antiAlias, P::cursor,
                P::borderRadius, P::borderRadiusTop, P::borderRadiusBottom, P::borderRadiusLeft, P::borderRadiusRight,
                P::borderRadiusTopLeft, P::borderRadiusTopRight, P::borderRadiusBottomLeft, P::borderRadiusBottomRight
            });
        }

        SkinBase::SkinBase() :
            Div() {
            setTag("Skin");
            _inlineStyle
                .shrink(1)
                .grow(1);
        }

        const PropertyMask &SkinBase::inheritedProperties() const {
            return skinInheritedProperties;
        }
    }
}
