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

#include "qexplore.h"
#include "helper.h"

#include <c4BlobStore.h>
#include <c4Document+Fleece.h>
#include <c4.hh>

#define MAX_QUERY_ARGS 3

#include <JlCompress.h>
#include <QTimer>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QUrl>
#include <QDateTime>
#include <QDebug>
#include "qcbl/qcbltest.h"

C4Slice C4RunQuery::highLightMarkBefore = C4STR("<span style=\"background-color: #FFFF00\">");
C4Slice C4RunQuery::highLightMarkAfter = C4STR("</span>");

static const QLatin1String defaultC4DbName("demodb");
static const QLatin1String logDirName("log");
static const QLatin1String typeTestDirName("tcheck");
static const QLatin1String defaultRepUrl ("ws://192.168.1.30:4984/demodb");


// This is our default configuration
static C4DatabaseConfig defaultC4DbConfig =  { kC4DB_Bundled | kC4DB_SharedKeys | kC4DB_Create,
                                               kC4SQLiteStorageEngine, kC4RevisionTrees,
                                               { kC4EncryptionNone, {0}}
                                             };

// This is our import configuration:
// c4DatabaseObserver is disabled.
static C4DatabaseConfig importC4DbConfig =  { kC4DB_NonObservable | kC4DB_Bundled | kC4DB_SharedKeys | kC4DB_Create,
                                              kC4SQLiteStorageEngine, kC4RevisionTrees,
                                              { kC4EncryptionNone, {0}}
                                            };

QExplore::QExplore(QObject* parent)
    : QObject(parent)
    , m_c4Database(nullptr)
    , m_c4Import(nullptr)
    , m_c4Query(nullptr)
    , m_c4IndexCreator(nullptr)
    , m_replicator(nullptr)
    , m_newQueryRequest(false)
    , m_docsCount(-1)
    , m_repContinuous(false)
    , m_repProgress(0.0)
    , m_impProgress(0.0)
    , m_loadProgress(0.0)
    , m_dbname(defaultC4DbName)
    , m_databasesModel(new QStringListModel(this))
    , m_importFileModel(new QStringListModel(this))
    , m_queryItemsModel(new QStringListModel(this))
    , m_queryVarsModel(new QStringListModel(this))
    , m_indexItemsModel_v(new QStringListModel(this))
    , m_indexItemsModel_t(new QStringListModel(this))
    , m_error(false)
    , m_repStatus("Stopped")
    , m_repBusy(false)
    , m_statusFlag(Qt::black)
    , m_webLoader(nullptr)
    , m_webLoadSuccess(true)
{
    QString typeTestDirName =  configDir() + "/tcheck";

    if (!(checkDirectory(typeTestDirName) && checkTypeCompatibility(typeTestDirName)))
        return;

    m_repUrl = defaultRepUrl;
    m_queryArgs.resize(MAX_QUERY_ARGS);
    m_queryText.resize(MAX_QUERY_ARGS);
    m_queryI.resize(MAX_QUERY_ARGS);

    for (int i = 0; i < MAX_QUERY_ARGS; i++)
        m_queryI[i] = false;

    readConfiguration();
    getDatabases();
    getIndices_v();
    getIndices_t();
    getQueries();
    getImports();
}

QExplore::~QExplore()
{
    c4repl_free(m_replicator);
}

void QExplore::readConfiguration()
{
    QVariantMap conf = Helper::readJsonFile(configFile());
    if(conf.isEmpty())
    {
        m_dbname = defaultC4DbName;
        m_repUrl = defaultRepUrl;
        return;
    }

    m_repUrl = conf["last_repurl"].toString();
    if(m_repUrl.isEmpty())
    {
        m_repUrl = defaultRepUrl;
        return;
    }
    m_dbname = conf["last_dbname"].toString();
    if(m_dbname.isEmpty())
    {
        m_dbname = defaultC4DbName;
        return;
    }

    QString queryName = conf["last_query"].toString();
    if(!queryName.isEmpty())
    {
        QFileInfo fi(queryFile(queryName));
        if(fi.exists())
            m_queryName = queryName;
    }
}

void QExplore::writeConfiguration()
{
    QVariantMap conf;
    conf.insert("last_dbname", m_dbname);
    if(!m_queryName.isEmpty())
        conf.insert("last_query", m_queryName);
    if(!m_repUrl.isEmpty())
        conf.insert("last_repurl", m_repUrl);
    Helper::writeJsonFileStyled(configFile(), conf);
}



QQmlListProperty<DocItem> QExplore::searchItems()
{
    return QQmlListProperty<DocItem>(this, 0,
                                     &QExplore::searchItemAppend,
                                     &QExplore::searchItemCount,
                                     &QExplore::searchItemAt,
                                     &QExplore::searchItemClear
                                     );
}

qreal QExplore::repProgress() const
{
    return m_repProgress;
}

void QExplore::setRepProgress(qreal repProgress)
{
    if (m_repProgress == repProgress)
        return;

    m_repProgress = repProgress;
    emit repProgressChanged();
}


const QString& QExplore::repStatus() const
{
    return m_repStatus;
}

void QExplore::setRepStatus(const QString& repInfo)
{
    if (m_repStatus == repInfo)
        return;

    m_repStatus = repInfo;
    emit repStatusChanged();
}

qreal QExplore::impProgress() const
{
    return m_impProgress;
}

void QExplore::setImpProgress(qreal impProgress)
{
    if (m_impProgress == impProgress)
        return;

    m_impProgress = impProgress;
    emit impProgressChanged();
}


qreal QExplore::loadProgress() const
{
    return m_loadProgress;
}

void QExplore::setLoadProgress(qreal loadProgress)
{
    if (m_loadProgress == loadProgress)
        return;

    m_loadProgress = loadProgress;
    emit loadProgressChanged();
}



const QString& QExplore::importPath() const
{
    return m_importPath;
}

void QExplore::setImportPath(const QString& importPath)
{
    if (m_importPath == importPath)
        return;

    m_importPath = importPath;
    emit importNameChanged();
    emit importPathChanged();
}


QString QExplore::importPathToNative(const QString &importPath)
{
    QUrl url(importPath);

    if (!url.isLocalFile())
        return QString();

    QString localPath = url.toLocalFile();
    return QDir::toNativeSeparators(localPath);
}

QString QExplore::importName() const
{
    QFileInfo fi(m_importPath);
    return fi.fileName();
}

bool QExplore::repBusy() const
{
    return m_repBusy;
}

void QExplore::setRepBusy(bool repBusy)
{
    if (m_repBusy == repBusy)
        return;

    m_repBusy = repBusy;
    emit repBusyChanged();
}

const QColor& QExplore::statusFlag() const
{
    return m_statusFlag;
}

void QExplore::setStatusFlag(const QColor& statusFlag)
{
    if (m_statusFlag == statusFlag)
        return;

    m_statusFlag = statusFlag;
    emit statusFlagChanged();
}

int QExplore::docsCount()
{
    return m_docsCount;
}

void QExplore::setDocsCount(int count)
{
    if (m_docsCount == count)
        return;

    m_docsCount = count;
    emit docsCountChanged();
}


const QString& QExplore::repUrl() const
{
    return m_repUrl;
}

void QExplore::setRepUrl(const QString& repUrl)
{
    if (m_repUrl == repUrl)
        return;

    m_repUrl = repUrl;
    writeConfiguration();
    emit repUrlChanged();
}

void QExplore::stopReplication()
{
    if (m_replicator == nullptr)
        return;

    c4repl_stop(m_replicator);
    //    c4repl_free(m_replicator);
    //    m_replicator = nullptr;
    setStatusFlag(Qt::black);
    setRepBusy(false);
    //   displayMessage("Replication stopped.");
}


void QExplore::replicatorStatusChanged(C4ReplicatorStatus status)
{
    if (status.error.code > 0)
    {
        setRepStatus("Error");
        setStatusFlag(Qt::red);
        setRepBusy(false);
        stopReplication();
        displayMessage(QExplore::logC4Error("Replication error occured, aborted", status.error));
        m_error = true;
        return;
    }

    QString info("Replication status:");

    switch (status.level)
    {
    case kC4Stopped:
    {
        repProgressInfo(status.progress.unitsCompleted, status.progress.unitsTotal);
        stopReplication();
        setStatusFlag(Qt::black);
        setRepBusy(false);
        setRepStatus("Stopped");
        break;
    }

    case kC4Offline:
    {
        repProgressInfo(status.progress.unitsCompleted, status.progress.unitsTotal);
        setStatusFlag(Qt::blue);
        setRepStatus("Offline");
        setRepBusy(false);
        break;
    }

    case kC4Connecting:
    {
        repProgressInfo(status.progress.unitsCompleted, status.progress.unitsTotal);
        setStatusFlag(Qt::darkGreen);
        setRepStatus("Connecting");
        setRepBusy(false);
        break;
    }

    case kC4Idle:
    {
        repProgressInfo(status.progress.unitsCompleted, status.progress.unitsTotal);
        setStatusFlag(Qt::green);
        setRepStatus("Idle");
        setRepBusy(false);
        break;
    }

    case kC4Busy:
    {
        repProgressInfo(status.progress.unitsCompleted, status.progress.unitsTotal);
        setStatusFlag(Qt::yellow);
        setRepStatus("Busy");
        setRepBusy(false);
        break;
    }

    default:
        setRepStatus("Unknown");
        break;
    }

    info += " " + m_repStatus + ".";
    qInfo("%s", qPrintable(info));
    displayMessage(info);
}

