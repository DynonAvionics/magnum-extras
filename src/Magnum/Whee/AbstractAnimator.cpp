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

#include "AbstractAnimator.h"

#include <Corrade/Containers/ArrayTuple.h>
#include <Corrade/Containers/BitArray.h>
#include <Corrade/Containers/BitArrayView.h>
#include <Corrade/Containers/EnumSet.hpp>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/StridedArrayView.h>
#include <Magnum/Math/Functions.h>
#include <Magnum/Math/Time.h>

#include "Magnum/Whee/AbstractLayer.h"
#include "Magnum/Whee/Handle.h"

namespace Magnum { namespace Whee {

using namespace Math::Literals;

Debug& operator<<(Debug& debug, const AnimatorFeature value) {
    debug << "Whee::AnimatorFeature" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case AnimatorFeature::value: return debug << "::" #value;
        _c(NodeAttachment)
        _c(DataAttachment)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << Debug::hex << UnsignedByte(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const AnimatorFeatures value) {
    return Containers::enumSetDebugOutput(debug, value, "Whee::AnimatorFeatures{}", {
        AnimatorFeature::NodeAttachment,
        AnimatorFeature::DataAttachment
    });
}

Debug& operator<<(Debug& debug, const AnimatorState value) {
    debug << "Whee::AnimatorState" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case AnimatorState::value: return debug << "::" #value;
        _c(NeedsAdvance)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << Debug::hex << UnsignedByte(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const AnimatorStates value) {
    return Containers::enumSetDebugOutput(debug, value, "Whee::AnimatorStates{}", {
        AnimatorState::NeedsAdvance
    });
}

Debug& operator<<(Debug& debug, const AnimationFlag value) {
    debug << "Whee::AnimationFlag" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case AnimationFlag::value: return debug << "::" #value;
        _c(KeepOncePlayed)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << Debug::hex << UnsignedByte(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const AnimationFlags value) {
    return Containers::enumSetDebugOutput(debug, value, "Whee::AnimationFlags{}", {
        AnimationFlag::KeepOncePlayed
    });
}

Debug& operator<<(Debug& debug, const AnimationState value) {
    debug << "Whee::AnimationState" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case AnimationState::value: return debug << "::" #value;
        _c(Scheduled)
        _c(Playing)
        _c(Paused)
        _c(Stopped)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << Debug::hex << UnsignedByte(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const NodeAnimation value) {
    debug << "Whee::NodeAnimation" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(value) case NodeAnimation::value: return debug << "::" #value;
        _c(OffsetSize)
        _c(Enabled)
        _c(Clip)
        _c(Removal)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << Debug::hex << UnsignedByte(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const NodeAnimations value) {
    return Containers::enumSetDebugOutput(debug, value, "Whee::NodeAnimations{}", {
        NodeAnimation::OffsetSize,
        NodeAnimation::Enabled,
        NodeAnimation::Clip,
        NodeAnimation::Removal
    });
}

namespace {

union Animation {
    explicit Animation() noexcept: used{} {}

    struct Used {
        /* Together with index of this item in `animations` used for creating
           an AnimatorDataHandle. Increased every time a handle reaches
           remove(). Has to be initially non-zero to differentiate the first
           ever handle (with index 0) from AnimatorDataHandle::Null. Once
           becomes `1 << AnimatorDataHandleGenerationBits` the handle gets
           disabled. */
        UnsignedShort generation = 1;

        AnimationFlags flags{NoInit};
        /* One byte free */
        UnsignedInt repeatCount;

        /* Duration. 0 only when the animation is freed, otherwise it's always
           positive. */
        Nanoseconds duration{NoInit};

        /* Time at which the animation is played, paused, stopped. All these
           have to be re-filled every time a handle is recycled, so it doesn't
           make sense to initialize them to anything. */
        Nanoseconds played{NoInit};
        Nanoseconds paused{NoInit};
        Nanoseconds stopped{NoInit};
    } used;

    /* Used only if the Animation is among free ones */
    struct Free {
        /* The generation value has to be preserved in order to increment it
           next time it gets used */
        UnsignedShort generation;

        /* Two bytes free */

        /* See State::firstFree for more information */
        UnsignedInt next;

        /* If this is 0, the animation is freed */
        Nanoseconds duration;
    } free;
};

#ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
static_assert(std::is_trivially_copyable<Animation>::value, "Animation not trivially copyable");
#endif
static_assert(
    offsetof(Animation::Used, generation) == offsetof(Animation::Free, generation) &&
    offsetof(Animation::Used, duration) == offsetof(Animation::Free, duration),
    "Animation::Used and Free layout not compatible");

}

struct AbstractAnimator::State {
    AnimatorHandle handle;
    AnimatorStates state;

    /* 1 byte free */

    /* Used only if AnimatorFeature::DataAttachment is supported. Combined with
       `layerData` to form DataHandles. */
    LayerHandle layer = LayerHandle::Null;

    Containers::Array<Animation> animations;
    /* Indices in the `animations` array. The Animation then has a nextFree
       member containing the next free index. New animations get taken from the
       front, removed are put at the end. A value of ~UnsignedInt{} means
       there's no (first/next/last) free animation. */
    UnsignedInt firstFree = ~UnsignedInt{};
    UnsignedInt lastFree = ~UnsignedInt{};

    /* Used only if AnimatorFeature::NodeAttachment is supported, has the same
       size as `animations` */
    Containers::Array<NodeHandle> nodes;

    /* Used only if AnimatorFeature::DataAttachment is supported, has the same
       size as `animations`. Combined with `layer` to form DataHandles. */
    Containers::Array<LayerDataHandle> layerData;

    Nanoseconds time{Math::ZeroInit};
};

AbstractAnimator::AbstractAnimator(const AnimatorHandle handle): _state{InPlaceInit} {
    CORRADE_ASSERT(handle != AnimatorHandle::Null,
        "Whee::AbstractAnimator: handle is null", );
    _state->handle = handle;
}

AbstractAnimator::AbstractAnimator(AbstractAnimator&&) noexcept = default;

AbstractAnimator::~AbstractAnimator() = default;

AbstractAnimator& AbstractAnimator::operator=(AbstractAnimator&&) noexcept = default;

AnimatorHandle AbstractAnimator::handle() const {
    return _state->handle;
}

AnimatorFeatures AbstractAnimator::features() const {
    const AnimatorFeatures features = doFeatures();
    CORRADE_ASSERT(!(features >= AnimatorFeature::NodeAttachment) ||
                   !(features >= AnimatorFeature::DataAttachment),
        "Whee::AbstractAnimator::features():" << AnimatorFeature::NodeAttachment << "and" << AnimatorFeature::DataAttachment << "are mutually exclusive", {});
    return features;
}

LayerHandle AbstractAnimator::layer() const {
    CORRADE_ASSERT(features() & AnimatorFeature::DataAttachment,
        "Whee::AbstractAnimator::layer(): feature not supported", {});
    return _state->layer;
}

void AbstractAnimator::setLayerInternal(const AbstractLayer& layer) {
    /* Assumes the caller already verified presence of
       AnimatorFeature::DataAttachment and that the layer isn't set yet */
    _state->layer = layer.handle();
}

AnimatorStates AbstractAnimator::state() const {
    return _state->state;
}

Nanoseconds AbstractAnimator::time() const {
    return _state->time;
}

std::size_t AbstractAnimator::capacity() const {
    return _state->animations.size();
}

std::size_t AbstractAnimator::usedCount() const {
    std::size_t free = 0;
    for(const Animation& i: _state->animations)
        if(i.used.duration == 0_nsec && i.used.generation != 1 << Implementation::AnimatorDataHandleGenerationBits)
            ++free;
    return _state->animations.size() - free;
}

bool AbstractAnimator::isHandleValid(const AnimatorDataHandle handle) const {
    if(handle == AnimatorDataHandle::Null)
        return false;
    const UnsignedInt index = animatorDataHandleId(handle);
    if(index >= _state->animations.size())
        return false;
    /* Unlike UserInterface::isHandleValid(AnimatorHandle), the generation
       counter here is 16bit and a disabled handle is signalized by 0x10000,
       not 0, so for disabled handles this will always fail without having to
       do any extra checks.

       Note that this can still return true for manually crafted handles that
       point to free animations with correct generation counters. All other
       isHandleValid() aren't capable of detecting that without adding extra
       state either. */
    return animatorDataHandleGeneration(handle) == _state->animations[index].used.generation;
}

bool AbstractAnimator::isHandleValid(const AnimationHandle handle) const {
    return animationHandleAnimator(handle) == _state->handle && isHandleValid(animationHandleData(handle));
}

namespace {

AnimationState animationState(const Animation& animation, const Nanoseconds time) {
    /* The animation is stopped if the stopped time is before the played time.
       Not critically important for behavior as without it the animation would
       still work correctly, eventually transitioning from Scheduled to Stopped
       without any Playing or Paused in between, but this makes it Stopped
       already, potentially avoiding the need for AnimatorState::NeedsAdvance
       and useless UI redraw. */
    if(animation.used.stopped > animation.used.played) {
        /* The animation isn't playing yet if the played time is in the
           future */
        if(animation.used.played > time) {
            return AnimationState::Scheduled;

        /* The animation isn't playing anymore if the stopped time already
           happened */
        } else if(animation.used.stopped > time) {
            CORRADE_INTERNAL_ASSERT(animation.used.played <= time);

            const Nanoseconds currentTime = Math::min(animation.used.paused, time);

            /* The animation isn't playing anymore if all repeats were already
               exhausted */
            if(animation.used.repeatCount == 0 || animation.used.played + animation.used.duration*animation.used.repeatCount > currentTime) {
                /* The animation isn't currently playing if the paused time
                   already happened */
                if(animation.used.paused > time)
                    return AnimationState::Playing;
                else
                    return AnimationState::Paused;
            }
        }
    }

    return AnimationState::Stopped;
}

}

AnimationHandle AbstractAnimator::create(const Nanoseconds played, const Nanoseconds duration, const UnsignedInt repeatCount, const AnimationFlags flags) {
    CORRADE_ASSERT(duration > 0_nsec,
        "Whee::AbstractAnimator::create(): expected positive duration, got" << duration, {});

    /* Find the first free animation if there is, update the free index to
       point to the next one (or none) */
    Animation* animation;
    if(_state->firstFree!= ~UnsignedInt{}) {
        animation = &_state->animations[_state->firstFree];

        if(_state->firstFree == _state->lastFree) {
            CORRADE_INTERNAL_ASSERT(animation->free.next == ~UnsignedInt{});
            _state->firstFree = _state->lastFree = ~UnsignedInt{};
        } else {
            _state->firstFree = animation->free.next;
        }

    /* If there isn't, allocate a new one */
    } else {
        CORRADE_ASSERT(_state->animations.size() < 1 << Implementation::AnimatorDataHandleIdBits,
            "Whee::AbstractAnimator::create(): can only have at most" << (1 << Implementation::AnimatorDataHandleIdBits) << "animations", {});
        animation = &arrayAppend(_state->animations, InPlaceInit);
        if(features() & AnimatorFeature::NodeAttachment) {
            CORRADE_INTERNAL_ASSERT(_state->nodes.size() == _state->animations.size() - 1);
            arrayAppend(_state->nodes, NoInit, 1);
        }
        if(features() & AnimatorFeature::DataAttachment) {
            CORRADE_INTERNAL_ASSERT(_state->layerData.size() == _state->animations.size() - 1);
            arrayAppend(_state->layerData, NoInit, 1);
        }
    }

    const UnsignedInt id = animation - _state->animations;

    /* Fill the data. In both above cases the generation is already set
       appropriately, either initialized to 1, or incremented when it got
       remove()d (to mark existing handles as invalid) */
    animation->used.flags = flags;
    animation->used.repeatCount = repeatCount;
    animation->used.duration = duration;
    animation->used.played = played;
    animation->used.paused = Nanoseconds::max();
    animation->used.stopped = Nanoseconds::max();
    if(features() & AnimatorFeature::NodeAttachment)
        _state->nodes[id] = NodeHandle::Null;
    if(features() & AnimatorFeature::DataAttachment)
        _state->layerData[id] = LayerDataHandle::Null;

    /* Mark the animator as needing an advance() call if the new animation
       is being scheduled or played. Creation alone doesn't make it possible to
       make the animation paused, but if the animation is already stopped, mark
       it also to perform automatic removal. */
    const AnimationState state = animationState(*animation, _state->time);
    CORRADE_INTERNAL_ASSERT(state != AnimationState::Paused);
    if(state == AnimationState::Scheduled ||
       state == AnimationState::Playing ||
      (state == AnimationState::Stopped && !(flags & AnimationFlag::KeepOncePlayed)))
        _state->state |= AnimatorState::NeedsAdvance;

    return animationHandle(_state->handle, id, animation->used.generation);
}

AnimationHandle AbstractAnimator::create(const Nanoseconds played, const Nanoseconds duration, const AnimationFlags flags) {
    return create(played, duration, 1, flags);
}

AnimationHandle AbstractAnimator::create(const Nanoseconds played, const Nanoseconds duration, const NodeHandle node, const UnsignedInt repeatCount, const AnimationFlags flags) {
    CORRADE_ASSERT(features() >= AnimatorFeature::NodeAttachment,
        "Whee::AbstractAnimator::create(): node attachment not supported", {});
    const AnimationHandle handle = create(played, duration, repeatCount, flags);
    _state->nodes[animationHandleId(handle)] = node;
    return handle;
}

AnimationHandle AbstractAnimator::create(const Nanoseconds played, const Nanoseconds duration, const NodeHandle node, const AnimationFlags flags) {
    return create(played, duration, node, 1, flags);
}

AnimationHandle AbstractAnimator::create(const Nanoseconds played, const Nanoseconds duration, const DataHandle data, const UnsignedInt repeatCount, const AnimationFlags flags) {
    CORRADE_ASSERT(features() >= AnimatorFeature::DataAttachment,
        "Whee::AbstractAnimator::create(): data attachment not supported", {});
    CORRADE_ASSERT(_state->layer != LayerHandle::Null,
        "Whee::AbstractAnimator::create(): no layer set for data attachment", {});
    CORRADE_ASSERT(data == DataHandle::Null || _state->layer == dataHandleLayer(data),
        "Whee::AbstractAnimator::create(): expected a data handle with" << _state->layer << "but got" << data, {});
    const AnimationHandle handle = create(played, duration, repeatCount, flags);
    _state->layerData[animationHandleId(handle)] = dataHandleData(data);
    return handle;
}

AnimationHandle AbstractAnimator::create(const Nanoseconds played, const Nanoseconds duration, const DataHandle data, const AnimationFlags flags) {
    return create(played, duration, data, 1, flags);
}

AnimationHandle AbstractAnimator::create(const Nanoseconds played, const Nanoseconds duration, const LayerDataHandle data, const UnsignedInt repeatCount, const AnimationFlags flags) {
    CORRADE_ASSERT(features() >= AnimatorFeature::DataAttachment,
        "Whee::AbstractAnimator::create(): data attachment not supported", {});
    CORRADE_ASSERT(_state->layer != LayerHandle::Null,
        "Whee::AbstractAnimator::create(): no layer set for data attachment", {});
    const AnimationHandle handle = create(played, duration, repeatCount, flags);
    _state->layerData[animationHandleId(handle)] = data;
    return handle;
}

AnimationHandle AbstractAnimator::create(const Nanoseconds played, const Nanoseconds duration, const LayerDataHandle data, const AnimationFlags flags) {
    return create(played, duration, data, 1, flags);
}

void AbstractAnimator::remove(const AnimationHandle handle) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::remove(): invalid handle" << handle, );

    /* Doesn't delegate to remove(LayouterNodeHandle) to avoid a double check;
       doesn't check just the layer portion of the handle and delegate to avoid
       a confusing assertion message if the data portion would be invalid */
    removeInternal(animationHandleId(handle));
}

void AbstractAnimator::remove(const AnimatorDataHandle handle) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::remove(): invalid handle" << handle, );

    removeInternal(animatorDataHandleId(handle));
}

void AbstractAnimator::removeInternal(const UnsignedInt id) {
    Animation& animation = _state->animations[id];

    /* Increase the layout generation so existing handles pointing to this
       layout are invalidated */
    ++animation.used.generation;

    /* Set the animation duration to 0 to avoid falsely recognizing this item
       as used when directly iterating the list */
    animation.used.duration = 0_nsec;

    /* Clear the node attachment to have null handles in the nodes() list for
       freed animations */
    if(features() & AnimatorFeature::NodeAttachment)
        _state->nodes[id] = NodeHandle::Null;
    if(features() & AnimatorFeature::DataAttachment)
        _state->layerData[id] = LayerDataHandle::Null;

    /* Put the layout at the end of the free list (while they're allocated from
       the front) to not exhaust the generation counter too fast. If the free
       list is empty however, update also the index of the first free layer.

       Don't do this if the generation wrapped around. That makes it disabled,
       i.e. impossible to be recycled later, to avoid aliasing old handles. */
    if(animation.used.generation != 1 << Implementation::AnimatorDataHandleGenerationBits) {
        animation.free.next = ~UnsignedInt{};
        if(_state->lastFree == ~UnsignedInt{}) {
            CORRADE_INTERNAL_ASSERT(
                _state->firstFree == ~UnsignedInt{} &&
                _state->lastFree == ~UnsignedInt{});
            _state->firstFree = id;
        } else {
            _state->animations[_state->lastFree].free.next = id;
        }
        _state->lastFree = id;
    }
}

Nanoseconds AbstractAnimator::duration(const AnimationHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::duration(): invalid handle" << handle, {});
    return _state->animations[animationHandleId(handle)].used.duration;
}

Nanoseconds AbstractAnimator::duration(const AnimatorDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::duration(): invalid handle" << handle, {});
    return _state->animations[animatorDataHandleId(handle)].used.duration;
}

UnsignedInt AbstractAnimator::repeatCount(const AnimationHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::repeatCount(): invalid handle" << handle, {});
    return _state->animations[animationHandleId(handle)].used.repeatCount;
}

UnsignedInt AbstractAnimator::repeatCount(const AnimatorDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::repeatCount(): invalid handle" << handle, {});
    return _state->animations[animatorDataHandleId(handle)].used.repeatCount;
}

void AbstractAnimator::setRepeatCount(const AnimationHandle handle, const UnsignedInt count) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::setRepeatCount(): invalid handle" << handle, );
    _state->animations[animationHandleId(handle)].used.repeatCount = count;
    /* No AnimatorState needs to be updated, it doesn't cause any
       already-stopped animations to start playing */
}

void AbstractAnimator::setRepeatCount(const AnimatorDataHandle handle, const UnsignedInt count) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::setRepeatCount(): invalid handle" << handle, );
    _state->animations[animatorDataHandleId(handle)].used.repeatCount = count;
    /* No AnimatorState needs to be updated */
}

AnimationFlags AbstractAnimator::flags(const AnimationHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::flags(): invalid handle" << handle, {});
    return _state->animations[animationHandleId(handle)].used.flags;
}

AnimationFlags AbstractAnimator::flags(const AnimatorDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::flags(): invalid handle" << handle, {});
    return _state->animations[animatorDataHandleId(handle)].used.flags;
}

void AbstractAnimator::setFlags(const AnimationHandle handle, const AnimationFlags flags) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::setFlags(): invalid handle" << handle, );
    setFlagsInternal(animationHandleId(handle), flags);
}

void AbstractAnimator::setFlags(const AnimatorDataHandle handle, const AnimationFlags flags) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::setFlags(): invalid handle" << handle, );
    setFlagsInternal(animatorDataHandleId(handle), flags);
}

void AbstractAnimator::addFlags(const AnimationHandle handle, const AnimationFlags flags) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::addFlags(): invalid handle" << handle, );
    const UnsignedInt id = animationHandleId(handle);
    setFlagsInternal(id, _state->animations[id].used.flags|flags);
}

