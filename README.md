# Qt and couchbase-lite-core: Type interoperability and experimental usage


# Features

* Implementation of Qt-Type (including QJsonValue and QVariant) interoperability for Slize / C4Slize / FLSlize / FLValue structures.

* Import json-files from web or local.

* Editing json-documents.

* Querying (sqlite) data from Qml including full-text search.

* Rudimentary replication support.

* Extensive logging.

# Platform Support

Tested on macOS, iOS, Linux (Ubuntu, CentOS) and Windows.

# Installation

* If couchbase-lite-core not installed:
  - run "git submodule update --init --recursive" to install couchbase lite core including other submodules
    into src/3rdparty/couchbase-lite-core.
  - Windows, linux: Open CMakeLists.txt from couchbase-lite-core root directory in qt-creator (ver. 4.4 or later) and build the project
    with the preferrred kit.
  - macOS, iOS: Open and build Xcode/LiteCore.xcodeproj


* Change locations of the couchbase-lite-core header and libraries in litecore.pri.

* Open QCblExplore.pro in Qt Creator.
- Windows, linux, macOS, iOS : build and run the project with the preferrred kit.
- macOS optionally: running qmake in a terminal generates QCblExplore.xcodeproj:
  qmake -spec macx-xcode
- iOS optionally: simple run qmake in Qt Creator with the iOS-kit selected.
  In Xcode open and run {build}/QCblExplore.xcodeproj.



# Authors

Alfred Weis (weis@dem-gmbh.de), Ludger Krämer (kraemer@dem-gmbh.de)

## Images
![qcblexp](https://user-images.githubusercontent.com/180036/30911529-420af994-a389-11e7-9ac6-092bd2bcc319.png)
