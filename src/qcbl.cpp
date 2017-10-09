/****************************************************************************
**
** The MIT License (MIT)
**
** Copyright (c) 2017 DEM GmbH, weis@dem-gmbh.de
**
** $BEGIN_LICENSE:MIT$
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
** $END_LICENSE$
**
****************************************************************************/

#include "qcbl.h"
#include "FleeceException.hh"

#include <math.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>


QVariant QFleece::toVariant(const FLSlice& slize, bool trusted,  bool* ok)
{
    FLValue root = trusted ? FLValue_FromTrustedData(slize)
                   : FLValue_FromData(slize);

    if (root != Q_NULLPTR)
        return toVariant(root, ok);

    if (ok != Q_NULLPTR)
        *ok = false;

    return QVariant();
}

QVariant QFleece::toVariant(FLValue val, bool* ok)
{
    if (ok != Q_NULLPTR)
        *ok = true;

    try
    {
        return toVariantPrivate(val);
    }
    catch (const fleece::FleeceException&) {}

    if (ok != Q_NULLPTR)
        *ok = false;

    return QVariant();
}


QVariant QFleece::toVariantPrivate(FLValue val)
{
    switch (FLValue_GetType(val))
    {
    case kFLNull:
        return QVariant::fromValue((void*) Q_NULLPTR);

    case kFLBoolean:
        return FLValue_AsBool(val);

    case kFLNumber:
    {
        if (FLValue_IsInteger(val))
        {
            if (FLValue_IsUnsigned(val))
            {
                return (quint64) FLValue_AsUnsigned(val);
            }

            return (qint64) FLValue_AsInt(val);
        }
        else if (FLValue_IsDouble(val))
        {
            return FLValue_AsDouble(val);
        }

        return FLValue_AsFloat(val);
    }

    case kFLString:
        return  QSlice::c4ToQString(FLValue_AsString(val));

    case kFLData:
    {
        return QSlice::c4ToQByteArray(FLValue_AsData(val));
    }

    case kFLArray:
    {
        FLArray array = FLValue_AsArray(val);
        FLArrayIterator iter;
        FLValue value;
        QVariantList result;

        for (FLArrayIterator_Begin(array, &iter);
             Q_NULLPTR != (value = FLArrayIterator_GetValue(&iter));
             FLArrayIterator_Next(&iter))
        {
            result.append(toVariant(value));
        }

        return result;
    }

    case kFLDict:
    {
        FLDict dict = FLValue_AsDict(val);
        FLDictIterator iter;
        FLValue value;
        FLString key;
        QVariantMap result;

        for (FLDictIterator_Begin(dict, &iter);
             Q_NULLPTR != (value = FLDictIterator_GetValue(&iter));
             FLDictIterator_Next(&iter))
        {
            key = FLDictIterator_GetKeyString(&iter);
            result[QSlice::c4ToQString(key)] = toVariant(value);
        }

        return result;
    }

    default:
        fleece::FleeceException::_throw(fleece::UnknownValue, "illegal typecode in Value; corrupt data?");
    }
}

FLSliceResult QFleece::fromVariant(const QVariant& val, FLError* flError)
{
    FLEncoder enc = FLEncoder_New();
    FLSliceResult res = fromVariant(val, enc,  flError);
    FLEncoder_Free(enc);
    return res;
}

FLSliceResult QFleece::fromVariant(const QVariant& val, FLEncoder enc,  FLError* flError)
{
    Q_ASSERT(enc);
    FLEncoder_Reset(enc);
    flEncodeVariant(enc,  val);
    return FLEncoder_Finish(enc, flError);
}

///
/// \brief QFleece::jvIsInteger
/// \param dbl
/// \return
///
bool QFleece::jvIsInteger(double dbl)
{
    // Unfortunately QJsonValue does not support 64-bit integers...
    // more at https://bugreports.qt.io/browse/QTBUG-28560
    if (floor(dbl) != dbl)
        return false;

    return dbl < 0. ? dbl >= (double) std::numeric_limits<int32_t>::min()
           : dbl <= (double) std::numeric_limits<uint32_t>::max();
}

