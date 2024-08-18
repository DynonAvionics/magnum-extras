#ifndef Magnum_Whee_TextLayer_h
#define Magnum_Whee_TextLayer_h
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
 * @brief Class @ref Magnum::Whee::TextLayer, struct @ref Magnum::Whee::TextLayerCommonStyleUniform, @ref Magnum::Whee::TextLayerStyleUniform, enum @ref Magnum::Whee::FontHandle, @ref Magnum::Whee::TextDataFlag, @ref Magnum::Whee::TextEdit, enum set @ref Magnum::Whee::TextDataFlags, function @ref Magnum::Whee::fontHandle(), @ref Magnum::Whee::fontHandleId(), @ref Magnum::Whee::fontHandleGeneration()
 * @m_since_latest
 */

#include <Corrade/Containers/StringView.h> /* for templated create() overloads */
#include <Corrade/PluginManager/Manager.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Text/Text.h>

#include "Magnum/Whee/AbstractVisualLayer.h"

namespace Magnum { namespace Whee {

/**
@brief Properties common to all @ref TextLayer style uniforms
@m_since_latest

Together with one or more @ref TextLayerStyleUniform instances contains style
properties that are used by the @ref TextLayer shaders to draw the layer data,
packed in a form that allows direct usage in uniform buffers. Is uploaded
using @ref TextLayer::Shared::setStyle(), style data that aren't used by the
shader are passed to the function separately.

Currently this is just a placeholder with no properties.
*/
struct alignas(4) TextLayerCommonStyleUniform {
    /** @brief Construct with default values */
    constexpr explicit TextLayerCommonStyleUniform(DefaultInitT = DefaultInit) noexcept {}

    /** @brief Construct without initializing the contents */
    explicit TextLayerCommonStyleUniform(NoInitT) noexcept {}

    /** @{
     * @name Convenience setters
     *
     * Provided to allow the use of method chaining for populating a structure
     * in a single expression, otherwise equivalent to accessing the fields
     * directly. Also guaranteed to provide backwards compatibility when
     * packing of the actual fields changes.
     */

    /**
     * @}
     */

    /** @todo remove the alignas once actual attributes are here */
    #ifndef DOXYGEN_GENERATING_OUTPUT
    Int:32;
    Int:32;
    Int:32;
    Int:32;
    #endif
};

/**
@brief @ref TextLayer style uniform
@m_since_latest

Instances of this class together with @ref TextLayerCommonStyleUniform contain
style properties that are used by the @ref TextLayer shaders to draw the layer
data, packed in a form that allows direct usage in uniform buffers. Total count
of styles is specified with the
@ref TextLayer::Shared::Configuration::Configuration() constructor, uniforms
are then uploaded using @ref TextLayer::Shared::setStyle(), style data that
aren't used by the shader are passed to the function separately.
*/
struct TextLayerStyleUniform {
    /** @brief Construct with default values */
    constexpr explicit TextLayerStyleUniform(DefaultInitT = DefaultInit) noexcept: color{1.0f} {}

    /** @brief Constructor */
    constexpr /*implicit*/ TextLayerStyleUniform(const Color4& color): color{color} {}

    /** @brief Construct without initializing the contents */
    explicit TextLayerStyleUniform(NoInitT) noexcept: color{NoInit} {}

    /** @{
     * @name Convenience setters
     *
     * Provided to allow the use of method chaining for populating a structure
     * in a single expression, otherwise equivalent to accessing the fields
     * directly. Also guaranteed to provide backwards compatibility when
     * packing of the actual fields changes.
     */

    /**
     * @brief Set the @ref color field
     * @return Reference to self (for method chaining)
     */
    TextLayerStyleUniform& setColor(const Color4& color) {
        this->color = color;
        return *this;
    }

    /**
     * @}
     */

    /**
     * @brief Color
     *
     * Default value is @cpp 0xffffffff_srgbf @ce.
     */
    Color4 color;
};

/* Unlike DataHandle, NodeHandle etc., which are global to the whole Whee
   library, FontHandle is specific to the TextLayer and thus isn't defined in
   Handle.h but here */

namespace Implementation {
    enum: UnsignedInt {
        FontHandleIdBits = 15,
        FontHandleGenerationBits = 1
    };
}

/**
@brief Font handle
@m_since_latest

Uses 15 bits for storing an ID and 1 bit for a generation.
@see @ref TextLayer::Shared::addFont(), @ref fontHandle(), @ref fontHandleId(),
    @ref fontHandleGeneration()
*/
enum class FontHandle: UnsignedShort {
    Null = 0    /**< Null handle */
};

/**
@debugoperatorenum{FontHandle}
@m_since_latest
*/
MAGNUM_WHEE_EXPORT Debug& operator<<(Debug& debug, FontHandle value);

/**
@brief Compose a font handle from an ID and a generation
@m_since_latest

Expects that the ID fits into 20 bits and the generation into 12 bits. Use
@ref fontHandleId() and @ref fontHandleGeneration() for an inverse operation.
*/
constexpr FontHandle fontHandle(UnsignedInt id, UnsignedInt generation) {
    return (CORRADE_CONSTEXPR_DEBUG_ASSERT(id < (1 << Implementation::FontHandleIdBits) && generation < (1 << Implementation::FontHandleGenerationBits),
        "Whee::fontHandle(): expected index to fit into" << Implementation::FontHandleIdBits << "bits and generation into" << Implementation::FontHandleGenerationBits << Debug::nospace << ", got" << Debug::hex << id << "and" << Debug::hex << generation), FontHandle(id|(generation << Implementation::FontHandleIdBits)));
}

/**
@brief Extract ID from a font handle
@m_since_latest

For @ref FontHandle::Null returns @cpp 0 @ce. Use @ref fontHandleGeneration()
for extracting the generation and @ref fontHandle() for an inverse operation.
*/
constexpr UnsignedInt fontHandleId(FontHandle handle) {
    return UnsignedInt(handle) & ((1 << Implementation::FontHandleIdBits) - 1);
}

/**
@brief Extract generation from a font handle
@m_since_latest

For @ref FontHandle::Null returns @cpp 0 @ce. Use @ref fontHandleId() for
extracting the ID and @ref fontHandle() for an inverse operation.
*/
constexpr UnsignedInt fontHandleGeneration(FontHandle handle) {
    return UnsignedInt(handle) >> Implementation::FontHandleIdBits;
}

/**
@brief Text layer data flag
@m_since_latest

@see @ref TextDataFlags, @ref TextLayer::create(), @ref TextLayer::flags()
*/
enum class TextDataFlag: UnsignedByte {
    /**
     * Editable text. If data that have it enabled are attached to a currently
     * focused node, the layer reacts to text input and key events, allowing to
     * edit the contents. Unlike non-editable text, the contents are also
     * accessible through @ref TextLayer::text().
     */
    Editable = 1 << 0,
};

/**
@debugoperatorenum{TextDataFlag}
@m_since_latest
*/
MAGNUM_WHEE_EXPORT Debug& operator<<(Debug& debug, TextDataFlag value);

/**
@brief Text layer data flags
@m_since_latest

@see @ref TextLayer::create(), @ref TextLayer::flags()
*/
typedef Containers::EnumSet<TextDataFlag> TextDataFlags;

CORRADE_ENUMSET_OPERATORS(TextDataFlags)

/**
@debugoperatorenum{TextDataFlags}
@m_since_latest
*/
MAGNUM_WHEE_EXPORT Debug& operator<<(Debug& debug, TextDataFlags value);

/**
@brief Text edit operation

@see @ref TextLayer::editText()
*/
enum class TextEdit: UnsignedByte {
    /**
     * Move cursor one character to the left and discard any selection,
     * equivalent to the @m_class{m-label m-default} **Left** key. The
     * @ref TextLayer::editText() @p text is expected to be empty.
     *
     * If the character on the left to the @ref TextLayer::cursor() is a valid
     * UTF-8 byte sequence, it moves past the whole sequence, otherwise just by
     * one byte. Note that, currently, if a character is combined out of
     * multiple Unicode codepoints (such as combining diacritics), the cursor
     * will not jump over the whole combined sequence but move inside.
     *
     * By default, the cursor moves backward in the byte stream, which is
     * optically to the left for left-to-right alphabets. If
     * @ref TextProperties::shapeDirection() is
     * @ref Text::ShapeDirection::RightToLeft or if it's
     * @relativeref{Text::ShapeDirection,Unspecified} and was detected to be
     * @relativeref{Text::ShapeDirection,RightToLeft} after calling
     * @ref Text::AbstractShaper::shape() internally, the cursor still moves
     * *optically* to the left, although it's forward in the byte stream.
     * @see @ref TextEdit::ExtendSelectionLeft
     */
    MoveCursorLeft,

    /**
     * Extend or shrink selection to the left of the cursor, equivalent to the
     * @m_class{m-label m-warning} **Shift** @m_class{m-label m-default} **Left**
     * key combination. The @ref TextLayer::editText() @p text is expected to
     * be empty.
     *
     * If the character on the left to the @ref TextLayer::cursor() is a valid
     * UTF-8 byte sequence, it extends past the whole sequence, otherwise just
     * by one byte. Note that, currently, if a character is combined out of
     * multiple Unicode codepoints (such as combining diacritics), the cursor
     * will not jump over the whole combined sequence but move inside.
     *
     * By default, the cursor moves backward in the byte stream, which is
     * optically to the left for left-to-right alphabets. If
     * @ref TextProperties::shapeDirection() is
     * @ref Text::ShapeDirection::RightToLeft or if it's
     * @relativeref{Text::ShapeDirection,Unspecified} and was detected to be
     * @relativeref{Text::ShapeDirection,RightToLeft} after calling
     * @ref Text::AbstractShaper::shape() internally, the cursor still moves
     * *optically* to the left, although it's forward in the byte stream.
     * @see @ref TextEdit::MoveCursorLeft
     */
    ExtendSelectionLeft,

