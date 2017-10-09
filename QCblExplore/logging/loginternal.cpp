#include "loginternal.h"
#include <QStandardPaths>
#include <QDir>
#include <c4Base.h>
#include <QMutex>

#include "QsLog/QsLog.h"


static const QString defaultLogname("c4db.log");


void logMessageInternal(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex logMutex;

    logMutex.lock();
    Q_UNUSED(context);
    switch (type) {
    case QtInfoMsg:
        QLOG_INFO() << msg;
        break;
    case QtDebugMsg:
        QLOG_DEBUG() << msg;
        break;
    case QtWarningMsg:
        QLOG_WARN() << msg;
        break;
    case QtCriticalMsg:
        QLOG_ERROR() << msg;
        break;
    case QtFatalMsg:
        QLOG_FATAL() << msg;
        abort();
    }

    logMutex.unlock();
}

void logC4Internal(C4LogDomain domain, C4LogLevel level, const char *fmt, va_list args)
{
    static const int qtLogLevelsForC4[5] = {QtDebugMsg, QtInfoMsg,
                                       QtInfoMsg, QtWarningMsg,
                                       QtCriticalMsg};

    QString tag("LiteCore");
    QString domainName (c4log_getDomainName(domain));
    if (!domainName.isEmpty())
        tag += " [" + domainName + "]";

    QString msg = tag + ": " + QString::vasprintf(fmt, args);
    QMessageLogContext context;
    logMessageInternal((QtMsgType) qtLogLevelsForC4[level], context, msg);
}



LogInternal::LogInternal()
{
    logInit();
}


bool LogInternal::logInit()
{

    QString logDirName =  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + "/log";

    bool success = checkDirectory(logDirName);

    if (success)
    {
        QString logPath = QString("%0/%1").arg(logDirName).
                arg(defaultLogname);

        qInfo("Logging into %s", qPrintable(QDir::toNativeSeparators(logPath)));

        QsLogging::Logger& logger = QsLogging::Logger::instance();
        logger.setLoggingLevel(QsLogging::DebugLevel);

        QsLogging::DestinationPtr fileDestination(
                    QsLogging::DestinationFactory::MakeFileDestination(
                        logPath,
                        QsLogging::EnableLogRotation,
                        QsLogging::MaxSizeBytes(1024*1024),
                        QsLogging::MaxOldLogCount(10) ));


        logger.addDestination(fileDestination);
        qInstallMessageHandler(&logMessageInternal);
        c4log_writeToCallback(kC4LogDebug, &logC4Internal, false);


        qInfo () << "Logging started.";
    }

    return success;
}

bool LogInternal::checkDirectory(const QString& dir)
{
    QDir d(dir);

    bool success = d.exists();
    if (!success)
    {
        success = d.mkpath(dir);
    }

    if(!success)
        qCritical("Unable to create log directory %s", qPrintable(QDir::toNativeSeparators(dir)));

    return success;

}

