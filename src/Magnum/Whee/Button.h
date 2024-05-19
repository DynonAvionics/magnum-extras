#ifndef Magnum_Whee_Button_h
#define Magnum_Whee_Button_h
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

/** @file
 * @brief Class @ref Magnum::Whee::Button, function @ref Magnum::Whee::button(), enum @ref Magnum::Whee::ButtonStyle
 * @m_since_latest
 */

#include "Magnum/Whee/Widget.h"

namespace Magnum { namespace Whee {

/**
@brief Button style
@m_since_latest

@see @ref Button, @ref button()
*/
enum class ButtonStyle: UnsignedByte {
    Default,        /**< Default */
    Primary,        /**< Primary */
    Success,        /**< Success */
    Warning,        /**< Warning */
    Danger,         /**< Danger */
    Info,           /**< Info */
    Dim,            /**< Dim */
    Flat            /**< Flat */
};

/**
@debugoperatorenum{ButtonStyle}
@m_since_latest
*/
MAGNUM_WHEE_EXPORT Debug& operator<<(Debug& debug, ButtonStyle value);

/**
@brief Button widget
@m_since_latest
*/
class MAGNUM_WHEE_EXPORT Button: public Widget {
    public:
        /**
         * @brief Construct an icon button
         * @param anchor            Positioning anchor
         * @param style             Button style
         * @param icon              Button icon. Passing @ref Icon::None makes
         *      the button empty.
         *
         * The button can be subsequently converted to text-only or icon + text
         * using @ref setIcon() and @ref setText().
         * @see @ref button(const Anchor&, ButtonStyle, Icon)
         */
        explicit Button(const Anchor& anchor, ButtonStyle style, Icon icon);

        /**
         * @brief Construct a text button
         * @param anchor            Positioning anchor
         * @param style             Button style
         * @param text              Button text. Passing an empty string makes
         *      the button empty.
         * @param textProperties    Text shaping and layouting properties
         *
         * The button can be subsequently converted to icon-only or icon + text
         * using @ref setIcon() and @ref setText().
         * @see @ref button(const Anchor&, ButtonStyle, Containers::StringView, const TextProperties&)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        explicit Button(const Anchor& anchor, ButtonStyle style, Containers::StringView text, const TextProperties& textProperties = {});
        #else
        /* To avoid having to include TextProperties.h */
        explicit Button(const Anchor& anchor, ButtonStyle style, Containers::StringView text, const TextProperties& textProperties);
        explicit Button(const Anchor& anchor, ButtonStyle style, Containers::StringView text);
        #endif

        /**
         * @brief Construct an icon + text button
         * @param anchor            Positioning anchor
         * @param style             Button style
         * @param icon              Button icon. Passing @ref Icon::None
         *      creates the button without an icon.
         * @param text              Button text. Passing an empty string
         *      creates the button without a text.
         * @param textProperties    Text shaping and layouting properties
         *
         * The button can be subsequently converted to icon-only or text-only
         * using @ref setIcon() and @ref setText().
         * @see @ref button(const Anchor&, ButtonStyle, Icon, Containers::StringView, const TextProperties&)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        explicit Button(const Anchor& anchor, ButtonStyle style, Icon icon, Containers::StringView text, const TextProperties& textProperties = {});
        #else
        /* To avoid having to include TextProperties.h */
        explicit Button(const Anchor& anchor, ButtonStyle style, Icon icon, Containers::StringView text, const TextProperties& textProperties);
        explicit Button(const Anchor& anchor, ButtonStyle style, Icon icon, Containers::StringView text);
        #endif

        /**
         * @brief Background data
         *
         * Exposed mainly for testing purposes, not meant to be modified
         * directly.
         */
        DataHandle backgroundData() const;

        /**
         * @brief Icon data or @ref DataHandle::Null
         *
         * Exposed mainly for testing purposes, not meant to be modified
         * directly.
         */
        DataHandle iconData() const;

        /** @brief Style */
        ButtonStyle style() const { return _style; }

