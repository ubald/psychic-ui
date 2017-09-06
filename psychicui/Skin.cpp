#include "Skin.hpp"

namespace psychicui {
namespace internal {
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
            {},
            {
                antiAlias,
                textAntiAlias
            }
        };

        SkinBase::SkinBase() :
            Div() {
            setTag("Skin");
            // But, forget about the margins since they have an external influence
            // We have to be explicit about margins since we want to avoid a fallback to default margins too
//                _inlineStyle
//                    ->set(marginLeft, 0)
//                    ->set(marginTop, 0)
//                    ->set(marginRight, 0)
//                    ->set(marginBottom, 0);
//                _ignoreExternalLayoutContraints = true;
        }

        const InheritableValues SkinBase::inheritableValues() const {
            return _inheritableValues;
        }
}
}