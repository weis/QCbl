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

* Change locations of the couchbase-lite-core header and libraries in litecore.pri.
* if needed: run "git submodule update --init --recursive" to install couchbase lite core including other submodules.

# Authors

Alfred Weis (weis@dem-gmbh.de), Ludger Krämer (kraemer@dem-gmbh.de)

## Images
![qcblexp](https://user-images.githubusercontent.com/180036/30911529-420af994-a389-11e7-9ac6-092bd2bcc319.png)