void QExplore::repProgressInfo(qulonglong progress, qulonglong total)
{
    QString info;
    qreal progressPercent = 0.0;

    if (total > 0l)
    {
        progressPercent = (qreal)(progress * 100) / (qreal) total ;
        info =  QString("%0 of %1 documents processed, %2%3 done.")
                .arg(progress)
                .arg(total)
                .arg(progressPercent, 0, 'f', 4)
                .arg('%');
    }

    setRepProgress(progressPercent);

    if (!info.isEmpty())
        qInfo("%s", qPrintable(info));
}


bool QExplore::startReplication(bool continuous)
{
    m_error = false;

    if (!m_replicator)
    {
        QSlString remoteDbName (m_dbname);
        QSlString repUrl = (QSlString)m_repUrl;

        C4String  c4RemoteDbName = remoteDbName;
        C4Address remoteAddress;
        if (!c4repl_parseURL(repUrl, &remoteAddress, &c4RemoteDbName))
        {
            QString errStr = QString("Unable to read remote url %1.").arg(m_repUrl);
            qWarning("%s", qPrintable(errStr));
            displayMessage(errStr);
            return false;
        }

        m_repContinuous = continuous;
        C4ReplicatorParameters params {};
        C4ReplicatorMode mode =  m_repContinuous ? kC4Continuous : kC4OneShot;
        params.pull = params.push = mode;
        params.callbackContext = (void*) this;
        params.onStatusChanged = [](C4Replicator*, C4ReplicatorStatus status, void *context) {
            ((QExplore*)context)->replicatorStatusChanged(status);
        };
        C4Error error;
        m_replicator = c4repl_new(m_c4Database, remoteAddress, remoteDbName, nullptr, params, &error);

        if (m_replicator == nullptr)
        {
            displayMessage(QExplore::logC4Error(QString("Unable to create Replicator for \"%1\"").arg(m_dbname), error));
            m_error = true;
            return false;
        }

        displayMessage("Replication started.");
    }

    return true;
}

void QExplore::clearSearchResult()
{
    clearSearchItems();
    emit queryCountChanged();
}


void QExplore::clearSearchItems(bool emitqml)
{
    for (QList<DocItem*>::ConstIterator  it = m_searchitems.constBegin();
         it != m_searchitems.constEnd();
         it++)
    {
        DocItem* o = *it;
        o->deleteLater();
    }

    m_searchitems.clear();

    if (emitqml)
        emit searchItemsChanged();
}

void QExplore::searchItemAppend(QQmlListProperty<DocItem>* l,
                                DocItem* o)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);

    if (qexplore)
    {
        qexplore->m_searchitems.append(o);
        emit qexplore->searchItemsChanged();
    }
}

void QExplore::searchItemClear(QQmlListProperty<DocItem>* l)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);

    if (qexplore && qexplore->m_searchitems.count() > 0)
    {
        qexplore->clearSearchItems();
    }
}

int QExplore::searchItemCount(QQmlListProperty<DocItem>* l)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    return qexplore->m_searchitems.count();
}

DocItem* QExplore::searchItemAt(QQmlListProperty<DocItem>* l, int index)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    // qDebug() << qexplore->m_searchitems.at(index)->text();
    return qexplore->m_searchitems.at(index);
}


QQmlListProperty<DocItem> QExplore::docItems()
{
    return QQmlListProperty<DocItem>(this, 0,
                                     &QExplore::itemAppend,
                                     &QExplore::itemCount,
                                     &QExplore::itemAt,
                                     &QExplore::itemClear
                                     );
}

QStringListModel*  QExplore::queries()
{
    return m_queryItemsModel;
}


QStringListModel*  QExplore::databases()
{
    return m_databasesModel;
}


QStringListModel*  QExplore::importfiles()
{
    return m_importFileModel;
}

QString QExplore::indexContent_v(const QString& name)
{
    if (!m_indexItems_v.contains(name))
        return QString();

    return Helper::readTextFile(indexFile_v(name));
}

QString QExplore::indexContent_t(const QString& name)
{
    if (!m_indexItems_t.contains(name))
        return QString();

    return Helper::readTextFile(indexFile_t(name));
}


int QExplore::hasQueries()
{
    return m_queryItems.size() > 0;
}

int QExplore::hasIndices_v()
{
    return m_indexItems_v.size() > 0;
}

int QExplore::hasIndices_t()
{
    return m_indexItems_t.size() > 0;
}


QString QExplore::queryName() const
{
    return m_queryName;
}

QString QExplore::queryContent(const QString& name)
{
    if (!m_queryItems.contains(name))
        return QString();

    QString dirName = queryDir();
    QString fName = dirName + "/" + name + ".json";
    QString content = Helper::readTextFile(fName);
    return Helper::formatJson(content);
}

void QExplore::queryVars(const QString& name)
{
    if (name.isEmpty())
        return;

    if(m_queryName != name)
    {
        m_queryName = name;
        writeConfiguration();
    }

    getQueryVars(queryContent(name));
}


bool QExplore::saveIndexContent_v(const QString& name, const QString& content, bool json5)
{
    FLError err = kFLNoError;
    QString json = json5
            ? QFleece::json5ToJson(content, &err)
            : QFleece::minifyJson(content, &err);

    if (err != kFLNoError)
    {
        logFlError(QString("Unable to save index %1").arg(name), err);
        return false;
    }

    QString fName = indexFile_v(name);

    if (!Helper::writeTextFile(fName, json))
    {
        QString msg = QString("Unable to write %1.").arg(fName);
        qWarning("%s", qPrintable(msg));
        displayMessage(msg);
        return false;
    }

    createIndexName(name, false);

    return true;
}

void QExplore::createQueryName(const QString& name)
{
    if (!m_queryItems.contains(name))
    {
        m_queryItems.append(name);
        m_queryItemsModel->setStringList(m_queryItems);
        emit queriesChanged();
    }

}

void QExplore::createIndexName(const QString& name, bool textIndex)
{
    if(textIndex)
    {
        if (!m_indexItems_t.contains(name))
        {
            m_indexItems_t.append(name);
            m_indexItemsModel_t->setStringList(m_indexItems_t);
            emit indicesChanged_t();
        }
        return;
    }

    if (!m_indexItems_v.contains(name))
    {
        m_indexItems_v.append(name);
        m_indexItemsModel_v->setStringList(m_indexItems_v);
        emit indicesChanged_v();
    }
}

bool QExplore::saveIndexContent_t(const QString& name, const QString& content, bool json5)
{
    FLError err = kFLNoError;
    QString json = json5
            ? QFleece::json5ToJson(content, &err)
            : QFleece::minifyJson(content, &err);

    if (err != kFLNoError)
    {
        logFlError(QString("Unable to save index %1").arg(name), err);
        return false;
    }

    QString fName = indexFile_t(name);


    if (!Helper::writeTextFile(fName, json))
    {
        QString msg = QString("Unable to write %1.").arg(fName);
        qWarning("%s", qPrintable(msg));
        displayMessage(msg);
        return false;
    }

    createIndexName(name, true);

    return true;
}

bool QExplore::createDatabase(const QString& name)
{
    if(name.isEmpty())
        return false;

    if(m_databasesItems.contains(name, Qt::CaseInsensitive))
    {
        QString errStr = QString("Database \"%1\" exists.").arg(name);
        qWarning("%s", qPrintable(errStr));
        displayMessage(errStr);
        return false;
    }
    QString path = configDir() + "/" + name;
    QDir newDbDir;
    if(!newDbDir.mkpath(path))
    {
        QString errStr = QString("Unable to create directory \"%1\".").arg(path);
        qWarning("%s", qPrintable(errStr));
        displayMessage(errStr);
        return false;
    }
    m_databasesItems.append(name);
    m_databasesModel->setStringList(m_databasesItems);
    emit databasesChanged();

    displayMessage(QString("Directory \"%1\" created.").arg(path));
    return true;
}

bool QExplore::selectDatabase()
{
    if(m_c4Database != nullptr)
    {
        c4db_free(m_c4Database);
        m_c4Database = nullptr;
    }
    setDbname(m_dbname);
    getIndices_v();
    getIndices_t();
    getQueries();

    //    if(hasQueries())
    //    {
    //        m_queryName = m_queryItems[0];
    //        queryVars(m_queryName);
    //    }
    //    else
    //    {
    //        m_queryName = QString();

    //    }

    return openC4Database();
}

bool QExplore::saveQueryContent(const QString& name, const QString& content, bool json5)
{
    FLError err = kFLNoError;
    QString json = json5
            ? QFleece::json5ToJson(content, &err)
            : QFleece::minifyJson(content, &err);

    if (err != kFLNoError)
    {
        displayMessage(logFlError(QString("Unable to save query %1").arg(name), err));
        return false;
    }

    QString queryDirName =  queryDir();
    QString fName = queryDirName + "/" + name + ".json";

    if (!Helper::writeTextFile(fName, json))
    {
        QString msg = QString("Unable to write %1.").arg(fName);
        qWarning("%s", qPrintable(msg));
        displayMessage(msg);
        return false;
    }

    m_queryCache[name] = json;

    createQueryName(name);

    return true;
}

bool QExplore::deleteIndexContent_v(const QString& name)
{
    QString fName = indexDir_v() + "/" + name + ".json";

    if (!Helper::removeFile(fName))
    {
        QString msg = QString("Unable to delete index file %1.").arg(fName);
        qWarning("%s", qPrintable(msg));
        displayMessage(msg);
        return false;
    }

    m_indexItems_v.removeOne(name);
    m_indexItemsModel_v->setStringList(m_indexItems_v);
    emit  indicesChanged_v();
    return true;
}

