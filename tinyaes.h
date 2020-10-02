/*
 ---------------------------------------------------------------------------
 Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.
 Copyright (c) 2013, Igor Saric. All rights reserved.
 Copyright (c) 2014-2017, Andre Beckedorf, Oldenburg, Germany. All rights reserved.
 Copyright (c) 2014-2017, Wincent Balin, Oldenburg, Germany. All rights reserved.

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
#include <QObject>
#include <QString>
#include <QIODevice>

#ifdef KCL_filesystemutils
#include "KCL/filesystemutils.h"
#endif

// ECB is not enabled by default because it should never be used!
//#define ENABLE_MODE_ECB 1

class TinyAES : public QObject
{
    Q_OBJECT
public:
#ifdef ENABLE_MODE_ECB
    enum Mode
    {
        CBC,
        ECB
    };
    Q_ENUMS(Mode)

    Q_INVOKABLE static QByteArray encrypt(const QByteArray &input, const QByteArray &key, const Mode mode = CBC);
    Q_INVOKABLE static QByteArray decrypt(QByteArray input, const QByteArray &key, const Mode mode = CBC);
#else
    Q_INVOKABLE static QByteArray encrypt(const QByteArray &input, const QByteArray &key);
    Q_INVOKABLE static QByteArray decrypt(QByteArray input, const QByteArray &key);
#endif
    Q_INVOKABLE static QByteArray encrypt(QByteArray input, const QByteArray &key, const QByteArray &iv);
    Q_INVOKABLE static QByteArray decrypt(const QByteArray &input, const QByteArray &key, const QByteArray &iv);

#ifdef ENABLE_MODE_ECB
    Q_INVOKABLE static bool encrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const Mode mode = CBC);
    Q_INVOKABLE static bool decrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const Mode mode = CBC);
#else
    Q_INVOKABLE static bool encrypt(QIODevice *input, QIODevice *output, const QByteArray &key);
    Q_INVOKABLE static bool decrypt(QIODevice *input, QIODevice *output, const QByteArray &key);
#endif
    Q_INVOKABLE static bool encrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const QByteArray &iv);
    Q_INVOKABLE static bool decrypt(QIODevice *input, QIODevice *output, const QByteArray &key, const QByteArray &iv);

#ifdef KCL_filesystemutils
#ifdef ENABLE_MODE_ECB
    Q_INVOKABLE static bool encrypt(IODevice *input, IODevice *output, const QByteArray &key, const Mode mode = CBC)  { return encrypt(input->device(), output->device(), key, mode); }
    Q_INVOKABLE static bool decrypt(IODevice *input, IODevice *output, const QByteArray &key, const Mode mode = CBC)  { return encrypt(input->device(), output->device(), key, mode); }
#else
    Q_INVOKABLE static bool encrypt(IODevice *input, IODevice *output, const QByteArray &key) { return encrypt(input->device(), output->device(), key); }
    Q_INVOKABLE static bool decrypt(IODevice *input, IODevice *output, const QByteArray &key) { return decrypt(input->device(), output->device(), key); }
#endif
    Q_INVOKABLE static bool encrypt(IODevice *input, IODevice *output, const QByteArray &key, const QByteArray &iv) { return encrypt(input->device(), output->device(), key, iv); }
    Q_INVOKABLE static bool decrypt(IODevice *input, IODevice *output, const QByteArray &key, const QByteArray &iv) { return decrypt(input->device(), output->device(), key, iv); }
#endif

    Q_INVOKABLE static QByteArray hexStringToByte(QString key);

    Q_INVOKABLE static QByteArray getRandom128Bits();

private:
    static bool checkParams(const QByteArray &key, const QByteArray &iv);
    static void removePadding(QByteArray &input);
    static void addPadding(QByteArray &input);
};

#endif // TINYAES_H
