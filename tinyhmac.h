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

#ifndef TINYHMAC_H
#define TINYHMAC_H

#include <QByteArray>
#include <QObject>
#include <QCryptographicHash>

#ifdef KCL_filesystemutils
#include "KCL/filesystemutils.h"
#endif

class TinyHMAC : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE static QByteArray hash(const QByteArray &input, const QByteArray &key, QCryptographicHash::Algorithm algorithm, int blockSize = 64);
    Q_INVOKABLE static bool isEqual(const QByteArray &hash1, const QByteArray &hash2);

#ifdef KCL_filesystemutils
    Q_INVOKABLE static QByteArray hash(const QByteArray &input, const QByteArray &key, CryptographicHash::Algorithm algorithm, int blockSize = 64)
    {
        return hash(input, key, (QCryptographicHash::Algorithm)algorithm, blockSize);
    }
#endif
};

#endif // TINYHMAC_H
