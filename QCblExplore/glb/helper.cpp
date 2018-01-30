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

#include "helper.h"
#include <qcbl.h>
#include <QVariant>
#include <QFile>
#include <QTextStream>
#include <QDebug>


Helper::Helper()
{
}

void Helper::writeIndent(QTextStream& jsonOut, int indent, int level)
{
    for (int i = 0; i < indent * level; ++i)
        jsonOut <<  ' ';
}

QString Helper::formatJson(const QString& jsonIn, int indent)
{
    QString jsonData;
    QTextStream jsonOut(&jsonData);
    int level = 0;
    QString::const_iterator it = jsonIn.constBegin();

    while (it != jsonIn.constEnd())
    {
        QChar  c = *it;

        // prefix
        switch (c.toLatin1())
        {
        case '{':
        {
            if (jsonOut.pos() > 0)
                jsonOut <<  '\n';

            break;
        }

        case '[':
        {
            break;
        }

        case ']':
        {
            if (indent > 0)
                --level;

            jsonOut <<  '\n';
            writeIndent(jsonOut, indent, level);
            break;
        }

        case '}':
        {
            if (indent > 0)
                --level;

            jsonOut <<  '\n';
            writeIndent(jsonOut, indent, level);
            break;
        }

        case '"':
        {
            char prev;
            do
            {
                jsonOut <<  c;
                prev = c.toLatin1();
                c = *(++it);
            }
            while (!(it == jsonIn.constEnd() || (c.toLatin1() == '"' &&  prev != '\\')));
            break;
        }

        default:
            ;
        }

        if(it == jsonIn.constEnd())
            break;

        // infix
        jsonOut <<  c;

        // suffix
        switch (c.toLatin1())
        {
        case '{':
        {
            ++level;
            jsonOut <<  '\n';
            writeIndent(jsonOut, indent, level);
            break;
        }

        case '[':
        {
            ++level;
            jsonOut <<  '\n';
            writeIndent(jsonOut, indent, level);
            break;
        }

        case ',':
        {
            jsonOut <<  '\n';
            writeIndent(jsonOut, indent, level);
            break;
        }

        case ':':
        {
            jsonOut << ' ';
            break;
        }

        default:
            ;
        }

        it++;
    }

    jsonOut.flush();
    return jsonData;
}

void Helper::writeJsonFileStyled(const QString& fileName, const QJsonValue& jval)
{
    QSlResult res = QFleece::fromJsonValue(jval);
    FLValue val = FLValue_FromData((FLSlice) res);
    QString jsonRaw =  QSlice::qslToQString((QSlResult) FLValue_ToJSONX(val, nullptr, false, false));
    QString jsonStyled = formatJson(jsonRaw, 2);
    writeTextFile(fileName, jsonStyled);
}

void Helper::writeJsonFileStyled(const QString& fileName, const QVariantMap& qmap)
{
    QSlResult res = QFleece::fromVariant(qmap);
    FLValue val = FLValue_FromData((FLSlice) res);
    QString jsonRaw = QSlice::qslToQString((QSlResult) FLValue_ToJSONX(val, nullptr, false, false));
    QString jsonStyled = formatJson(jsonRaw, 2);
    writeTextFile(fileName, jsonStyled);
}

QVariantMap Helper::readJsonFile(const QString& fileName)
{
    QString content = readTextFile(fileName);
    if(content.isEmpty())
        return QVariantMap ();

    QSlResult res =  FLData_ConvertJSON((QSlString) content, nullptr);
    if(res.isNull() || res.isEmpty())
        return QVariantMap ();

    return  QFleece::toVariant(FLValue_FromTrustedData(res)).toMap();
}

bool Helper::removeFile(const QString &fileName)
{
    QFile f(fileName);
    if(!f.exists())
        return true;

    return f.remove();
}

QString Helper::readTextFile(const QString& fileName)
{

    QFile f(fileName);
    QString content;

    bool success = f.exists();

    if (success)
        success = f.open(QFile::ReadOnly | QFile::Text);

    if (success)
    {
        QTextStream in(&f);
        in.setCodec("UTF-8");
        content = in.readAll();
        f.close();
    }

    return content;
}

bool Helper::writeTextFile(const QString& fileName, const QString& text)
{
    QString filePath = fileName;
    QFile file(filePath);

    if (!file.open(QFile::WriteOnly |
                   QFile::Text))
    {
        return false;
    }

    QTextStream out(&file);
    out << text;
    file.flush();
    file.close();

    return true;
}