void QFleece::flEncodeJValue(FLEncoder enc, const QJsonValue& val)
{
    switch (val.type())
    {
    case QJsonValue::Bool:
        FLEncoder_WriteBool(enc, val.toBool());
        return;

    case QJsonValue::Double:
    {
        double dbl = val.toDouble();

        if (jvIsInteger(dbl))
            if (dbl < 0)
                FLEncoder_WriteInt(enc, (int64_t) dbl);
            else
                FLEncoder_WriteUInt(enc, (uint64_t) dbl);
        else
            FLEncoder_WriteDouble(enc, dbl);

        return ;
    }

    case QJsonValue::String:
        FLEncoder_WriteString(enc, (QSlString) val.toString());
        return;

    case QJsonValue::Array:
    {
        QJsonArray arr = val.toArray();
        FLEncoder_BeginArray(enc, (uint32_t)arr.size());

        for (QJsonArray::const_iterator it = arr.constBegin(); it != arr.constEnd(); it++)
            flEncodeJValue(enc, *it);

        FLEncoder_EndArray(enc);
        return;
    }

    case QJsonValue::Object:
    {
        QJsonObject obj = val.toObject();
        FLEncoder_BeginDict(enc, (uint32_t)obj.size());

        for (QJsonObject::const_iterator it = obj.constBegin(); it != obj.constEnd(); it++)
        {
            FLEncoder_WriteKey(enc, (QSlString) it.key());
            flEncodeJValue(enc, it.value());
        }

        FLEncoder_EndDict(enc);
        return;
    }

    case QJsonValue::Null:
    case QJsonValue::Undefined:
        FLEncoder_WriteNull(enc);
        return;
    }
}

QJsonValue QFleece::toJsonValue(const FLSlice& slize, bool trusted, bool* ok)
{
    FLValue root = trusted ? FLValue_FromTrustedData(slize)
                   : FLValue_FromData(slize);

    if (root == Q_NULLPTR)
    {
        if (ok != Q_NULLPTR)
            *ok = false;

        return QJsonValue();
    }

    if (ok != Q_NULLPTR)
        *ok = true;

    try
    {
        return toJsonValuePrivate(root);
    }
    catch (const fleece::FleeceException&) {}

    if (ok != Q_NULLPTR)
        *ok = false;

    return QJsonValue();
}

FLSliceResult QFleece::fromJsonValue(const QJsonValue& val, FLEncoder enc, FLError* flError)
{
    Q_ASSERT(enc);
    FLEncoder_Reset(enc);
    flEncodeJValue(enc,  val);
    return FLEncoder_Finish(enc, flError);
}

FLSliceResult QFleece::fromJsonValue(const QJsonValue& val, FLError* flError)
{
    FLEncoder enc = FLEncoder_New();
    FLSliceResult res = fromJsonValue(val, enc,  flError);
    FLEncoder_Free(enc);
    return res;
}

// IMPORTANT:: No support of 64-bit integers and raw data!!
QJsonValue QFleece::toJsonValuePrivate(FLValue val)
{
    switch (FLValue_GetType(val))
    {
    case kFLNull:
        return QJsonValue(QJsonValue::Null);

    case kFLBoolean:
        return QJsonValue(FLValue_AsBool(val));

    case kFLNumber:
        return QJsonValue(FLValue_AsDouble(val));

    case kFLString:
        return  QJsonValue(QSlice::c4ToQString(FLValue_AsString(val)));

    case kFLData:
    {
        return QJsonValue(QJsonValue::Null);
    }

    case kFLArray:
    {
        FLArray array = FLValue_AsArray(val);
        FLArrayIterator iter;
        FLValue value;
        QJsonArray result;

        for (FLArrayIterator_Begin(array, &iter);
             Q_NULLPTR != (value = FLArrayIterator_GetValue(&iter));
             FLArrayIterator_Next(&iter))
        {
            result.append(toJsonValuePrivate(value));
        }

        return result;
    }

    case kFLDict:
    {
        FLDict dict = FLValue_AsDict(val);
        FLDictIterator iter;
        FLValue value;
        FLString key;
        QJsonObject result;

        for (FLDictIterator_Begin(dict, &iter);
             Q_NULLPTR != (value = FLDictIterator_GetValue(&iter));
             FLDictIterator_Next(&iter))
        {
            key = FLDictIterator_GetKeyString(&iter);
            result[QSlice::c4ToQString(key)] = toJsonValuePrivate(value);
        }

        return result;
    }

    default:
        fleece::FleeceException::_throw(fleece::UnknownValue, "illegal typecode in Value; corrupt data?");
    }
}

