/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>

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

#include "AbstractRenderer.h"

#include <Corrade/Containers/EnumSet.hpp>
#include <Corrade/Utility/Assert.h>
#include <Magnum/Math/Vector2.h>

namespace Magnum { namespace Ui {

Debug& operator<<(Debug& debug, const RendererFeature value) {
    debug << "Ui::RendererFeature" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case RendererFeature::value: return debug << "::" #value;
        _c(Composite)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << Debug::hex << UnsignedByte(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const RendererFeatures value) {
    return Containers::enumSetDebugOutput(debug, value, "Ui::RendererFeatures{}", {
        RendererFeature::Composite
    });
}

Debug& operator<<(Debug& debug, const RendererTargetState value) {
    debug << "Ui::RendererTargetState" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case RendererTargetState::value: return debug << "::" #value;
        _c(Initial)
        _c(Draw)
        _c(Composite)
        _c(Final)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << Debug::hex << UnsignedByte(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const RendererDrawState value) {
    debug << "Ui::RendererDrawState" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case RendererDrawState::value: return debug << "::" #value;
        _c(Blending)
        _c(Scissor)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << Debug::hex << UnsignedByte(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const RendererDrawStates value) {
    return Containers::enumSetDebugOutput(debug, value, "Ui::RendererDrawStates{}", {
        RendererDrawState::Blending,
        RendererDrawState::Scissor
    });
}

struct AbstractRenderer::State {
    Vector2i framebufferSize;
    RendererTargetState currentTargetState = RendererTargetState::Initial;
    RendererDrawStates currentDrawStates;
};

AbstractRenderer::AbstractRenderer(): _state{InPlaceInit} {}

AbstractRenderer::AbstractRenderer(AbstractRenderer&&) noexcept = default;

AbstractRenderer::~AbstractRenderer() = default;

AbstractRenderer& AbstractRenderer::operator=(AbstractRenderer&&) noexcept = default;

Vector2i AbstractRenderer::framebufferSize() const {
    return _state->framebufferSize;
}

RendererTargetState AbstractRenderer::currentTargetState() const {
    return _state->currentTargetState;
}

RendererDrawStates AbstractRenderer::currentDrawStates() const {
    return _state->currentDrawStates;
}

void AbstractRenderer::setupFramebuffers(const Vector2i& size) {
    State& state = *_state;
    CORRADE_ASSERT(size.product(),
        "Ui::AbstractRenderer::setupFramebuffers(): expected non-zero size, got" << Debug::packed << size, );
    CORRADE_ASSERT(state.currentTargetState == RendererTargetState::Initial || state.currentTargetState == RendererTargetState::Final,
        "Ui::AbstractRenderer::setupFramebuffers(): not allowed to be called in" << state.currentTargetState, );
    state.framebufferSize = size;
    doSetupFramebuffers(size);
}

void AbstractRenderer::transition(RendererTargetState targetState, RendererDrawStates drawStates) {
    State& state = *_state;

    CORRADE_ASSERT(!state.framebufferSize.isZero(),
        "Ui::AbstractRenderer::transition(): framebuffer size wasn't set up", );
    CORRADE_ASSERT(targetState != RendererTargetState::Composite || features() & RendererFeature::Composite,
        "Ui::AbstractRenderer::transition(): transition to" << targetState << "not supported", );
    CORRADE_ASSERT(
        (state.currentTargetState == RendererTargetState::Initial && targetState == RendererTargetState::Initial) ||
        (state.currentTargetState == RendererTargetState::Initial && targetState == RendererTargetState::Draw) ||
        (state.currentTargetState == RendererTargetState::Initial && targetState == RendererTargetState::Composite) ||
        (state.currentTargetState == RendererTargetState::Initial && targetState == RendererTargetState::Final) ||
        (state.currentTargetState == RendererTargetState::Draw && targetState == RendererTargetState::Draw) ||
        (state.currentTargetState == RendererTargetState::Draw && targetState == RendererTargetState::Composite) ||
        (state.currentTargetState == RendererTargetState::Draw && targetState == RendererTargetState::Final) ||
        (state.currentTargetState == RendererTargetState::Composite && targetState == RendererTargetState::Draw) ||
        (state.currentTargetState == RendererTargetState::Final && targetState == RendererTargetState::Initial),
        "Ui::AbstractRenderer::transition(): invalid transition from" << state.currentTargetState << "to" << targetState, );
    CORRADE_ASSERT((targetState != RendererTargetState::Initial && targetState != RendererTargetState::Composite && targetState != RendererTargetState::Final) || !drawStates,
        "Ui::AbstractRenderer::transition(): invalid" << drawStates << "in a transition to" << targetState, );

    if(targetState != state.currentTargetState ||
       drawStates != state.currentDrawStates) {
        doTransition(state.currentTargetState, targetState, state.currentDrawStates, drawStates);
        state.currentTargetState = targetState;
        state.currentDrawStates = drawStates;
    }
}

}}
