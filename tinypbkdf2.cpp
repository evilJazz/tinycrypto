/****************************************************************************
**
** Copyright (C) 2014-2016 Andre Beckedorf
** Contact: <evilJazz _AT_ katastrophos _DOT_ net>
**
** This file is part of the TinyCrypto library
**
** $TINYCRYPTO_BEGIN_LICENSE$
** GNU Lesser General Public License Usage
** This library is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License version
** 2.1 or 3.0 as published by the Free Software Foundation.
**
** This library is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details:
** https://www.gnu.org/licenses/lgpl-2.1.html
** https://www.gnu.org/licenses/lgpl-3.0.html
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
** 02110-1301  USA
**
** Mozilla Public License Usage
** Alternatively, this file is available under the Mozilla Public
** License Version 1.1.  You may obtain a copy of the License at
** http://www.mozilla.org/MPL/
**
** $TINYCRYPTO_END_LICENSE$
**
****************************************************************************/

#include "tinypbkdf2.h"

#include "tinyhmac.h"

#include <limits>

// Partly based on https://bugreports.qt.io/browse/QTBUG-30550

QByteArray TinyPBKDF2::derive(const QByteArray &password, const QByteArray &salt, QCryptographicHash::Algorithm method, int dstKeyLength, int iterations)
{
    if (iterations < 1 || dstKeyLength < 1)
        return QByteArray();

    const int saltLength = salt.length();

    if (saltLength == 0 || saltLength > std::numeric_limits<int>::max() - 4)
        return QByteArray();

    QByteArray result;

    QByteArray asalt = salt;
    asalt.resize(saltLength + 4);

    for (int i = 1, remainingBytes = dstKeyLength; remainingBytes > 0; ++i)
    {
        asalt[saltLength + 0] = static_cast<char>((i >> 24) & 0xff);
        asalt[saltLength + 1] = static_cast<char>((i >> 16) & 0xff);
        asalt[saltLength + 2] = static_cast<char>((i >> 8) & 0xff);
        asalt[saltLength + 3] = static_cast<char>(i & 0xff);

        QByteArray d1 = TinyHMAC::hash(asalt, password, method);
        QByteArray obuf = d1;

        for (int j = 1; j < iterations; ++j)
        {
            d1 = TinyHMAC::hash(d1, password, method);

            for (int k = 0; k < obuf.length(); ++k)
                obuf[k] = obuf.at(k) ^ d1.at(k);
        }

        result = result.append(obuf);
        remainingBytes -= obuf.size();

        d1.fill('\0');
        obuf.fill('\0');
    }

    asalt.fill('\0');

    return result.mid(0, dstKeyLength);
}