void AbstractAnimator::addFlags(const AnimatorDataHandle handle, const AnimationFlags flags) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::addFlags(): invalid handle" << handle, );
    const UnsignedInt id = animatorDataHandleId(handle);
    setFlagsInternal(id, _state->animations[id].used.flags|flags);
}

void AbstractAnimator::clearFlags(const AnimationHandle handle, const AnimationFlags flags) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::clearFlags(): invalid handle" << handle, );
    const UnsignedInt id = animationHandleId(handle);
    setFlagsInternal(id, _state->animations[id].used.flags & ~flags);
}

void AbstractAnimator::clearFlags(const AnimatorDataHandle handle, const AnimationFlags flags) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::clearFlags(): invalid handle" << handle, );
    const UnsignedInt id = animatorDataHandleId(handle);
    setFlagsInternal(id, _state->animations[id].used.flags & ~flags);
}

void AbstractAnimator::setFlagsInternal(const UnsignedInt id, const AnimationFlags flags) {
    _state->animations[id].used.flags = flags;
}

Nanoseconds AbstractAnimator::played(const AnimationHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::played(): invalid handle" << handle, {});
    return _state->animations[animationHandleId(handle)].used.played;
}

Nanoseconds AbstractAnimator::played(const AnimatorDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::played(): invalid handle" << handle, {});
    return _state->animations[animatorDataHandleId(handle)].used.played;
}