bool QExplore::deleteIndexContent_t(const QString& name)
{
    QString fName = indexDir_t() + "/" + name + ".json";

    if (!Helper::removeFile(fName))
    {
        QString msg = QString("Unable to delete index file %1.").arg(fName);
        qWarning("%s", qPrintable(msg));
        displayMessage(msg);
        return false;
    }

    m_indexItems_t.removeOne(name);
    m_indexItemsModel_t->setStringList(m_indexItems_t);
    emit  indicesChanged_t();
    return true;
}

bool QExplore::deleteQueryContent(const QString& name)
{
    QString fName = queryDir() + "/" + name + ".json";

    if (!Helper::removeFile(fName))
    {
        QString msg = QString("Unable to delete query file %1.").arg(fName);
        qWarning("%s", qPrintable(msg));
        displayMessage(msg);
        return false;
    }

    m_queryItems.removeOne(name);
    m_queryItemsModel->setStringList(m_queryItems);
    emit queriesChanged();
    return true;
}


QString QExplore::defaultValue(const QString& var)
{
    QVector<QStringRef> split = var.splitRef('$', QString::SkipEmptyParts);
    return split.size() < 2
            ? QString()
            : split[1].toString();
}

QString QExplore::varName(const QString& var)
{
    return var.isEmpty()
            ? QString()
            : var.split('%', QString::SkipEmptyParts)[0];
}

char QExplore::varType(const QString& var)
{
    QStringList sl = var.split('%', QString::SkipEmptyParts);
    return sl.size() > 1
            ? (sl[1].at(0).toLower().toLatin1())
            : 's';
    }

    bool QExplore:: getQueryVars(const QString& content)
    {
    if (content.isEmpty())
    return true;

    int from = 0;
    int varIndex = 0;
    QVector<QString> queryArgs;
    queryArgs.resize(MAX_QUERY_ARGS);

    while (varIndex < MAX_QUERY_ARGS)
    {
        int start = content.indexOf((QLatin1String) "<<", from);

        if (start < 0)
            break;

        start += 2;
        int end = content.indexOf((QLatin1String) ">>", start);

        if (end < 0 || end <= start)
            return false;

        QString varname = content.mid(start, end - start);

        if (!queryArgs.contains(varname))
        {
            queryArgs[varIndex++] = varname;
        }

        from = end;
    } ;

    bool changed = false;

    for (int i = 0; i < queryArgs.size(); i++)
    {
        if (queryArgs[i] != m_queryArgs[i])
        {
            changed = true;
            m_queryArgs[i] = queryArgs[i];

            switch (i)
            {
            case 0:
            {
                emit hasQueryVar1Changed();
                emit queryVar1Changed();
                emit queryDefault1Changed();
                break;
            }

            case 1:
            {
                emit hasQueryVar2Changed();
                emit queryVar2Changed();
                emit queryDefault2Changed();
                break;
            }

            case 2:
            {
                emit hasQueryVar3Changed();
                emit queryVar3Changed();
                emit queryDefault3Changed();
                break;
            }

            default:
                break;
            }
        }
    }

    if (changed)
    {
        m_queryVarsModel->setStringList(queryArgs.toList());
        emit queryvarsChanged();
    }

    return true;
}


QStringListModel*  QExplore::indices_v()
{
    return m_indexItemsModel_v;
}

QStringListModel*  QExplore::indices_t()
{
    return m_indexItemsModel_t;
}


QStringListModel*  QExplore::queryvars()
{
    return m_queryVarsModel;
}


bool QExplore::hasQueryVar1()
{
    return !m_queryArgs[0].isEmpty();
}

QString QExplore::queryVar1()
{
    return  varName(m_queryArgs[0]);
}

bool QExplore::hasQueryVar2()
{
    return !m_queryArgs[1].isEmpty();
}

QString QExplore::queryVar2()
{
    return varName(m_queryArgs[1]);
}

bool QExplore::hasQueryVar3()
{
    return !m_queryArgs[2].isEmpty();
}

QString QExplore::queryVar3()
{
    return varName(m_queryArgs[2]);
}

QString QExplore::queryText1()
{
    return m_queryText[0];
}

void QExplore::setQueryText1(const QString& text)
{
    if (m_queryText[0] == text)
        return;

    m_queryText[0] = text;

    if (m_queryI[0])
    {
        runQueryInteractive(text);
    }

    emit queryText1Changed();
}

void QExplore::runQueryInteractive(const QString& queryText)
{
    if (m_c4Query == nullptr)
    {
        runQuery(m_queryName);
        return;
    }

    if (m_c4Query->isRunning())
    {
        m_newQueryRequest = true;
        return;
    }

    delete m_c4Query;
    m_c4Query = nullptr;

    if (queryText.isEmpty() || queryText == "%")
    {
        clearSearchItems(true);
        return;
    }

    runQuery(m_queryName) ;
}


QString QExplore::queryText2()
{
    return m_queryText[1];
}

void QExplore::setQueryText2(const QString& text)
{
    if (m_queryText[1] == text)
        return;

    m_queryText[1] = text;

    if (m_queryI[1])
    {
        runQueryInteractive(text);
    }

    emit queryText1Changed();
}


QString QExplore::queryText3()
{
    return m_queryText[2];
}

void QExplore::setQueryText3(const QString& text)
{
    if (m_queryText[2] == text)
        return;

    m_queryText[2] = text;

    if (m_queryI[2])
    {
        runQueryInteractive(text);
    }

    emit queryText1Changed();
}


QString QExplore::queryDefault1()
{
    return defaultValue(m_queryArgs[0]);
}

QString QExplore::queryDefault2()
{
    return defaultValue(m_queryArgs[1]);
}

QString QExplore::queryDefault3()
{
    return defaultValue(m_queryArgs[2]);
}

bool QExplore::queryI1()
{
    return m_queryI[0];
}

void QExplore::setQueryI1(bool val)
{
    if (m_queryI[0] == val)
        return;

    m_queryI[0] = val;
    emit queryI1Changed();
}


bool QExplore::queryI2()
{
    return m_queryI[1];
}

void QExplore::setQueryI2(bool val)
{
    if (m_queryI[1] == val)
        return;

    m_queryI[1] = val;
    emit queryI2Changed();
}


bool QExplore::queryI3()
{
    return m_queryI[2];
}

void QExplore::setQueryI3(bool val)
{
    if (m_queryI[2] == val)
        return;

    m_queryI[2] = val;
    emit queryI3Changed();
}

bool QExplore::queryError()
{
    return m_c4Query == nullptr ? false :  m_c4Query->error();
}

bool QExplore::indexError()
{
    return m_c4IndexCreator == nullptr ? false :  m_c4IndexCreator->error();
}

bool QExplore::queryRunning()
{
    return m_c4Query == nullptr ? false :  m_c4Query->isRunning();
}

bool QExplore::queryFulltext()
{
    return m_c4Query == nullptr ? false :  m_c4Query->isFulltext();
}


bool QExplore::dbOpenSuccess()
{
    return m_c4Database != nullptr;
}


bool QExplore::dbLoaded()
{
    return m_docsCount >= 0;
}

bool QExplore::webLoadSuccess()
{
    return m_webLoadSuccess;
}



bool QExplore::queryHasResultsNext()
{
    return m_c4Query == nullptr ? false : m_c4Query->row() + 1 < m_c4Query->count();
}


bool QExplore::indexRunning_v()
{
    return m_c4IndexCreator == nullptr ? false :  m_c4IndexCreator->isRunning() && !m_c4IndexCreator->fulltext();
}

bool QExplore::indexRunning_t()
{
    return m_c4IndexCreator == nullptr ? false :  m_c4IndexCreator->isRunning() && m_c4IndexCreator->fulltext();
}


bool QExplore::importRunning()
{
    return m_c4Import == nullptr ? false :  m_c4Import->isRunning();
}

void QExplore::clearDocItems(bool emitqml)
{
    for (QList<DocItem*>::ConstIterator  it = m_docItems.constBegin();
         it != m_docItems.constEnd();
         it++)
    {
        DocItem* o = *it;
        o->deleteLater();
    }

    m_docItems.clear();

    if (emitqml)
        emit docItemsChanged();
}

void QExplore::itemAppend(QQmlListProperty<DocItem>* l,
                          DocItem* o)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);

    if (qexplore)
    {
        qexplore->m_docItems.append(o);
        emit qexplore->docItemsChanged();
    }
}

void QExplore::itemClear(QQmlListProperty<DocItem>* l)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);

    if (qexplore && qexplore->m_docItems.count() > 0)
    {
        qexplore->clearDocItems();
    }
}

int QExplore::itemCount(QQmlListProperty<DocItem>* l)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    return qexplore->m_docItems.count();
}

DocItem* QExplore::itemAt(QQmlListProperty<DocItem>* l, int index)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    return qexplore->m_docItems.at(index);
}

void QExplore::setDbname(const QString& name)
{
    if (m_dbname.isEmpty() || m_dbname == name)
        return;

    m_dbname = name;
    writeConfiguration();
    emit dbnameChanged();
    emit dbFileSizeChanged();
}

const QString& QExplore::dbname() const
{
    return m_dbname;
}

QString QExplore::defaultDirectory() const
{
    return QString();
    // return Manager::defaultDirectory();
}

QString QExplore::message() const
{
    return m_message.join('\n');
}

