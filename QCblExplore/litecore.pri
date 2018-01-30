# Avoid Multiple Inclusion
isEmpty(CONFIG_PRI) {
CONFIG_PRI = 1

CONFIG += c++11

LITECORE_DIR  = $$absolute_path($$quote($$PWD/../src/3rdparty/couchbase-lite-core))
LITECORE_BUILD_DIR  = $$absolute_path($$quote($$PWD/../build/3rdparty/couchbase-lite-core))

INCLUDEPATH   += \
    $$LITECORE_DIR/LiteCore \
    $$LITECORE_DIR/LiteCore/Database \
    $$LITECORE_DIR/LiteCore/Storage \
    $$LITECORE_DIR/LiteCore/Support \
    $$LITECORE_DIR/C \
    $$LITECORE_DIR/C/include \
    $$LITECORE_DIR/vendor/fleece/Fleece

win32 {

   LITECORE_BUILD_DIR  = $$quote($${LITECORE_BUILD_DIR}/windows)
   DEBUG_EXTENSION = ""

   contains(QMAKE_TARGET.arch, x86) {
      LITECORE_BUILD_DIR  = $$LITECORE_BUILD_DIR/x86
   } else {
      LITECORE_BUILD_DIR  = $$LITECORE_BUILD_DIR/x64
   }

   CONFIG(debug, debug|release){
       LITECORE_BUILD_DIR  = $$LITECORE_BUILD_DIR/debug
       DEBUG_EXTENSION = "d"
   }
   CONFIG(release, debug|release){
       LITECORE_BUILD_DIR  = $$LITECORE_BUILD_DIR/release
   }

    # LiteCore
    LIBS += -L$$LITECORE_BUILD_DIR  -lLiteCoreStatic

    # FleeceStatic
    LIBS += -L$$LITECORE_BUILD_DIR/vendor/fleece  -lFleeceStatic

    # SQLite3_UnicodeSN
    LIBS += -L$$LITECORE_BUILD_DIR/vendor/sqlite3-unicodesn  -lSQLite3_UnicodeSN

    # sqlcipher
    LIBS += -L$$LITECORE_BUILD_DIR/vendor/sqlcipher  -lsqlcipher
    LIBS += -L$$LITECORE_BUILD_DIR/vendor/sqlcipher/vendor/mbedtls/library  -lmbedcrypto

    # BLIPStatic
    LIBS += -L$$LITECORE_BUILD_DIR/vendor/BLIP-Cpp  -lBLIPStatic

    # zlib
    LIBS += -L$$LITECORE_BUILD_DIR/vendor/BLIP-Cpp/vendor/zlib -lzlibstatic$$DEBUG_EXTENSION

    # CivetWeb
    LIBS += -L$$LITECORE_BUILD_DIR/REST -lCivetWeb$$

    # from system
    LIBS += \
    -luser32 \
    -lgdi32  \
    -lws2_32 \
    -ldbghelp
}

macx {
   LITECORE_BUILD_DIR  = $$quote($${LITECORE_BUILD_DIR}/apple)

   CONFIG(debug, debug|release){
       LIBCBL_DIR        = $${LITECORE_BUILD_DIR}/Debug
   }
   CONFIG(release, debug|release){
       LIBCBL_DIR        = $${LITECORE_BUILD_DIR}/Release
   }

   LIBS += -framework Foundation -framework CoreServices -framework Security -framework SystemConfiguration

   LIBS += -L$$LIBCBL_DIR \
     -lLiteCore-static  \
     -lLiteCoreREST-static \
     -lSQLCipher \
     -lFleece  \
     -lTokenizer   \
     -lblip_cpp  \
     -lcivetweb
}

ios {
   LITECORE_BUILD_DIR  = $$quote($${LITECORE_BUILD_DIR}/apple)

   CONFIG(debug, debug|release){
       LIBCBL_DIR        = $${LITECORE_BUILD_DIR}/Debug-iphoneos
   }
   CONFIG(release, debug|release){
       LIBCBL_DIR        = $${LITECORE_BUILD_DIR}/Release-iphoneos
   }
#   message ("LIBCBL_DIR = $$LIBCBL_DIR")


   LIBS += -L$$LIBCBL_DIR \
     -lLiteCore-static  \
     -lLiteCoreREST-static \
     -lSQLCipher \
     -lFleece  \
     -lTokenizer   \
     -lblip_cpp  \
     -lcivetweb

}

unix:!macx:!ios {
   LITECORE_BUILD_DIR  = $$quote($${LITECORE_BUILD_DIR}/linux)

   CONFIG(debug, debug|release){
       LITECORE_BUILD_DIR        = $${LITECORE_BUILD_DIR}/debug

   }
   CONFIG(release, debug|release){
       LITECORE_BUILD_DIR        = $${LITECORE_BUILD_DIR}/release
   }

# message ("LITECORE_BUILD_DIR  unix = $$LITECORE_BUILD_DIR")

SOURCES += $$LITECORE_DIR/LiteCore/Android/arc4random.cc

# FleeceStatic
LIBS += -L$$LITECORE_BUILD_DIR/vendor/fleece  -lFleeceStatic

# LiteCore
LIBS += -L$$LITECORE_BUILD_DIR  -lLiteCoreStatic

# SQLite3_UnicodeSN
LIBS += -L$$LITECORE_BUILD_DIR/vendor/sqlite3-unicodesn  -lSQLite3_UnicodeSN

# sqlcipher
LIBS += -L$$LITECORE_BUILD_DIR/vendor/sqlcipher  -lsqlcipher
LIBS += -L$$LITECORE_BUILD_DIR/vendor/sqlcipher/vendor/mbedtls/library  -lmbedcrypto

# BLIPStatic
LIBS += -L$$LITECORE_BUILD_DIR/vendor/BLIP-Cpp  -lBLIPStatic -lz

# CivetWeb
LIBS += -L$$LITECORE_BUILD_DIR/REST -lCivetWeb

# from system
LIBS += -ldl -licuuc -licui18n

LIBS += -L/usr/lib \
     -lc++  \


}

}

HEADERS += \
    $$PWD/../src/qcbl.h

SOURCES += \
    $$PWD/../src/qcbl.cpp

INCLUDEPATH   +=$$PWD/../src