    /**
     * Move cursor one character to the right and discard any selection,
     * equivalent to the @m_class{m-label m-default} **Right** key. The
     * @ref TextLayer::editText() @p text is expected to be empty.
     *
     * If the character on the right to the @ref TextLayer::cursor() is a valid
     * UTF-8 byte sequence, it moves past the whole sequence, otherwise just by
     * one byte. Note that, currently, if a character is combined out of
     * multiple Unicode codepoints (such as combining diacritics), the cursor
     * will not jump over the whole combined sequence but move inside.
     *
     * By default, the cursor moves backward in the byte stream, which is
     * optically to the left for left-to-right alphabets. If
     * @ref TextProperties::shapeDirection() is
     * @ref Text::ShapeDirection::RightToLeft or if it's
     * @relativeref{Text::ShapeDirection,Unspecified} and was detected to be
     * @relativeref{Text::ShapeDirection,RightToLeft} after calling
     * @ref Text::AbstractShaper::shape() internally, the cursor still moves
     * *optically* to the right, although it's forward in the byte stream.
     * @see @ref TextEdit::ExtendSelectionRight
     */
    MoveCursorRight,

    /**
     * Extend or shrink selection to the right of the cursor, equivalent to the
     * @m_class{m-label m-warning} **Shift** @m_class{m-label m-default} **Right**
     * key combination. The @ref TextLayer::editText() @p text is expected to
     * be empty.
     *
     * If the character on the right to the @ref TextLayer::cursor() is a valid
     * UTF-8 byte sequence, it moves past the whole sequence, otherwise just by
     * one byte. Note that, currently, if a character is combined out of
     * multiple Unicode codepoints (such as combining diacritics), the cursor
     * will not jump over the whole combined sequence but move inside.
     *
     * By default, the cursor moves backward in the byte stream, which is
     * optically to the left for left-to-right alphabets. If
     * @ref TextProperties::shapeDirection() is
     * @ref Text::ShapeDirection::RightToLeft or if it's
     * @relativeref{Text::ShapeDirection,Unspecified} and was detected to be
     * @relativeref{Text::ShapeDirection,RightToLeft} after calling
     * @ref Text::AbstractShaper::shape() internally, the cursor still moves
     * *optically* to the right, although it's forward in the byte stream.
     * @see @ref TextEdit::MoveCursorRight
     */
    ExtendSelectionRight,

    /**
     * Move cursor at the beginning of the line and discard any selection,
     * equivalent to the @m_class{m-label m-default} **Home** key. The
     * @ref TextLayer::editText() @p text is expected to be empty.
     *
     * The cursor moves to the begin of the byte stream, which is optically on
     * the left for left-to-right alphabets. If
     * @ref TextProperties::shapeDirection() is
     * @ref Text::ShapeDirection::RightToLeft or if it's
     * @relativeref{Text::ShapeDirection,Unspecified} and was detected to be
     * @relativeref{Text::ShapeDirection,RightToLeft} after calling
     * @ref Text::AbstractShaper::shape() internally, it's still the begin of
     * the byte stream, but *optically* after the rightmost character.
     * @see @ref TextEdit::ExtendSelectionLineBegin
     */
    MoveCursorLineBegin,

    /**
     * Extend selection to the beginning of the line, equivalent to the
     * @m_class{m-label m-warning} **Shift** @m_class{m-label m-default} **Home**
     * key combination. The @ref TextLayer::editText() @p text is expected to
     * be empty.
     *
     * The cursor moves to the begin of the byte stream, which is optically on
     * the left for left-to-right alphabets. If
     * @ref TextProperties::shapeDirection() is
     * @ref Text::ShapeDirection::RightToLeft or if it's
     * @relativeref{Text::ShapeDirection,Unspecified} and was detected to be
     * @relativeref{Text::ShapeDirection,RightToLeft} after calling
     * @ref Text::AbstractShaper::shape() internally, it's still the begin of
     * the byte stream, but *optically* after the rightmost character.
     * @see @ref TextEdit::MoveCursorLineBegin
     */
    ExtendSelectionLineBegin,

    /**
     * Move cursor at the end of the line and discard any selection, equivalent
     * to the @m_class{m-label m-default} **End** key. The
     * @ref TextLayer::editText() @p text is expected to be empty.
     *
     * The cursor moves to the end of in the byte stream, which is optically on
     * the right for left-to-right alphabets. If
     * @ref TextProperties::shapeDirection() is
     * @ref Text::ShapeDirection::RightToLeft or if it's
     * @relativeref{Text::ShapeDirection,Unspecified} and was detected to be
     * @relativeref{Text::ShapeDirection,RightToLeft} after calling
     * @ref Text::AbstractShaper::shape() internally, it's still the end ofthe
     * byte stream, but *optically* before the leftmost character.
     * @see @ref TextEdit::ExtendSelectionLineEnd
     */
    MoveCursorLineEnd,

    /**
     * Extend selection to the end of the line, equivalent to the
     * @m_class{m-label m-warning} **Shift** @m_class{m-label m-default} **End**
     * key combination. The @ref TextLayer::editText() @p text is expected to
     * be empty.
     *
     * The cursor moves to the end of in the byte stream, which is optically on
     * the right for left-to-right alphabets. If
     * @ref TextProperties::shapeDirection() is
     * @ref Text::ShapeDirection::RightToLeft or if it's
     * @relativeref{Text::ShapeDirection,Unspecified} and was detected to be
     * @relativeref{Text::ShapeDirection,RightToLeft} after calling
     * @ref Text::AbstractShaper::shape() internally, it's still the end ofthe
     * byte stream, but *optically* before the leftmost character.
     * @see @ref TextEdit::MoveCursorLineEnd
     */
    ExtendSelectionLineEnd,

    /**
     * Remove selection or character before cursor, equivalent to the
     * @m_class{m-label m-default} **Backspace** key. The
     * @ref TextLayer::editText() @p text is expected to be empty.
     *
     * If the character before @ref TextLayer::cursor() is a valid UTF-8 byte
     * sequence, it deletes the whole sequence, otherwise just one byte. Note
     * that, currently, if a character is combined out of multiple Unicode
     * codepoints (such as combining diacritics), the cursor will not delete
     * the whole combined sequence but only a part.
     *
     * The deletion goes backward in the byte stream, which is optically to the
     * left for left-to-right alphabets. If
     * @ref TextProperties::shapeDirection() is
     * @ref Text::ShapeDirection::RightToLeft or if it's
     * @relativeref{Text::ShapeDirection,Unspecified} and was detected to be
     * @relativeref{Text::ShapeDirection,RightToLeft} after calling
     * @ref Text::AbstractShaper::shape() internally, the deletion still goes
     * backward in the byte stream, but *optically* it's to the right.
     *
     * For a non-empty selection the behavior is the same as
     * @ref TextEdit::RemoveAfterCursor.
     */
    RemoveBeforeCursor,

    /**
     * Remove selection or character after cursor, equivalent to the
     * @m_class{m-label m-default} **Delete** key. The
     * @ref TextLayer::editText() @p text is expected to be empty.
     *
     * If the character after @ref TextLayer::cursor() is a valid UTF-8 byte
     * sequence, it deletes the whole sequence, otherwise just one byte. Note
     * that, currently, if a character is combined out of multiple Unicode
     * codepoints (such as combining diacritics), the cursor will not delete
     * the whole combined sequence but only a part.
     *
     * The deletion goes forward in the byte stream, which is optically to the
     * right for left-to-right alphabets. If
     * @ref TextProperties::shapeDirection() is
     * @ref Text::ShapeDirection::RightToLeft or if it's
     * @relativeref{Text::ShapeDirection,Unspecified} and was detected to be
     * @relativeref{Text::ShapeDirection,RightToLeft} after calling
     * @ref Text::AbstractShaper::shape() internally, the deletion still goes
     * forward in the byte stream, but *optically* it's to the left.
     *
     * For a non-empty selection the behavior is the same as
     * @ref TextEdit::RemoveBeforeCursor.
     */
    RemoveAfterCursor,

    /**
     * Insert text before the cursor, replacing the selection if any. This will
     * cause the cursor to advance *after* the inserted text, which is the
     * usual text editing behavior.
     *
     * No UTF-8 byte sequence adjustment is performed in this case, i.e. it's
     * assumed that the cursor is at a valid UTF-8 character boundary and the
     * inserted text ends with a valid UTF-8 character as well.
     *
     * The insertion goes forward in the byte stream, which is optically to the
     * right for left-to-right alphabets. If
     * @ref TextProperties::shapeDirection() is
     * @ref Text::ShapeDirection::RightToLeft or if it's
     * @relativeref{Text::ShapeDirection,Unspecified} and was detected to be
     * @relativeref{Text::ShapeDirection,RightToLeft} after calling
     * @ref Text::AbstractShaper::shape() internally, the insertion still goes
     * forward in the byte stream, but *optically* it's to the left.
     */
    InsertBeforeCursor,

    /**
     * Insert text after the cursor, replacing the selection if any. Compared
     * to @ref TextEdit::InsertBeforeCursor, the cursor stays at the original
     * position, *before* the inserted text, other than that the behavior is
     * the same. Useful for autocompletion hints, for example.
     *
     * Like with @ref TextEdit::InsertBeforeCursor, no UTF-8 byte sequence
     * adjustment is performed in this case, i.e. it's assumed that the cursor
     * is at a valid UTF-8 character boundary and the inserted text ends with a
     * valid UTF-8 character as well.
     */
    InsertAfterCursor,
};

/**
@debugoperatorenum{TextEdit}
@m_since_latest
*/
MAGNUM_WHEE_EXPORT Debug& operator<<(Debug& debug, TextEdit value);

/**
@brief Text layer
@m_since_latest

Draws text laid out using the @ref Text library. You'll most likely instantiate
the class through @ref TextLayerGL, which contains a concrete OpenGL
implementation.
@see @ref UserInterface::textLayer(),
    @ref UserInterface::setTextLayerInstance(), @ref StyleFeature::TextLayer,
    @ref TextLayerStyleAnimator
*/
class MAGNUM_WHEE_EXPORT TextLayer: public AbstractVisualLayer {
    public:
        class Shared;

        /**
         * @brief Shared state used by this layer
         *
         * Reference to the instance passed to @ref TextLayerGL::TextLayerGL(LayerHandle, Shared&).
         */
        inline Shared& shared();
        inline const Shared& shared() const; /**< @overload */