void QExplore::displayMessage(const QString& message)
{
    m_message.prepend(message);
    if(m_message.count() > 12)
        m_message.removeLast();
    emit messageChanged();
}


void QExplore::replaceQueryArgument(QString& result, const QString& queryArg,  const QString& queryText)
{
    if (queryArg.isEmpty())
        return;

    QString strReplace(queryArg);
    char type = varType(queryArg);

    if (type == 'n' || type == 'b')
        strReplace.prepend("\"<<").append(">>\"");
    else
        strReplace.prepend("<<").append(">>");

    result.replace(strReplace, queryText);
}

QString QExplore::replaceQueryArguments(const QString& content)
{
    QString result(content);

    for (int i = 0; i < MAX_QUERY_ARGS; i++)
    {
        const QString& arg =  m_queryArgs[i];

        if (arg.isEmpty())
            break;

        replaceQueryArgument(result, arg, m_queryText[i]);
    }

    return result;
}


void QExplore::onQueryFinished()
{

    if(m_c4Query->isDocumentCount())
    {
        setDocsCount(m_c4Query->count());
        m_c4Query->deleteLater();
        m_c4Query = nullptr;
        return;
    }

    if (m_c4Query->error())
    {
        emit queryRunningChanged();
        m_newQueryRequest = false;
        return;
    }

    if(m_newQueryRequest)
    {
        displayMessage(QString("Query part finished, count = %0, elapsed time = %1 ms.").arg(m_c4Query->count()).arg(m_c4Query->elapsed()));
        m_newQueryRequest = false;
        delete m_c4Query;
        m_c4Query = nullptr;
        runQuery(m_queryName) ;
        return;
    }

    emit queryRunningChanged();

    displayMessage(QString("Query finished, count = %0, elapsed time = %1 ms.").arg(m_c4Query->count()).arg(m_c4Query->elapsed()));
    viewQueryResults();
}

bool QExplore::viewQueryResults(int fetch)
{
    if (m_c4Query == nullptr)
        return true;

    clearSearchItems(false);
    int row = 0;
    C4Error error;
    error.code = 0;
    C4QueryEnumerator* qenum = nullptr;

    while (row < fetch && (nullptr != (qenum = m_c4Query->next())))
    {
        DocItem* item = new DocItem();
        item->setDocId(QSlice::c4ToQString(qenum->docID));
        item->setCurrRevision(QSlice::c4ToQString(qenum->revID));
        item->setRevision(item->currRevision());
        item->setRowInfo(QString("Row: %1/%2").arg(m_c4Query->row()).arg(m_c4Query->count())) ;
        item->setQueryFulltext(m_c4Query->isFulltext());
        QString resText;
        int col = 0;

        if (m_c4Query->isFulltext())
        {
            item->setText(m_c4Query->fullTextRes());
        }
        else
        {
            FLValue value;

            for (FLArrayIterator iter = qenum->columns;
                 nullptr != (value = FLArrayIterator_GetValue(&iter));
                 FLArrayIterator_Next(&iter))
            {
                if (!resText.isEmpty())
                    resText.append("; ");

                QVariant result = QFleece::toVariant(value);

                // Check for kFLNull
                if ((QMetaType::Type) result.type() == QMetaType::VoidStar)
                    resText += QString("%0 = NULL").arg(m_c4Query->nameOfColumn(col++));
                else if ((QMetaType::Type)result.type() == QMetaType::QString)
                    resText += QString("%0 = \"%1\"").arg(m_c4Query->nameOfColumn(col++)).arg(result.toString());
                else
                    resText += QString("%0 = %1").arg(m_c4Query->nameOfColumn(col++)).arg(result.toString());
            }

            item->setText(resText);
        }

        row++;
        m_searchitems.append(item);
    }

    emit searchItemsChanged();
    return true;
}

bool QExplore::runQuery(const QString& fname)
{
    QString content = m_queryCache[fname];

    if (content.isEmpty())
    {
        content = Helper::readTextFile(queryDir() + "/" + fname + ".json");

        if (content.isEmpty())
        {
            QString msg = QString("Query not found: %0").arg(fname);
            qWarning("%s", qPrintable(msg));
            displayMessage(msg);
            return false;
        }

        m_queryCache[fname] = content;
    }

    QString queryStr = hasQueryVar1() ? replaceQueryArguments(content) : content;

    if (m_c4Query == nullptr)
    {
        m_c4Query = new C4RunQuery(this);
        QObject::connect(m_c4Query, SIGNAL(queryFinished()), this, SLOT(onQueryFinished()));
    }
    else
        m_c4Query->reset();

    m_c4Query->setQuery(queryStr);
    displayMessage(QString("Start Query: %1").arg(queryStr));
    if(!m_c4Query->isRunning())
        m_c4Query->start();
    return true;
}

void QExplore::onIndexCreationFinished()
{
    displayMessage(QString("Index %0 created, elapsed time = %1 ms.").arg(m_c4IndexCreator->expJson()).arg(m_c4IndexCreator->elapsed()));
    bool ft = m_c4IndexCreator->fulltext();
    m_c4IndexCreator->deleteLater();
    m_c4IndexCreator = nullptr;
    if(ft)
        emit indexRunningChanged_t();
    else
        emit indexRunningChanged_v();
}

bool QExplore::createIndex(const QString& fname, bool fullText)
{
    if (m_c4IndexCreator != nullptr)
    {
        QString msg = QString("Index creation active.");
        qWarning("%s", qPrintable(msg));
        displayMessage(msg);
        return false;
    }

    QString content = Helper::readTextFile(fullText ? indexFile_t(fname) : indexFile_v(fname) );

    if (content.isEmpty())
    {
        QString msg = QString("Index file not found: %0").arg(fname);
        qWarning("%s", qPrintable(msg));
        displayMessage(msg);
        return false;
    }

    m_c4IndexCreator = new C4IndexCreator(this, fullText);
    QObject::connect(m_c4IndexCreator, SIGNAL(indexCreationFinished()),
                     this, SLOT(onIndexCreationFinished()));
    m_c4IndexCreator->setExpJson(content);
    m_c4IndexCreator->setName(fname);
    displayMessage(QString("Start creation index: %1").arg(fname));
    m_c4IndexCreator->start();
    return true;
}


bool QExplore::deleteIndex(const QString& fname, bool fullText)
{
    if (m_c4IndexCreator != nullptr)
    {
        QString msg = QString("Index creation active.");
        qWarning("%s", qPrintable(msg));
        displayMessage(msg);
        return false;
    }

    QString content = Helper::readTextFile(fullText ? indexFile_t(fname) : indexFile_v(fname) );

    if (content.isEmpty())
    {
        QString msg = QString("Index not found: %0").arg(fname);
        qWarning("%s", qPrintable(msg));
        displayMessage(msg);
        return false;
    }

    C4Error error;

    if (!c4db_deleteIndex(m_c4Database, (QSlString) fname, &error))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to delete Index %1").arg(fname), error));
        return false;
    }

    displayMessage(QString("Index %1 deleted.").arg(fname));
    return true;
}

QString QExplore::getAllDocuments(const QString& startDocId, int limit)
{

    if (!m_c4Database)
        return QString();

    QString startDocIdNext;
    QSlString  c4StartDocId(startDocId);

    // Apply options:
    C4EnumeratorOptions options;
    options.flags = kC4InclusiveStart | kC4InclusiveEnd | kC4IncludeNonConflicted;
    options.skip = 0;
    // TODO: Implement startkey, endkey, skip, limit, etc.
    // Create enumerator:
    C4Error err;

    c4::ref<C4DocEnumerator> e = c4db_enumerateAllDocs(m_c4Database,
                                                       startDocId.isEmpty()
                                                       ? kC4SliceNull
                                                       : (C4Slice) c4StartDocId,
                                                       kC4SliceNull, &options, &err);

    if (!e)
    {
        displayMessage(QExplore::logC4Error("Unable to create alldocs enumerator", err));
        return startDocIdNext;
    }

    clearDocItems(false);
    // Enumerate
    int count = 0;

    // If count == limit, we set the successor for the next requets.
    while (c4enum_next(e, &err) && count <= limit)
    {
        C4DocumentInfo info;
        err.code = 0;
        c4enum_getDocumentInfo(e, &info);
        C4Document* doc = c4enum_getDocument(e, &err);

        if (!doc)
        {
            displayMessage(QExplore::logC4Error("Unable get document from enumerator", err));
            continue ;
        }


        if (count < limit)
            m_docItems.append(createDocItem(doc));
        else
            startDocIdNext = QSlice::c4ToQString(doc->docID);

        c4doc_free(doc);
        count++;
    }

    if (err.code > 0)
    {
        displayMessage(QExplore::logC4Error("Unable to get the next document", err));
        return startDocIdNext;
    }

    emit docItemsChanged();
    return startDocIdNext;
}

bool QExplore::error()
{
    return m_error;
}

QString QExplore::dbFileSize()
{
    QFileInfo dbFile (databaseFile());
    return Helper::convertFileSizeToHumanReadable(dbFile.size());
}


DocItem* QExplore::findDocId(const QString& docId)
{
    DocItem* retItem = 0;

    for (QList<DocItem*>::ConstIterator  it = m_docItems.constBegin();
         it != m_docItems.constEnd();
         it++)
    {
        DocItem* docItem = *it;

        if (docItem->docId() == docId)
        {
            retItem = docItem;
            break;
        }
    }

    return retItem;
}