        /**
         * @brief Set style
         *
         * Note that calling this function doesn't change the font if the new
         * style uses a different one, you have to call @ref setText()
         * afterwards to make it pick it up.
         * @see @ref setIcon(), @ref setText()
         */
        void setStyle(ButtonStyle style);

        /** @brief Icon */
        Icon icon() const { return _icon; }

        /**
         * @brief Set icon
         *
         * Passing @ref Icon::None removes the icon.
         * @see @ref setText(), @ref setStyle()
         */
        void setIcon(Icon icon);

        /**
         * @brief Text data or @ref DataHandle::Null
         *
         * Exposed mainly for testing purposes, not meant to be modified
         * directly.
         */
        DataHandle textData() const;

        /**
         * @brief Set text
         *
         * Passing an empty @p text removes the text.
         * @see @ref setIcon(), @ref setStyle()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        void setText(Containers::StringView text, const TextProperties& textProperties = {});
        #else
        /* To avoid having to include TextProperties.h */
        void setText(Containers::StringView text, const TextProperties& textProperties);
        void setText(Containers::StringView text);
        #endif

    private:
        ButtonStyle _style;
        /* 3 bytes free */
        Icon _icon;
        LayerDataHandle _backgroundData, _iconData, _textData;
};

/**
@brief Stateless icon button widget
@param anchor           Positioning anchor
@param style            Button style
@param icon             Button icon. Passing @ref Icon::None makes the button
    empty.
@m_since_latest

Compared to @ref Button::Button(const Anchor&, ButtonStyle, Icon)
this creates a stateless button that doesn't have any class instance that would
need to be kept in scope and eventually destructed, making it more lightweight.
As a consequence it can't have its style, icon or text subsequently changed and
is removed only when the node or its parent get removed.
*/
MAGNUM_WHEE_EXPORT NodeHandle button(const Anchor& anchor, ButtonStyle style, Icon icon);

/**
@brief Stateless text button widget
@param anchor           Positioning anchor
@param style            Button style
@param text             Button text. Passing an empty string makes the button
    empty.
@param textProperties   Text shaping and layouting properties
@m_since_latest

Compared to @ref Button::Button(const Anchor&, ButtonStyle, Containers::StringView, const TextProperties&)
this creates a stateless button that doesn't have any class instance that would
need to be kept in scope and eventually destructed, making it more lightweight.
As a consequence it can't have its style, icon or text subsequently changed and
is removed only when the node or its parent get removed.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
MAGNUM_WHEE_EXPORT NodeHandle button(const Anchor& anchor, ButtonStyle style, Containers::StringView text, const TextProperties& textProperties = {});
#else
/* To avoid having to include TextProperties.h */
MAGNUM_WHEE_EXPORT NodeHandle button(const Anchor& anchor, ButtonStyle style, Containers::StringView text, const TextProperties& textProperties);
MAGNUM_WHEE_EXPORT NodeHandle button(const Anchor& anchor, ButtonStyle style, Containers::StringView text);
#endif

/**
@brief Stateless icon + text button widget
@param anchor           Positioning anchor
@param style            Button style
@param icon             Button icon. Passing @ref Icon::None creates the button
    without an icon.
@param text             Button text. Passing an empty string creates the button
    without a text.
@param textProperties   Text shaping and layouting properties
@m_since_latest

Compared to @ref Button::Button(const Anchor&, ButtonStyle, Icon, Containers::StringView, const TextProperties&)
this creates a stateless button that doesn't have any class instance that would
need to be kept in scope and eventually destructed, making it more lightweight.
As a consequence it can't have its style, icon or text subsequently changed and
is removed only when the node or its parent get removed.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
MAGNUM_WHEE_EXPORT NodeHandle button(const Anchor& anchor, ButtonStyle style, Icon icon, Containers::StringView text, const TextProperties& textProperties = {});
#else
/* To avoid having to include TextProperties.h */
MAGNUM_WHEE_EXPORT NodeHandle button(const Anchor& anchor, ButtonStyle style, Icon icon, Containers::StringView text, const TextProperties& textProperties);
MAGNUM_WHEE_EXPORT NodeHandle button(const Anchor& anchor, ButtonStyle style, Icon icon, Containers::StringView text);
#endif

}}

#endif