Nanoseconds AbstractAnimator::paused(const AnimationHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::paused(): invalid handle" << handle, {});
    return _state->animations[animationHandleId(handle)].used.paused;
}

Nanoseconds AbstractAnimator::paused(const AnimatorDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::paused(): invalid handle" << handle, {});
    return _state->animations[animatorDataHandleId(handle)].used.paused;
}

Nanoseconds AbstractAnimator::stopped(const AnimationHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::stopped(): invalid handle" << handle, {});
    return _state->animations[animationHandleId(handle)].used.stopped;
}

Nanoseconds AbstractAnimator::stopped(const AnimatorDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::stopped(): invalid handle" << handle, {});
    return _state->animations[animatorDataHandleId(handle)].used.stopped;
}

void AbstractAnimator::attach(const AnimationHandle animation, const NodeHandle node) {
    CORRADE_ASSERT(isHandleValid(animation),
        "Whee::AbstractAnimator::attach(): invalid handle" << animation, );
    attachInternal(animationHandleId(animation), node);
}

void AbstractAnimator::attach(const AnimatorDataHandle animation, const NodeHandle node) {
    CORRADE_ASSERT(isHandleValid(animation),
        "Whee::AbstractAnimator::attach(): invalid handle" << animation, );
    attachInternal(animatorDataHandleId(animation), node);
}