QString QExplore::readDocument(const QString& docId, const QString& revId)
{
    m_error = false;

    if (!m_c4Database)
        return QString();

    C4Error err;
    C4Document* doc = c4doc_get(m_c4Database, (QSlString)(docId), true, &err);

    if (!doc)
    {
        displayMessage(QExplore::logC4Error(QString("Unable to get document, docId = %0").arg(docId), err));
        return QString();
    }

    if (!c4doc_selectRevision(doc, (QSlString) revId, false, &err))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to get revision, docId = %0, revId = %1").arg(docId, revId), err));
        return QString();
    }

    if (!c4doc_loadRevisionBody(doc, &err))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to load revision body, docId = %0, revId = %1").arg(docId, revId), err));
        return QString();
    }

    QString body = QSlice::qslToQString((QSlStringResult) c4doc_bodyAsJSON(doc, false, &err));

    if (body.isNull())
    {
        displayMessage(QExplore::logC4Error(QString("Unable to get body from revision, docId = %0, revId = %1").arg(docId, revId), err));
        return QString();
    }

    return Helper::formatJson(body, 2);
}


QString QExplore::getStartDocId()
{
    if (!m_c4Database)
        return QString();

    C4Error c4err;
    C4EnumeratorOptions options;
    options.flags =  kC4InclusiveStart | kC4InclusiveEnd | kC4IncludeNonConflicted;
    options.skip = 0;
    c4::ref<C4DocEnumerator> e = c4db_enumerateAllDocs(m_c4Database, kC4SliceNull, kC4SliceNull, &options, &c4err);

    if (!e)
    {
        displayMessage(QExplore::logC4Error("Unable to create alldocs enumerator", c4err));
        return QString();
    }

    if (!c4enum_next(e, &c4err))
    {
        return QString();
    }

    C4DocumentInfo info;
    c4enum_getDocumentInfo(e, &info);
    return QSlice::c4ToQString(info.docID);
}

bool QExplore::newDocument()
{
    if (!m_c4Database)
        return false;

    C4Error c4err;
    C4EnumeratorOptions options;
    options.flags =  kC4Descending | kC4InclusiveStart | kC4InclusiveEnd | kC4IncludeNonConflicted;
    options.skip = 0;
    c4::ref<C4DocEnumerator> e = c4db_enumerateAllDocs(m_c4Database, kC4SliceNull, kC4SliceNull, &options, &c4err);

    if (!e)
    {
        displayMessage(QExplore::logC4Error("Unable to create alldocs enumerator", c4err));
        return false;
    }

    if (!c4enum_next(e, &c4err))
    {
        displayMessage(QExplore::logC4Error("Unable to get next enum", c4err));
        return false;
    }

    C4DocumentInfo info;
    c4enum_getDocumentInfo(e, &info);
    QString docIdEnd = QSlice::c4ToQString(info.docID);
    QSlString docIdNew = QString::asprintf("%07u", docIdEnd.toInt() + 1);
    c4::Transaction t(m_c4Database);

    if (!t.begin(&c4err))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to begin transaction for creating new document, DocId = %0").
                                            arg(QSlice::qslToQString(docIdNew)), c4err));
        return false;
    }

    QString data = QString("{\"internal_created\":\"%0\"}").arg(QDateTime::currentDateTime().toString(Qt::ISODate));
    QSlResult body = c4db_encodeJSON(m_c4Database, (QSlString) data, &c4err);

    if (body.isNull())
    {
        displayMessage(QExplore::logC4Error(QString("Unable to encode json for new document %1").arg(QSlice::qslToQString(docIdNew)), c4err));
        return false;
    }

    C4DocPutRequest rq = {};
    rq.docID = (QSlString) docIdNew;
    rq.body =  body;
    rq.save = true;
    rq.existingRevision = false;
    rq.revFlags = kRevKeepBody | kRevNew;
    c4::ref<C4Document> docNew = c4doc_put(m_c4Database, &rq, nullptr, &c4err);

    if (!docNew)
    {
        displayMessage(QExplore::logC4Error(QString("Unable to create a new document, docId = %1").arg(QSlice::qslToQString(docIdNew)), c4err));
        return false;
    }

    if (!t.end(true, &c4err))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to end transaction for creating new document, docId = %0").
                                            arg(QSlice::qslToQString(docIdNew)), c4err));
        return false;
    }

    m_docItems.insert(0, createDocItem(docNew));
    setDocsCount(c4db_getDocumentCount(m_c4Database));
    emit docItemsChanged();
    return true;
}

bool QExplore::updateDocument(DocItem* docItem, const QString& jsonText)
{
    if (!m_c4Database)
        return false;

    if (docItem == nullptr)
    {
        qCritical("Unable to select document, docItem not defined.");
        return false;
    }

    C4Error c4err {};
    c4::Transaction t(m_c4Database);

    if (!t.begin(&c4err))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to start transaction for update, docId = %0").arg(docItem->docId()), c4err));
        return false;
    }

    QSlResult body = c4db_encodeJSON(m_c4Database, (QSlString) jsonText, &c4err);

    if (body.isNull())
    {
        displayMessage(QExplore::logC4Error("Unable to encode json", c4err));
        return false;
    }

    c4::ref<C4Document> c4CurrDoc(c4doc_get(m_c4Database, (QSlString)  docItem->docId(), true, &c4err));

    if (!c4CurrDoc)
    {
        displayMessage(QExplore::logC4Error(QString("Unable to get document, docId = %0").arg(docItem->docId()), c4err));
        return false;
    }


    //    C4DocPutRequest rq = {};
    //    rq.existingRevision = false;
    //    rq.docID = c4CurrDoc->docID;
    //    rq.body = body;
    //    rq.revFlags = kRevKeepBody;
    //    rq.save = true;

    c4::ref<C4Document> c4Doc(c4doc_update(c4CurrDoc, body, kRevKeepBody, &c4err));

    if (!c4Doc)
    {
        displayMessage(QExplore::logC4Error(QString("Unable to update, docId = %0").arg(docItem->docId()), c4err));
        return false;
    }

    if (!t.end(true, &c4err))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to end transaction for update, docId = %0").arg(docItem->docId()), c4err));
        return false;
    }

    QString revId = QSlice::c4ToQString(c4Doc->revID);
    docItem->setCurrRevision(revId);
    docItem->setRevision(revId);
    QString info = QString("Document %0 successfully updated.").arg(docItem->docId());
    qInfo("%s", qPrintable(info));
    this->displayMessage(info);
    return true;
}

bool QExplore::removeDocument(DocItem* docItem)
{
    if (docItem == nullptr)
    {
        qCritical("Unable to select document, docItem not defined.");
        return false;
    }

    C4Error c4err;
    c4::Transaction t(m_c4Database);

    if (!t.begin(&c4err))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to start transaction for remove, docId = %0").arg(docItem->docId()), c4err));
        return false;
    }

    if (!c4db_purgeDoc(m_c4Database, (QSlString) docItem->docId(), &c4err))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to remove document, DocId = %0").arg(docItem->docId()), c4err));
        return false;
    }

    if (!t.end(true, &c4err))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to end transaction for remove, docId = %0").arg(docItem->docId()), c4err));
        return false;
    }

    setDocsCount(c4db_getDocumentCount(m_c4Database));
    m_docItems.removeAt(m_docItems.indexOf(docItem));
    docItem->deleteLater();
    emit docItemsChanged();
    return true;
}

bool QExplore::removeRevision(DocItem* docItem)
{
    if (docItem == nullptr)
    {
        qCritical("Unable to select document, docItem not defined.");
        return false;
    }

    C4Error c4err;
    {
        c4::ref<C4Document> c4Doc(c4doc_get(m_c4Database, (QSlString)  docItem->docId(), false, &c4err));

        if (!c4Doc)
        {
            displayMessage(QExplore::logC4Error(QString("Unable to get document, docId = %0").arg(docItem->docId()), c4err));
            return false;
        }

        if (!c4doc_selectRevision(c4Doc, (QSlString) docItem->revision(), false, &c4err))
        {
            displayMessage(QExplore::logC4Error(QString("Unable to get revision, docId = %0, revId = %1").
                                                arg(docItem->docId(), docItem->revision()), c4err));
            return false;
        }

        if (c4Doc->selectedRev.flags != (C4RevisionFlags)kRevLeaf)
        {
            QString msg = QString("Unable to remove revision %0, no leaf.").arg(docItem->revision());
            qWarning("%s", qPrintable(msg));
            displayMessage(msg);
            return false;
        }

        C4Error c4err;
        c4::Transaction t(m_c4Database);

        if (!t.begin(&c4err))
        {
            displayMessage(QExplore::logC4Error(QString("Unable to begin transaction for removing revision, DocId = %0, RevId = %1").
                                                arg(docItem->docId(), docItem->revision()), c4err));
            return false;
        }

        int nPurged = c4doc_purgeRevision(c4Doc, (QSlString) docItem->revision(), &c4err);

        if (nPurged < 0)
        {
            displayMessage(QExplore::logC4Error(QString("Unable to remove revision, DocId = %0, RevId = %1").
                                                arg(docItem->docId(), docItem->revision()), c4err));
            return false;
        }

        QString msg = QString("%0 revision(s) purged.").arg(nPurged);
        qInfo("%s", qPrintable(msg));
        displayMessage(msg);

        if (!c4doc_save(c4Doc, 0, &c4err))
        {
            displayMessage(QExplore::logC4Error(QString("Unable to save document, DocId = %0").arg(docItem->docId()), c4err));
            return false;
        }

        if (!t.end(true, &c4err))
        {
            displayMessage(QExplore::logC4Error(QString("Unable to end transaction for removing revision, DocId = %0, RevId = %1").
                                                arg(docItem->docId(), docItem->revision()), c4err));
            return false;
        }
    }
    // Check if the document was completly removed.
    c4::ref<C4Document> c4DocCheck(c4doc_get(m_c4Database, (QSlString)  docItem->docId(), true, &c4err));

    if (!c4DocCheck)
    {
        QString msg = QString("Document %0 completly removed.").arg(docItem->docId());
        qInfo("%s", qPrintable(msg));
        displayMessage(msg);
        m_docItems.removeAt(m_docItems.indexOf(docItem));
        docItem->deleteLater();
        setDocsCount(c4db_getDocumentCount(m_c4Database));
        emit docItemsChanged();
    }

    return true;
}

