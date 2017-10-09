#ifndef HELPER_H
#define HELPER_H

#include <QString>
#include <QVariant>
#include <QTextStream>
#include <QJsonDocument>

class Helper
{
public:
    Helper();
    // Writes qmap as a formatted json string into fileName
    static QString formatJson(const QString& jsonIn, int writeIndent = 2);
    static void writeJsonFileStyled(const QString& fileName, const QVariantMap &qmap );
    static void writeJsonFileStyled(const QString& fileName, const QJsonValue& jval);
    static QVariantMap readJsonFile(const QString& fileName);
    // Writes text into fileName
    static bool writeTextFile(const QString& fileName, const QString &text );
    static  QString readTextFile(const QString& fileName);
    static bool removeFile(const QString& fileName);
    static QString loadJsonFileFormatted(const QString& fileIn, bool *ok = nullptr, QString *errmsg = nullptr);
    static QString convertFileSizeToHumanReadable(quint64 bytes);

private:
    static void writeIndent(QTextStream &jsonOut, int writeIndent, int level);
};

#endif // HELPER_H
