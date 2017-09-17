#include "Skin.hpp"

namespace psychic_ui {
    namespace internal {
        // TODO: This is getting out of hand, maybe just an exclusion list would be better
        const InheritableValues SkinBase::_inheritableValues{
            {
                color,
                backgroundColor,
                borderColor,
                borderHorizontalColor,
                borderLeftColor,
                borderRightColor,
                borderVerticalColor,
                borderTopColor,
                borderBottomColor
            },
            {
                fontFamily, textAlign, textJustify,
                justifyContent, direction, alignContent, alignItems,
                wrap, overflow,
                orientation
            },
            {
                opacity,
                fontSize,  letterSpacing, lineHeight,
                padding,   paddingHorizontal, paddingLeft, paddingRight, paddingVertical, paddingTop, paddingBottom,
                border, borderHorizontal, borderLeft, borderRight, borderVertical, borderTop, borderBottom,
                borderRadius, borderRadiusTop, borderRadiusBottom, borderRadiusLeft, borderRadiusRight,
                borderRadiusTopLeft, borderRadiusTopRight, borderRadiusBottomLeft, borderRadiusBottomRight
            },
            {
                cursor,
                gap
            },
            {
                antiAlias,
                textAntiAlias,
                multiline
            }
        };

        SkinBase::SkinBase() :
            Div() {
            setTag("Skin");
            // But, forget about the margins since they have an external influence
            // We have to be explicit about margins since we want to avoid a fallback to default margins too
            _inlineStyle
                ->set(shrink, 1)
                ->set(grow, 1);
        }

        const InheritableValues SkinBase::inheritableValues() const {
            return _inheritableValues;
        }
    }
}