bool QExplore::getNextRevision(DocItem* docItem)
{
    if (docItem == nullptr)
    {
        qCritical("Unable to select document, docItem not defined.");
        return false;
    }

    C4Error c4err;
    c4::ref<C4Document> c4Doc(c4doc_get(m_c4Database, (QSlString)  docItem->docId(), false, &c4err));

    if (!c4Doc)
    {
        displayMessage(QExplore::logC4Error(QString("Unable to get document, docId = %0").arg(docItem->docId()), c4err));
        return false;
    }

    if (!c4doc_selectRevision(c4Doc, (QSlString) docItem->revision(), false, &c4err))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to get revision, revId = %0").arg(docItem->revision()), c4err));
        return false;
    }

    if (!c4doc_selectNextRevision(c4Doc))
    {
        displayMessage(QExplore::logC4Error(QString("Unable to get next revision from revId = %0").arg(docItem->revision()), c4err));
        return false;
    }

    docItem->setRevision(QSlice::c4ToQString(c4Doc->selectedRev.revID));
    return true;
}



bool QExplore::getCurrRevision(DocItem* docItem)
{
    if (docItem == nullptr)
    {
        qCritical("Unable to select document, docItem not defined.");
        return false;
    }

    C4Error c4err;
    c4::ref<C4Document> c4Doc(c4doc_get(m_c4Database, (QSlString)  docItem->docId(), false, &c4err));

    if (!c4Doc)
    {
        displayMessage(QExplore::logC4Error(QString("Unable to get document, docId = %0").arg(docItem->docId()), c4err));
        return false;
    }

    docItem->setRevision(QSlice::c4ToQString(c4Doc->selectedRev.revID));
    return true;
}


void QExplore::createDocItemContent(C4Document* docCbl, DocItem* docItem, const QString& currRevId)
{
    if (docCbl == 0 || docItem == 0)
        return;

    QString text =  "Text";
    docItem->setText(text);
    docItem->setDocId(QSlice::c4ToQString(docCbl->docID));
    docItem->setRevision(QSlice::c4ToQString(docCbl->revID));
    docItem->setCurrRevision(currRevId);
}


DocItem* QExplore::createDocItem(C4Document* docCbl)
{
    if (docCbl == nullptr)
        return nullptr;

    DocItem* docItem = new DocItem();
    QString  currRevision = QSlice::c4ToQString(docCbl->revID);
    QString text =  "Text";
    docItem->setText(text);
    docItem->setDocId(QSlice::c4ToQString(docCbl->docID));
    docItem->setRevision(currRevision);
    docItem->setCurrRevision(currRevision);
    return docItem;
}

bool QExplore::checkDirectory(const QString& dir)
{
    QDir d(dir);
    bool success = d.exists();

    if (!success)
    {
        success = d.mkpath(dir);
    }

    if (!success)
        qCritical("Unable to create db directory %s", qPrintable(QDir::toNativeSeparators(dir)));

    return success;
}

QString QExplore::logFlError(const QString& msg, FLError error)
{
    QString errMsg = QString("%0: %1").arg(msg, QString(QFleece::flerror_getMessage(error)));
    qWarning("%s", qPrintable(errMsg));
    return errMsg;
}

QString QExplore::logC4Error(const QString& msg, const C4Error& error)
{
    QString errMsg = QString("%0: %1").arg(msg, QSlice::qslToQString((QSlStringResult) c4error_getMessage(error)));
    qCritical("%s", qPrintable(errMsg));
    return errMsg;
}

bool QExplore::databaseOpen() const
{
    return m_c4Database != nullptr;
}

void QExplore::getDatabases()
{
    m_databasesItems.clear();

    QString databasesDirName =  configDir();

    if (!checkDirectory(databasesDirName))
        return;

    QDirIterator it(databasesDirName, QDir::AllDirs | QDir::NoDotAndDotDot);
    while (!it.next().isEmpty()) {
        QString name = it.fileName();
        if(name.compare(logDirName, Qt::CaseInsensitive)
                && name.compare(typeTestDirName, Qt::CaseInsensitive)
                && name.compare("import", Qt::CaseInsensitive) )
            m_databasesItems.append(name);
    }

    m_databasesModel->setStringList(m_databasesItems);
    emit databasesChanged();

}

void QExplore::getQueries()
{
    m_queryItems.clear();
    m_queryArgs[0] = m_queryArgs[1] = m_queryArgs[2]  = QString ();
    m_queryVarsModel->setStringList(m_queryArgs.toList());

    QString queryDirName =  queryDir();

    if (!checkDirectory(queryDirName))
        return;

    QDir dir(queryDirName);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList("*.json"));
    dir.setSorting(QDir::Name | QDir::IgnoreCase);
    QFileInfoList entries = dir.entryInfoList();

    for (QFileInfoList::ConstIterator  it = entries.constBegin();
         it != entries.constEnd();
         it++)
    {
        QFileInfo fi = *it;
        m_queryItems.append(fi.baseName());
    }

    if (m_queryItems.size() > 0)
        queryContent(m_queryItems[0]);

    m_queryItemsModel->setStringList(m_queryItems);
    emit queriesChanged();

}


void QExplore::getImports()
{

    m_importFileItems.clear();

    QString importDirName =  importDir();

    if (!checkDirectory(importDirName))
        return;

    QDir dir(importDirName);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList("*.json"));
    dir.setSorting(QDir::Name | QDir::IgnoreCase);
    QFileInfoList entries = dir.entryInfoList();

    for (QFileInfoList::ConstIterator  it = entries.constBegin();
         it != entries.constEnd();
         it++)
    {
        QFileInfo fi = *it;
        m_importFileItems.append(fi.baseName());
    }

    m_importFileModel->setStringList(m_importFileItems);
    emit importfilesChanged();

}


void QExplore::getIndices_v()
{
    QString indDirName = indexDir_v();

    if (!checkDirectory(indDirName))
        return;

    m_indexItems_v.clear();
    QDir dir(indDirName);
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Name | QDir::IgnoreCase);
    QFileInfoList entries = dir.entryInfoList();

    for (QFileInfoList::ConstIterator  it = entries.constBegin();
         it != entries.constEnd();
         it++)
    {
        QFileInfo fi = *it;
        m_indexItems_v.append(fi.baseName());
    }

    m_indexItemsModel_v->setStringList(m_indexItems_v);
    emit indicesChanged_v();
}

void QExplore::getIndices_t()
{
    QString indDirName = indexDir_t();

    if (!checkDirectory(indDirName))
        return;

    m_indexItems_t.clear();
    QDir dir(indDirName);
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Name | QDir::IgnoreCase);
    QFileInfoList entries = dir.entryInfoList();

    for (QFileInfoList::ConstIterator  it = entries.constBegin();
         it != entries.constEnd();
         it++)
    {
        QFileInfo fi = *it;
        m_indexItems_t.append(fi.baseName());
    }

    m_indexItemsModel_t->setStringList(m_indexItems_t);
    emit indicesChanged_t();
}


void QExplore::requestDocumentsCount()
{
    if(m_c4Database == nullptr)
        return;

    m_docsCount = -1;

    C4RunQuery*  c4Query = new C4RunQuery(this);
    c4Query->setQuery(QLatin1String("count"));

    connect(c4Query, &C4RunQuery::queryFinished, this, [c4Query, this]() {
        setDocsCount(c4Query->count());
        QString countMsg = QString("Counting all docs in %0 ms.").arg(c4Query->elapsed());
        qInfo("%s", qPrintable(countMsg));
        displayMessage(countMsg);
        c4Query->abort();
        delete c4Query;
        emit dbLoadedChanged();
    });

    c4Query->start();

}

bool QExplore::openC4Database(C4DatabaseConfig* config)
{
    QString dbDirName =  databaseDir();

    if (!checkDirectory(dbDirName))
        return false;

    QString dbPath = QString("%0/%1").arg(dbDirName).
            arg(m_dbname);


    qInfo("Using database path %s", qPrintable(QDir::toNativeSeparators(dbPath)));
    displayMessage(QString("Using path: %0").arg(QDir::toNativeSeparators(dbPath)));

    QElapsedTimer  timer;
    timer.start();

    C4Error error;
    m_c4Database = c4db_open(QSlString(dbPath), config == nullptr ?  &defaultC4DbConfig : config, &error);

    if (m_c4Database == nullptr)
    {
        displayMessage(QExplore::logC4Error(QString("Error opening database %0.").arg(m_dbname), error));
        emit dbOpenSuccessChanged();
        return false;
    }
    // setDocsCount(33);
    // setDocsCount(c4db_getDocumentCount(m_c4Database));
    QString dbOpen = QString("Database %0 open in %1 ms.").arg(m_dbname).arg(timer.elapsed());
    qInfo("%s", qPrintable(dbOpen));

    displayMessage(dbOpen);
    clearDocItems(true);
    clearSearchResult();

    emit dbOpenSuccessChanged();
    return true;
}

