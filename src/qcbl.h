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

#ifndef QCBL_H
#define QCBL_H

#include <Fleece.hh>
#include <c4Base.h>
#include <QVariant>
#include <QJsonValue>

using namespace fleece;
///
/// \brief The QSlice class provides Qt-Type
/// interoperability to Slize / C4Slize / FLSlize structures.
///
class QSlice
{
public:
    // Standard constructors.
    Q_DECL_CONSTEXPR inline QSlice() Q_DECL_NOTHROW :
        m_data(Q_NULLPTR), m_size(0) {}
    Q_DECL_CONSTEXPR inline explicit QSlice(const char* s) Q_DECL_NOTHROW :
        m_data(s), m_size(s ? strlen(s) : 0) {}
    Q_DECL_CONSTEXPR inline explicit QSlice(const char* s, int sz) Q_DECL_NOTHROW :
        m_data(s), m_size(sz)  {}
    Q_DECL_CONSTEXPR inline explicit QSlice(const void* b, size_t sz) Q_DECL_NOTHROW :
        m_data((const char*) b), m_size((int) sz)   {}
    inline explicit QSlice(const QByteArray& b) Q_DECL_NOTHROW :
        m_data(b.constData()), m_size(b.size()) {}


    ///
    /// \brief Compatibility constructor.
    ///
    Q_DECL_CONSTEXPR inline QSlice(slice s) Q_DECL_NOTHROW :
        m_data((const char*) s.buf), m_size((int) s.size)   {}

    ///
    /// \brief Returns the number of bytes in the buffer.
    ///
    Q_DECL_CONSTEXPR inline int size() const Q_DECL_NOTHROW {
        return m_size;
    }

    ///
    /// \brief Returns a pointer to the data.
    ///
    Q_DECL_CONSTEXPR inline const char* data() const Q_DECL_NOTHROW {
        return m_data;
    }


    ///
    /// \brief slice compatibility cast.
    ///
    inline operator slice() const Q_DECL_NOTHROW {
        return  { (const void*) m_data, (size_t) m_size };
    }
    ///
    /// \brief C4Slice compatibility cast.
    ///
    inline operator C4Slice() const Q_DECL_NOTHROW {
        return  { (const void*) m_data, (size_t) m_size };
    }

    ///
    /// \brief QString compatibility cast.
    ///
    inline operator QString() const {
        return  QString::fromUtf8(data(), size()) ;
    }

    ///
    /// \brief Checks for a null slize.
    ///
    inline bool isNull() const Q_DECL_NOTHROW {
        return m_data == Q_NULLPTR;
    }

    ///
    /// \brief Checks for an empty slize.
    ///
    inline bool isEmpty() const Q_DECL_NOTHROW {
        return m_data != Q_NULLPTR && m_size == 0;
    }

    ///
    /// \brief Same as isNull.
    ///
    inline operator bool() const Q_DECL_NOTHROW {
        return m_data != Q_NULLPTR;
    }

    ///
    /// \brief Returns true if this slize is equal to other;
    /// otherwise returns false.
    ///
    inline bool operator==(const QSlice& s) const Q_DECL_NOTHROW {
        return m_size == s.m_size &&
                memcmp(m_data, s.m_data, m_size) == 0;
    }

    ///
    /// \brief Returns true if this slize is not equal to other;
    /// otherwise returns false.
    ///
    inline bool operator!=(const QSlice& s) const Q_DECL_NOTHROW {
        return !(*this == s);
    }


    ///
    /// \brief Constructs a QByteArray that uses the bytes of qslize.
    /// The bytes are not copied.
    ///
    inline static QByteArray qslToQByteArray(const QSlice& qslize) Q_DECL_NOTHROW
    {
        return QByteArray::fromRawData(qslize.data(), qslize.size());
    }

    ///
    /// \brief Constructs a QByteArray that uses the bytes of c4slize.
    /// The bytes are not copied.
    ///
    inline static QByteArray c4ToQByteArray(const slice& c4slize) Q_DECL_NOTHROW
    {
        return QByteArray::fromRawData((const char*) c4slize.buf, (int) c4slize.size);
    }

    ///
    /// \brief Constructs a slice from a QByteArray.
    ///
    inline static slice c4FromQByteArray(const QByteArray& array) Q_DECL_NOTHROW
    {
        return { (const void*) array.constData(), (size_t) array.size() };
    }


