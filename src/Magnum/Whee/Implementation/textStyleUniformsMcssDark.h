/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/* TextStyleUniform enum name, then TextLayerStyleUniform constructor
   arguments. Put into a dedicated file to be able to put as much as possible
   into constexpr arrays without losing the correspondence between TextStyle
   and the actual items. Matching order tested in StyleTest, see all #includes
   of this file in Style.cpp for actual uses. */

#ifdef _c
_c(Button,                      0x22272e_rgbf)
_c(ButtonDisabled,              0x2f363fff_rgbaf*0.8f)

/* Flat button is styled the same as links (--link-color, --link-active-color).
   Disabled state is --dim-link-color with 80% opacity. */
_c(ButtonFlatInactiveOut,       0x8fa5ec_rgbf)
_c(ButtonFlatInactiveOver,      0xd0d9f7_rgbf)
_c(ButtonFlatPressedOut,        0xd0d9f7_rgbf)
_c(ButtonFlatPressedOver,       0xd0d9f7_rgbf)
_c(ButtonFlatDisabled,          0xacacacff_rgbaf*0.3f)

_c(LabelDefault,                0xdcdcdc_rgbf)
_c(LabelDefaultDisabled,        0xdcdcdcff_rgbaf*0.3f)
_c(LabelPrimary,                0xa5c9ea_rgbf)
_c(LabelPrimaryDisabled,        0xa5c9eaff_rgbaf*0.3f)
_c(LabelSuccess,                0x3bd267_rgbf)
_c(LabelSuccessDisabled,        0x3bd267ff_rgbaf*0.3f)
_c(LabelWarning,                0xc7cf2f_rgbf)
_c(LabelWarningDisabled,        0xc7cf2fff_rgbaf*0.3f)
_c(LabelDanger,                 0xcd3431_rgbf)
_c(LabelDangerDisabled,         0xcd3431ff_rgbaf*0.3f)
_c(LabelInfo,                   0x2f83cc_rgbf)
_c(LabelInfoDisabled,           0x2f83ccff_rgbaf*0.3f)
_c(LabelDim,                    0x747474_rgbf)
_c(LabelDimDisabled,            0x747474ff_rgbaf*0.3f)

_c(InputDefault,                0xdcdcdc_rgbf)
_c(InputDefaultDisabled,        0xdcdcdcff_rgbaf*0.3f)
_c(InputSelection,              0x22272e_rgbf)
// TODO InputSelectionDisabled??
#endif
