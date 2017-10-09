# Qt and couchbase-lite-core: Type interoperability and experimental usage


# Features

* Impementation of Qt-Type (including QJsonValue and QVariant) interoperability to Slize / C4Slize / FLSlize structures.

* Import json-files from web or local.

* Editing json-documents.

* Querying (sqlite) data from Qml including full-text search.

* Rudimentary replication support.

* Extensive logging.

# Platform Support

Runs currently on Mac OS, iOS, Linux (Ubuntu, CentOS) and Windows.

# Installation

* if needed: run "git submodule update --init --recursive" to install couchbase lite core including other submodules
into src/3rdparty/couchbase-lite-core.
Open CMakeLists.txt from couchbase-lite-core root directory in qt-creator (ver. 4.4 or later) and build the project
with the preferrred kit.

* Change locations of the couchbase-lite-core header and libraries in litecore.pri.



# Authors

Alfred Weis (weis@dem-gmbh.de), Ludger Krämer (kraemer@dem-gmbh.de)

## Images
![qcblexp](https://user-images.githubusercontent.com/180036/30911529-420af994-a389-11e7-9ac6-092bd2bcc319.png)