void AbstractAnimator::attachInternal(const UnsignedInt id, const NodeHandle node) {
    CORRADE_ASSERT(features() >= AnimatorFeature::NodeAttachment,
        "Whee::AbstractAnimator::attach(): node attachment not supported", );
    _state->nodes[id] = node;
}

NodeHandle AbstractAnimator::node(const AnimationHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::node(): invalid handle" << handle, {});
    return nodeInternal(animationHandleId(handle));
}

NodeHandle AbstractAnimator::node(const AnimatorDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::node(): invalid handle" << handle, {});
    return nodeInternal(animatorDataHandleId(handle));
}

NodeHandle AbstractAnimator::nodeInternal(const UnsignedInt id) const {
    CORRADE_ASSERT(features() >= AnimatorFeature::NodeAttachment,
        "Whee::AbstractAnimator::node(): feature not supported", {});
    return _state->nodes[id];
}

Containers::StridedArrayView1D<const NodeHandle> AbstractAnimator::nodes() const {
    CORRADE_ASSERT(features() >= AnimatorFeature::NodeAttachment,
        "Whee::AbstractAnimator::nodes(): feature not supported", {});
    CORRADE_INTERNAL_ASSERT(_state->nodes.size() == _state->animations.size());
    return _state->nodes;
}

void AbstractAnimator::attach(const AnimationHandle animation, const DataHandle data) {
    CORRADE_ASSERT(isHandleValid(animation),
        "Whee::AbstractAnimator::attach(): invalid handle" << animation, );
    attachInternal(animationHandleId(animation), data);
}

