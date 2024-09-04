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

#include "Event.h"

#include <Corrade/Containers/EnumSet.hpp>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Utility/Debug.h>

namespace Magnum { namespace Whee {

Debug& operator<<(Debug& debug, const Pointer value) {
    debug << "Whee::Pointer" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case Pointer::value: return debug << "::" #value;
        _c(MouseLeft)
        _c(MouseMiddle)
        _c(MouseRight)
        _c(Finger)
        _c(Pen)
        _c(Eraser)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << Debug::hex << UnsignedByte(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const Pointers value) {
    return Containers::enumSetDebugOutput(debug, value, "Whee::Pointers{}", {
        Pointer::MouseLeft,
        Pointer::MouseMiddle,
        Pointer::MouseRight,
        Pointer::Finger,
        Pointer::Pen,
        Pointer::Eraser
    });
}

PointerMoveEvent::PointerMoveEvent(const Containers::Optional<Pointer> type, const Pointers types): PointerMoveEvent{type, types, {}} {}

PointerMoveEvent::PointerMoveEvent(const Containers::Optional<Pointer> type, const Pointers types, const Vector2& relativePosition): _relativePosition{relativePosition}, _type{type ? *type : Pointer{}}, _types{types} {}

Containers::Optional<Pointer> PointerMoveEvent::type() const {
    return _type == Pointer{} ? Containers::NullOpt : Containers::optional(_type);
}

}}
