lessThan(QT_MAJOR_VERSION, 5): error(This project requires Qt 5 or later)

QT += gui core qml quick network websockets

TEMPLATE = app
CONFIG += c++11

TARGET = QCblExplore
win32 {
CONFIG += console
}
CONFIG -= app_bundle

TEMPLATE = app

DEFINES += USE_IPV6
DEFINES += USE_WEBSOCKET

SOURCES += main.cpp \
    glb/helper.cpp \
    src/docitem.cpp \
    src/documenthandler.cpp \
    src/jsonhighlighter.cpp \
    ui/qmlgui.cpp \
    src/qurlloader.cpp \
    src/qexplore.cpp

include(logging/logging.pri)
include(quazip/quazip.pri)
include($$PWD/litecore.pri)

INCLUDEPATH   += $$PWD/quazip
INCLUDEPATH   += $$PWD/glb

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    glb/helper.h \
    src/docitem.h \
    src/documenthandler.h \
    src/jsonhighlighter.h \
    src/qexplore.h \
    src/qexplorelists.h \
    ui/qmlgui.h \
    src/qurlloader.h \
    src/httpstatus.h \
    qcbl/qcbltest.h


RESOURCES += \
    qnative.qrc

DISTFILES += \
    qml/Main.qml \
    qml/DatabasePage.qml \
    qml/DocumentsPage.qml \
    qml/QueryPage.qml \
    qml/ReplicationPage.qml \
    qml/JsonEdit.qml \
    qml/CheckBoxRect.qml \
    qml/IconSuccess.qml \
    qml/IconButton.qml \
    README.md