        /**
         * @brief Set this layer to be associated with a style animator
         * @return Reference to self (for method chaining)
         *
         * Expects that @ref Shared::dynamicStyleCount() is non-zero and that
         * this function hasn't been called with on animator yet. On the other
         * hand, it's possible to associate multiple different animators with
         * the same layer.
         */
        TextLayer& setAnimator(TextLayerStyleAnimator& animator);

        /**
         * @brief Dynamic style uniforms
         *
         * Size of the returned view is @ref Shared::dynamicStyleCount(). These
         * uniforms are used by style indices greater than or equal to
         * @ref Shared::styleCount().
         * @see @ref dynamicStyleFonts(), @ref dynamicStyleAlignments(),
         *      @ref dynamicStyleFeatures(), @ref dynamicStylePaddings(),
         *      @ref setDynamicStyle()
         */
        Containers::ArrayView<const TextLayerStyleUniform> dynamicStyleUniforms() const;

        /**
         * @brief Dynamic style fonts
         *
         * Size of the returned view is @ref Shared::dynamicStyleCount(). These
         * fonts are used by style indices greater than or equal to
         * @ref Shared::styleCount().
         * @see @ref dynamicStyleUniforms(), @ref dynamicStyleAlignments(),
         *      @ref dynamicStyleFeatures(), @ref dynamicStylePaddings(),
         *      @ref setDynamicStyle()
         */
        Containers::StridedArrayView1D<const FontHandle> dynamicStyleFonts() const;

        /**
         * @brief Dynamic style alignments
         *
         * Size of the returned view is @ref Shared::dynamicStyleCount(). These
         * alignments are used by style indices greater than or equal to
         * @ref Shared::styleCount().
         * @see @ref dynamicStyleUniforms(), @ref dynamicStyleFonts(),
         *      @ref dynamicStyleFeatures(), @ref dynamicStylePaddings(),
         *      @ref setDynamicStyle()
         */
        Containers::StridedArrayView1D<const Text::Alignment> dynamicStyleAlignments() const;

        /**
         * @brief Dynamic style font features
         *
         * Expects that the @p id is less than @ref Shared::dynamicStyleCount().
         * @see @ref dynamicStyleUniforms(), @ref dynamicStyleFonts(),
         *      @ref dynamicStyleAlignments(), @ref dynamicStylePaddings(),
         *      @ref setDynamicStyle()
         */
        Containers::ArrayView<const TextFeatureValue> dynamicStyleFeatures(UnsignedInt id) const;

        /**
         * @brief Dynamic style paddings
         *
         * Size of the returned view is @ref Shared::dynamicStyleCount(). These
         * paddings are used by style indices greater than or equal to
         * @ref Shared::styleCount().
         * @see @ref dynamicStyleUniforms(), @ref dynamicStyleFonts(),
         *      @ref dynamicStyleAlignments(), @ref setDynamicStyle()
         */
        Containers::StridedArrayView1D<const Vector4> dynamicStylePaddings() const;

        /**
         * @brief Set a dynamic style
         * @param id                Dynamic style ID
         * @param uniform           Style uniform
         * @param font              Font handle
         * @param padding           Padding inside the node in order left, top,
         *      right, bottom
         *
         * Expects that the @p id is less than @ref Shared::dynamicStyleCount(),
         * @p font is either @ref FontHandle::Null or valid and @p alignment is
         * not `*GlyphBounds` as the implementation can only align based on
         * font metrics and cursor position, not actual glyph bounds.
         * @ref Shared::styleCount() plus @p id is then a style index that can
         * be passed to @ref create(), @ref createGlyph() or @ref setStyle() in
         * order to use this style. Compared to @ref Shared::setStyle() the
         * mapping between dynamic styles and uniforms is implicit. All dynamic
         * styles are initially default-constructed @ref TextLayerStyleUniform
         * instances, @ref FontHandle::Null, @ref Text::Alignment::MiddleCenter
         * with empty feature lists and zero padding vectors.
         *
         * Calling this function causes @ref LayerState::NeedsCommonDataUpdate
         * to be set to trigger an upload of changed dynamic style uniform
         * data. If @p padding changed, @ref LayerState::NeedsDataUpdate gets
         * set as well. On the other hand, changing the @p font, @p alignment
         * or @p features doesn't cause @ref LayerState::NeedsDataUpdate to be
         * set --- the @p font, @p alignment and @p features get only used in
         * the following call to @ref create(), @ref createGlyph(),
         * @ref setText() or @ref setGlyph().
         * @see @ref dynamicStyleUniforms(), @ref dynamicStyleFonts(),
         *      @ref dynamicStyleAlignments(), @ref dynamicStyleFeatures(),
         *      @ref dynamicStylePaddings(), @ref allocateDynamicStyle(),
         *      @ref recycleDynamicStyle()
         */
        void setDynamicStyle(UnsignedInt id, const TextLayerStyleUniform& uniform, FontHandle font, Text::Alignment alignment, Containers::ArrayView<const TextFeatureValue> features, const Vector4& padding);
        /** @overload */
        void setDynamicStyle(UnsignedInt id, const TextLayerStyleUniform& uniform, FontHandle font, Text::Alignment alignment, std::initializer_list<TextFeatureValue> features, const Vector4& padding);

        /**
         * @brief Create a text
         * @param style         Style index
         * @param text          Text to render
         * @param properties    Text properties
         * @param flags         Flags
         * @param node          Node to attach to
         * @return New data handle
         *
         * Expects that @ref Shared::setStyle() has been called, @p style is
         * less than @ref Shared::totalStyleCount() and
         * @ref TextProperties::font() is either @ref FontHandle::Null or
         * valid. Styling is driven from the @ref TextLayerStyleUniform at
         * index @p style. If @ref TextProperties::font() is not null it's
         * used, otherwise the default @ref FontHandle assigned to given style
         * is used and is expected to not be null. The @ref FontHandle, whether
         * coming from the @p style or from @p properties, is expected to have
         * a font instance. Instance-less fonts can be only used to create
         * single glyphs (such as various icons or images) with
         * @ref createGlyph().
         *
         * If @p flags contain @ref TextDataFlag::Editable, the @p text and
         * @p properties are remembered and subsequently accessible through
         * @ref text() and @ref textProperties(), @ref cursor() position and
         * selection are both set to @p text size. Currently, for editable
         * text, the @p properties are expected to have empty
         * @ref TextProperties::features() --- only the features supplied by
         * the style are used for editable text.
         *
         * Use @ref create(UnsignedInt, Containers::StringView, const TextProperties&, const Color3&, TextDataFlags, NodeHandle)
         * for creating a text with a custom color. This function is equivalent
         * to calling it with @cpp 0xffffff_srgbf @ce.
         * @see @ref Shared::hasFontInstance(), @ref setText(),
         *      @ref setCursor(), @ref updateText(), @ref editText()
         */
        DataHandle create(UnsignedInt style, Containers::StringView text, const TextProperties& properties, TextDataFlags flags = {}, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        ) {
            return create(style, text, properties, Color3{1.0f}, flags, node);
        }

        /** @overload */
        DataHandle create(UnsignedInt style, Containers::StringView text, const TextProperties& properties, NodeHandle node) {
            return create(style, text, properties, TextDataFlags{}, node);
        }

        /**
         * @brief Create a text with a style index in a concrete enum type
         *
         * Casts @p style to @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref create(UnsignedInt, Containers::StringView, const TextProperties&, TextDataFlags, NodeHandle).
         */
        template<class StyleIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value>::type
            #endif
        > DataHandle create(StyleIndex style, Containers::StringView text, const TextProperties& properties, TextDataFlags flags = {}, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        ) {
            return create(UnsignedInt(style), text, properties, flags, node);
        }

        /** @overload */
        template<class StyleIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value>::type
            #endif
        > DataHandle create(StyleIndex style, Containers::StringView text, const TextProperties& properties, NodeHandle node) {
            return create(style, text, properties, TextDataFlags{}, node);
        }

        /**
         * @brief Create a text with a custom color
         * @param style         Style index
         * @param text          Text to render
         * @param properties    Text properties
         * @param flags         Flags
         * @param color         Custom color
         * @param node          Node to attach to
         * @return New data handle
         *
         * Expects that @ref Shared::setStyle() has been called, @p style is
         * less than @ref Shared::totalStyleCount() and
         * @ref TextProperties::font() is either @ref FontHandle::Null or
         * valid. Styling is driven from the @ref TextLayerStyleUniform at
         * index @p style. If @ref TextProperties::font() is not null it's
         * used, otherwise the default @ref FontHandle assigned to given style
         * is used and is expected to not be null. In addition
         * @ref TextLayerStyleUniform::color is multiplied with @p color. The
         * @ref FontHandle, whether coming from the @p style or from
         * @p properties, is expected to have a font instance. Instance-less
         * fonts can be only used to create single glyphs (such as various
         * icons or images) with @ref createGlyph().
         *
         * If @p flags contain @ref TextDataFlag::Editable, the @p text and
         * @p properties are remembered and subsequently accessible through
         * @ref text() and @ref textProperties(), @ref cursor() position and
         * selection are both set to @p text size. Currently, for editable
         * text, the @p properties are expected to have empty
         * @ref TextProperties::features() --- only the features supplied by
         * the style are used for editable text.
         *
         * @see @ref create(UnsignedInt, Containers::StringView, const TextProperties&, TextDataFlags, NodeHandle),
         *      @ref Shared::hasFontInstance(), @ref setText(),
         *      @ref setCursor(), @ref updateText(), @ref editText()
         */
        DataHandle create(UnsignedInt style, Containers::StringView text, const TextProperties& properties, const Color3& color, TextDataFlags flags = {}, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        );

        /** @overload */
        DataHandle create(UnsignedInt style, Containers::StringView text, const TextProperties& properties, const Color3& color, NodeHandle node) {
            return create(style, text, properties, color, TextDataFlags{}, node);
        }

        /**
         * @brief Create a text with a style index in a concrete enum type and a custom color
         *
         * Casts @p style to @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref create(UnsignedInt, Containers::StringView, const TextProperties&, const Color3&, TextDataFlags, NodeHandle).
         */
        template<class StyleIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value>::type
            #endif
        > DataHandle create(StyleIndex style, Containers::StringView text, const TextProperties& properties, const Color3& color, TextDataFlags flags = {}, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        ) {
            return create(UnsignedInt(style), text, properties, color, flags, node);
        }

