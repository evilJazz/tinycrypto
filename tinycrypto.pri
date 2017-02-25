TINYCRYPTO_SRC_PATH = $${PWD}
TINYCRYPTO_INC_PATH = $${PWD}

SOURCES += \
    $$TINYCRYPTO_SRC_PATH/tinyaes.cpp \
    $$TINYCRYPTO_SRC_PATH/tinyhmac.cpp \
    $$TINYCRYPTO_SRC_PATH/tinypbkdf2.cpp

HEADERS += \
    $$TINYCRYPTO_INC_PATH/tinyaes.h \
    $$TINYCRYPTO_INC_PATH/tinyhmac.h \
    $$TINYCRYPTO_INC_PATH/tinypbkdf2.h

INCLUDEPATH += $$TINYCRYPTO_INC_PATH