bool QExplore::closeC4Database()
{
    if(m_c4Database == nullptr)
        return true;

    C4Error error;
    bool success = c4db_close(m_c4Database, &error);
    if (success)
        qInfo("Database %s closed.", qPrintable(m_dbname));
    else
        displayMessage(QExplore::logC4Error("Unable to close database", error));

    c4db_free(m_c4Database);
    m_c4Database = nullptr;
    m_docsCount = -1;
    displayMessage(QString("Database %0 closed.").arg(m_dbname));
    if(success)
    {
        emit dbLoadedChanged();
        emit dbOpenSuccessChanged();
        emit docsCountChanged();
    }
    return success;
}

void QExplore::importAbort()
{
    if (m_c4Import != nullptr)
    {
        QString msg = QString("Import from %1 aborted.").arg(m_importPath);
        qInfo("%s", qPrintable(msg));
        displayMessage(msg);
        m_c4Import->abort();
    }
}


void QExplore::shutDown()
{
    if (m_c4Import != nullptr)
    {
        m_c4Import->abort();
        m_c4Import->deleteLater();
        m_c4Import = nullptr;
    }

    closeC4Database();
    writeConfiguration();
}

bool QExplore::deleteC4Database(bool reopen)
{
    C4Error c4err;
    if(m_c4Database == nullptr)
        return true;

    if (!c4db_delete(m_c4Database, &c4err))
    {
        logC4Error(QString("Unable to delete database %1").arg(m_dbname), c4err);
        return false;
    }

    if (!c4db_free(m_c4Database))
    {
        qCritical("Unable to free database %s", qPrintable(m_dbname));
        return false;
    }

    m_c4Database = nullptr;
    displayMessage("Database deleted.");

    if (!reopen)
        return true;

    return openC4Database();
}


bool QExplore::removeC4Database()
{
    if (!c4db_free(m_c4Database))
    {
        qCritical("Unable to free database %s", qPrintable(m_dbname));
        return false;
    }

    QString dbDirName =  databaseDir();

    if (!checkDirectory(dbDirName))
        return true;

    QDir dir(dbDirName);
    if(!dir.removeRecursively())
    {
        QString msg = QString("Unable to remove directory %1.").arg(dbDirName);
        qWarning("%s", qPrintable(msg));
        displayMessage(msg);
        return false;
    }
    displayMessage(QString("Directory %1 removed.").arg(dbDirName));


    getDatabases();
    setDbname(m_databasesItems.size()<=0 ? QString() : m_databasesItems[0]);

    writeConfiguration();

    m_c4Database = nullptr;
    emit dbOpenSuccessChanged();

    return true;
}

void QExplore::onImportFinished()
{
    closeC4Database();
    m_c4Import->deleteLater();
    m_c4Import = nullptr;

    if (openC4Database())
    {
        qInfo("Import from %s into %s successful.", qPrintable(importName()) , qPrintable(m_dbname));
        displayMessage("Import successful.");
        closeC4Database();

        emit dbFileSizeChanged();
        emit importFinished();
    }
    emit importRunningChanged();

}

// Read a file that contains a JSON document per line. Every line becomes a document.
void QExplore::importJSON(const QString& name)
{
    if (m_c4Import != nullptr && m_c4Import->isRunning())
    {
        qWarning("%s", "Another import is running, execution stopped.");
        return;
    }

    qInfo("New import requested.");

    if(m_c4Database == nullptr)
    {
        openC4Database();
    }

    if (m_c4Database != nullptr)
    {
        // Delete old database.
        if (!deleteC4Database(false))
            return;
    }

    if (!openC4Database(&importC4DbConfig))
        return;
    setImportPath(importFile(name));


    if (m_c4Import == nullptr)
    {
        m_c4Import = new C4Import(this);
        QObject::connect(m_c4Import, SIGNAL(importFinished()), this, SLOT(onImportFinished()), Qt::QueuedConnection);
    }

    m_c4Import->start();
}

C4Import::C4Import(QExplore* explore)
    : m_explore(explore), m_c4DatabaseI(nullptr)
{
    // Identify our importthread
    setObjectName("THREAD_IMJ");
    moveToThread(this);
}


bool C4Import::writeJson(const QByteArray& data, int line)
{
    C4Error c4err;
    QSlResult body = c4db_encodeJSON(m_c4DatabaseI, (QSlString) data, &c4err);

    if (body.isNull())
    {
        m_explore->displayMessage(QExplore::logC4Error(QString("Unable to encode json at line %1").arg(line), c4err));
        return false;
    }

    QSlString docId = QString::asprintf("%07u", line);
    // Save document:
    C4DocPutRequest rq = {};
    rq.revFlags = kRevKeepBody;
    rq.existingRevision = false;
    rq.docID = docId;
    rq.body = body;
    rq.save = true;


    C4Document* doc = c4doc_put(m_c4DatabaseI, &rq, nullptr, &c4err);

    if (doc == nullptr)
    {
        m_explore->displayMessage(QExplore::logC4Error(QString("Unable to create a new document at line %1").arg(line), c4err));
        return false;
    }

    c4doc_free(doc);
    return true;
}

// Read a file that contains a JSON document per line. Every line becomes a document.
void C4Import::runImport()
{
    QString path = m_explore->importPath();
    qInfo("Reading %s ...  ", qPrintable(path));
    QFile jsonFile(m_explore->importPath());

    if (!jsonFile.open(QIODevice::ReadOnly))
    {
        qCritical("Unable to open file %s", qPrintable(path));
        return;
    }

    unsigned count = 0;
    m_explore->setDocsCount(0);
    m_explore->setImpProgress(0.);
    QByteArray l;

    while (!jsonFile.atEnd())
    {
        l =  jsonFile.readLine();
        count++;
    }

    jsonFile.seek(0);
    C4Error c4err;
    c4::Transaction t(m_c4DatabaseI);

    if (!t.begin(&c4err))
    {
        m_explore->displayMessage(QExplore::logC4Error("Unable to start transaction for import", c4err));
        return;
    }

    unsigned numDocs = 0;
    QElapsedTimer  timer;
    timer.start();

    while (!jsonFile.atEnd() && !m_cancel)
    {
        QByteArray buffer = jsonFile.readLine();
        writeJson(buffer, numDocs + 1);
        numDocs++;

        if (numDocs % 100 == 0)
        {
            m_explore->setImpProgress((qreal) numDocs / (qreal) count);
            m_explore->setDocsCount(numDocs);
        }
    }

    jsonFile.close();

    if (!t.end(true, &c4err))
    {
        m_explore->displayMessage(QExplore::logC4Error("Unable to end transaction for import", c4err));
    }

    m_explore->setImpProgress(numDocs / count);
    m_explore->setDocsCount(numDocs);
    qint64 ms = timer.elapsed();
    QString info;

    if (numDocs > 0)
    {
        info  = QString("%0 documents imported in %1 sec (%2 ms per document).")
                .arg(numDocs)
                .arg((double) ms / 1000.0, 0, 'f', 2)
                .arg((double) ms / (double) numDocs, 0, 'f', 2) ;
    }
    else
        info = "No documents imported.";

    qInfo("%s", qPrintable(info));
    return;
}


void C4Import::abort()
{
    if (isRunning())
    {
        qInfo("%s", "Abort the import.");
        m_cancel = true;
        quit();
        wait();
    }
}

void C4Import::run()
{
    m_cancel = false;
    C4Error c4Error;
    m_c4DatabaseI =  c4db_openAgain(m_explore->database(), &c4Error);

    if (m_c4DatabaseI == nullptr)
    {
        m_explore->displayMessage(QExplore::logC4Error("Unable to create an import handle", c4Error));
        emit importFinished();
        return;
    }
    emit m_explore->importRunningChanged();

    runImport();
    c4db_free(m_c4DatabaseI);
    m_c4DatabaseI = nullptr;
    emit importFinished();
}


C4RunQuery::C4RunQuery(QExplore* explore)
    : m_c4DatabaseI(nullptr)
    , m_c4query(nullptr)
    , m_qenum(nullptr)
    , m_isFulltext(false)
    , m_cancel(false)
    , m_count(0)
    , m_error(false)
    , m_explore(explore)
{
    // Identify our importthread
    moveToThread(this);
}

void C4RunQuery::abort()
{
    if (isRunning())
    {
        qInfo("%s", "Abort Query.");
        m_cancel = true;
        quit();
        wait();
    }
}


C4RunQuery::~C4RunQuery()
{
    if (m_qenum != nullptr) c4queryenum_free(m_qenum);

    if (m_c4query != nullptr) c4query_free(m_c4query);

    if (m_c4DatabaseI != nullptr) c4db_free(m_c4DatabaseI);
}

QString C4RunQuery::nameOfColumn(int col)
{
    if (m_c4query == nullptr || col > m_colNameCache.size())
        return QString();

    if (m_colNameCache[col].isEmpty())
    {
        QString rawName = QSlice::qslToQString((QSlStringResult) c4query_nameOfColumn(m_c4query, col));
        m_colNameCache[col] = rawName.split('\'', QString::SkipEmptyParts)[1];
    }

    return m_colNameCache[col];
}

