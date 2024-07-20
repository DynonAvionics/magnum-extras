#ifndef Magnum_Whee_Anchor_h
#define Magnum_Whee_Anchor_h
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
 * @brief Class @ref Magnum::Whee::Anchor
 * @m_since_latest
 */

#include <Corrade/Containers/EnumSet.h>
#include <Corrade/Containers/Reference.h>

#include "Magnum/Whee/visibility.h"
#include "Magnum/Whee/Whee.h"

namespace Magnum { namespace Whee {

/**
@brief Anchor for widget positioning

Wraps a @ref UserInterface reference, @ref NodeHandle and optional
@ref LayoutHandle. Meant to be returned from layouter instances to construct
widget isntanced with.
*/
class MAGNUM_WHEE_EXPORT Anchor {
    public:
        /**
         * @brief Constructor
         * @param ui        User interface instance
         * @param node      Node handle
         * @param layout    Layout associated with given node or
         *      @ref LayoutHandle::Null
         *
         * The @p node is expected to be valid in @p ui. If @p layout is not
         * @ref LayoutHandle::Null, it's expected to be valid in @p ui and
         * associated with @p node.
         * @see @ref AbstractUserInterface::isHandleValid(NodeHandle) const,
         *      @ref AbstractUserInterface::isHandleValid(LayoutHandle) const,
         */
        explicit Anchor(UserInterface& ui, NodeHandle node, LayoutHandle layout);

        /**
         * @brief Create a custom-positioned anchor
         *
         * Calls @ref AbstractUserInterface::createNode() with @p parent,
         * @p offset, @p size and @p flags, and remembers the created
         * @ref NodeHandle. The @ref layout() is @ref LayoutHandle::Null.
         */
        /*implicit*/ Anchor(UserInterface& ui, NodeHandle parent, const Vector2& offset, const Vector2& size, NodeFlags flags = {});

        /**
         * @brief Create a custom-sized anchor
         *
         * Calls @ref AbstractUserInterface::createNode() with @p parent,
         * zero offset, @p size and @p flags, and remembers the created
         * @ref NodeHandle. The @ref layout() is @ref LayoutHandle::Null.
         */
        /*implicit*/ Anchor(UserInterface& ui, NodeHandle parent, const Vector2& size, NodeFlags flags = {});

        /**
         * @brief Create a custom-positioned top-level anchor
         *
         * Equivalent to calling @ref Anchor(UserInterface&, NodeHandle, const Vector2&, const Vector2&, NodeFlags)
         * with @ref NodeHandle::Null as the parent.
         */
        /*implicit*/ Anchor(UserInterface& ui, const Vector2& offset, const Vector2& size, NodeFlags flags = {});

        /**
         * @brief Create a custom-sized top-level anchor
         *
         * Equivalent to calling @ref Anchor(UserInterface&, NodeHandle, const Vector2&, NodeFlags)
         * with @ref NodeHandle::Null as the parent.
         */
        /*implicit*/ Anchor(UserInterface& ui, const Vector2& size, NodeFlags flags = {});

        /** @brief User interface instance */
        UserInterface& ui() const { return _ui; }

        /**
         * @brief Node handle
         *
         * Guaranteed to be never @ref NodeHandle::Null.
         */
        NodeHandle node() const { return _node; }

        /**
         * @brief Node handle
         *
         * Guaranteed to be never @ref NodeHandle::Null.
         */
        operator NodeHandle() const { return _node; }

        /**
         * @brief Layout handle
         *
         * Can be @ref LayoutHandle::Null, in which case the anchor doesn't
         * have any associated layout.
         */
        LayoutHandle layout() const { return _layout; }

        /**
         * @brief Layout handle
         *
         * Unlike @ref layout() expects that the handle is not null.
         */
        operator LayoutHandle() const;

    private:
        Containers::Reference<UserInterface> _ui;
        /* 0/4 bytes free */
        NodeHandle _node;
        LayoutHandle _layout;
};

}}

#endif