void AbstractAnimator::attach(const AnimatorDataHandle animation, const DataHandle data) {
    CORRADE_ASSERT(isHandleValid(animation),
        "Whee::AbstractAnimator::attach(): invalid handle" << animation, );
    attachInternal(animatorDataHandleId(animation), data);
}

void AbstractAnimator::attachInternal(const UnsignedInt id, const DataHandle data) {
    CORRADE_ASSERT(features() >= AnimatorFeature::DataAttachment,
        "Whee::AbstractAnimator::attach(): data attachment not supported", );
    CORRADE_ASSERT(_state->layer != LayerHandle::Null,
        "Whee::AbstractAnimator::attach(): no layer set for data attachment", );
    CORRADE_ASSERT(data == DataHandle::Null || _state->layer == dataHandleLayer(data),
        "Whee::AbstractAnimator::attach(): expected a data handle with" << _state->layer << "but got" << data, );
    _state->layerData[id] = dataHandleData(data);
}

void AbstractAnimator::attach(const AnimationHandle animation, const LayerDataHandle data) {
    CORRADE_ASSERT(isHandleValid(animation),
        "Whee::AbstractAnimator::attach(): invalid handle" << animation, );
    attachInternal(animationHandleId(animation), data);
}

void AbstractAnimator::attach(const AnimatorDataHandle animation, const LayerDataHandle data) {
    CORRADE_ASSERT(isHandleValid(animation),
        "Whee::AbstractAnimator::attach(): invalid handle" << animation, );
    attachInternal(animatorDataHandleId(animation), data);
}

void AbstractAnimator::attachInternal(const UnsignedInt id, const LayerDataHandle data) {
    CORRADE_ASSERT(features() >= AnimatorFeature::DataAttachment,
        "Whee::AbstractAnimator::attach(): data attachment not supported", );
    CORRADE_ASSERT(_state->layer != LayerHandle::Null,
        "Whee::AbstractAnimator::attach(): no layer set for data attachment", );
    _state->layerData[id] = data;
}

DataHandle AbstractAnimator::data(const AnimationHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::data(): invalid handle" << handle, {});
    return dataInternal(animationHandleId(handle));
}

DataHandle AbstractAnimator::data(const AnimatorDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::data(): invalid handle" << handle, {});
    return dataInternal(animatorDataHandleId(handle));
}

DataHandle AbstractAnimator::dataInternal(const UnsignedInt id) const {
    CORRADE_ASSERT(features() >= AnimatorFeature::DataAttachment,
        "Whee::AbstractAnimator::data(): feature not supported", {});
    const LayerDataHandle data = _state->layerData[id];
    if(data == LayerDataHandle::Null)
        return DataHandle::Null;
    /* attach() isn't possible to be called without a layer set, so the layer
       should always be a non-null handle at this point */
    const LayerHandle layer = _state->layer;
    CORRADE_INTERNAL_ASSERT(layer != LayerHandle::Null);
    return dataHandle(layer, data);
}

Containers::StridedArrayView1D<const LayerDataHandle> AbstractAnimator::layerData() const {
    CORRADE_ASSERT(features() >= AnimatorFeature::DataAttachment,
        "Whee::AbstractAnimator::layerData(): feature not supported", {});
    CORRADE_INTERNAL_ASSERT(_state->layerData.size() == _state->animations.size());
    return _state->layerData;
}

AnimationState AbstractAnimator::state(const AnimationHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::state(): invalid handle" << handle, {});
    return animationState(_state->animations[animationHandleId(handle)], _state->time);
}

AnimationState AbstractAnimator::state(const AnimatorDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::state(): invalid handle" << handle, {});
    return animationState(_state->animations[animatorDataHandleId(handle)], _state->time);
}

