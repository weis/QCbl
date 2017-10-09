#ifndef QCBLTEST_H
#define QCBLTEST_H

#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QRect>
#include <QDataStream>
#include <QIODevice>
#include <QBuffer>

#include "qcbl.h"
#include "helper.h"

#include <QDebug>

static bool testJsonValue(const QString &path)
{
    QJsonObject jRoot =
    {
        {"Name", "QJsonObject"}
        , {"Id",  "K0-M123"}
        , {"uint16", 91177984}
        , {"myMap", QJsonObject{
               {"string", "helloMap"}
               , {"uint", (quint16) 1234}
               , {"float", (float) 2.456 }
               , {"int",  6789 }
               , {"myMapList",  QJsonArray({
                      "BeginMapList"
                      , (quint16) 1234
                      , (qint16) 6789
                      , QJsonObject({
                          {"rectf", "myRect1"}
                          , {"xf", 0}
                          , {"yf", -100}
                          , {"widthf", 400.19 }
                          , {"heightf", 600.84}
                      })
                      , "EndMapList"
                  })
               }
           }
        }
        , {"myList",  QJsonArray({
               "BeginList"
               , (quint16) 1234
               , (qint16) 6789
               , QJsonObject({
                   {"rect", "myRect2"}
                   , {"x", 0}
                   , {"y", -100}
                   , {"width", 400.19 }
                   , {"height", 600.84}
               })
               , "EndList"
           })
        }

    };
    Helper::writeJsonFileStyled(path + "/vRootJval.json", jRoot);
    FLError flError;
    QSlResult vSliceRoot =  QFleece::fromJsonValue(jRoot, &flError);
    QString dump = QSlice::c4ToQString((QSlStringResult) FLData_Dump(vSliceRoot));
    Helper::writeTextFile(path + "/dumpJval.txt", dump);
    bool ok = false;
    QJsonValue vCheckjVal = QFleece::toJsonValue((FLSlice) vSliceRoot, false, &ok);

    if (!ok)
    {
        qCritical("QFleece::toJsonValue failed.");
        return false;
    }

    QSlResult vSliceCheck =  QFleece::fromJsonValue(vCheckjVal, &flError);
    Helper::writeJsonFileStyled(path + "/vCheckJval.json", vCheckjVal);
    bool eq = vSliceCheck == vSliceRoot;

    if (!eq)
    {
        qCritical("Binarycheck of JsonValue failed.");
        return false;
    }

    return true;
}

static bool testVariant(const QString &path)
{
    QByteArray array1 = QByteArray::fromHex("517420697320677265617421");
    QVariantMap vRoot =
    {
        {"Name", "QVariantMap"}
        , {"Id",  "K0-M123"}
        , {"QtRect", QRectF(0., 1., 10., 20.)}
        , {"myMap", QVariantMap({
               {"string", "helloMap"}
               , {"null",  QFleece::nullVariant()}
               , {"uint", (quint16) 1234}
               , {"int", (qint16) 6789 }
               , {"int64", (qint64) 817462349691177984}
               , {"uint64", (quint64) 12817462349691177984}
               , {"double", (double) 2.123 }
               , {"float", (float) 2.456 }
               , {"arr", array1}
               , {"myMapList",  QVariantList({
                      "BeginMapList"
                      , (quint16) 1234
                      , (qint16) 6789
                      , QVariantMap({
                          {"rectf", "myRectf"}
                          , {"xf", 0}
                          , {"yf", -100}
                          , {"widthf", 400.19 }
                          , {"heightf", 600.84}
                          , {"cont", array1}
                      })
                      , "EndMapList"
                  })
               }
           })
        }
        , {"myList",  QVariantList({
               "BeginList"
               , (quint16) 1234
               , {QFleece::nullVariant()}
               , (qint16) 6789
               , QVariantMap({
                   {"rect", "myRect"}
                   , {"x", 0}
                   , {"y", -100}
                   , {"width", 400.19 }
                   , {"height", 600.84}
                   , {"cont", array1}
               })
               , "EndList"
           })
        }
    };
    Helper::writeJsonFileStyled(path + "/vRoot.json", vRoot);
    FLError flError;
    QSlResult vSliceRoot =  QFleece::fromVariant(vRoot, &flError);
    QString dump = QSlice::c4ToQString((QSlStringResult) FLData_Dump(vSliceRoot));
    Helper::writeTextFile(path + "/dump.txt", dump);
    bool ok = false;
    QVariantMap vCheck = (QFleece::toVariant((FLSlice) vSliceRoot, false, &ok)).toMap();

    if (!ok)
    {
        qCritical("QFleece::toVariant failed.");
        return false;
    }

    QSlResult vSliceCheck =  QFleece::fromVariant(vCheck, &flError);
    Helper::writeJsonFileStyled(path + "/vCheck.json", vCheck);
    bool eq = vSliceCheck == vSliceRoot;

    if (!eq)
    {
        qCritical("Binarycheck of QVariant failed.");
        return false;
    }

    return true;
}

bool checkTypeCompatibility(const QString &path)
{
    bool successVariant = testVariant(path);
    bool successJValue = testJsonValue(path);
    bool success =  successVariant && successJValue;

    if (!success)
        qFatal("checkTypeCompatibility failed, abort.");

    return success;
}

#endif // QCBLTEST_H