        /** @overload */
        template<class StyleIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value>::type
            #endif
        > DataHandle create(StyleIndex style, Containers::StringView text, const TextProperties& properties, const Color3& color, NodeHandle node) {
            return create(style, text, properties, color, TextDataFlags{}, node);
        }

        /**
         * @brief Create a single glyph
         * @param style         Style index
         * @param glyph         Glyph ID to render
         * @param properties    Text properties
         * @param node          Node to attach to
         * @return New data handle
         *
         * Expects that @ref Shared::setStyle() has been called, @p style is
         * less than @ref Shared::totalStyleCount() and
         * @ref TextProperties::font() is either @ref FontHandle::Null or
         * valid. Styling is driven from the @ref TextLayerStyleUniform at
         * index @p style. If @ref TextProperties::font() is not null it's
         * used, otherwise the default @ref FontHandle assigned to given style
         * is used and is expected to not be null. The @p glyph is expected to
         * be less than @ref Text::AbstractGlyphCache::fontGlyphCount() for
         * given font. Use @ref createGlyph(UnsignedInt, UnsignedInt, const TextProperties&, const Color3&, NodeHandle)
         * for creating a glyph with a custom color. This function is
         * equivalent to calling it with @cpp 0xffffff_srgbf @ce.
         *
         * Compared to @ref create(), the glyph is aligned according to
         * @ref TextProperties::alignment() based on its bounding rectangle
         * coming from the glyph cache, not based on font metrics. If the
         * @ref Text::Alignment is `*Start` or `*End`, it's converted to an
         * appropriate `*Left` or `*Right` value based on
         * @ref TextProperties::layoutDirection() and
         * @relativeref{TextProperties,shapeDirection()}. The
         * @ref TextProperties::script(),
         * @relativeref{TextProperties,language()} and
         * @relativeref{TextProperties,features()} properties aren't used in
         * any way.
         * @see @ref Shared::glyphCacheFontId(), @ref setGlyph()
         */
        DataHandle createGlyph(UnsignedInt style, UnsignedInt glyph, const TextProperties& properties, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        ) {
            return createGlyph(style, glyph, properties, Color3{1.0f}, node);
        }

        /**
         * @brief Create a single glyph with a style index in a concrete enum type
         *
         * Casts @p style to @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref createGlyph(UnsignedInt, UnsignedInt, const TextProperties&, NodeHandle).
         */
        template<class StyleIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value>::type
            #endif
        > DataHandle createGlyph(StyleIndex style, UnsignedInt glyph, const TextProperties& properties, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        ) {
            return createGlyph(UnsignedInt(style), glyph, properties, node);
        }

        /**
         * @brief Create a single glyph with a glyph ID in a concrete enum type
         *
         * Casts @p glyph to @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref createGlyph(UnsignedInt, UnsignedInt, const TextProperties&, NodeHandle).
         */
        template<class GlyphIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<GlyphIndex>::value>::type
            #endif
        > DataHandle createGlyph(UnsignedInt style, GlyphIndex glyph, const TextProperties& properties, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        ) {
            return createGlyph(style, UnsignedInt(glyph), properties, node);
        }

        /**
         * @brief Create a single glyph with a style index and glyph ID in a concrete enum type
         *
         * Casts @p style and @p glyph to @relativeref{Magnum,UnsignedInt} and
         * delegates to @ref createGlyph(UnsignedInt, UnsignedInt, const TextProperties&, NodeHandle).
         */
        template<class StyleIndex, class GlyphIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value && std::is_enum<GlyphIndex>::value>::type
            #endif
        > DataHandle createGlyph(StyleIndex style, GlyphIndex glyph, const TextProperties& properties, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        ) {
            return createGlyph(UnsignedInt(style), UnsignedInt(glyph), properties, node);
        }

        /**
         * @brief Create a single glyph with a custom color
         * @param style         Style index
         * @param glyph         Glyph to render
         * @param properties    Text properties
         * @param color         Custom color
         * @param node          Node to attach to
         * @return New data handle
         *
         * Expects that @ref Shared::setStyle() has been called, @p style is
         * less than @ref Shared::totalStyleCount() and
         * @ref TextProperties::font() is either @ref FontHandle::Null or
         * valid. Styling is driven from the @ref TextLayerStyleUniform at
         * index @p style. If @ref TextProperties::font() is not null it's
         * used, otherwise the default @ref FontHandle assigned to given style
         * is used and is expected to not be null. In addition
         * @ref TextLayerStyleUniform::color is multiplied with @p color. The
         * @p glyphId is expected to be less than
         * @ref Text::AbstractGlyphCache::fontGlyphCount() for given font.
         *
         * Compared to @ref create(), the glyph is aligned according to
         * @ref TextProperties::alignment() based on its bounding rectangle
         * coming from the glyph cache, not based on font metrics. The
         * @ref TextProperties::script(),
         * @relativeref{TextProperties,language()},
         * @relativeref{TextProperties,shapeDirection()},
         * @relativeref{TextProperties,layoutDirection()} and
         * @relativeref{TextProperties,features()} properties aren't used in
         * any way.
         * @see @ref Shared::glyphCacheFontId(), @ref setGlyph()
         */
        DataHandle createGlyph(UnsignedInt style, UnsignedInt glyph, const TextProperties& properties, const Color3& color, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        );

        /**
         * @brief Create a single glyph with a style index in a concrete enum type and a custom color
         *
         * Casts @p style to @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref createGlyph(UnsignedInt, UnsignedInt, const TextProperties&, const Color3&, NodeHandle).
         */
        template<class StyleIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value>::type
            #endif
        > DataHandle createGlyph(StyleIndex style, UnsignedInt glyph, const TextProperties& properties, const Color3& color, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        ) {
            return createGlyph(UnsignedInt(style), glyph, properties, color, node);
        }

        /**
         * @brief Create a single glyph with a glyph ID in a concrete enum type and a custom color
         *
         * Casts @p glyph to @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref createGlyph(UnsignedInt, UnsignedInt, const TextProperties&, const Color3&, NodeHandle).
         */
        template<class GlyphIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<GlyphIndex>::value>::type
            #endif
        > DataHandle createGlyph(UnsignedInt style, GlyphIndex glyph, const TextProperties& properties, const Color3& color, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        ) {
            return createGlyph(style, UnsignedInt(glyph), properties, color, node);
        }

        /**
         * @brief Create a single glyph with a style index and glyph ID in a concrete enum type and a custom color
         *
         * Casts @p style and @p glyph to @relativeref{Magnum,UnsignedInt} and
         * delegates to @ref createGlyph(UnsignedInt, UnsignedInt, const TextProperties&, const Color3&, NodeHandle).
         */
        template<class StyleIndex, class GlyphIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value && std::is_enum<GlyphIndex>::value>::type
            #endif
        > DataHandle createGlyph(StyleIndex style, GlyphIndex glyph, const TextProperties& properties, const Color3& color, NodeHandle node =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            NodeHandle::Null
            #else
            NodeHandle{} /* To not have to include Handle.h */
            #endif
        ) {
            return createGlyph(UnsignedInt(style), UnsignedInt(glyph), properties, color, node);
        }

        /**
         * @brief Remove a text
         *
         * Delegates to @ref AbstractLayer::remove(DataHandle) and additionally
         * marks the now-unused glyph run for removal in the next @ref update().
         */
        void remove(DataHandle handle);

        /**
         * @brief Remove a text assuming it belongs to this layer
         *
         * Delegates to @ref AbstractLayer::remove(LayerDataHandle) and
         * additionally marks the now-unused glyph run for removal in the next
         * @ref update().
         */
        void remove(LayerDataHandle handle);

        /**
         * @brief Text flags
         *
         * Expects that @p handle is valid. Note that, to avoid implementation
         * complexity, the flags can be only specified in @ref create() or
         * @ref setText(DataHandle, Containers::StringView, const TextProperties&, TextDataFlags)
         * and cannot be modified independently. Data created with
         * @ref createGlyph() or updated with @ref setGlyph() have the flags
         * always empty.
         * @see @ref isHandleValid(DataHandle) const
         */
        TextDataFlags flags(DataHandle handle) const;

        /**
         * @brief Text flags assuming it belongs to this layer
         *
         * Like @ref flags(DataHandle) const but without checking that
         * @p handle indeed belongs to this layer. See its documentation for
         * more information.
         */
        TextDataFlags flags(LayerDataHandle handle) const;

        /**
         * @brief Text glyph count
         *
         * Expects that @p handle is valid.
         * @see @ref isHandleValid(DataHandle) const
         */
        UnsignedInt glyphCount(DataHandle handle) const;

        /**
         * @brief Text glyph count assuming it belongs to this layer
         *
         * Expects that @p handle is valid.
         * @see @ref isHandleValid(LayerDataHandle) const
         */
        UnsignedInt glyphCount(LayerDataHandle handle) const;

        /**
         * @brief Size of the laid out text
         *
         * Expects that @p handle is valid. For text laid out with
         * @ref create() or @ref setText() the size is derived from ascent,
         * descent and advances of individual glyphs, not from actual area of
         * the glyphs being drawn, as that may not be known at that time. For
         * @ref createGlyph() or @ref setGlyph() the size is based on the
         * actual glyph size coming out of the glyph cache.
         * @see @ref isHandleValid(DataHandle) const
         */
        Vector2 size(DataHandle handle) const;

        /**
         * @brief Size of the laid out text assuming it belongs to this layer
         *
         * Like @ref size(DataHandle) const but without checking that @p handle
         * indeed belongs to this layer. See its documentation for more
         * information.
         */
        Vector2 size(LayerDataHandle handle) const;

        /**
         * @brief Cursor and selection position in an editable text
         *
         * Expects that @p handle is valid and the text was created or set with
         * @ref TextDataFlag::Editable present. Both values are guaranteed to
         * be always within bounds of a corresponding
         * @ref text(DataHandle) const. The first value is cursor position, the
         * second value denotes the other end of the selection. If both are the
         * same, there's no selection.
         * @see @ref isHandleValid(DataHandle) const,
         *      @ref flags(DataHandle) const
         */
        Containers::Pair<UnsignedInt, UnsignedInt> cursor(DataHandle handle) const;