namespace {

inline Float animationFactor(const Nanoseconds duration, const Nanoseconds played, const Nanoseconds time) {
    CORRADE_INTERNAL_ASSERT(time >= played);
    const Nanoseconds difference = (time - played) % duration;
    /* Using doubles for the division to avoid precision loss even though
       floats seem to work even for the 292 year duration */
    return Double(Long(difference))/Double(Long(duration));
}

/* Shared between factorInternal() and advance() */
inline Float animationFactor(const Animation& animation, const Nanoseconds time, const AnimationState state) {
    if(state == AnimationState::Playing)
        return animationFactor(animation.used.duration, animation.used.played, time);
    if(state == AnimationState::Paused)
        return animationFactor(animation.used.duration, animation.used.played, animation.used.paused);
    if(state == AnimationState::Stopped)
        return 1.0f;

    CORRADE_INTERNAL_ASSERT_UNREACHABLE(); /* LCOV_EXCL_LINE */
}

}

Float AbstractAnimator::factor(const AnimationHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::factor(): invalid handle" << handle, {});
    return factorInternal(animationHandleId(handle));
}

Float AbstractAnimator::factor(const AnimatorDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::factor(): invalid handle" << handle, {});
    return factorInternal(animatorDataHandleId(handle));
}

Float AbstractAnimator::factorInternal(const UnsignedInt id) const {
    const Animation& animation = _state->animations[id];
    const Nanoseconds time = _state->time;
    const AnimationState state = animationState(animation, time);
    if(state == AnimationState::Scheduled)
        return 0.0f;
    return animationFactor(animation, time, state);
}

void AbstractAnimator::play(const AnimationHandle handle, const Nanoseconds time) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::play(): invalid handle" << handle, );
    playInternal(animationHandleId(handle), time);
}

void AbstractAnimator::play(const AnimatorDataHandle handle, const Nanoseconds time) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::play(): invalid handle" << handle, );
    playInternal(animatorDataHandleId(handle), time);
}

void AbstractAnimator::playInternal(const UnsignedInt id, const Nanoseconds time) {
    Animation& animation = _state->animations[id];

    /* If the animation
        - wasn't paused before (paused time is Nanoseconds::max()),
        - was stopped earlier than paused (paused time is >= stopped time),
        - was paused earlier than actually played,
        - we resume before the actual pause happens,
        - or we resume after it was stopped,
       play it from the start */
    if(animation.used.paused >= animation.used.stopped ||
       animation.used.played >= animation.used.paused ||
       animation.used.paused >= time ||
       time >= animation.used.stopped)
    {
        animation.used.played = time;

    /* Otherwise the played time is shortened by the duration for which it
       already played, i.e. `played = time - (paused - played)`, and the
       duration is non-negative */
    } else {
        CORRADE_INTERNAL_ASSERT(animation.used.paused > animation.used.played);
        animation.used.played += time - animation.used.paused;
    }

    animation.used.paused = Nanoseconds::max();
    animation.used.stopped = Nanoseconds::max();

    /* Mark the animator as needing advance() if the animation is now scheduled
       or playing. Can't be paused because the paused time was reset above. */
    const AnimationState state = animationState(animation, _state->time);
    CORRADE_INTERNAL_ASSERT(state != AnimationState::Paused);
    if(state == AnimationState::Scheduled ||
       state == AnimationState::Playing)
        _state->state |= AnimatorState::NeedsAdvance;
}

void AbstractAnimator::pause(const AnimationHandle handle, const Nanoseconds time) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::pause(): invalid handle" << handle, );
    pauseInternal(animationHandleId(handle), time);
}

void AbstractAnimator::pause(const AnimatorDataHandle handle, const Nanoseconds time) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::pause(): invalid handle" << handle, );
    pauseInternal(animatorDataHandleId(handle), time);
}

void AbstractAnimator::pauseInternal(const UnsignedInt id, const Nanoseconds time) {
    Animation& animation = _state->animations[id];
    #ifndef CORRADE_NO_ASSERT
    const AnimationState stateBefore = animationState(animation, _state->time);
    #endif
    animation.used.paused = time;

    #ifndef CORRADE_NO_ASSERT
    /* If the animation was scheduled, playing or paused before, it should be
       now as well, i.e. no need to mark the animator as needing advance() if
       it didn't need it before already. */
    const AnimationState stateAfter = animationState(animation, _state->time);
    if(stateBefore != AnimationState::Stopped)
        CORRADE_INTERNAL_ASSERT(stateAfter != AnimationState::Stopped && _state->state & AnimatorState::NeedsAdvance);
    #endif
}

void AbstractAnimator::stop(const AnimationHandle handle, const Nanoseconds time) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::stop(): invalid handle" << handle, );
    stopInternal(animationHandleId(handle), time);
}

void AbstractAnimator::stop(const AnimatorDataHandle handle, const Nanoseconds time) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::AbstractAnimator::stop(): invalid handle" << handle, );
    stopInternal(animatorDataHandleId(handle), time);
}

void AbstractAnimator::stopInternal(const UnsignedInt id, const Nanoseconds time) {
    Animation& animation = _state->animations[id];
    #ifndef CORRADE_NO_ASSERT
    const AnimationState stateBefore = animationState(animation, _state->time);
    #endif
    animation.used.stopped = time;

    #ifndef CORRADE_NO_ASSERT
    /* If the animation was stopped before, it should be now as well, i.e. no
       need to mark the animator as needing advance() if it didn't need it
       before already. */
    const AnimationState stateAfter = animationState(animation, _state->time);
    if(stateBefore == AnimationState::Stopped)
        CORRADE_INTERNAL_ASSERT(stateAfter == AnimationState::Stopped);
    else
        CORRADE_INTERNAL_ASSERT(_state->state & AnimatorState::NeedsAdvance);
    #endif
}