    ///
    /// \brief Constructs a QByteArray containing the copied bytes of qslize.
    ///
    inline static QByteArray qslToQByteArrayCopy(const QSlice& qslize) Q_DECL_NOTHROW
    {
        return QByteArray(qslize.data(), qslize.size());
    }

    ///
    /// \brief Constructs a QByteArray containing the copied bytes of c4slize.
    ///
    inline static QByteArray c4ToQByteArrayCopy(const slice& c4slize) Q_DECL_NOTHROW
    {
        return QByteArray((const char*) c4slize.buf, (int) c4slize.size);
    }

    ///
    /// \brief Returns a QString from the UTF-8-encoded data in qslize.
    ///
    inline static QString qslToQString(const QSlice& qslize) Q_DECL_NOTHROW
    {
        return QString::fromUtf8(qslize.data(), qslize.size());
    }

    ///
    /// \brief Returns a QString from the UTF-8-encoded data in c4slize.
    ///
    inline static QString c4ToQString(const slice& c4slize) Q_DECL_NOTHROW
    {
        return QString::fromUtf8((const char*) c4slize.buf, (int) c4slize.size);
    }


protected:
    const char* m_data;
    int         m_size;
};
// Let Qt's generic containers choose appropriate storage methods and algorithms:
Q_DECLARE_TYPEINFO(QSlice, Q_MOVABLE_TYPE);


///
/// \brief The QSlString class represents the C4String / FLString structure
/// and inherits the public methods of QSlice.
///
class QSlString: public QSlice
{
public:
    // Standard constructors.
    inline QSlString() Q_DECL_NOTHROW :
        QSlice() {}
    inline explicit QSlString(const char* s) Q_DECL_NOTHROW :
        QSlice(s) {}
    inline explicit QSlString(const char* s, int sz) Q_DECL_NOTHROW :
        QSlice(s, sz) {}
    inline explicit QSlString(const void* b, size_t sz) Q_DECL_NOTHROW :
        QSlice(b, sz)  {}
    inline explicit QSlString(const QByteArray& b) Q_DECL_NOTHROW :
        QSlice(b) {}

    ///
    /// \brief Compatibility constructor.
    ///
    inline QSlString(C4String s) Q_DECL_NOTHROW :
        QSlice(s) {}


    ///
    /// \brief Constructs a QC4String from QString str.
    /// Needs a separate databuffer to store the converted Utf8 chars
    /// for further operations.
    ///
    inline QSlString(const QString& str) Q_DECL_NOTHROW :
        m_utf8(str.toUtf8()) {
        m_data = m_utf8.constData();
        m_size = m_utf8.size();
    }


    ///
    /// \brief Constructs a QSlString from a qt string.
    ///
    inline static QSlString qslStringFromQString(const QString& str) Q_DECL_NOTHROW
    {
        return QSlString(str);
    }


private:
    // databuffer to store the converted Utf8 chars.
    const QByteArray m_utf8;
};
// Let Qt's generic containers choose appropriate storage methods and algorithms:
Q_DECLARE_TYPEINFO(QSlString, Q_MOVABLE_TYPE);

///
/// \brief The QSlResult class represents the C4SliceResult / FLSliceResult structure
/// and inherits the public methods of QSlice.
/// Frees the slize on destruction.
///
class QSlResult : public QSlice
{
public:
    // Standard constructor.
    Q_DECL_CONSTEXPR inline QSlResult() Q_DECL_NOTHROW :
        QSlice() {}

    ///
    /// \brief Compatibility constructor.
    ///
    Q_DECL_CONSTEXPR inline QSlResult(C4SliceResult slize)  Q_DECL_NOTHROW:
        QSlice(slize.buf, slize.size) {}

    //  Destructor frees the slize.
    inline ~QSlResult() {
        c4slice_free({(void*) data(), (size_t) size()});
    }
};
// Let Qt's generic containers choose appropriate storage methods and algorithms:
Q_DECLARE_TYPEINFO(QSlResult, Q_MOVABLE_TYPE);

