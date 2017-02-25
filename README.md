# TinyCrypto
A tiny collection of essential crypto algorithms for Qt 4.x and up.

**Note: This library is intended for light use cases and does not replace a full-fledged and optimized crypto library** like Botan, Libre-/OpenSSL or Crypto++.

This library includes

 - AES / Rijndael 128/192/256-bit CBC encryption with support for QByteArray and QIODevice inputs/outputs;
 - HMAC (Keyed-Hash Message Authentication Code) with support for all hash algorithms available in QCryptographicHash;
 - PBKDF2 (Password-Based Key Derivation Function 2) with support for all hash algorithms available in QCryptographicHash.

Add in your .pro file:

    include($$PWD/tinycrypto/tinycrypto.pri)

For usage of the various methods, please refer to the projects included in the "example" and "tests" sub-directory.

All methods are tested against known test vectors published in RFC2104 (HMAC), RFC6070 (PBKDF2) and relevant Wikipedia pages. Please refer to the Qt unit test in the tests sub-directory.

For license information please refer to the preamble in each file.
