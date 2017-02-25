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

#include "tinyhmac.h"

QByteArray TinyHMAC::hash(const QByteArray &input, const QByteArray &key, QCryptographicHash::Algorithm method, int blockSize)
{
    QByteArray inputKey = key;

    if (inputKey.length() > blockSize)
        inputKey = QCryptographicHash::hash(inputKey, method); // keys longer than blocksize are shortened using the hash method
    else if (inputKey.length() < blockSize)
        inputKey = inputKey.leftJustified(blockSize, 0); // keys shorter than blocksize are zero-padded

    QByteArray innerKeyPadding(blockSize, 0x36);
    QByteArray outerKeyPadding(blockSize, 0x5c);

    for (int i = 0; i < blockSize; ++i)
    {
        innerKeyPadding[i] = innerKeyPadding.at(i) ^ inputKey.at(i);
        outerKeyPadding[i] = outerKeyPadding.at(i) ^ inputKey.at(i);
    }

    QCryptographicHash ch(method);
    ch.addData(innerKeyPadding);
    ch.addData(input);

    QByteArray result = QCryptographicHash::hash(outerKeyPadding + ch.result(), method);
    return result;
}

bool TinyHMAC::isEqual(const QByteArray &hash1, const QByteArray &hash2)
{
    bool result = true;

    int minLength = qMin(hash1.length(), hash2.length());

    // Don't break, we do a time constant comparison by index to counter timing attacks
    for (int i = 0; i < minLength; ++i)
        result = result && hash1.at(i) == hash2.at(i);

    return result;
}