        /**
         * @brief Cursor and selection position in an editable text assuming it belongs to this layer
         *
         * Like @ref cursor(DataHandle) const but without checking that
         * @p handle indeed belongs to this layer. See its documentation for
         * more information.
         */
        Containers::Pair<UnsignedInt, UnsignedInt> cursor(LayerDataHandle handle) const;

        /**
         * @brief Set cursor position and selection in an editable text
         *
         * Low-level interface for cursor positioning. See @ref updateText()
         * for a low-level interface to perform text modifications together
         * with cursor positioning, use @ref editText() to perform higher-level
         * operations with UTF-8 and text directionality awareness.
         *
         * Expects that @p handle is valid and the text was created or set with
         * @ref TextDataFlag::Editable enabled. Both the @p position and
         * @p selection is expected to be less or equal to @ref text() size.
         * The distance between the two is what's selected --- if @p selection
         * is less than @p position, the selection is before the cursor, if
         * @p position is less than @p selection, the selection is after the
         * cursor. If they're the same, there's no selection. No UTF-8 sequence
         * boundary adjustment is done for either of the two, i.e. it's
         * possible to move the cursor or selection inside a multi-byte UTF-8
         * sequence.
         *
         * Calling this function causes @ref LayerState::NeedsDataUpdate to be
         * set, unless the operation performed is a no-op, which is when both
         * @p removeSize and @p insertText size are both @cpp 0 @ce and
         * @p cursor is equal to @ref cursor().
         * @see @ref isHandleValid(DataHandle) const,
         *      @ref flags(DataHandle) const
         */
        void setCursor(DataHandle handle, UnsignedInt position, UnsignedInt selection);

        /**
         * @brief Set cursor position in an editable text
         *
         * Same as calling @ref setCursor(DataHandle, UnsignedInt, UnsignedInt)
         * with @p position passed to both @p position and @p selection, i.e.
         * with nothing selected.
         */
        void setCursor(DataHandle handle, UnsignedInt position) {
            setCursor(handle, position, position);
        }

        /**
         * @brief Set cursor position and selection in an editable text assuming it belongs to this layer
         *
         * Like @ref setCursor(DataHandle, UnsignedInt, UnsignedInt) but
         * without checking that @p handle indeed belongs to this layer. See
         * its documentation for more information.
         */
        void setCursor(LayerDataHandle handle, UnsignedInt position, UnsignedInt selection);

        /**
         * @brief Set cursor position in an editable text assuming it belongs to this layer
         *
         * Same as calling @ref setCursor(LayerDataHandle, UnsignedInt, UnsignedInt)
         * with @p position passed to both @p position and @p selection, i.e.
         * with nothing selected.
         */
        void setCursor(LayerDataHandle handle, UnsignedInt position) {
            setCursor(handle, position, position);
        }

        /**
         * @brief Properties used for shaping an editable text
         *
         * Expects that @p handle is valid and the text was created with
         * @ref TextDataFlag::Editable set. Returns content of
         * @ref TextProperties passed in previous @ref create() or
         * @ref setText() call for @p handle, except for
         * @ref TextProperties::font() that is picked from the style if it was
         * passed as @ref FontHandle::Null.
         * @see @ref isHandleValid(DataHandle) const,
         *      @ref flags(DataHandle) const, @ref style(DataHandle) const
         */
        TextProperties textProperties(DataHandle handle) const;

        /**
         * @brief Properties used for shaping an editable text assuming it belongs to this layer
         *
         * Like @ref textProperties(DataHandle) const but without checking that
         * @p handle indeed belongs to this layer. See its documentation for
         * more information.
         */
        TextProperties textProperties(LayerDataHandle handle) const;

        /**
         * @brief Contents of an editable text
         *
         * Expects that @p handle is valid and the text was created or set with
         * @ref TextDataFlag::Editable present. The returned view is only valid
         * until the next @ref create(), @ref setText() or @ref update() call
         * and is never
         * @relativeref{Corrade,Containers::StringViewFlag::NullTerminated}.
         * @see @ref isHandleValid(DataHandle) const
         */
        Containers::StringView text(DataHandle handle) const;

        /**
         * @brief Contents of an editable text assuming it belongs to this layer
         *
         * Like @ref text(DataHandle) const but without checking that @p handle
         * indeed belongs to this layer. See its documentation for more
         * information.
         */
        Containers::StringView text(LayerDataHandle handle) const;

        /**
         * @brief Set text
         *
         * Expects that @p handle is valid and @ref TextProperties::font() is
         * either @ref FontHandle::Null or valid. If not null, the
         * @ref TextProperties::font() is used, otherwise the default
         * @ref FontHandle assigned to @ref style() is used and expected to not
         * be null. Note that it's not possible to change the font alone with
         * @ref setStyle(), it only can be done when setting the text. The
         * @ref FontHandle, whether coming from the style or from
         * @p properties, is expected to have a font instance. Instance-less
         * fonts can be only used to set single glyphs (such as various icons
         * or images) with @ref setGlyph().
         *
         * This function preserves existing @ref flags() for given @p handle,
         * use @ref setText(DataHandle, Containers::StringView, const TextProperties&, TextDataFlags)
         * to supply different @ref TextDataFlags for the new text.
         *
         * If @ref flags() contain @ref TextDataFlag::Editable, the @p text and
         * @p properties are remembered and subsequently accessible through
         * @ref text() and @ref textProperties(), @ref cursor() position and
         * selection are both set to @p text size. Currently, for editable
         * text, the @p properties are expected to have empty
         * @ref TextProperties::features() --- only the features supplied by
         * the style are used for editable text.
         *
         * Calling this function causes @ref LayerState::NeedsDataUpdate to be
         * set.
         * @see @ref isHandleValid(DataHandle) const,
         *      @ref Shared::isHandleValid(FontHandle) const, @ref setCursor(),
         *      @ref updateText(), @ref editText()
         */
        void setText(DataHandle handle, Containers::StringView text, const TextProperties& properties);

        /**
         * @brief Set text with different flags
         *
         * Like @ref setText(DataHandle, Containers::StringView, const TextProperties&)
         * but supplying different @ref TextDataFlags for the new text instead
         * of preserving existing @ref flags().
         */
        void setText(DataHandle handle, Containers::StringView text, const TextProperties& properties, TextDataFlags flags);

        /**
         * @brief Set text assuming it belongs to this layer
         *
         * Like @ref setText(DataHandle, Containers::StringView, const TextProperties&)
         * but without checking that @p handle indeed belongs to this layer.
         * See its documentation for more information.
         */
        void setText(LayerDataHandle handle, Containers::StringView text, const TextProperties& properties);

        /**
         * @brief Set text with different flags assuming it belongs to this layer
         *
         * Like @ref setText(DataHandle, Containers::StringView, const TextProperties&, TextDataFlags)
         * but without checking that @p handle indeed belongs to this layer.
         * See its documentation for more information.
         */
        void setText(LayerDataHandle handle, Containers::StringView text, const TextProperties& properties, TextDataFlags flags);

        /**
         * @brief Update text, cursor position and selection in an editable text
         * @param handle        Handle which to update
         * @param removeOffset  Offset at which to remove
         * @param removeSize    Count of bytes to remove
         * @param insertOffset  Offset at which to insert after the removal
         * @param insertText    Text to insert after the removal
         * @param cursor        Cursor position to set after the removal and
         *      subsequent insert
         * @param selection     Selection to set after the removal and
         *      subsequent insert
         *
         * Low-level interface for text removal and insertion together with
         * cursor positioning. See @ref setCursor() for just cursor positioning
         * alone, use @ref editText() to perform higher-level operations with
         * UTF-8 and text directionality awareness.
         *
         * Expects that @p handle is valid and the text was created or set with
         * @ref TextDataFlag::Editable enabled. The @p removeOffset together
         * with @p removeSize is expected to be less or equal to @ref text()
         * size; @p insertOffset then equal to the text size without
         * @p removeSize; @p cursor and @p selection then to text size without
         * @p removeSize but with @p insertText size. The distance between
         * @p cursor and @p selection is what's selected --- if @p selection
         * is less than @p cursor, the selection is before the cursor, if
         * @p cursor is less than @p selection, the selection is after the
         * cursor. If they're the same, there's no selection. No UTF-8
         * sequence boundary adjustment is done for any of these, i.e. it's
         * possible to remove or insert partial multi-byte UTF-8 sequences and
         * position the cursor inside them as well.
         *
         * Calling this function causes @ref LayerState::NeedsDataUpdate to be
         * set, unless the operation performed is a no-op, which is when both
         * @p removeSize and @p insertText size are both @cpp 0 @ce and
         * @p cursor is equal to @ref cursor().
         * @see @ref isHandleValid(DataHandle) const,
         *      @ref flags(DataHandle) const, @ref setText()
         */
        void updateText(DataHandle handle, UnsignedInt removeOffset, UnsignedInt removeSize, UnsignedInt insertOffset, Containers::StringView insertText, UnsignedInt cursor, UnsignedInt selection);

        /**
         * @brief Update text and cursor position in an editable text
         *
         * Same as calling @ref updateText(DataHandle, UnsignedInt, UnsignedInt, UnsignedInt, Containers::StringView, UnsignedInt, UnsignedInt)
         * with @p position passed to both @p position and @p selection, i.e.
         * with nothing selected.
         */
        void updateText(DataHandle handle, UnsignedInt removeOffset, UnsignedInt removeSize, UnsignedInt insertOffset, Containers::StringView insertText, UnsignedInt cursor) {
            updateText(handle, removeOffset, removeSize, insertOffset, insertText, cursor, cursor);
        }

        /**
         * @brief Update text, cursor position and selection in an editable text assuming it belongs to this layer
         *
         * Like @ref updateText(DataHandle, UnsignedInt, UnsignedInt, UnsignedInt, Containers::StringView, UnsignedInt, UnsignedInt)
         * but without checking that @p handle indeed belongs to this layer.
         * See its documentation for more information.
         */
        void updateText(LayerDataHandle handle, UnsignedInt removeOffset, UnsignedInt removeSize, UnsignedInt insertOffset, Containers::StringView insertText, UnsignedInt cursor, UnsignedInt selection);

