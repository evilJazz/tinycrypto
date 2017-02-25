/****************************************************************************
**
** Copyright (C) 2014-2017 Andre Beckedorf
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

#include <QString>
#include <QtTest>

#include "tinyhmac.h"
#include "tinypbkdf2.h"
#include "tinyaes.h"

class TinyCryptoTest : public QObject
{
    Q_OBJECT

private:
    QByteArray inputData_;
    QByteArray inputDataDigest_;
    QByteArray expectedInputDataDigest_;
    QByteArray salt_;
    QByteArray password_;
    QByteArray key_;

private Q_SLOTS:
    void initTestCase();

    void testTinyHMAC_RFC2104();
    void testTinyHMAC_Wikipedia();

    void testTinyPBDKF2_RFC6070();

    void testTinyAES_QByteArray();
    void testTinyAES_QIODevice();
};

void TinyCryptoTest::initTestCase()
{
    inputData_ = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
    inputDataDigest_ = QCryptographicHash::hash(inputData_, QCryptographicHash::Sha1);
    expectedInputDataDigest_ = QByteArray::fromHex("19afa2a4a37462c7b940a6c4c61363d49c3a35f4");

    QVERIFY2(TinyHMAC::isEqual(inputDataDigest_, expectedInputDataDigest_), "Failure");

    salt_ = QByteArray("My secret salt");
    password_ = QByteArray("Hello world!");
    key_ = TinyPBKDF2::derive(password_, salt_, QCryptographicHash::Sha1, 256 / 8, 4096);

    QVERIFY2(TinyHMAC::isEqual(key_, QByteArray::fromHex("1eb68700a2e8290bda138181cda5e560a3298e4c42297c8b2ec0158edbf781d3")), "Failure");
}

void TinyCryptoTest::testTinyHMAC_RFC2104()
{
    // Check test sets from RFC2104
    QVERIFY2(TinyHMAC::isEqual(TinyHMAC::hash("Hi There", QByteArray::fromHex("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b"), QCryptographicHash::Md5), QByteArray::fromHex("9294727a3638bb1c13f48ef8158bfc9d")), "Failure RFC2104 Case 1");
    QVERIFY2(TinyHMAC::isEqual(TinyHMAC::hash("what do ya want for nothing?", "Jefe", QCryptographicHash::Md5), QByteArray::fromHex("750c783e6ab0b503eaa86e310a5db738")), "Failure RFC2104 Case 2");
    QVERIFY2(TinyHMAC::isEqual(TinyHMAC::hash(QByteArray::fromHex("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"), QByteArray::fromHex("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"), QCryptographicHash::Md5), QByteArray::fromHex("56be34521d144c88dbb8c733f0e8b3f6")), "Failure RFC2104 Case 3");
}

void TinyCryptoTest::testTinyHMAC_Wikipedia()
{
    // Check test sets from https://en.wikipedia.org/wiki/Hash-based_message_authentication_code
    QVERIFY2(TinyHMAC::isEqual(TinyHMAC::hash("", "", QCryptographicHash::Md5), QByteArray::fromHex("74e6f7298a9c2d168935f58c001bad88")), "Failure Wikipedia HMAC Case 1");
    QVERIFY2(TinyHMAC::isEqual(TinyHMAC::hash("", "", QCryptographicHash::Sha1), QByteArray::fromHex("fbdb1d1b18aa6c08324b7d64b71fb76370690e1d")), "Failure Wikipedia HMAC Case 2");
    QVERIFY2(TinyHMAC::isEqual(TinyHMAC::hash("The quick brown fox jumps over the lazy dog", "key", QCryptographicHash::Md5), QByteArray::fromHex("80070713463e7749b90c2dc24911e275")), "Failure Wikipedia HMAC Case 3");
    QVERIFY2(TinyHMAC::isEqual(TinyHMAC::hash("The quick brown fox jumps over the lazy dog", "key", QCryptographicHash::Sha1), QByteArray::fromHex("de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9")), "Failure Wikipedia HMAC Case 4");

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    // QCryptographicHash::Sha256 only available since Qt 5
    QVERIFY2(TinyHMAC::isEqual(TinyHMAC::hash("", "", QCryptographicHash::Sha256), QByteArray::fromHex("b613679a0814d9ec772f95d778c35fc5ff1697c493715653c6c712144292c5ad")), "Failure Wikipedia HMAC Case 5");
    QVERIFY2(TinyHMAC::isEqual(TinyHMAC::hash("The quick brown fox jumps over the lazy dog", "key", QCryptographicHash::Sha256), QByteArray::fromHex("f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8")), "Failure Wikipedia HMAC Case 6");
#endif
}

void TinyCryptoTest::testTinyPBDKF2_RFC6070()
{
    // Check test sets from RFC6070
    QVERIFY2(TinyHMAC::isEqual(TinyPBKDF2::derive("password", "salt", QCryptographicHash::Sha1, 20, 1), QByteArray::fromHex("0c 60 c8 0f 96 1f 0e 71 f3 a9 b5 24 af 60 12 06 2f e0 37 a6")), "Failure RFC6070 Case 1");
    QVERIFY2(TinyHMAC::isEqual(TinyPBKDF2::derive("password", "salt", QCryptographicHash::Sha1, 20, 4096), QByteArray::fromHex("4b 00 79 01 b7 65 48 9a be ad 49 d9 26 f7 21 d0 65 a4 29 c1")), "Failure RFC6070 Case 2");

    // The following takes a very long time without optimization!
    //QVERIFY2(TinyHMAC::isEqual(TinyPBKDF2::derive("password", "salt", QCryptographicHash::Sha1, 20, 16777216), QByteArray::fromHex("ee fe 3d 61 cd 4d a4 e4 e9 94 5b 3d 6b a2 15 8c 26 34 e9 84")), "Failure RFC6070 Case 3");

    QVERIFY2(TinyHMAC::isEqual(TinyPBKDF2::derive("passwordPASSWORDpassword", "saltSALTsaltSALTsaltSALTsaltSALTsalt", QCryptographicHash::Sha1, 25, 4096), QByteArray::fromHex("3d 2e ec 4f e4 1c 84 9b 80 c8 d8 36 62 c0 e4 4a 8b 29 1a 96 4c f2 f0 70 38")), "Failure RFC6070 Case 4");

    QByteArray password("pass"); password.append('\0'); password.append("word");
    QByteArray salt("sa"); salt.append('\0'); salt.append("lt");
    QVERIFY2(TinyHMAC::isEqual(TinyPBKDF2::derive(password, salt, QCryptographicHash::Sha1, 16, 4096), QByteArray::fromHex("56 fa 6a a7 55 48 09 9d cc 37 d7 f0 34 25 e0 c3")), "Failure RFC60760 Case 5");
}

void TinyCryptoTest::testTinyAES_QByteArray()
{
    QByteArray encryptedData = TinyAES::encrypt(inputData_, key_);
    QByteArray decryptedData = TinyAES::decrypt(encryptedData, key_);

    QByteArray decryptedDataDigest = QCryptographicHash::hash(decryptedData, QCryptographicHash::Sha1);

    QVERIFY2(TinyHMAC::isEqual(decryptedDataDigest, expectedInputDataDigest_), "Failure");

    QVERIFY2(TinyHMAC::isEqual(inputDataDigest_, decryptedDataDigest), "Failure");
}

void TinyCryptoTest::testTinyAES_QIODevice()
{
    QBuffer inputDataBuffer; inputDataBuffer.setData(inputData_); inputDataBuffer.open(QIODevice::ReadWrite);

    QByteArray encryptedData;
    QBuffer encryptedDataBuffer; encryptedDataBuffer.setBuffer(&encryptedData); encryptedDataBuffer.open(QIODevice::ReadWrite);

    QByteArray decryptedData;
    QBuffer decryptedDataBuffer; decryptedDataBuffer.setBuffer(&decryptedData); decryptedDataBuffer.open(QIODevice::ReadWrite);

    TinyAES::encrypt(&inputDataBuffer, &encryptedDataBuffer, key_);
    inputDataBuffer.seek(0);

    encryptedDataBuffer.seek(0);
    TinyAES::decrypt(&encryptedDataBuffer, &decryptedDataBuffer, key_);

    decryptedDataBuffer.seek(0);
    QByteArray decryptedDataDigest = QCryptographicHash::hash(decryptedDataBuffer.data(), QCryptographicHash::Sha1);

    QVERIFY2(TinyHMAC::isEqual(decryptedDataDigest, expectedInputDataDigest_), "Failure");

    QVERIFY2(TinyHMAC::isEqual(inputDataDigest_, decryptedDataDigest), "Failure");
}

QTEST_APPLESS_MAIN(TinyCryptoTest)

#include "tst_tinycryptotest.moc"
