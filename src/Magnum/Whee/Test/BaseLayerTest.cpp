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

#include <new>
#include <sstream> /** @todo remove once Debug is stream-free */
#include <Corrade/Containers/StridedArrayView.h>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/TestSuite/Compare/Container.h>
#include <Corrade/Utility/DebugStl.h> /** @todo remove once Debug is stream-free */

#include "Magnum/Whee/BaseLayer.h"
#include "Magnum/Whee/Handle.h"
#include "Magnum/Whee/Implementation/baseLayerState.h" /* for updateDataOrder() */

namespace Magnum { namespace Whee { namespace Test { namespace {

struct BaseLayerTest: TestSuite::Tester {
    explicit BaseLayerTest();

    template<class T> void styleSizeAlignment();

    void styleCommonConstructDefault();
    void styleCommonConstructNoInit();
    void styleCommonSetters();

    void styleItemConstructDefault();
    void styleItemConstructNoInit();
    void styleItemSetters();

    void sharedConstruct();
    void sharedConstructNoCreate();
    void sharedConstructCopy();
    void sharedConstructMove();

    void construct();
    void constructCopy();
    void constructMove();

    template<class T> void create();
    template<class T> void setStyle();
    void setColor();
    void setOutlineWidth();

    void invalidHandle();
    void styleOutOfRange();