        /**
         * @brief Update text and cursor position in an editable text
         *
         * Same as calling @ref updateText(DataHandle, UnsignedInt, UnsignedInt, UnsignedInt, Containers::StringView, UnsignedInt, UnsignedInt)
         * with @p position passed to both @p position and @p selection, i.e.
         * with nothing selected.
         */
        void updateText(LayerDataHandle handle, UnsignedInt removeOffset, UnsignedInt removeSize, UnsignedInt insertOffset, Containers::StringView insertText, UnsignedInt cursor) {
            updateText(handle, removeOffset, removeSize, insertOffset, insertText, cursor, cursor);
        }

        /**
         * @brief Edit text at current cursor position
         *
         * High-level interface for text editing, mapping to usual user
         * interface operations. Delegates to @ref setCursor() or
         * @ref updateText() internally.
         *
         * Expects that @p handle is valid, the text was created or set with
         * @ref TextDataFlag::Editable enabled and @p insert is non-empty only
         * if appropriate @p edit operation is used. See documentation of the
         * @ref TextEdit enum for detailed behavior of each operation.
         *
         * Calling this function causes @ref LayerState::NeedsDataUpdate to be
         * set, unless the operation performed is a no-op such as inserting
         * empty text or moving a cursor / deleting a character to the left
         * with the cursor already being on the leftmost side of the text.
         * @see @ref isHandleValid(DataHandle) const,
         *      @ref flags(DataHandle) const, @ref setText()
         */
        void editText(DataHandle handle, TextEdit edit, Containers::StringView insert);

        /**
         * @brief Edit text at current cursor position assuming it belongs to this layer
         *
         * Like @ref editText(DataHandle, TextEdit, Containers::StringView) but
         * without checking that @p handle indeed belongs to this layer. See
         * its documentation for more information.
         */
        void editText(LayerDataHandle handle, TextEdit edit, Containers::StringView insert);

        /**
         * @brief Set a single glyph
         *
         * Expects that @p handle is valid and @ref TextProperties::font() is
         * either @ref FontHandle::Null or valid. If not null, the
         * @ref TextProperties::font() is used, otherwise the default
         * @ref FontHandle assigned to @ref style() is used and expected to not
         * be null. Note that it's not possible to change the font alone with
         * @ref setStyle(), it only can be done when setting the glyph. The
         * @p glyph is expected to be less than
         * @ref Text::AbstractGlyphCache::fontGlyphCount() for given font.
         *
         * Compared to @ref setText(), the glyph is aligned according to
         * @ref TextProperties::alignment() based on its bounding rectangle
         * coming from the glyph cache, not based on font metrics. The
         * @ref TextProperties::script(),
         * @relativeref{TextProperties,language()},
         * @relativeref{TextProperties,shapeDirection()},
         * @relativeref{TextProperties,layoutDirection()} and
         * @relativeref{TextProperties,features()} properties aren't used in
         * any way; @ref flags() get reset to empty. Note that it's also
         * possible to change a handle that previously contained a text to a
         * single glyph and vice versa --- the internal representation of both
         * is the same.
         *
         * Calling this function causes @ref LayerState::NeedsDataUpdate to be
         * set.
         * @see @ref isHandleValid(DataHandle) const,
         *      @ref Shared::isHandleValid(FontHandle) const,
         *      @ref Shared::glyphCacheFontId()
         */
        void setGlyph(DataHandle handle, UnsignedInt glyph, const TextProperties& properties);

        /**
         * @brief Set a single glyph with a glyph ID in a concrete enum type
         *
         * Casts @p glyph to @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref setGlyph(DataHandle, UnsignedInt, const TextProperties&).
         */
        template<class GlyphIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<GlyphIndex>::value>::type
            #endif
        > void setGlyph(DataHandle handle, GlyphIndex glyph, const TextProperties& properties) {
            return setGlyph(handle, UnsignedInt(glyph), properties);
        }

        /**
         * @brief Set a single glyph assuming it belongs to this layer
         *
         * Like @ref setGlyph(DataHandle, UnsignedInt, const TextProperties&)
         * but without checking that @p handle indeed belongs to this layer.
         * See its documentation for more information.
         */
        void setGlyph(LayerDataHandle handle, UnsignedInt glyph, const TextProperties& properties);

        /**
         * @brief Set a single glyph with a glyph ID in a concrete enum type assuming it belongs to this layer
         *
         * Casts @p glyph to @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref setGlyph(LayerDataHandle, UnsignedInt, const TextProperties&).
         */
        template<class GlyphIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<GlyphIndex>::value>::type
            #endif
        > void setGlyph(LayerDataHandle handle, GlyphIndex glyph, const TextProperties& properties) {
            return setGlyph(handle, UnsignedInt(glyph), properties);
        }

        /**
         * @brief Text custom base color
         *
         * Expects that @p handle is valid.
         * @see @ref isHandleValid(DataHandle) const
         */
        Color3 color(DataHandle handle) const;

        /**
         * @brief Text custom base color assuming it belongs to this layer
         *
         * Expects that @p handle is valid.
         * @see @ref isHandleValid(LayerDataHandle) const
         */
        Color3 color(LayerDataHandle handle) const;

        /**
         * @brief Set text custom base color
         *
         * Expects that @p handle is valid. @ref TextLayerStyleUniform::color
         * is multiplied with @p color. By default, unless specified in
         * @ref create() / @ref createGlyph() already, the custom color is
         * @cpp 0xffffff_srgbf @ce, i.e. not affecting the style in any way.
         *
         * Calling this function causes @ref LayerState::NeedsDataUpdate to be
         * set.
         * @see @ref isHandleValid(DataHandle) const
         */
        void setColor(DataHandle handle, const Color3& color);

        /**
         * @brief Set text custom base color assuming it belongs to this layer
         *
         * Like @ref setColor(DataHandle, const Color3&) but without checking
         * that @p handle indeed belongs to this layer. See its documentation
         * for more information.
         */
        void setColor(LayerDataHandle handle, const Color3& color);

        /**
         * @brief Text custom padding
         *
         * In order left, top. right, bottom. Expects that @p handle is valid.
         * @see @ref isHandleValid(DataHandle) const
         */
        Vector4 padding(DataHandle handle) const;

        /**
         * @brief Text custom padding assuming it belongs to this layer
         *
         * In order left, top. right, bottom. Expects that @p handle is valid.
         * @see @ref isHandleValid(LayerDataHandle) const
         */
        Vector4 padding(LayerDataHandle handle) const;

        /**
         * @brief Set text custom padding
         *
         * Expects that @p handle is valid. The @p padding is in order left,
         * top, right, bottom and is added to the per-style padding values
         * specified in @ref Shared::setStyle().
         *
         * Calling this function causes @ref LayerState::NeedsDataUpdate to be
         * set.
         * @see @ref isHandleValid(DataHandle) const
         */
        void setPadding(DataHandle handle, const Vector4& padding);

        /**
         * @brief Set text custom padding assuming it belongs to this layer
         *
         * Like @ref setPadding(DataHandle, const Vector4&) but without
         * checking that @p handle indeed belongs to this layer. See its
         * documentation for more information.
         */
        void setPadding(LayerDataHandle handle, const Vector4& padding);

        /**
         * @brief Set text custom padding with all edges having the same value
         *
         * Expects that @p handle is valid. The @p padding is added to the
         * per-style padding values specified in @ref Shared::setStyle().
         *
         * Calling this function causes @ref LayerState::NeedsDataUpdate to be
         * set.
         * @see @ref isHandleValid(DataHandle) const
         */
        void setPadding(DataHandle handle, Float padding) {
            setPadding(handle, Vector4{padding});
        }

        /**
         * @brief Set text custom padding with all edges having the same value assuming it belongs to this layer
         *
         * Like @ref setPadding(DataHandle, Float) but without checking that
         * @p handle indeed belongs to this layer. See its documentation for
         * more information.
         */
        void setPadding(LayerDataHandle handle, Float padding) {
            setPadding(handle, Vector4{padding});
        }

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #else
    protected:
    #endif
        struct State;

        MAGNUM_WHEE_LOCAL explicit TextLayer(LayerHandle handle, Containers::Pointer<State>&& state);
        /* Used by tests to avoid having to include / allocate the state */
        explicit TextLayer(LayerHandle handle, Shared& shared);

        /* These can't be MAGNUM_WHEE_LOCAL otherwise deriving from this class
           in tests causes linker errors */

        /* Advertises LayerFeature::Draw but *does not* implement doDraw(),
           that's on the subclass */
        LayerFeatures doFeatures() const override;

        LayerStates doState() const override;
        void doUpdate(LayerStates states, const Containers::StridedArrayView1D<const UnsignedInt>& dataIds, const Containers::StridedArrayView1D<const UnsignedInt>& clipRectIds, const Containers::StridedArrayView1D<const UnsignedInt>& clipRectDataCounts, const Containers::StridedArrayView1D<const Vector2>& nodeOffsets, const Containers::StridedArrayView1D<const Vector2>& nodeSizes, Containers::BitArrayView nodesEnabled, const Containers::StridedArrayView1D<const Vector2>& clipRectOffsets, const Containers::StridedArrayView1D<const Vector2>& clipRectSizes, const Containers::StridedArrayView1D<const Vector2>& compositeRectOffsets, const Containers::StridedArrayView1D<const Vector2>& compositeRectSizes) override;

