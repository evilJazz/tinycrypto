# TinyCrypto
A tiny collection of essential crypto algorithms missing in (older) Qt.

This library includes
- AES / Rijndael 128/192/256-bit CBC encryption with support for QByteArray and QIODevice inputs/outputs;
- HMAC (Keyed-Hash Message Authentication Code) with support for all hash methods available in QCryptohraphicHash;
- PBKDF2 (Password-Based Key Derivation Function 2) with support for all hash methods available in QCryptohraphicHash.

Add in your .pro file:

    include($$PWD/tinycrypto/tinycrypto.pri)

For usage of the various methods, please refer to the projects included in the "example" and "tests" sub-directory.
All methods are tested against known test vectors published in RFC2104 (HMAC), RFC6070 (PBKDF2) and relevant Wikipedia pages. Please refer to the Qt unit test in the tests sub-directory.