void QFleece::flEncodeVariant(FLEncoder enc, const QVariant& val)
{
    switch (val.type())
    {
    case QVariant::Hash:
        flEncodeHashToDictionary(val.toHash(), enc);
        return;

    case QVariant::Map:
        flEncodeMapToDictionary(val.toMap(), enc);
        return;

    case QVariant::List:
        flEncodeVariantListToArray(val.toList(), enc);
        return;

    case QVariant::StringList:
        flEncodeStringListToArray(val.toStringList(), enc);
        return;

    case QVariant::String:
    case QVariant::Char:
        FLEncoder_WriteString(enc, (QSlString) val.toString());
        return;

    case QVariant::ByteArray:
        FLEncoder_WriteData(enc, QSlice(val.toByteArray()));
        return;

    case QVariant::Bool:
        FLEncoder_WriteBool(enc, val.toBool());
        return;

    case QVariant::Int:
    case QVariant::LongLong:
        FLEncoder_WriteInt(enc, (int64_t)val.toLongLong());
        return;

    case QVariant::UInt:
    case QVariant::ULongLong:
        FLEncoder_WriteUInt(enc, (uint64_t)val.toULongLong());
        return;

    case QMetaType::Double:
        FLEncoder_WriteDouble(enc, val.toDouble());
        return;

    case QMetaType::Float:
        FLEncoder_WriteFloat(enc, val.toDouble());
        return;

    // our json null representation
    case QMetaType::VoidStar:
        if (val.canConvert<void*>() && val.value<void*>() == Q_NULLPTR)
        {
            FLEncoder_WriteNull(enc);
            return;
        }

    default:
    {
        FLEncoder_WriteNull(enc);
        qWarning("flEncodeVariant: unable to encode type %s (%d).",
                 qPrintable(QMetaType::typeName(val.type())), val.type());
    }
    }
}

void QFleece::flEncodeHashToDictionary(const QVariantHash& hash, FLEncoder enc)
{
    FLEncoder_BeginDict(enc, (uint32_t)hash.size());

    for (QVariantHash::const_iterator it = hash.constBegin(); it != hash.constEnd(); ++it)
    {
        FLEncoder_WriteKey(enc, (QSlString) it.key());
        flEncodeVariant(enc, it.value());
    }

    FLEncoder_EndDict(enc);
}

void QFleece::flEncodeMapToDictionary(const QVariantMap& map, FLEncoder enc)
{
    FLEncoder_BeginDict(enc, (uint32_t)map.size());

    for (QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
    {
        FLEncoder_WriteKey(enc, (QSlString) it.key());
        flEncodeVariant(enc, it.value());
    }

    FLEncoder_EndDict(enc);
}

void QFleece::flEncodeVariantListToArray(const QVariantList& list, FLEncoder enc)
{
    FLEncoder_BeginArray(enc, (uint32_t)list.size());

    for (QVariantList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
        flEncodeVariant(enc, *it);

    FLEncoder_EndArray(enc);
}

void QFleece::flEncodeStringListToArray(const QStringList& list, FLEncoder enc)
{
    FLEncoder_BeginArray(enc, (uint32_t)list.size());

    for (QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
        flEncodeVariant(enc, *it);

    FLEncoder_EndArray(enc);
}

QString QFleece::json5ToJson(const QString& json5, FLError* err)
{
    if (err)
        *err = kFLNoError;

    QSlResult json = FLJSON5_ToJSON((QSlString) json5 , err);

    // Attention: err not set.
    if (json.isNull() && err)
    {
        *err = kFLInvalidData;
    }

    if (err && *err != kFLNoError)
        return QString();

    return QSlice::qslToQString(json);
}

QString QFleece::minifyJson(const QString& json, FLError* err)
{
    if (err)
        *err = kFLNoError;

    // Don't sort keys
    FLEncoder enc = FLEncoder_NewWithOptions(kFLEncodeFleece, 0, true, false);
    FLEncoder_ConvertJSON(enc, QSlString(json));
    QSlResult res = FLEncoder_Finish(enc, err);
    FLEncoder_Free(enc);

    if (err && *err != kFLNoError)
        return QString();

    FLValue root = FLValue_FromTrustedData(res);
    return  QSlice::qslToQString((QSlResult) FLValue_ToJSON(root));
}

const char* QFleece::flerror_getMessage(FLError error)
{
    static const char* flErrorString[] =
    {
        "Out of memory, or allocation failed",
        "Array index or iterator out of range",
        "Bad input data",
        "Structural error encoding",
        "Error parsing JSON",
        "Unparseable data in a Value",
        "Internal error",
        "Key not found",
        "Misuse of shared keys",
    };
    return  flErrorString[error];
}