    private:
        MAGNUM_WHEE_LOCAL DataHandle createInternal(NodeHandle node);
        MAGNUM_WHEE_LOCAL void shapeTextInternal(UnsignedInt id, UnsignedInt style, Containers::StringView text, const TextProperties& properties, FontHandle font);
        MAGNUM_WHEE_LOCAL void shapeRememberTextInternal(
            #ifndef CORRADE_NO_ASSERT
            const char* messagePrefix,
            #endif
            UnsignedInt id, UnsignedInt style, Containers::StringView text, const TextProperties& properties, TextDataFlags flags);
        MAGNUM_WHEE_LOCAL void shapeGlyphInternal(
            #ifndef CORRADE_NO_ASSERT
            const char* messagePrefix,
            #endif
            UnsignedInt id, UnsignedInt style, UnsignedInt glyphId, const TextProperties& properties);
        MAGNUM_WHEE_LOCAL void removeInternal(UnsignedInt id);
        MAGNUM_WHEE_LOCAL Containers::Pair<UnsignedInt, UnsignedInt> cursorInternal(UnsignedInt id) const;
        MAGNUM_WHEE_LOCAL void setCursorInternal(UnsignedInt id, UnsignedInt position, UnsignedInt selection);
        MAGNUM_WHEE_LOCAL TextProperties textPropertiesInternal(UnsignedInt id) const;
        MAGNUM_WHEE_LOCAL Containers::StringView textInternal(UnsignedInt id) const;
        MAGNUM_WHEE_LOCAL void setTextInternal(UnsignedInt id, Containers::StringView text, const TextProperties& properties, TextDataFlags flags);
        MAGNUM_WHEE_LOCAL void updateTextInternal(UnsignedInt id, UnsignedInt removeOffset, UnsignedInt removeSize, UnsignedInt insertOffset, Containers::StringView text, UnsignedInt cursor, UnsignedInt selection);
        MAGNUM_WHEE_LOCAL void editTextInternal(UnsignedInt id, TextEdit edit, Containers::StringView text);
        MAGNUM_WHEE_LOCAL void setGlyphInternal(UnsignedInt id, UnsignedInt glyph, const TextProperties& properties);
        MAGNUM_WHEE_LOCAL void setColorInternal(UnsignedInt id, const Color3& color);
        MAGNUM_WHEE_LOCAL void setPaddingInternal(UnsignedInt id, const Vector4& padding);

        /* Can't be MAGNUM_WHEE_LOCAL otherwise deriving from this class in
           tests causes linker errors */
        void doClean(Containers::BitArrayView dataIdsToRemove) override;
        void doAdvanceAnimations(Nanoseconds time, const Containers::Iterable<AbstractStyleAnimator>& animators) override;
        void doKeyPressEvent(UnsignedInt dataId, KeyEvent& event) override;
        void doTextInputEvent(UnsignedInt dataId, TextInputEvent& event) override;
};

/**
@brief Shared state for the text layer

Contains a set of fonts and a glyph cache used by all of them. In order to use
the layer it's expected that @ref setGlyphCache() was called and at least one
font was added with @ref addFont(). In order to update or draw the layer it's
expected that @ref setStyle() was called.

Pre-filling the glyph cache with appropriate glyphs for a particular font is
the user responsibility, the implementation currently won't perform that on its
own, neither it does any on-demand cache filling.
*/
class MAGNUM_WHEE_EXPORT TextLayer::Shared: public AbstractVisualLayer::Shared {
    public:
        class Configuration;

        /**
         * @brief Style uniform count
         *
         * Size of the style uniform buffer excluding dynamic styles. May or
         * may not be the same as @ref styleCount(). For dynamic styles, the
         * style uniform count is the same as @ref dynamicStyleCount().
         * @see @ref Configuration::Configuration(UnsignedInt, UnsignedInt),
         *      @ref setStyle()
         */
        UnsignedInt styleUniformCount() const;

        /**
         * @brief Whether a glyph cache has been set
         *
         * @see @ref glyphCache(), @ref setGlyphCache()
         */
        bool hasGlyphCache() const;

        /**
         * @brief Glyph cache instance
         *
         * Expects that a glyph cache was set with @ref setGlyphCache().
         * @see @ref hasGlyphCache()
         */
        Text::AbstractGlyphCache& glyphCache();
        const Text::AbstractGlyphCache& glyphCache() const; /**< @overload */

        /**
         * @brief Count of added fonts
         *
         * Can be at most 32768.
         */
        std::size_t fontCount() const;

        /**
         * @brief Whether a font handle is valid
         *
         * A handle is valid if it has been returned from @ref addFont()
         * before. Note that the implementation has no way to distinguish
         * between handles returned from different @ref TextLayer::Shared
         * instances. A handle from another shared state instance thus may or
         * may not be treated as valid by another instance, and it's the user
         * responsibility to not mix them up. For @ref FontHandle::Null always
         * returns @cpp false @ce.
         */
        bool isHandleValid(FontHandle handle) const;

        /**
         * @brief Add a font
         * @param font      Font instance
         * @param size      Size at which to render the font
         * @return New font handle
         *
         * Expects that @ref glyphCache() is set and contains @p font. Doesn't
         * perform any operation with the glyph cache, pre-filling is left to
         * the caller. The returned handle can be subsequently assigned to a
         * particular style via @ref setStyle(), or passed to
         * @ref TextProperties::setFont() to create a text with a font that's
         * different from the one assigned to a particular style.
         *
         * It's the caller responsibility to ensure @p font stays in scope for
         * as long as the shared state is used. Use the
         * @ref addFont(Containers::Pointer<Text::AbstractFont>&&, Float)
         * overload to make the shared state take over the font instance.
         * @see @ref addInstancelessFont()
         */
        FontHandle addFont(Text::AbstractFont& font, Float size);

        /**
         * @brief Add a font and take over its ownership
         * @param font      Font instance
         * @param size      Size at which to render the font
         * @return New font handle
         *
         * Like @ref addFont(Text::AbstractFont&, Float), but the shared state
         * takes over the font ownership. You can access the instance using
         * @ref font() later. It's the caller responsibility to ensure the
         * plugin manager the font is coming from stays in scope for as long as
         * the shared state is used.
         */
        FontHandle addFont(Containers::Pointer<Text::AbstractFont>&& font, Float size);

        /**
         * @brief Add an instance-less font
         * @param glyphCacheFontId  ID of the font in the glyph cache
         * @param scale             Scale to apply to glyph rectangles coming
         *      from the glyph cache
         * @return New font handle
         *
         * Makes it possible to render arbitrary custom glyphs added to the
         * glyph cache, such as icons or other image data.
         *
         * Expects that @ref glyphCache() is set, @p glyphCacheFontId is less
         * than @ref Text::AbstractGlyphCache::fontCount() and
         * @ref Text::AbstractGlyphCache::fontPointer() is @cpp nullptr @ce for
         * @p glyphCacheFontId. Doesn't perform any operation with the glyph
         * cache, the caller is expected to populate the cache with desired
         * data. The resulting handle can be subsequently assigned to a
         * particular style via @ref setStyle(), or passed to
         * @ref TextProperties::setFont(). The instance-less font however can
         * be only used with @ref createGlyph() or @ref setGlyph() that takes a
         * glyph ID, not with @ref create() or @ref setText().
         * @see @ref hasFontInstance(), @ref addFont()
         */
        FontHandle addInstancelessFont(UnsignedInt glyphCacheFontId, Float scale);

        /**
         * @brief ID of a font in a glyph cache
         *
         * Returns ID under which given font glyphs are stored in the
         * @ref glyphCache(). For fonts with an instance is equivalent to the
         * ID returned from @ref Text::AbstractGlyphCache::findFont(). Expects
         * that @p handle is valid.
         * @see @ref isHandleValid(FontHandle) const, @ref hasFontInstance()
         */
        UnsignedInt glyphCacheFontId(FontHandle handle) const;

        /**
         * @brief Whether a font has an instance
         *
         * Returns @cpp true @ce if the font was created with @ref addFont(),
         * @cpp false @ce if with @ref addInstancelessFont(). Expects that
         * @p handle is valid.
         * @see @ref isHandleValid(FontHandle) const
         */
        bool hasFontInstance(FontHandle handle) const;

        /**
         * @brief Font instance
         *
         * Expects that @p handle is valid and has a font instance, i.e. was
         * created with @ref addFont() and not @ref addInstancelessFont().
         * @see @ref isHandleValid(FontHandle) const, @ref hasFontInstance()
         */
        Text::AbstractFont& font(FontHandle handle);
        const Text::AbstractFont& font(FontHandle handle) const; /**< @overload */

        /**
         * @brief Set style data with implicit mapping between styles and uniforms
         * @param commonUniform Common style uniform data
         * @param uniforms      Style uniforms
         * @param fonts         Font handles corresponding to style uniforms
         * @param alignments    Text alignment corresponding to style uniforms
         * @param features      Font feature data for all styles
         * @param featureOffsets  Offsets into @p features corresponding to
         *      style uniforms
         * @param featureCounts  Counts into @p features corresponding to style
         *      uniforms
         * @param paddings      Padding inside the node in order left, top,
         *      right, bottom corresponding to style uniforms
         * @return Reference to self (for method chaining)
         *
         * The @p uniforms view is expected to have the same size as
         * @ref styleUniformCount(), the @p fonts and @p alignments views the
         * same size as @ref styleCount(). All font handles are expected to be
         * either @ref FontHandle::Null or valid, all @p alignments values are
         * expected to not be `*GlyphBounds` as the implementation can only
         * align based on font metrics and cursor position, not actual glyph
         * bounds. See @ref TextProperties::setAlignment() for a detailed
         * explanation of alignment behavior respectively to the nodes the text
         * is attached to.
         *
         * The @p featureOffsets and @p featureCounts views are expected to
         * both either have a size of @ref styleCount(), or both be empty. If
         * non-empty, they contain offsets and counts into the @p features view
         * for each style, with the ranges expected to fit into the view size.
         * The ranges are allowed to overlap, i.e. multiple styles can share
         * the same default feature set. If empty, the @p features view is
         * expected to be empty as well and no default font features are
         * specified for any styles. Features coming from @ref TextProperties
         * are *appended* after these, making it possible to override them.
         *
         * The @p paddings view is expected to either have the same size
         * as @ref styleCount() or be empty, in which case all paddings are
         * implicitly zero.
         *
         * Can only be called if @ref styleUniformCount() and @ref styleCount()
         * were set to the same value in @ref Configuration passed to the
         * constructor, otherwise you have to additionally provide a mapping
         * from styles to uniforms using
         * @ref setStyle(const TextLayerCommonStyleUniform&, Containers::ArrayView<const TextLayerStyleUniform>, const Containers::StridedArrayView1D<const UnsignedInt>&, const Containers::StridedArrayView1D<const FontHandle>&, const Containers::StridedArrayView1D<const Text::Alignment>&, Containers::ArrayView<const TextFeatureValue>, const Containers::StridedArrayView1D<const UnsignedInt>&, const Containers::StridedArrayView1D<const UnsignedInt>&, const Containers::StridedArrayView1D<const Vector4>&)
         * instead.
         *
         * Calling this function causes @ref LayerState::NeedsDataUpdate to be
         * set on all layers that are constructed using this shared instance.
         * If @ref dynamicStyleCount() is non-zero,
         * @ref LayerState::NeedsCommonDataUpdate is set as well to trigger an
         * upload of changed dynamic style uniform data.
         * @see @ref isHandleValid(FontHandle) const
         */
        Shared& setStyle(const TextLayerCommonStyleUniform& commonUniform, Containers::ArrayView<const TextLayerStyleUniform> uniforms, const Containers::StridedArrayView1D<const FontHandle>& fonts, const Containers::StridedArrayView1D<const Text::Alignment>& alignments, Containers::ArrayView<const TextFeatureValue> features, const Containers::StridedArrayView1D<const UnsignedInt>& featureOffsets, const Containers::StridedArrayView1D<const UnsignedInt>& featureCounts, const Containers::StridedArrayView1D<const Vector4>& paddings);
        /** @overload */
        Shared& setStyle(const TextLayerCommonStyleUniform& commonUniform, std::initializer_list<TextLayerStyleUniform> uniforms, std::initializer_list<FontHandle> fonts, std::initializer_list<Text::Alignment> alignments, std::initializer_list<TextFeatureValue> features, std::initializer_list<UnsignedInt> featureOffsets, std::initializer_list<UnsignedInt> featureCounts, std::initializer_list<Vector4> paddings);