QString Helper::loadJsonFileFormatted(const QString& fileIn, bool* ok,  QString* errmsg)
{

    QFile f(fileIn);
    QString content;

    bool success = f.exists();
    if(!success && errmsg != nullptr)
    {
        *errmsg = QString("File %1 does not exist.").arg(fileIn);
    }

    if (success)
        success = f.open(QFile::ReadOnly | QFile::Text);

    if (success)
    {
        QTextStream in(&f);
        in.setCodec("UTF-8");

        QJsonParseError jerr;
        QJsonDocument jdoc = QJsonDocument::fromJson(in.readAll().toUtf8(), &jerr);
        success = jerr.error == QJsonParseError::NoError;
        if (success)
        {
            content = jdoc.toJson(QJsonDocument::Indented);
        }
        else
        {
            if(errmsg != nullptr)
            {
                *errmsg = QString("Unable to parse json in file %1, error:%2.").arg(fileIn, jerr.errorString());
            }
        }
        f.close();

    }
    else
    {
        if(errmsg != nullptr)
        {
            *errmsg = QString("Unable to open file %1.").arg(fileIn);
        }

    }
    if(ok != nullptr)
        *ok = success;
    return content;
}

// https://stackoverflow.com/questions/30958043/qt-easiest-way-to-convert-int-to-file-size-format-kb-mb-or-gb/30958189
QString Helper::convertFileSizeToHumanReadable(quint64  bytes)
{
    QString number;

    if(bytes < 0x400) //If less than 1 KB, report in B
    {
        number = QLocale::system().toString(bytes);
        number.append(" B");
        return number;
    }
    else
    {
        if(bytes >= 0x400 && bytes < 0x100000) //If less than 1 MB, report in KB, unless rounded result is 1024 KB, then report in MB
        {
            qlonglong result = (bytes + (0x400 / 2)) / 0x400;

            if(result < 0x400)
            {
                number = QLocale::system().toString(result);
                number.append(" KB");
                return number;
            }
            else
            {
                qlonglong result = (bytes + (0x100000 / 2)) / 0x100000;
                number = QLocale::system().toString(result);
                number.append(" MB");
                return number;
            }
        }
        else
        {
            if(bytes >= 0x100000 && bytes < 0x40000000) //If less than 1 GB, report in MB, unless rounded result is 1024 MB, then report in GB
            {
                qlonglong result = (bytes + (0x100000 / 2)) / 0x100000;

                if(result < 0x100000)
                {
                    number = QLocale::system().toString(result);
                    number.append(" MB");
                    return number;
                }
                else
                {
                    qlonglong result = (bytes + (0x40000000 / 2)) / 0x40000000;
                    number = QLocale::system().toString(result);
                    number.append(" GB");
                    return number;
                }
            }
            else
            {
                if(bytes >= 0x40000000 && bytes < 0x10000000000) //If less than 1 TB, report in GB, unless rounded result is 1024 GB, then report in TB
                {
                    qlonglong result = (bytes + (0x40000000 / 2)) / 0x40000000;

                    if(result < 0x40000000)
                    {
                        number = QLocale::system().toString(result);
                        number.append(" GB");
                        return number;
                    }
                    else
                    {
                        qlonglong result = (bytes + (0x10000000000 / 2)) / 0x10000000000;
                        number = QLocale::system().toString(result);
                        number.append(" TB");
                        return number;
                    }
                }
                else
                {
                    qlonglong result = (bytes + (0x10000000000 / 2)) / 0x10000000000; //If more than 1 TB, report in TB
                    number = QLocale::system().toString(result);
                    number.append(" TB");
                    return number;
                }
            }
        }
    }
}

QVariant Helper::jStringToVariant(const QString& json)
{
    QJsonParseError jerr;
    QVariant res;
    QJsonDocument doc =QJsonDocument::fromJson(json.toUtf8(), &jerr);
    if(jerr.error == QJsonParseError::NoError)
        res = doc.toVariant();

    return res;
}

QJsonArray Helper::jStringToJArray(const QString& json)
{
    QJsonParseError jerr;

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &jerr);
    if(jerr.error == QJsonParseError::NoError)
    {
        if(doc.isArray())
            return doc.array();
    }
    return QJsonArray();
}
