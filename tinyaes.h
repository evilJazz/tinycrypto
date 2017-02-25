/*
 ---------------------------------------------------------------------------
 Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.
 Copyright (c) 2013, Igor Saric. All rights reserved.
 Copyright (c) 2014, Andre Beckedorf, Oldenburg, Germany. All rights reserved.

 LICENSE TERMS

 The redistribution and use of this software (with or without changes)
 is allowed without the payment of fees or royalties provided that:

  1. source code distributions include the above copyright notice, this
     list of conditions and the following disclaimer;

  2. binary distributions include the above copyright notice, this list
     of conditions and the following disclaimer in their documentation;

  3. the name of the copyright holder is not used to endorse products
     built using this software without specific written permission.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
*/

#ifndef TINYAES_H
#define TINYAES_H

#include <QByteArray>
#include <QString>
#include <QIODevice>

// ECB is not enabled by default because it should never be used!
//#define ENABLE_MODE_ECB 1

class TinyAES
{
public:
#ifdef ENABLE_MODE_ECB
    enum Mode
    {
        CBC,
        ECB
    };

    static QByteArray encrypt(const QByteArray &input, const QByteArray &key, const Mode mode = CBC);
    static QByteArray decrypt(QByteArray input, const QByteArray &key, const Mode mode = CBC);
#else
    static QByteArray encrypt(const QByteArray &input, const QByteArray &key);
    static QByteArray decrypt(QByteArray input, const QByteArray &key);
#endif
    static QByteArray encrypt(QByteArray input, const QByteArray &key, const QByteArray &iv);
    static QByteArray decrypt(const QByteArray &input, const QByteArray &key, const QByteArray &iv);

#ifdef ENABLE_MODE_ECB
    static bool encrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const Mode mode = CBC);
    static bool decrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const Mode mode = CBC);
#else
    static bool encrypt(QIODevice *input, QIODevice *output, const QByteArray &key);
    static bool decrypt(QIODevice *input, QIODevice *output, const QByteArray &key);
#endif
    static bool encrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const QByteArray &iv);
    static bool decrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const QByteArray &iv);

    static QByteArray hexStringToByte(QString key);

    static QByteArray getRandom128Bits();

private:
    static bool checkParams(const QByteArray &key, const QByteArray &iv);
    static void removePadding(QByteArray &input);
    static void addPadding(QByteArray &input);
};

#endif // TINYAES_H