void AbstractAnimator::clean(const Containers::BitArrayView animationIdsToRemove) {
    CORRADE_ASSERT(animationIdsToRemove.size() == _state->animations.size(),
        "Whee::AbstractAnimator::clean(): expected" << _state->animations.size() << "bits but got" << animationIdsToRemove.size(), );
    /** @todo some way to efficiently iterate set bits */
    for(std::size_t i = 0; i != animationIdsToRemove.size(); ++i) {
        if(animationIdsToRemove[i]) removeInternal(i);
    }
    doClean(animationIdsToRemove);
}

void AbstractAnimator::doClean(Containers::BitArrayView) {}

void AbstractAnimator::cleanNodes(const Containers::StridedArrayView1D<const UnsignedShort>& nodeHandleGenerations) {
    CORRADE_ASSERT(features() >= AnimatorFeature::NodeAttachment,
        "Whee::AbstractAnimator::cleanNodes(): feature not supported", );

    /** @todo have some bump allocator for this */
    Containers::BitArray animationIdsToRemove{ValueInit, _state->animations.size()};

    CORRADE_INTERNAL_ASSERT(_state->nodes.size() == _state->animations.size());
    for(std::size_t i = 0; i != _state->nodes.size(); ++i) {
        const NodeHandle node = _state->nodes[i];

        /* Skip animations that are free or that aren't attached to any node */
        if(node == NodeHandle::Null)
            continue;

        /* For used & attached animations compare the generation of the node
           they're attached to. If it differs, remove the animation and mark
           the corresponding index so the implementation can do its own cleanup
           in doClean(). */
        /** @todo check that the ID is in bounds and if it's not, remove as
            well? to avoid OOB access if the animation is accidentally attached
            to a NodeHandle from a different UI instance that has more nodes */
        if(nodeHandleGeneration(node) != nodeHandleGenerations[nodeHandleId(node)]) {
            removeInternal(i);
            animationIdsToRemove.set(i);
        }
    }

    /* As removeInternal() was already called in the above loop, we don't need
       to delegate to clean() but can call doClean() directly */
    doClean(animationIdsToRemove);
}

void AbstractAnimator::cleanData(const Containers::StridedArrayView1D<const UnsignedShort>& dataHandleGenerations) {
    CORRADE_ASSERT(features() >= AnimatorFeature::DataAttachment,
        "Whee::AbstractAnimator::cleanData(): feature not supported", );
    CORRADE_ASSERT(_state->layer != LayerHandle::Null,
        "Whee::AbstractAnimator::cleanData(): no layer set for data attachment", );

    /** @todo have some bump allocator for this */
    Containers::BitArray animationIdsToRemove{ValueInit, _state->animations.size()};

    CORRADE_INTERNAL_ASSERT(_state->layerData.size() == _state->animations.size());
    for(std::size_t i = 0; i != _state->layerData.size(); ++i) {
        const LayerDataHandle data = _state->layerData[i];

        /* Skip animations that are free or that aren't attached to any data */
        if(data == LayerDataHandle::Null)
            continue;

        /* For used & attached animations compare the generation of the data
           they're attached to. If it differs, remove the animation and mark
           the corresponding index so the implementation can do its own cleanup
           in doClean(). */
        /** @todo check that the ID is in bounds and if it's not, remove as
            well? to avoid OOB access if the animation is accidentally attached
            to a LayerDataHandle from a different layer that has more data */
        if(layerDataHandleGeneration(data) != dataHandleGenerations[layerDataHandleId(data)]) {
            removeInternal(i);
            animationIdsToRemove.set(i);
        }
    }

    /* As removeInternal() was already called in the above loop, we don't need
       to delegate to clean() but can call doClean() directly */
    doClean(animationIdsToRemove);
}

Containers::Pair<bool, bool> AbstractAnimator::advance(const Nanoseconds time, const Containers::MutableBitArrayView active, const Containers::StridedArrayView1D<Float>& factors, const Containers::MutableBitArrayView remove) {
    CORRADE_ASSERT(active.size() == _state->animations.size() &&
                   factors.size() == _state->animations.size() &&
                   remove.size() == _state->animations.size(),
        "Whee::AbstractAnimator::advance(): expected active, factors and remove views to have a size of" << _state->animations.size() << "but got" << active.size() << Debug::nospace << "," << factors.size() << "and" << remove.size(), {});
    CORRADE_ASSERT(time >= _state->time,
        "Whee::AbstractAnimator::advance(): expected a time at least" << _state->time << "but got" << time, {});

    const Nanoseconds timeBefore = _state->time;
    bool cleanNeeded = false;
    bool advanceNeeded = false;
    bool anotherAdvanceNeeded = false;
    for(std::size_t i = 0; i != _state->animations.size(); ++i) {
        /* Animations with zero duration are freed items, skip */
        const Animation& animation = _state->animations[i];
        if(animation.used.duration == 0_nsec)
            continue;

        const AnimationState stateBefore = animationState(animation, timeBefore);
        const AnimationState stateAfter = animationState(animation, time);

        /* AnimationState has 4 values so there should be 16 different cases */
        switch((UnsignedShort(stateBefore) << 8)|UnsignedShort(stateAfter)) {
            #define _c(before, after) case (UnsignedShort(AnimationState::before) << 8)|UnsignedShort(AnimationState::after):
            /* The same calculation, together with dealing with a Scheduled
               state, is in factorInternal() */
            _c(Scheduled,Playing)
            _c(Playing,Playing)
            _c(Scheduled,Paused)
            _c(Playing,Paused)
            _c(Scheduled,Stopped)
            _c(Playing,Stopped)
            _c(Paused,Stopped)
                active.set(i);
                advanceNeeded = true;
                factors[i] = animationFactor(animation, time, stateAfter);
                break;

            /* These don't get advanced in any way */
            _c(Scheduled,Scheduled)
            _c(Paused,Paused)
            _c(Stopped,Stopped)
                break;

            /* These transitions shouldn't happen */
            /* LCOV_EXCL_START */
            _c(Playing,Scheduled)
            _c(Paused,Scheduled)
            _c(Paused,Playing)
            _c(Stopped,Scheduled)
            _c(Stopped,Playing)
            _c(Stopped,Paused)
                CORRADE_INTERNAL_ASSERT_UNREACHABLE();
            /* LCOV_EXCL_STOP */
            #undef _c
        }

        /* If the animation was stopped and isn't meant to be kept, schedule it
           for removal. In this case it isn't needed to ensure that it's only
           removed once, as in next advance() it'll be freed already and thus
           skipped. */
        if(stateAfter == AnimationState::Stopped && !(animation.used.flags & AnimationFlag::KeepOncePlayed)) {
            remove.set(i);
            cleanNeeded = true;
        }

        /* If the animation is still active, request another advance() */
        if(stateAfter == AnimationState::Scheduled ||
           stateAfter == AnimationState::Playing ||
           stateAfter == AnimationState::Paused)
            anotherAdvanceNeeded = true;
    }

    /* Update current time, mark the animator as needing an advance() call only
       if there are any actually active animations left */
    _state->time = time;
    if(anotherAdvanceNeeded)
        _state->state |= AnimatorState::NeedsAdvance;
    else
        _state->state &= ~AnimatorState::NeedsAdvance;

    return {advanceNeeded, cleanNeeded};
}

