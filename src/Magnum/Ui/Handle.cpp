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

#include "Handle.h"

#include <Corrade/Utility/Debug.h>

namespace Magnum { namespace Ui {

Debug& operator<<(Debug& debug, const LayerHandle value) {
    if(value == LayerHandle::Null)
        return debug << "Ui::LayerHandle::Null";
    return debug << "Ui::LayerHandle(" << Debug::nospace << Debug::hex << layerHandleId(value) << Debug::nospace << "," << Debug::hex << layerHandleGeneration(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const LayerDataHandle value) {
    if(value == LayerDataHandle::Null)
        return debug << "Ui::LayerDataHandle::Null";
    return debug << "Ui::LayerDataHandle(" << Debug::nospace << Debug::hex << layerDataHandleId(value) << Debug::nospace << "," << Debug::hex << layerDataHandleGeneration(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const DataHandle value) {
    if(value == DataHandle::Null)
        return debug << "Ui::DataHandle::Null";

    debug << "Ui::DataHandle(" << Debug::nospace;
    if(dataHandleLayer(value) == LayerHandle::Null)
        debug << "Null,";
    else
        debug << "{" << Debug::nospace << Debug::hex << dataHandleLayerId(value) << Debug::nospace << "," << Debug::hex << dataHandleLayerGeneration(value) << Debug::nospace << "},";

    if(dataHandleData(value) == LayerDataHandle::Null)
        debug << "Null)";
    else
        debug << "{" << Debug::nospace << Debug::hex << dataHandleId(value) << Debug::nospace << "," << Debug::hex << dataHandleGeneration(value) << Debug::nospace << "})";

    return debug;
}

Debug& operator<<(Debug& debug, const NodeHandle value) {
    if(value == NodeHandle::Null)
        return debug << "Ui::NodeHandle::Null";
    return debug << "Ui::NodeHandle(" << Debug::nospace << Debug::hex << nodeHandleId(value) << Debug::nospace << "," << Debug::hex << nodeHandleGeneration(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const LayouterHandle value) {
    if(value == LayouterHandle::Null)
        return debug << "Ui::LayouterHandle::Null";
    return debug << "Ui::LayouterHandle(" << Debug::nospace << Debug::hex << layouterHandleId(value) << Debug::nospace << "," << Debug::hex << layouterHandleGeneration(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const LayouterDataHandle value) {
    if(value == LayouterDataHandle::Null)
        return debug << "Ui::LayouterDataHandle::Null";
    return debug << "Ui::LayouterDataHandle(" << Debug::nospace << Debug::hex << layouterDataHandleId(value) << Debug::nospace << "," << Debug::hex << layouterDataHandleGeneration(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const LayoutHandle value) {
    if(value == LayoutHandle::Null)
        return debug << "Ui::LayoutHandle::Null";

    debug << "Ui::LayoutHandle(" << Debug::nospace;
    if(layoutHandleLayouter(value) == LayouterHandle::Null)
        debug << "Null,";
    else
        debug << "{" << Debug::nospace << Debug::hex << layoutHandleLayouterId(value) << Debug::nospace << "," << Debug::hex << layoutHandleLayouterGeneration(value) << Debug::nospace << "},";

    if(layoutHandleData(value) == LayouterDataHandle::Null)
        debug << "Null)";
    else
        debug << "{" << Debug::nospace << Debug::hex << layoutHandleId(value) << Debug::nospace << "," << Debug::hex << layoutHandleGeneration(value) << Debug::nospace << "})";

    return debug;
}

Debug& operator<<(Debug& debug, const AnimatorHandle value) {
    if(value == AnimatorHandle::Null)
        return debug << "Ui::AnimatorHandle::Null";
    return debug << "Ui::AnimatorHandle(" << Debug::nospace << Debug::hex << animatorHandleId(value) << Debug::nospace << "," << Debug::hex << animatorHandleGeneration(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const AnimatorDataHandle value) {
    if(value == AnimatorDataHandle::Null)
        return debug << "Ui::AnimatorDataHandle::Null";
    return debug << "Ui::AnimatorDataHandle(" << Debug::nospace << Debug::hex << animatorDataHandleId(value) << Debug::nospace << "," << Debug::hex << animatorDataHandleGeneration(value) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const AnimationHandle value) {
    if(value == AnimationHandle::Null)
        return debug << "Ui::AnimationHandle::Null";

    debug << "Ui::AnimationHandle(" << Debug::nospace;
    if(animationHandleAnimator(value) == AnimatorHandle::Null)
        debug << "Null,";
    else
        debug << "{" << Debug::nospace << Debug::hex << animationHandleAnimatorId(value) << Debug::nospace << "," << Debug::hex << animationHandleAnimatorGeneration(value) << Debug::nospace << "},";

    if(animationHandleData(value) == AnimatorDataHandle::Null)
        debug << "Null)";
    else
        debug << "{" << Debug::nospace << Debug::hex << animationHandleId(value) << Debug::nospace << "," << Debug::hex << animationHandleGeneration(value) << Debug::nospace << "})";

    return debug;
}

}}
