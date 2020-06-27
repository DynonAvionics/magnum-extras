#ifndef Magnum_versionExtras_h
#define Magnum_versionExtras_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020 Vladimír Vondruš <mosra@centrum.cz>

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

/* Note: this header is deliberately not included from anywhere except
   VersionTest to avoid triggering a full rebuild every time the Git commit
   hash changes. */

#define MAGNUMEXTRAS_VERSION_YEAR ${MAGNUMEXTRAS_VERSION_YEAR}
#define MAGNUMEXTRAS_VERSION_MONTH ${MAGNUMEXTRAS_VERSION_MONTH}
#cmakedefine MAGNUMEXTRAS_VERSION_COMMIT ${MAGNUMEXTRAS_VERSION_COMMIT}
#cmakedefine MAGNUMEXTRAS_VERSION_HASH 0x${MAGNUMEXTRAS_VERSION_HASH}
#cmakedefine MAGNUMEXTRAS_VERSION_STRING "${MAGNUMEXTRAS_VERSION_STRING}"

#endif