///
/// \brief The QSlStringResult class represents the C4StringResult structure
/// and inherits the public methods of QSlice.
/// Frees the slize on destruction.
///
class QSlStringResult: public QSlice
{
public:
    // Standard constructor.
    Q_DECL_CONSTEXPR QSlStringResult() Q_DECL_NOTHROW :
        QSlice() {}
    ///
    /// \brief Compatibility constructor.
    ///
    Q_DECL_CONSTEXPR inline QSlStringResult(C4StringResult slize) Q_DECL_NOTHROW :
        QSlice(slize.buf, slize.size) {}

    ~QSlStringResult() {
        c4slice_free({(void*) data(), (size_t) size() });
    }

};
// Let Qt's generic containers choose appropriate storage methods and algorithms:
Q_DECLARE_TYPEINFO(QSlStringResult, Q_MOVABLE_TYPE);


///
/// \brief The QFleece class supplies interoperability between
/// QJsonValue and QVariant classes.
///
/// 1. The QJsonValue class encapsulates a JSON value in binary format.
/// Qt's JSON C++ API for creating and manipulating JSON data is
/// strictly limited to the JavaScript types. Therefore no 64bit
/// integers or raw data are supported.
///
/// 2. QVariant is a union wrapper for all Qt's basic types
/// with a more or less complex data checking and conversion interface.
///
class QFleece
{

public:
    ///
    /// \brief Constructs a new QVariant from a val reference.
    /// If a conversion error occurs, *ok is set to false; otherwise *ok is set to true.
    ///
    static QVariant toVariant(Doc val, bool* ok = Q_NULLPTR);

    ///
    /// \brief Constructs a new QVariant from slize.
    ///  If trusted is true, the data validation is minimal.
    ///
    static QVariant toVariant(const slice& slize, SharedKeys sk, bool trusted = true, bool* ok = Q_NULLPTR);

    ///
    /// \brief Returns a FLSliceResult from a Variant and a Encoder enc.
    ///
    static FLSliceResult fromVariant(const QVariant& val, FLEncoder enc, FLError* flError = Q_NULLPTR);

    ///
    /// \brief Returns a FLSliceResult from a Variant.
    ///
    static FLSliceResult fromVariant(const QVariant& val, FLError* flError = Q_NULLPTR);

    ///
    /// \brief Returns a "null" Variant.
    ///
    static inline QVariant nullVariant() {
        return QVariant::fromValue((void*) Q_NULLPTR) ;
    }
    ///
    /// \brief Constructs a new QJsonValue from a val reference.
    /// If a conversion error occurs, *ok is set to false; otherwise *ok is set to true.
    ///
    static QJsonValue toJsonValue(FLValue val, bool* ok = Q_NULLPTR);

    ///
    /// \brief Constructs a new QJsonValue from slize.
    ///  If trusted is true, the data validation is minimal.
    ///
    static QJsonValue toJsonValue(const FLSlice& slize, bool trusted = true, bool* ok = Q_NULLPTR);

    ///
    /// \brief Returns a FLSliceResult from a QJsonValue and a given FLEncoder.
    ///
    static FLSliceResult fromJsonValue(const QJsonValue& val, FLEncoder enc, FLError* flError = Q_NULLPTR);

    ///
    /// \brief Returns a FLSliceResult from a QJsonValue.
    ///
    static FLSliceResult fromJsonValue(const QJsonValue& val, FLError* flError = Q_NULLPTR);

    ///
    /// \brief Returns standard json from json5.
    ///
    static QString json5ToJson(const QString& json5, FLError* err = Q_NULLPTR);

    ///
    /// \brief Returns and validates a minified version of a json string.
    ///
    static QString minifyJson(const QString& json, FLError* err = Q_NULLPTR);

    ///
    /// \brief Returns the error message from error.
    ///
    static const char* flerror_getMessage(FLError error);


private:
    static void flEncodeVariant(FLEncoder enc, const QVariant& val);
    static void flEncodeJValue(FLEncoder enc, const QJsonValue& val);
    static QVariant toVariantPrivate(Value val);
    static QJsonValue toJsonValuePrivate(FLValue val);
    static void flEncodeHashToDictionary(const QVariantHash& hash, FLEncoder enc);
    static void flEncodeMapToDictionary(const QVariantMap& map, FLEncoder enc);
    static void flEncodeVariantListToArray(const QVariantList& list, FLEncoder enc);
    static void flEncodeStringListToArray(const QStringList& list, FLEncoder enc);
    static bool jvIsInteger(double dbl);
};

#endif // QCBL_H
