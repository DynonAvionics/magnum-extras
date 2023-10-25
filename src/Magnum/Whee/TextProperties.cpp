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

#include "TextProperties.h"

#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/String.h>
#include <Magnum/Text/Alignment.h>
#include <Magnum/Text/Direction.h>
#include <Magnum/Text/Feature.h>
#include <Magnum/Text/Script.h>

#include "Magnum/Whee/TextLayer.h"

namespace Magnum { namespace Whee {

struct TextProperties::State {
    Containers::String languageStorage;
    Containers::Array<Text::FeatureRange> features;
};

/** @todo change LayoutDirection to Unspecified as well, once Text APIs don't
    enforce HorizontalTopToBottom anymore */
TextProperties::TextProperties(): _script{Text::Script::Unspecified}, _font{FontHandle::Null}, _alignment{Text::Alignment::MiddleCenter}, _direction{UnsignedByte(Text::ShapeDirection::Unspecified)|(UnsignedByte(Text::LayoutDirection::HorizontalTopToBottom) << 4)} {}

TextProperties::TextProperties(TextProperties&&) noexcept = default;

TextProperties::~TextProperties() = default;

TextProperties& TextProperties::operator=(TextProperties&&) noexcept = default;

TextProperties& TextProperties::setAlignment(const Text::Alignment alignment) {
    CORRADE_ASSERT(!(UnsignedByte(alignment) & Text::Implementation::AlignmentGlyphBounds),
        "Whee::TextProperties::setAlignment():" << alignment << "is not supported", *this);
    _alignment = alignment;
    return *this;
}

TextProperties& TextProperties::setLanguage(const Containers::StringView language) {
    /* If the string is global, just save it, otherwise make a copy and
       reference that */
    if(language.flags() & Containers::StringViewFlag::Global) {
        _language = language;
    } else {
        if(!_state)
            _state.emplace();
        _language = _state->languageStorage = language;
    }

    return *this;
}

Containers::ArrayView<const Text::FeatureRange> TextProperties::features() const {
    if(!_state)
        return {};
    return _state->features;
}

TextProperties& TextProperties::setFeatures(const Containers::ArrayView<const Text::FeatureRange> features) {
    if(!_state)
        _state.emplace();
    /* Using a growable array as the users may want to reuse instances to avoid
       temporary allocations */
    /** @todo arrayClear(), or maybe arrayAssign() here */
    arrayResize(_state->features, NoInit, 0);
    arrayAppend(_state->features, features);
    return *this;
}

TextProperties& TextProperties::setFeatures(const std::initializer_list<Text::FeatureRange> features) {
    return setFeatures(Containers::arrayView(features));
}

}}