        /**
         * @brief Set style data
         * @param commonUniform     Common style uniform data
         * @param uniforms          Style uniforms
         * @param styleToUniform    Style to style uniform mapping
         * @param styleFonts        Per-style font handles
         * @param styleAlignments   Per-style text alignment
         * @param styleFeatures     Font feature data for all styles
         * @param styleFeatureOffsets  Per-style offsets into @p styleFeatures
         * @param styleFeatureCounts  Per-style counts into @p styleFeatures
         * @param stylePaddings     Per-style padding inside the node in order
         *      left, top, right, bottom
         * @return Reference to self (for method chaining)
         *
         * The @p uniforms view is expected to have the same size as
         * @ref styleUniformCount(), the @p styleToUniform, @p styleFonts and
         * @p styleAlignments views the same size as @ref styleCount(). All
         * font handles are expected to be either @ref FontHandle::Null or
         * valid, all @p styleAlignments values are expected to not be
         * `*GlyphBounds` as the implementation can only align based on font
         * metrics and cursor position, not actual glyph bounds. See
         * @ref TextProperties::setAlignment() for a detailed explanation of
         * alignment behavior respectively to the nodes the text is attached
         * to.
         *
         * The @p styleFeatureOffsets and @p styleFeatureCounts views are
         * expected to both either have a size of @ref styleCount(), or both be
         * empty. If non-empty, they contain offsets and counts into the
         * @p styleFeatures view for each style, with the ranges expected to
         * fit into the view size. The ranges are allowed to overlap, i.e.
         * multiple styles can share the same default feature set. If empty,
         * the @p styleFeatures view is expected to be empty as well and no
         * default font features are specified for any styles. Features coming
         * from @ref TextProperties are *appended* after these, making it
         * possible to override them.
         *
         * The @p stylePaddings view is expected to either have the same size
         * as @ref styleCount() or be empty, in which case all paddings are
         * implicitly zero.
         *
         * Value of @cpp styleToUniform[i] @ce should give back an index into
         * the @p uniforms array for style @cpp i @ce. If
         * @ref styleUniformCount() and @ref styleCount() is the same and the
         * mapping is implicit, you can use the
         * @ref setStyle(const TextLayerCommonStyleUniform&, Containers::ArrayView<const TextLayerStyleUniform>, const Containers::StridedArrayView1D<const FontHandle>&, const Containers::StridedArrayView1D<const Text::Alignment>&, Containers::ArrayView<const TextFeatureValue>, const Containers::StridedArrayView1D<const UnsignedInt>&, const Containers::StridedArrayView1D<const UnsignedInt>&, const Containers::StridedArrayView1D<const Vector4>&)
         * convenience overload instead.
         *
         * Calling this function causes @ref LayerState::NeedsDataUpdate to be
         * set on all layers that are constructed using this shared instance.
         * If @ref dynamicStyleCount() is non-zero,
         * @ref LayerState::NeedsCommonDataUpdate is set as well to trigger an
         * upload of changed dynamic style uniform data.
         * @see @ref isHandleValid(FontHandle) const
         */
        Shared& setStyle(const TextLayerCommonStyleUniform& commonUniform, Containers::ArrayView<const TextLayerStyleUniform> uniforms, const Containers::StridedArrayView1D<const UnsignedInt>& styleToUniform, const Containers::StridedArrayView1D<const FontHandle>& styleFonts, const Containers::StridedArrayView1D<const Text::Alignment>& styleAlignments, Containers::ArrayView<const TextFeatureValue> styleFeatures, const Containers::StridedArrayView1D<const UnsignedInt>& styleFeatureOffsets, const Containers::StridedArrayView1D<const UnsignedInt>& styleFeatureCounts, const Containers::StridedArrayView1D<const Vector4>& stylePaddings);
        /** @overload */
        Shared& setStyle(const TextLayerCommonStyleUniform& commonUniform, std::initializer_list<TextLayerStyleUniform> uniforms, std::initializer_list<UnsignedInt> styleToUniform, std::initializer_list<FontHandle> styleFonts, std::initializer_list<Text::Alignment> styleAlignments, std::initializer_list<TextFeatureValue> styleFeatures, std::initializer_list<UnsignedInt> styleFeatureOffsets, std::initializer_list<UnsignedInt> styleFeatureCounts, std::initializer_list<Vector4> stylePaddings);

        /* Overloads to remove a WTF factor from method chaining order */
        #ifndef DOXYGEN_GENERATING_OUTPUT
        MAGNUMEXTRAS_WHEE_ABSTRACTVISUALLAYER_SHARED_SUBCLASS_IMPLEMENTATION()
        #endif

    protected:
        struct State;

        /**
         * @brief Set a glyph cache instance
         * @return Reference to self (for method chaining)
         *
         * Has to be called before any @ref addFont(), is expected to be called
         * exactly once. You'll most probably want to set it through the public
         * @ref TextLayerGL::Shared::setGlyphCache(Text::GlyphCache&).
         */
        Shared& setGlyphCache(Text::AbstractGlyphCache& cache);

    #ifdef DOXYGEN_GENERATING_OUTPUT
    private:
    #else
    protected:
    #endif
        friend TextLayer;
        friend TextLayerStyleAnimator;

        MAGNUM_WHEE_LOCAL explicit Shared(Containers::Pointer<State>&& state);
        /* Used by tests to avoid having to include / allocate the state */
        explicit Shared(const Configuration& configuration);
        /* Can't be MAGNUM_WHEE_LOCAL, used by tests */
        explicit Shared(NoCreateT) noexcept;

    private:
        MAGNUM_WHEE_LOCAL void setStyleInternal(const TextLayerCommonStyleUniform& commonUniform, Containers::ArrayView<const TextLayerStyleUniform> uniforms, const Containers::StridedArrayView1D<const FontHandle>& styleFonts, const Containers::StridedArrayView1D<const Text::Alignment>& styleAlignments, Containers::ArrayView<const TextFeatureValue> styleFeatures, const Containers::StridedArrayView1D<const UnsignedInt>& styleFeatureOffsets, const Containers::StridedArrayView1D<const UnsignedInt>& styleFeatureCounts, const Containers::StridedArrayView1D<const Vector4>& stylePaddings);

        /* The items are guaranteed to have the same size as
           styleUniformCount(). Called only if there are no dynamic styles,
           otherwise the data are copied to internal arrays to be subsequently
           combined with dynamic uniforms and uploaded together in doDraw(). */
        virtual void doSetStyle(const TextLayerCommonStyleUniform& commonUniform, Containers::ArrayView<const TextLayerStyleUniform> uniforms) = 0;
};

/**
@brief Configuration of a base layer shared state

@see @ref TextLayerGL::Shared::Shared(const Configuration&)
*/
class MAGNUM_WHEE_EXPORT TextLayer::Shared::Configuration {
    public:
        /**
         * @brief Constructor
         *
         * The @p styleUniformCount parameter specifies the size of the uniform
         * array, @p styleCount then the number of distinct styles to use for
         * drawing. The sizes are independent in order to allow styles with
         * different fonts or paddings share the same uniform data. Either both
         * @p styleUniformCount and @p styleCount is expected to be non-zero,
         * or both zero with a non-zero dynamic style count specified with
         * @ref setDynamicStyleCount(). Style data are then set with
         * @ref setStyle(), dynamic style data then with
         * @ref TextLayer::setDynamicStyle().
         */
        explicit Configuration(UnsignedInt styleUniformCount, UnsignedInt styleCount);

        /**
         * @brief Construct with style uniform count being the same as style count
         *
         * Equivalent to calling @ref Configuration(UnsignedInt, UnsignedInt)
         * with both parameters set to @p styleCount.
         */
        explicit Configuration(UnsignedInt styleCount): Configuration{styleCount, styleCount} {}

        /** @brief Style uniform count */
        UnsignedInt styleUniformCount() const { return _styleUniformCount; }

        /** @brief Style count */
        UnsignedInt styleCount() const { return _styleCount; }

        /** @brief Dynamic style count */
        UnsignedInt dynamicStyleCount() const { return _dynamicStyleCount; }

        /**
         * @brief Set dynamic style count
         * @return Reference to self (for method chaining)
         *
         * Initial count is @cpp 0 @ce.
         * @see @ref Configuration(UnsignedInt, UnsignedInt),
         *      @ref AbstractVisualLayer::allocateDynamicStyle(),
         *      @ref AbstractVisualLayer::dynamicStyleUsedCount()
         */
        Configuration& setDynamicStyleCount(UnsignedInt count) {
            _dynamicStyleCount = count;
            return *this;
        }

    private:
        UnsignedInt _styleUniformCount, _styleCount;
        UnsignedInt _dynamicStyleCount = 0;
};

inline TextLayer::Shared& TextLayer::shared() {
    return static_cast<Shared&>(AbstractVisualLayer::shared());
}

inline const TextLayer::Shared& TextLayer::shared() const {
    return static_cast<const Shared&>(AbstractVisualLayer::shared());
}

}}

#endif