    void updateDataOrder();
};

const struct {
    const char* name;
    bool emptyUpdate;
} UpdateDataOrderData[]{
    {"empty update", true},
    {"", false},
};

enum class Enum: UnsignedShort {};

Debug& operator<<(Debug& debug, Enum value) {
    return debug << UnsignedInt(value);
}

BaseLayerTest::BaseLayerTest() {
    addTests({&BaseLayerTest::styleSizeAlignment<BaseLayerStyleCommon>,
              &BaseLayerTest::styleSizeAlignment<BaseLayerStyleItem>,

              &BaseLayerTest::styleCommonConstructDefault,
              &BaseLayerTest::styleCommonConstructNoInit,
              &BaseLayerTest::styleCommonSetters,

              &BaseLayerTest::styleItemConstructDefault,
              &BaseLayerTest::styleItemConstructNoInit,
              &BaseLayerTest::styleItemSetters,

              &BaseLayerTest::sharedConstruct,
              &BaseLayerTest::sharedConstructNoCreate,
              &BaseLayerTest::sharedConstructCopy,
              &BaseLayerTest::sharedConstructMove,

              &BaseLayerTest::construct,
              &BaseLayerTest::constructCopy,
              &BaseLayerTest::constructMove,

              &BaseLayerTest::create<UnsignedInt>,
              &BaseLayerTest::create<Enum>,
              &BaseLayerTest::setStyle<UnsignedInt>,
              &BaseLayerTest::setStyle<Enum>,
              &BaseLayerTest::setColor,
              &BaseLayerTest::setOutlineWidth,

              &BaseLayerTest::invalidHandle,
              &BaseLayerTest::styleOutOfRange});

    addInstancedTests({&BaseLayerTest::updateDataOrder},
        Containers::arraySize(UpdateDataOrderData));
}

using namespace Math::Literals;

template<class> struct StyleTraits;
template<> struct StyleTraits<BaseLayerStyleCommon> {
    static const char* name() { return "BaseLayerStyleCommon"; }
};
template<> struct StyleTraits<BaseLayerStyleItem> {
    static const char* name() { return "BaseLayerStyleItem"; }
};

template<class T> void BaseLayerTest::styleSizeAlignment() {
    setTestCaseTemplateName(StyleTraits<T>::name());

    CORRADE_FAIL_IF(sizeof(T) % sizeof(Vector4) != 0, sizeof(T) << "is not a multiple of vec4 for UBO alignment.");

    /* 48-byte structures are fine, we'll align them to 768 bytes and not
       256, but warn about that */
    CORRADE_FAIL_IF(768 % sizeof(T) != 0, sizeof(T) << "can't fit exactly into 768-byte UBO alignment.");
    if(256 % sizeof(T) != 0)
        CORRADE_WARN(sizeof(T) << "can't fit exactly into 256-byte UBO alignment, only 768.");

    CORRADE_COMPARE(alignof(T), 4);
}

void BaseLayerTest::styleCommonConstructDefault() {
    BaseLayerStyleCommon a;
    BaseLayerStyleCommon b{DefaultInit};
    CORRADE_COMPARE(a.smoothness, 0.0f);
    CORRADE_COMPARE(b.smoothness, 0.0f);
    CORRADE_COMPARE(a.innerOutlineSmoothness, 0.0f);
    CORRADE_COMPARE(b.innerOutlineSmoothness, 0.0f);

    constexpr BaseLayerStyleCommon ca;
    constexpr BaseLayerStyleCommon cb{DefaultInit};
    CORRADE_COMPARE(ca.smoothness, 0.0f);
    CORRADE_COMPARE(cb.smoothness, 0.0f);
    CORRADE_COMPARE(ca.innerOutlineSmoothness, 0.0f);
    CORRADE_COMPARE(cb.innerOutlineSmoothness, 0.0f);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<BaseLayerStyleCommon>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<BaseLayerStyleCommon, DefaultInitT>::value);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<DefaultInitT, BaseLayerStyleCommon>::value);
}

void BaseLayerTest::styleCommonConstructNoInit() {
    /* Testing only some fields, should be enough */
    BaseLayerStyleCommon a;
    a.smoothness = 3.0f;
    a.innerOutlineSmoothness = 20.0f;

    new(&a) BaseLayerStyleCommon{NoInit};
    {
        /* Explicitly check we're not on Clang because certain Clang-based IDEs
           inherit __GNUC__ if GCC is used instead of leaving it at 4 like
           Clang itself does */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ >= 601 && __OPTIMIZE__
        CORRADE_EXPECT_FAIL("GCC 6.1+ misoptimizes and overwrites the value.");
        #endif
        CORRADE_COMPARE(a.smoothness, 3.0f);
        CORRADE_COMPARE(a.innerOutlineSmoothness, 20.0f);
    }
}

void BaseLayerTest::styleCommonSetters() {
    BaseLayerStyleCommon a;
    a.setSmoothness(34.0f, 12.0f);
    CORRADE_COMPARE(a.smoothness, 34.0f);
    CORRADE_COMPARE(a.innerOutlineSmoothness, 12.0f);

    /* Convenience overload setting both smoothness values */
    a.setSmoothness(2.5f);
    CORRADE_COMPARE(a.smoothness, 2.5f);
    CORRADE_COMPARE(a.innerOutlineSmoothness, 2.5f);
}

void BaseLayerTest::styleItemConstructDefault() {
    BaseLayerStyleItem a;
    BaseLayerStyleItem b{DefaultInit};
    CORRADE_COMPARE(a.topColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(b.topColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(a.bottomColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(b.bottomColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(a.outlineColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(b.outlineColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(a.outlineWidth, Vector4{0.0f});
    CORRADE_COMPARE(b.outlineWidth, Vector4{0.0f});
    CORRADE_COMPARE(a.cornerRadius, Vector4{0.0f});
    CORRADE_COMPARE(b.cornerRadius, Vector4{0.0f});
    CORRADE_COMPARE(a.innerOutlineCornerRadius, Vector4{0.0f});
    CORRADE_COMPARE(b.innerOutlineCornerRadius, Vector4{0.0f});

    constexpr BaseLayerStyleItem ca;
    constexpr BaseLayerStyleItem cb{DefaultInit};
    CORRADE_COMPARE(ca.topColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(cb.topColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(ca.bottomColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(cb.bottomColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(ca.outlineColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(cb.outlineColor, 0xffffffff_srgbaf);
    CORRADE_COMPARE(ca.outlineWidth, Vector4{0.0f});
    CORRADE_COMPARE(cb.outlineWidth, Vector4{0.0f});
    CORRADE_COMPARE(ca.cornerRadius, Vector4{0.0f});
    CORRADE_COMPARE(cb.cornerRadius, Vector4{0.0f});
    CORRADE_COMPARE(ca.innerOutlineCornerRadius, Vector4{0.0f});
    CORRADE_COMPARE(cb.innerOutlineCornerRadius, Vector4{0.0f});

    CORRADE_VERIFY(std::is_nothrow_default_constructible<BaseLayerStyleItem>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<BaseLayerStyleItem, DefaultInitT>::value);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<DefaultInitT, BaseLayerStyleItem>::value);
}

void BaseLayerTest::styleItemConstructNoInit() {
    /* Testing only some fields, should be enough */
    BaseLayerStyleItem a;
    a.bottomColor = 0xff3366_rgbf;
    a.innerOutlineCornerRadius = {1.0f, 2.0f, 3.0f, 4.0f};

    new(&a) BaseLayerStyleItem{NoInit};
    {
        /* Explicitly check we're not on Clang because certain Clang-based IDEs
           inherit __GNUC__ if GCC is used instead of leaving it at 4 like
           Clang itself does */
        #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ >= 601 && __OPTIMIZE__
        CORRADE_EXPECT_FAIL("GCC 6.1+ misoptimizes and overwrites the value.");
        #endif
        CORRADE_COMPARE(a.bottomColor, 0xff3366_rgbf);
        CORRADE_COMPARE(a.innerOutlineCornerRadius, (Vector4{1.0f, 2.0f, 3.0f, 4.0f}));
    }
}

void BaseLayerTest::styleItemSetters() {
    BaseLayerStyleItem a;
    a.setColor(0xff336699_rgbaf, 0xaabbccdd_rgbaf)
     .setOutlineColor(0x663399cc_rgbaf)
     .setOutlineWidth({1.0f, 2.0f, 3.0f, 4.0f})
     .setCornerRadius({5.0f, 6.0f, 7.0f, 8.0f})
     .setInnerOutlineCornerRadius({0.1f, 0.2f, 0.3f, 0.4f});
    CORRADE_COMPARE(a.topColor, 0xff336699_rgbaf);
    CORRADE_COMPARE(a.bottomColor, 0xaabbccdd_rgbaf);
    CORRADE_COMPARE(a.outlineColor, 0x663399cc_rgbaf);
    CORRADE_COMPARE(a.outlineWidth, (Vector4{1.0f, 2.0f, 3.0f, 4.0f}));
    CORRADE_COMPARE(a.cornerRadius, (Vector4{5.0f, 6.0f, 7.0f, 8.0f}));
    CORRADE_COMPARE(a.innerOutlineCornerRadius, (Vector4{0.1f, 0.2f, 0.3f, 0.4f}));

    /* Convenience overloads setting both colors and all edges/corners to the
       same value */
    a.setColor(0x11223344_rgbaf)
     .setOutlineWidth(2.75f)
     .setCornerRadius(3.25f)
     .setInnerOutlineCornerRadius(5.5f);
    CORRADE_COMPARE(a.topColor, 0x11223344_rgbaf);
    CORRADE_COMPARE(a.bottomColor, 0x11223344_rgbaf);
    CORRADE_COMPARE(a.outlineWidth, Vector4{2.75f});
    CORRADE_COMPARE(a.cornerRadius, Vector4{3.25f});
    CORRADE_COMPARE(a.innerOutlineCornerRadius, Vector4{5.5f});
}

void BaseLayerTest::sharedConstruct() {
    struct Shared: BaseLayer::Shared {
        explicit Shared(UnsignedInt styleCount): BaseLayer::Shared{styleCount} {}
    } shared{3};
    CORRADE_COMPARE(shared.styleCount(), 3);
}

void BaseLayerTest::sharedConstructNoCreate() {
    struct Shared: BaseLayer::Shared {
        explicit Shared(NoCreateT): BaseLayer::Shared{NoCreate} {}
    } shared{NoCreate};

    /* Shouldn't crash */
    CORRADE_VERIFY(true);

    /* Implicit construction is not allowed */
    CORRADE_VERIFY(!std::is_convertible<NoCreateT, BaseLayer::Shared>::value);
}

void BaseLayerTest::sharedConstructCopy() {
    CORRADE_VERIFY(!std::is_copy_constructible<BaseLayer::Shared>{});
    CORRADE_VERIFY(!std::is_copy_assignable<BaseLayer::Shared>{});
}

void BaseLayerTest::sharedConstructMove() {
    struct Shared: BaseLayer::Shared {
        explicit Shared(UnsignedInt styleCount): BaseLayer::Shared{styleCount} {}
    };

    Shared a{3};

    Shared b{Utility::move(a)};
    CORRADE_COMPARE(b.styleCount(), 3);

    Shared c{5};
    c = Utility::move(b);
    CORRADE_COMPARE(c.styleCount(), 3);

    CORRADE_VERIFY(std::is_nothrow_move_constructible<BaseLayer::Shared>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<BaseLayer::Shared>::value);
}

void BaseLayerTest::construct() {
    struct LayerShared: BaseLayer::Shared {
        explicit LayerShared(UnsignedInt styleCount): BaseLayer::Shared{styleCount} {}
    } shared{3};

    struct Layer: BaseLayer {
        explicit Layer(LayerHandle handle, Shared& shared): BaseLayer{handle, shared} {}

        LayerFeatures doFeatures() const override { return {}; }
    } layer{layerHandle(137, 0xfe), shared};

    /* There isn't anything to query on the BaseLayer itself */
    CORRADE_COMPARE(layer.handle(), layerHandle(137, 0xfe));
}

void BaseLayerTest::constructCopy() {
    CORRADE_VERIFY(!std::is_copy_constructible<BaseLayer>{});
    CORRADE_VERIFY(!std::is_copy_assignable<BaseLayer>{});
}

void BaseLayerTest::constructMove() {
    struct LayerShared: BaseLayer::Shared {
        explicit LayerShared(UnsignedInt styleCount): BaseLayer::Shared{styleCount} {}
    };

    struct Layer: BaseLayer {
        explicit Layer(LayerHandle handle, Shared& shared): BaseLayer{handle, shared} {}

        LayerFeatures doFeatures() const override { return {}; }
    };

    LayerShared shared{3};
    LayerShared shared2{5};

    Layer a{layerHandle(137, 0xfe), shared};

    Layer b{Utility::move(a)};
    /* There isn't anything to query on the BaseLayer itself */
    CORRADE_COMPARE(b.handle(), layerHandle(137, 0xfe));

    Layer c{layerHandle(0, 2), shared2};
    c = Utility::move(b);
    CORRADE_COMPARE(c.handle(), layerHandle(137, 0xfe));

    CORRADE_VERIFY(std::is_nothrow_move_constructible<Layer>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<Layer>::value);
}

template<class T> void BaseLayerTest::create() {
    setTestCaseTemplateName(std::is_same<T, Enum>::value ? "Enum" : "UnsignedInt");

    struct LayerShared: BaseLayer::Shared {
        explicit LayerShared(UnsignedInt styleCount): BaseLayer::Shared{styleCount} {}
    } shared{38};

    struct Layer: BaseLayer {
        explicit Layer(LayerHandle handle, Shared& shared): BaseLayer{handle, shared} {}

        LayerFeatures doFeatures() const override { return {}; }
    } layer{layerHandle(0, 1), shared};

    /* Just to be sure the getters aren't picking up the first ever data
       always */
    layer.create(2);

    /* Default color and outline width */
    {
        DataHandle data = layer.create(T(17));
        CORRADE_COMPARE(layer.style(data), 17);
        CORRADE_COMPARE(layer.color(data), 0xffffff_rgbf);
        CORRADE_COMPARE(layer.outlineWidth(data), Vector4{0.0f});

    /* Default outline width */
    } {
        DataHandle data = layer.create(T(23), 0xff3366_rgbf);
        CORRADE_COMPARE(layer.style(data), 23);
        CORRADE_COMPARE(layer.color(data), 0xff3366_rgbf);
        CORRADE_COMPARE(layer.outlineWidth(data), Vector4{0.0f});

    /* Single-value outline width */
    } {
        DataHandle data = layer.create(T(19), 0xff3366_rgbf, 4.0f);
        CORRADE_COMPARE(layer.style(data), 19);
        CORRADE_COMPARE(layer.color(data), 0xff3366_rgbf);
        CORRADE_COMPARE(layer.outlineWidth(data), Vector4{4.0f});

    /* Everything explicit, testing also the getter overloads and templates */
    } {
        DataHandle data = layer.create(T(37), 0xff3366_rgbf, {3.0f, 2.0f, 1.0f, 4.0f});
        CORRADE_COMPARE(layer.style(data), 37);
        /* Can't use T, as the function restrict to enum types which woulf fail
           for T == UnsignedInt */
        CORRADE_COMPARE(layer.template style<Enum>(data), Enum(37));
        CORRADE_COMPARE(layer.style(dataHandleData(data)), 37);
        /* Can't use T, as the function restrict to enum types which woulf fail
           for T == UnsignedInt */
        CORRADE_COMPARE(layer.template style<Enum>(dataHandleData(data)), Enum(37));
        CORRADE_COMPARE(layer.color(data), 0xff3366_rgbf);
        CORRADE_COMPARE(layer.color(dataHandleData(data)), 0xff3366_rgbf);
        CORRADE_COMPARE(layer.outlineWidth(data), (Vector4{3.0f, 2.0f, 1.0f, 4.0f}));
        CORRADE_COMPARE(layer.outlineWidth(dataHandleData(data)), (Vector4{3.0f, 2.0f, 1.0f, 4.0f}));
    }

    /* Creation alone shouldn't set any state flags */
    CORRADE_COMPARE(layer.state(), LayerStates{});
}

template<class T> void BaseLayerTest::setStyle() {
    setTestCaseTemplateName(std::is_same<T, Enum>::value ? "Enum" : "UnsignedInt");

    struct LayerShared: BaseLayer::Shared {
        explicit LayerShared(UnsignedInt styleCount): BaseLayer::Shared{styleCount} {}
    } shared{67};

    struct Layer: BaseLayer {
        explicit Layer(LayerHandle handle, Shared& shared): BaseLayer{handle, shared} {}

        LayerFeatures doFeatures() const override { return {}; }
    } layer{layerHandle(0, 1), shared};

    /* Just to be sure the setters aren't picking up the first ever data
       always */
    layer.create(2);

    DataHandle data = layer.create(17);
    CORRADE_COMPARE(layer.style(data), 17);
    CORRADE_COMPARE(layer.state(), LayerStates{});

    /* Setting a style marks the layer as dirty */
    layer.setStyle(data, T(37));
    CORRADE_COMPARE(layer.style(data), 37);
    CORRADE_COMPARE(layer.state(), LayerState::NeedsUpdate);

    /* Testing also the other overload */
    layer.setStyle(dataHandleData(data), T(66));
    CORRADE_COMPARE(layer.style(data), 66);
    CORRADE_COMPARE(layer.state(), LayerState::NeedsUpdate);
}

void BaseLayerTest::setColor() {
    struct LayerShared: BaseLayer::Shared {
        explicit LayerShared(UnsignedInt styleCount): BaseLayer::Shared{styleCount} {}
    } shared{3};

    struct Layer: BaseLayer {
        explicit Layer(LayerHandle handle, Shared& shared): BaseLayer{handle, shared} {}

        LayerFeatures doFeatures() const override { return {}; }
    } layer{layerHandle(0, 1), shared};

    /* Just to be sure the setters aren't picking up the first ever data
       always */
    layer.create(1);

    DataHandle data = layer.create(2, 0xff3366_rgbf);
    CORRADE_COMPARE(layer.color(data), 0xff3366_rgbf);
    CORRADE_COMPARE(layer.state(), LayerStates{});

    /* Setting a color marks the layer as dirty */
    layer.setColor(data, 0xaabbc_rgbf);
    CORRADE_COMPARE(layer.color(data), 0xaabbc_rgbf);
    CORRADE_COMPARE(layer.state(), LayerState::NeedsUpdate);

    /* Testing also the other overload */
    layer.setColor(dataHandleData(data), 0x112233_rgbf);
    CORRADE_COMPARE(layer.color(data), 0x112233_rgbf);
    CORRADE_COMPARE(layer.state(), LayerState::NeedsUpdate);
}

void BaseLayerTest::setOutlineWidth() {
    struct LayerShared: BaseLayer::Shared {
        explicit LayerShared(UnsignedInt styleCount): BaseLayer::Shared{styleCount} {}
    } shared{3};

    struct Layer: BaseLayer {
        explicit Layer(LayerHandle handle, Shared& shared): BaseLayer{handle, shared} {}

        LayerFeatures doFeatures() const override { return {}; }
    } layer{layerHandle(0, 1), shared};

    /* Just to be sure the setters aren't picking up the first ever data
       always */
    layer.create(2);

    DataHandle data = layer.create(1, 0xff3366_rgbf, {3.0f, 1.0f, 2.0f, 4.0f});
    CORRADE_COMPARE(layer.outlineWidth(data), (Vector4{3.0f, 1.0f, 2.0f, 4.0f}));
    CORRADE_COMPARE(layer.state(), LayerStates{});

    /* Setting an outline width marks the layer as dirty */
    layer.setOutlineWidth(data, {2.0f, 4.0f, 3.0f, 1.0f});
    CORRADE_COMPARE(layer.outlineWidth(data), (Vector4{2.0f, 4.0f, 3.0f, 1.0f}));
    CORRADE_COMPARE(layer.state(), LayerState::NeedsUpdate);

    /* Testing also the other overload */
    layer.setOutlineWidth(dataHandleData(data), {1.0f, 2.0f, 3.0f, 4.0f});
    CORRADE_COMPARE(layer.outlineWidth(data), (Vector4{1.0f, 2.0f, 3.0f, 4.0f}));
    CORRADE_COMPARE(layer.state(), LayerState::NeedsUpdate);

    /* Single-value width */
    layer.setOutlineWidth(data, 4.0f);
    CORRADE_COMPARE(layer.outlineWidth(data), Vector4{4.0f});
    CORRADE_COMPARE(layer.state(), LayerState::NeedsUpdate);

    /* Testing also the other overload */
    layer.setOutlineWidth(dataHandleData(data), 3.0f);
    CORRADE_COMPARE(layer.outlineWidth(data), Vector4{3.0f});
    CORRADE_COMPARE(layer.state(), LayerState::NeedsUpdate);
}

void BaseLayerTest::invalidHandle() {
    CORRADE_SKIP_IF_NO_ASSERT();

    struct LayerShared: BaseLayer::Shared {
        explicit LayerShared(UnsignedInt styleCount): BaseLayer::Shared{styleCount} {}
    } shared{1};

    struct Layer: BaseLayer {
        explicit Layer(LayerHandle handle, Shared& shared): BaseLayer{handle, shared} {}

        LayerFeatures doFeatures() const override { return {}; }
    } layer{layerHandle(0, 1), shared};

    std::ostringstream out;
    Error redirectError{&out};
    layer.style(DataHandle::Null);
    layer.style(LayerDataHandle::Null);
    layer.setStyle(DataHandle::Null, 0);
    layer.setStyle(LayerDataHandle::Null, 0);
    layer.color(DataHandle::Null);
    layer.color(LayerDataHandle::Null);
    layer.setColor(DataHandle::Null, {});
    layer.setColor(LayerDataHandle::Null, {});
    layer.outlineWidth(DataHandle::Null);
    layer.outlineWidth(LayerDataHandle::Null);
    layer.setOutlineWidth(DataHandle::Null, {});
    layer.setOutlineWidth(LayerDataHandle::Null, {});
    CORRADE_COMPARE(out.str(),
        "Whee::BaseLayer::style(): invalid handle Whee::DataHandle::Null\n"
        "Whee::BaseLayer::style(): invalid handle Whee::LayerDataHandle::Null\n"
        "Whee::BaseLayer::setStyle(): invalid handle Whee::DataHandle::Null\n"
        "Whee::BaseLayer::setStyle(): invalid handle Whee::LayerDataHandle::Null\n"
        "Whee::BaseLayer::color(): invalid handle Whee::DataHandle::Null\n"
        "Whee::BaseLayer::color(): invalid handle Whee::LayerDataHandle::Null\n"
        "Whee::BaseLayer::setColor(): invalid handle Whee::DataHandle::Null\n"
        "Whee::BaseLayer::setColor(): invalid handle Whee::LayerDataHandle::Null\n"
        "Whee::BaseLayer::outlineWidth(): invalid handle Whee::DataHandle::Null\n"
        "Whee::BaseLayer::outlineWidth(): invalid handle Whee::LayerDataHandle::Null\n"
        "Whee::BaseLayer::setOutlineWidth(): invalid handle Whee::DataHandle::Null\n"
        "Whee::BaseLayer::setOutlineWidth(): invalid handle Whee::LayerDataHandle::Null\n");
}

void BaseLayerTest::styleOutOfRange() {
    CORRADE_SKIP_IF_NO_ASSERT();

    struct LayerShared: BaseLayer::Shared {
        explicit LayerShared(UnsignedInt styleCount): BaseLayer::Shared{styleCount} {}
    } shared{3};

    struct Layer: BaseLayer {
        explicit Layer(LayerHandle handle, Shared& shared): BaseLayer{handle, shared} {}

        LayerFeatures doFeatures() const override { return {}; }
    } layer{layerHandle(0, 1), shared};

    DataHandle data = layer.create(2);

    std::ostringstream out;
    Error redirectError{&out};
    layer.create(3);
    layer.setStyle(data, 3);
    layer.setStyle(dataHandleData(data), 3);
    CORRADE_COMPARE(out.str(),
        "Whee::BaseLayer::create(): style 3 out of range for 3 styles\n"
        "Whee::BaseLayer::setStyle(): style 3 out of range for 3 styles\n"
        "Whee::BaseLayer::setStyle(): style 3 out of range for 3 styles\n");
}

void BaseLayerTest::updateDataOrder() {
    auto&& data = UpdateDataOrderData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Does just extremely basic verification that the vertex and index data
       get filled with correct contents and in correct order. The actual visual
       output is checked in BaseLayerGLTest. */

    struct LayerShared: BaseLayer::Shared {
        explicit LayerShared(UnsignedInt styleCount): BaseLayer::Shared{styleCount} {}
    } shared{4};

    struct Layer: BaseLayer {
        explicit Layer(LayerHandle handle, Shared& shared): BaseLayer{handle, shared} {}
        const BaseLayer::State& stateData() const { return *_state; }

        LayerFeatures doFeatures() const override { return {}; }
    } layer{layerHandle(0, 1), shared};

    /* Create 10 data handles. Only three get filled and actually used. */
    layer.create(0);                                                    /* 0 */
    layer.create(0);                                                    /* 1 */
    layer.create(0);                                                    /* 2 */
    DataHandle data3 = layer.create(2, 0xff3366_rgbf, {1.0f, 2.0f, 3.0f, 4.0f});
    layer.create(0);                                                    /* 4 */
    layer.create(0);                                                    /* 5 */
    layer.create(0);                                                    /* 6 */
    DataHandle data7 = layer.create(1, 0x112233_rgbf, Vector4{2.0f});
    layer.create(0);                                                    /* 8 */
    DataHandle data9 = layer.create(3, 0x663399_rgbf, {3.0f, 2.0f, 1.0f, 4.0f});

    /* Two node handles to attach the data to */
    NodeHandle node6 = nodeHandle(6, 0);
    NodeHandle node15 = nodeHandle(15, 0);
    layer.attach(data7, node15);
    layer.attach(data3, node6);
    layer.attach(data9, node15);

    Vector2 nodeOffsets[16];
    Vector2 nodeSizes[16];
    nodeOffsets[6] = {1.0f, 2.0f};
    nodeSizes[6] = {10.0f, 15.0f};
    nodeOffsets[15] = {3.0f, 4.0f};
    nodeSizes[15] = {20.0f, 5.0f};

    /* An empty update should generate an empty draw list */
    if(data.emptyUpdate) {
        layer.update({}, {}, {}, nodeOffsets, nodeSizes, {}, {});
        CORRADE_COMPARE_AS(layer.stateData().indices,
            Containers::ArrayView<const UnsignedInt>{},
            TestSuite::Compare::Container);
        return;
    }

    /* Just the filled subset is getting updated */
    UnsignedInt dataIds[]{9, 7, 3};
    layer.update(dataIds, {}, {}, nodeOffsets, nodeSizes, {}, {});

    /* The indices should be filled just for the three items */
    CORRADE_COMPARE_AS(layer.stateData().indices, Containers::arrayView<UnsignedInt>({
        9*4 + 0, 9*4 + 2, 9*4 + 1, 9*4 + 2, 9*4 + 3, 9*4 + 1, /* quad 9 */
        7*4 + 0, 7*4 + 2, 7*4 + 1, 7*4 + 2, 7*4 + 3, 7*4 + 1, /* quad 7 */
        3*4 + 0, 3*4 + 2, 3*4 + 1, 3*4 + 2, 3*4 + 3, 3*4 + 1, /* quad 3 */
    }), TestSuite::Compare::Container);

    /* The vertices are there for all data, but only the actually used are
       filled */
    CORRADE_COMPARE(layer.stateData().vertices.size(), 10*4);
    for(std::size_t i = 0; i != 4; ++i) {
        CORRADE_ITERATION(i);
        CORRADE_COMPARE(layer.stateData().vertices[3*4 + i].color, 0xff3366_rgbf);
        CORRADE_COMPARE(layer.stateData().vertices[3*4 + i].outlineWidth, (Vector4{1.0f, 2.0f, 3.0f, 4.0f}));
        CORRADE_COMPARE(layer.stateData().vertices[3*4 + i].style, 2);

        CORRADE_COMPARE(layer.stateData().vertices[7*4 + i].color, 0x112233_rgbf);
        CORRADE_COMPARE(layer.stateData().vertices[7*4 + i].outlineWidth, Vector4{2.0f});
        CORRADE_COMPARE(layer.stateData().vertices[7*4 + i].style, 1);

        CORRADE_COMPARE(layer.stateData().vertices[9*4 + i].color, 0x663399_rgbf);
        CORRADE_COMPARE(layer.stateData().vertices[9*4 + i].outlineWidth, (Vector4{3.0f, 2.0f, 1.0f, 4.0f}));
        CORRADE_COMPARE(layer.stateData().vertices[9*4 + i].style, 3);
    }

    Containers::StridedArrayView1D<const Vector2> positions = stridedArrayView(layer.stateData().vertices).slice(&Implementation::BaseLayerVertex::position);
    Containers::StridedArrayView1D<const Vector2> centerDistances = stridedArrayView(layer.stateData().vertices).slice(&Implementation::BaseLayerVertex::centerDistance);

    /* Data 3 is attached to node 6 */
    CORRADE_COMPARE_AS(positions.sliceSize(3*4, 4), Containers::arrayView<Vector2>({
        {1.0f, 2.0f},
        {11.0f, 2.0f},
        {1.0f, 17.0f},
        {11.0f, 17.0f},
    }), TestSuite::Compare::Container);
    CORRADE_COMPARE_AS(centerDistances.sliceSize(3*4, 4), Containers::arrayView<Vector2>({
        {-5.0f, -7.5f},
        { 5.0f, -7.5f},
        {-5.0f,  7.5f},
        { 5.0f,  7.5f},
    }), TestSuite::Compare::Container);

    /* Data 7 and 9 are both attached to node 15 */
    for(std::size_t i: {7, 9}) {
        CORRADE_COMPARE_AS(positions.sliceSize(i*4, 4), Containers::arrayView<Vector2>({
            {3.0f, 4.0f},
            {23.0f, 4.0f},
            {3.0f, 9.0f},
            {23.0f, 9.0f},
        }), TestSuite::Compare::Container);
        CORRADE_COMPARE_AS(centerDistances.sliceSize(i*4, 4), Containers::arrayView<Vector2>({
            {-10.0f, -2.5f},
            { 10.0f, -2.5f},
            {-10.0f,  2.5f},
            { 10.0f,  2.5f},
        }), TestSuite::Compare::Container);
    }
}

}}}}

CORRADE_TEST_MAIN(Magnum::Whee::Test::BaseLayerTest)