AbstractGenericAnimator::AbstractGenericAnimator(AnimatorHandle handle): AbstractAnimator{handle} {}

AbstractGenericAnimator::AbstractGenericAnimator(AbstractGenericAnimator&&) noexcept = default;

AbstractGenericAnimator::~AbstractGenericAnimator() = default;

AbstractGenericAnimator& AbstractGenericAnimator::operator=(AbstractGenericAnimator&&) noexcept = default;

void AbstractGenericAnimator::setLayer(const AbstractLayer& layer) {
    CORRADE_ASSERT(features() & AnimatorFeature::DataAttachment,
        "Whee::AbstractGenericAnimator::setLayer(): feature not supported", );
    CORRADE_ASSERT(this->layer() == LayerHandle::Null,
        "Whee::AbstractGenericAnimator::setLayer(): layer already set to" << this->layer(), );

    setLayerInternal(layer);
}

void AbstractGenericAnimator::advance(const Nanoseconds time) {
    /** @todo have some bump allocator for this (doesn't make sense to have it
        as a persistent allocation as the memory could be shared among several
        animators) */
    Containers::ArrayView<Float> factors;
    Containers::MutableBitArrayView active;
    Containers::MutableBitArrayView remove;
    const Containers::ArrayTuple storage{
        {NoInit, capacity(), factors},
        {ValueInit, capacity(), active},
        {ValueInit, capacity(), remove},
    };
    const Containers::Pair<bool, bool> advanceCleanNeeded = AbstractAnimator::advance(time, active, factors, remove);

    if(advanceCleanNeeded.first())
        doAdvance(active, factors);
    if(advanceCleanNeeded.second())
        clean(remove);
}

AbstractNodeAnimator::AbstractNodeAnimator(AnimatorHandle handle): AbstractAnimator{handle} {}

AbstractNodeAnimator::AbstractNodeAnimator(AbstractNodeAnimator&&) noexcept = default;

AbstractNodeAnimator::~AbstractNodeAnimator() = default;

AbstractNodeAnimator& AbstractNodeAnimator::operator=(AbstractNodeAnimator&&) noexcept = default;

AnimatorFeatures AbstractNodeAnimator::doFeatures() const {
    return AnimatorFeature::NodeAttachment;
}

NodeAnimations AbstractNodeAnimator::advance(const Nanoseconds time, const Containers::StridedArrayView1D<Vector2>& nodeOffsets, const Containers::StridedArrayView1D<Vector2>& nodeSizes, const Containers::StridedArrayView1D<NodeFlags>& nodeFlags, const Containers::MutableBitArrayView nodesRemove) {
    CORRADE_ASSERT(nodeOffsets.size() == nodeSizes.size() &&
                   nodeFlags.size() == nodeSizes.size() &&
                   nodesRemove.size() == nodeSizes.size(),
        "Whee::AbstractNodeAnimator::advance(): expected node offset, size, flags and remove views to have the same size but got" << nodeOffsets.size() << Debug::nospace << "," << nodeSizes.size() << Debug::nospace << "," << nodeFlags.size() << "and" << nodesRemove.size(), {});

    /** @todo have some bump allocator for this (doesn't make sense to have it
        as a persistent allocation as the memory could be shared among several
        animators) */
    Containers::ArrayView<Float> factors;
    Containers::MutableBitArrayView active;
    Containers::MutableBitArrayView remove;
    const Containers::ArrayTuple storage{
        {NoInit, capacity(), factors},
        {ValueInit, capacity(), active},
        {ValueInit, capacity(), remove},
    };
    const Containers::Pair<bool, bool> advanceCleanNeeded = AbstractAnimator::advance(time, active, factors, remove);

    NodeAnimations animations;
    if(advanceCleanNeeded.first())
        animations = doAdvance(active, factors, nodeOffsets, nodeSizes, nodeFlags, nodesRemove);
    if(advanceCleanNeeded.second())
        clean(remove);
    return animations;
}

}}