void C4RunQuery::runQueryCount()
{
    m_elapsed = 0;
    QElapsedTimer timer;
    timer.start();
    m_count =  c4db_getDocumentCount(m_c4DatabaseI);
    m_elapsed = timer.elapsed();
}

void C4RunQuery::runQuery()
{
    reset();

    m_elapsed = 0;
    C4Error error;
    m_c4query = c4query_new(m_c4DatabaseI, (QSlString)m_query , &error);

    if (m_c4query == nullptr)
    {
        m_explore->displayMessage(QExplore::logC4Error("Unable to create Query", error));
        m_error = true;
        return;
    }

    QElapsedTimer timer;
    timer.start();
    C4QueryOptions opt;
    //    opt.limit = 10;
    //    opt.skip = 0;
    opt.rankFullText = true;
    // m_qenum = c4query_run(m_c4query, &kC4DefaultQueryOptions, kC4SliceNull, &error);
    m_qenum = c4query_run(m_c4query, &opt, kC4SliceNull, &error);

    if (!m_qenum)
    {
        m_explore->displayMessage(QExplore::logC4Error("Unable to get query enumerator", error));
        m_error = true;
        return;
    }

    m_count =  c4queryenum_getRowCount(m_qenum, &error);

    if (m_count < 0)
    {
        m_explore->displayMessage(QExplore::logC4Error("Unable to get total number of rows in the query", error));
        m_error = true;
        return;
    }

    QString  qp = QSlice::qslToQString((QSlStringResult) c4query_explain(m_c4query));

    m_explore->displayMessage(QString("Queryplan: %1").arg(qp));
    m_elapsed = timer.elapsed();
    m_colNameCache.resize(c4query_columnCount(m_c4query));
}


void C4RunQuery::reset()
{
    if (m_qenum != nullptr)
    {
        c4queryenum_free(m_qenum);
        m_qenum = nullptr;
    }

    if (m_c4query != nullptr)
    {
        c4query_free(m_c4query);
        m_c4query = nullptr;
    }
}

QString C4RunQuery::markResult(const C4FullTextTerm* ft, int ftCount, C4Slice c4res)
{
    QByteArray content = QSlice::c4ToQByteArrayCopy(c4res);

    for (int i = ftCount - 1; i >= 0; i--)
    {
        quint32 idxStart = (ft + i)->start;
        quint32 idxEnd = idxStart + (ft + i)->length;
        content.insert(idxEnd, (const char*) highLightMarkAfter.buf, highLightMarkAfter.size);
        content.insert(idxStart, (const char*) highLightMarkBefore.buf, highLightMarkBefore.size);
    }

    return QString::fromUtf8(content);
}


C4QueryEnumerator* C4RunQuery::next()
{
    if (m_qenum == nullptr)
        return nullptr;

    C4Error error;
    error.code = 0;

    if (c4queryenum_next(m_qenum, &error))
    {
        const C4FullTextTerm* ft = m_qenum->fullTextTerms;

        if (ft != nullptr)
        {
            if (!m_isFulltext)
                emit m_explore->queryFulltextChanged();

            m_isFulltext = true;

            // symbol(s) not found for MacOs
            // C4StringResult c4resMatch = c4queryenum_fullTextMatched(m_qenum, &error);
            C4StringResult c4resMatch = c4query_fullTextMatched(m_c4query, m_qenum->docID,  m_qenum->docSequence, &error);

            if (error.code > 0)
            {
                m_explore->displayMessage(QExplore::logC4Error("Unable to get fulltext result", error));
                m_error = true;
                return m_qenum;
            }

            int ftCount = m_qenum->fullTextTermCount;
            m_fullTextRes = C4RunQuery::markResult(ft, ftCount, (C4Slice) c4resMatch);
            c4slice_free(c4resMatch);
        }

        m_rowsQueried++;
        return m_qenum;
    }
    else
    {
        if (error.code > 0)
        {
            m_explore->displayMessage(QExplore::logC4Error("Unable to get next row", error));
            m_error = true;
        }

        return nullptr;
    }
}

void C4RunQuery::run()
{
    m_rowsQueried = 0;
    m_isFulltext = false;
    C4Error c4Error;
    m_c4DatabaseI =  c4db_openAgain(m_explore->database(), &c4Error);

    if (m_c4DatabaseI == nullptr)
    {
        m_explore->displayMessage(QExplore::logC4Error("Unable to create an query handle", c4Error));
        m_error = true;
        emit queryFinished();
        return;
    }
    emit m_explore->queryFulltextChanged();
    emit m_explore->queryRunningChanged();
    m_isDocumentCount = m_query.compare("count", Qt::CaseInsensitive) == 0;
    if(m_isDocumentCount)
        runQueryCount();
    else
        runQuery();

    emit queryFinished();
}


C4IndexCreator::C4IndexCreator(QExplore* explore, bool fullText)
    : m_c4DatabaseI(nullptr)
    , m_cancel(false)
    , m_error(false)
    , m_fulltext(fullText)
    , m_explore(explore)
{
    // Identify our importthread
    setObjectName("THREAD_IDX");
    moveToThread(this);
}

void C4IndexCreator::abort()
{
    if (isRunning())
    {
        qInfo("%s", "Abort index creation.");
        m_cancel = true;
        quit();
        wait();
    }
}


C4IndexCreator::~C4IndexCreator()
{
    if (m_c4DatabaseI != nullptr) c4db_free(m_c4DatabaseI);
}


void C4IndexCreator::runCreation()
{
    m_elapsed = 0;
    QElapsedTimer timer;
    timer.start();
    C4Error error;

    if (!c4db_createIndex(m_c4DatabaseI, (QSlString) m_name, (QSlString) m_expJson, m_fulltext ?  kC4FullTextIndex : kC4ValueIndex, nullptr, &error))
    {
        m_explore->displayMessage(QExplore::logC4Error(QString("Unable to create Index \"%1\"").arg(m_expJson), error));
        m_error = true;
        return;
    }

    m_elapsed = timer.elapsed();
}

void C4IndexCreator::run()
{

    C4Error c4Error;
    m_c4DatabaseI =  c4db_openAgain(m_explore->database(), &c4Error);

    if (m_c4DatabaseI == nullptr)
    {
        m_explore->displayMessage(QExplore::logC4Error("Unable to create  db handle", c4Error));
        m_error = true;
        emit indexCreationFinished();
        return;
    }
    if(m_fulltext)
        emit m_explore->indexRunningChanged_t();
    else
        emit m_explore->indexRunningChanged_v();
    runCreation();
    c4db_free(m_c4DatabaseI);
    m_c4DatabaseI = nullptr;
    emit indexCreationFinished();
}



bool QExplore::importLocalData(const QString& fileName)
{
    QFileInfo fi(fileName);
    QString ext = fi.suffix();
    if(ext.compare("zip", ext, Qt::CaseInsensitive) == 0)
    {
        QStringList files = JlCompress::extractDir(fileName, configDir());
        displayMessage(files.join('\n'));
        displayMessage("Files extracted:");
        getDatabases();
        getIndices_v();
        getIndices_t();
        getQueries();
        getImports();
        return true;
    }

    QString importDirName =  importDir();

    if (!checkDirectory(importDirName))
        return false;

    bool success = QFile::copy(fileName, importDirName + "/" + fi.fileName());

    if(success)
        getImports();

    return success;
}

bool QExplore::checkWebLoadSuccess()
{
    m_webLoadSuccess = !m_webLoader->lastError();
    if(!m_webLoadSuccess)
    {
        displayMessage(m_webLoader->lastErrorMsg());
        qWarning("%s", qPrintable(m_webLoader->lastErrorMsg()));
        delete m_webLoader;
        m_webLoader = nullptr;
        emit webLoadSuccessChanged();

    }
    return m_webLoadSuccess;
}

void QExplore::onReady()
{
    if(m_c4Database != nullptr)
        closeC4Database();

    m_webLoadSuccess = !m_webLoader->lastError();
    if(!checkWebLoadSuccess())
        return;

    QString fileName = m_webLoader->localFileName();


    QFileInfo fi(fileName);
    QString ext = fi.suffix();
    if(ext.compare("zip", ext, Qt::CaseInsensitive) == 0)
    {
        QStringList files = JlCompress::extractDir(fileName, configDir());
        displayMessage(files.join('\n'));
        displayMessage("Files extracted:");
        QFile f(fileName);
        f.remove();
        getDatabases();
        getIndices_v();
        getIndices_t();
        getQueries();
    }
    getImports();
    delete m_webLoader;
    m_webLoader = nullptr;
    emit webLoadSuccessChanged();
}

void QExplore::onProgress(qint64 curr, qint64 count)
{
    if(m_webLoader->lastError())
        return;

    setLoadProgress(count > 0. ? (qreal) curr / (qreal) count  : 0.0);
}


bool QExplore::importWebData(const QString& fileUrl)
{
    if(m_webLoader != nullptr)
    {
        displayMessage("Loading active");
        return false;
    }

    QString importDirName =  importDir();

    if (!checkDirectory(importDirName))
        return false;


    m_webLoader = new QUrlLoader(fileUrl, importDirName, this);

    m_webLoadSuccess = false;

    QObject::connect(m_webLoader, SIGNAL(ready()),
                     this, SLOT(onReady()));
    QObject::connect(m_webLoader, SIGNAL(progress(qint64,qint64)),
                     this, SLOT(onProgress(qint64,qint64)));
    m_webLoader->run();
    if(!checkWebLoadSuccess())
    {
        return false;
    }

    return true;
}
