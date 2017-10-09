
DEFINES += QUAZIP_STATIC

win32 {

   LITECORE_DIR  = $$absolute_path($$quote($$PWD/../../src/3rdparty/couchbase-lite-core))
   LITECORE_BUILD_DIR  = $$absolute_path($$quote($$PWD/../../build/3rdparty/couchbase-lite-core/windows))
   contains(QMAKE_TARGET.arch, x86) {
      LITECORE_BUILD_DIR  = $$LITECORE_BUILD_DIR/x86
   } else {
      LITECORE_BUILD_DIR  = $$LITECORE_BUILD_DIR/x64
   }

   CONFIG(debug, debug|release){
       LITECORE_BUILD_DIR  = $$LITECORE_BUILD_DIR/debug
   }

   CONFIG(release, debug|release){
       LITECORE_BUILD_DIR  = $$LITECORE_BUILD_DIR/release
   }

   INCLUDEPATH   += $$LITECORE_DIR/vendor/BLIP-Cpp/vendor/zlib

   # zconf.h
   INCLUDEPATH   += $$LITECORE_BUILD_DIR/vendor/BLIP-Cpp/vendor/zlib


   LITECORE_BUILD_DIR  = $$quote($${LITECORE_DIR}/build)

   !contains(QMAKE_TARGET.arch, x86) {
       INCLUDEPATH   += $$LITECORE_DIR/build_cmake/x64/vendor/BLIP-Cpp/vendor/zlib
   } else {
       INCLUDEPATH   += $$LITECORE_DIR/build_cmake/x86/vendor/BLIP-Cpp/vendor/zlib
   }
}

macx {
   LIBS +=  -lz
}

unix:!macx {
   LITECORE_BUILD_DIR  = $$absolute_path($$quote($${LITECORE_DIR}/build))
}


INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
HEADERS += \
        $$PWD/crypt.h \
        $$PWD/ioapi.h \
        $$PWD/JlCompress.h \
        $$PWD/quaadler32.h \
        $$PWD/quachecksum32.h \
        $$PWD/quacrc32.h \
        $$PWD/quagzipfile.h \
        $$PWD/quaziodevice.h \
        $$PWD/quazipdir.h \
        $$PWD/quazipfile.h \
        $$PWD/quazipfileinfo.h \
        $$PWD/quazip_global.h \
        $$PWD/quazip.h \
        $$PWD/quazipnewinfo.h \
        $$PWD/unzip.h \
        $$PWD/zip.h

SOURCES += $$PWD/qioapi.cpp \
           $$PWD/JlCompress.cpp \
           $$PWD/quaadler32.cpp \
           $$PWD/quacrc32.cpp \
           $$PWD/quagzipfile.cpp \
           $$PWD/quaziodevice.cpp \
           $$PWD/quazip.cpp \
           $$PWD/quazipdir.cpp \
           $$PWD/quazipfile.cpp \
           $$PWD/quazipfileinfo.cpp \
           $$PWD/quazipnewinfo.cpp \
           $$PWD/unzip.c \
           $$PWD/zip.c
