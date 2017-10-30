#ifndef QEXPLORE_H
#define QEXPLORE_H

#include <QObject>
#include <QStandardPaths>
#include <QQmlListProperty>
#include <QColor>
#include <QStringListModel>
#include <QThread>
#include <qcbl.h>
#include "docitem.h"
#include "qurlloader.h"

#include <c4.hh>

class C4IndexCreator;
class C4RunQuery;
class C4Import;

class QExplore : public QObject
{

    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<DocItem> docItems READ docItems NOTIFY docItemsChanged)
    Q_PROPERTY(QStringListModel* databases READ databases NOTIFY databasesChanged)
    Q_PROPERTY(QStringListModel* importfiles  READ importfiles NOTIFY importfilesChanged)
    Q_PROPERTY(QStringListModel* queries READ queries NOTIFY queriesChanged)
    Q_PROPERTY(QStringListModel* queryvars READ queryvars NOTIFY queryvarsChanged)
    Q_PROPERTY(QStringListModel* indices_v READ indices_v NOTIFY indicesChanged_v)
    Q_PROPERTY(QStringListModel* indices_t READ indices_t NOTIFY indicesChanged_t)

    Q_PROPERTY(QQmlListProperty<DocItem> searchItems READ searchItems NOTIFY searchItemsChanged)
    Q_PROPERTY(QString queryVar1 READ queryVar1 NOTIFY queryVar1Changed)
    Q_PROPERTY(bool hasQueryVar1 READ hasQueryVar1 NOTIFY hasQueryVar1Changed)
    Q_PROPERTY(QString queryVar2 READ queryVar2 NOTIFY queryVar2Changed)
    Q_PROPERTY(bool hasQueryVar2 READ hasQueryVar2 NOTIFY hasQueryVar2Changed)
    Q_PROPERTY(QString queryVar3 READ queryVar3 NOTIFY queryVar3Changed)
    Q_PROPERTY(bool hasQueryVar3 READ hasQueryVar3 NOTIFY hasQueryVar3Changed)
    Q_PROPERTY(QString queryText1 READ queryText1 WRITE setQueryText1 NOTIFY queryText1Changed)
    Q_PROPERTY(QString queryText2 READ queryText2 WRITE setQueryText2 NOTIFY queryText2Changed)
    Q_PROPERTY(QString queryText3 READ queryText3 WRITE setQueryText3 NOTIFY queryText3Changed)
    Q_PROPERTY(QString queryDefault1 READ queryDefault1 NOTIFY queryDefault1Changed)
    Q_PROPERTY(QString queryDefault2 READ queryDefault2 NOTIFY queryDefault2Changed)
    Q_PROPERTY(QString queryDefault3 READ queryDefault3 NOTIFY queryDefault3Changed)
    Q_PROPERTY(bool queryI1 READ queryI1 WRITE setQueryI1 NOTIFY queryI1Changed)
    Q_PROPERTY(bool queryI2 READ queryI2 WRITE setQueryI2 NOTIFY queryI2Changed)
    Q_PROPERTY(bool queryI3 READ queryI3 WRITE setQueryI3 NOTIFY queryI3Changed)

    Q_PROPERTY(bool dbOpenSuccess READ dbOpenSuccess NOTIFY dbOpenSuccessChanged)
    Q_PROPERTY(bool dbLoaded READ dbLoaded NOTIFY dbLoadedChanged)
    Q_PROPERTY(bool webLoadSuccess READ webLoadSuccess NOTIFY webLoadSuccessChanged)
    Q_PROPERTY(bool queryFulltext READ queryFulltext NOTIFY queryFulltextChanged)
    Q_PROPERTY(bool queryRunning READ queryRunning NOTIFY queryRunningChanged)
    Q_PROPERTY(bool queryError READ queryError CONSTANT)
    Q_PROPERTY(bool indexError READ indexError CONSTANT)
    Q_PROPERTY(bool queryHasResultsNext READ queryHasResultsNext NOTIFY searchItemsChanged)
    Q_PROPERTY(bool indexRunning_v READ indexRunning_v NOTIFY indexRunningChanged_v)
    Q_PROPERTY(bool indexRunning_t READ indexRunning_t NOTIFY indexRunningChanged_t)
    Q_PROPERTY(bool importRunning READ importRunning NOTIFY importRunningChanged)


    Q_PROPERTY(bool error READ error CONSTANT)
    Q_PROPERTY(bool databaseOpen READ databaseOpen CONSTANT)
    Q_PROPERTY(QString dbname READ dbname WRITE setDbname NOTIFY dbnameChanged)
    Q_PROPERTY(int docsCount READ docsCount WRITE setDocsCount NOTIFY docsCountChanged)
    Q_PROPERTY(QString dbFileSize READ dbFileSize NOTIFY dbFileSizeChanged)

    Q_PROPERTY(QString defaultDirectory READ defaultDirectory CONSTANT)
    Q_PROPERTY(QString message READ message  NOTIFY messageChanged)
    Q_PROPERTY(QString repUrl READ repUrl WRITE setRepUrl  NOTIFY repUrlChanged)
    Q_PROPERTY(qreal repProgress READ repProgress WRITE setRepProgress  NOTIFY repProgressChanged)
    Q_PROPERTY(QString repStatus READ repStatus WRITE setRepStatus  NOTIFY repStatusChanged)
    Q_PROPERTY(qreal impProgress READ impProgress WRITE setImpProgress  NOTIFY impProgressChanged)
    Q_PROPERTY(qreal loadProgress READ loadProgress WRITE setLoadProgress  NOTIFY loadProgressChanged)
    Q_PROPERTY(QString importPath READ importPath WRITE setImportPath  NOTIFY importPathChanged)
    Q_PROPERTY(QString importName READ importName  NOTIFY importNameChanged)
    Q_PROPERTY(bool repBusy READ repBusy WRITE setRepBusy  NOTIFY repBusyChanged)
    Q_PROPERTY(QColor statusFlag READ statusFlag WRITE setStatusFlag  NOTIFY statusFlagChanged)


public:
    Q_INVOKABLE QString getAllDocuments(const QString& startDocId = QString(), int limit = 1000);

    Q_INVOKABLE QString getAllDocumentsQuery(const QString& startDocId = QString(), int limit = 1000);

    Q_INVOKABLE bool runQuery(const QString& fname);
    Q_INVOKABLE bool viewQueryResults(int fetch = 1000);

    Q_INVOKABLE bool createIndex(const QString& fname, bool fullText=false);
    Q_INVOKABLE bool deleteIndex(const QString& fname, bool fullText=false);

     Q_INVOKABLE void clearSearchResult();

    Q_INVOKABLE bool startReplication(bool continuous);
    Q_INVOKABLE void stopReplication();

    Q_INVOKABLE bool removeDocument(DocItem* docItem);
    Q_INVOKABLE bool removeRevision(DocItem* docItem);
    Q_INVOKABLE bool getNextRevision(DocItem* docItem);
    Q_INVOKABLE bool getCurrRevision(DocItem* docItem);

    Q_INVOKABLE QString readDocument(const QString& docId, const QString& revId);
    Q_INVOKABLE bool newDocument();
    Q_INVOKABLE QString getStartDocId();
    Q_INVOKABLE bool updateDocument(DocItem* docItem,  const QString& jsonText);
    Q_INVOKABLE bool openC4Database(C4DatabaseConfig* config = nullptr);
    Q_INVOKABLE void requestDocumentsCount();


    Q_INVOKABLE bool deleteC4Database(bool reopen = true);
    Q_INVOKABLE bool removeC4Database();
    Q_INVOKABLE bool closeC4Database();
    Q_INVOKABLE void importJSON(const QString &name);
    Q_INVOKABLE QString indexContent_v(const QString& name);
    Q_INVOKABLE QString indexContent_t(const QString& name);
    Q_INVOKABLE QString queryContent(const QString& name);
    Q_INVOKABLE QString queryName() const;
    Q_INVOKABLE void queryVars(const QString& name);
    Q_INVOKABLE bool saveIndexContent_v(const QString& name, const QString& content, bool json5 = false);
    Q_INVOKABLE bool saveIndexContent_t(const QString& name, const QString& content, bool json5 = false);
    Q_INVOKABLE void createIndexName(const QString& name, bool textIndex);
    Q_INVOKABLE bool saveQueryContent(const QString& name, const QString& content, bool json5 = false);
    Q_INVOKABLE void createQueryName(const QString& name);
    Q_INVOKABLE bool deleteQueryContent(const QString& name);
    Q_INVOKABLE bool deleteIndexContent_v(const QString& name);
    Q_INVOKABLE bool deleteIndexContent_t(const QString& name);
    Q_INVOKABLE bool createDatabase(const QString& name);
    Q_INVOKABLE bool selectDatabase();
    Q_INVOKABLE QString importPathToNative(const QString& importPath);

    Q_INVOKABLE void importAbort();
    Q_INVOKABLE bool importLocalData(const QString& fileName);
    Q_INVOKABLE bool importWebData(const QString& fileUrl);

    Q_INVOKABLE int hasQueries();
    Q_INVOKABLE int hasIndices_v();
    Q_INVOKABLE int hasIndices_t();


    Q_INVOKABLE void dbObserverCalled();





public:
    explicit QExplore(QObject* parent = 0);
    ~QExplore();

    QQmlListProperty<DocItem> docItems();
    QStringListModel* databases();
    QStringListModel* importfiles();
    QStringListModel* queries();
    QStringListModel* queryvars();
    QStringListModel* indices_v();
    QStringListModel* indices_t();
    bool hasQueryVar1();
    QString queryVar1();
    bool hasQueryVar2();
    QString queryVar2();
    bool hasQueryVar3();
    QString queryVar3();
    QString queryText1();
    void setQueryText1(const QString& text);
    QString queryText2();
    void setQueryText2(const QString& text);
    QString queryText3();
    void setQueryText3(const QString& text);

    QString queryDefault1();
    QString queryDefault2();
    QString queryDefault3();

    bool queryI1();
    void setQueryI1(bool val);

    bool queryI2();
    void setQueryI2(bool val);

    bool queryI3();
    void setQueryI3(bool val);

    bool dbOpenSuccess();
    bool dbLoaded();
    bool webLoadSuccess();
    bool queryFulltext();
    bool queryRunning();
    bool queryError();
    bool indexError();
    bool queryHasResultsNext();
    bool indexRunning_v();
    bool indexRunning_t();
    bool importRunning();


    QQmlListProperty<DocItem> searchItems();


    const QString& repUrl() const;
    void setRepUrl(const QString& repUrl);

    bool databaseOpen() const;

    const QString& dbname() const;
    void setDbname(const QString& name);

    QString defaultDirectory() const;

    QString dbFileSize();

    int docsCount();
    void setDocsCount(int count);

    qreal repProgress() const;
    void setRepProgress(qreal repProgress);

    qreal impProgress() const;
    void setImpProgress(qreal impProgress);

    qreal loadProgress() const;
    void setLoadProgress(qreal loadProgress);

    const QString& repStatus() const;
    void setRepStatus(const QString& repStatus);

    const QString& importPath() const;
    void setImportPath(const QString& importPath);

    QString importName() const;

    bool repBusy() const;
    void setRepBusy(bool repBusy);

    const QColor& statusFlag() const;
    void setStatusFlag(const QColor& statusFlag);

    bool error();


    QString message() const;
    void displayMessage(const QString& message);

    void clearSearchItems(bool emitqml = true);

    inline C4Database* database() const {
        return m_c4Database;
    }

    bool getQueryVars(const QString& content);

    QString defaultValue(const QString& var);
    QString varName(const QString& var);
    char varType(const QString& var);

    static QString logC4Error(const QString& msg, const C4Error& error);
    static QString logFlError(const QString& msg, FLError error);

    void replaceQueryArgument(QString& result, const QString& queryArg,  const QString& queryText);
    QString replaceQueryArguments(const QString& content);

    void createDefaultDb (const QString& defaultDir);
    void readConfiguration();
    void writeConfiguration();

    void runQueryInteractive(const QString& queryText);

    inline QString configDir() {
        return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }

    inline QString configFile() {
        return QString("%0/config.json")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }

    inline QString databaseDir() {
        return QString("%0/%1")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                .arg(m_dbname);
    }

    inline QString databaseFile() {
        return QString("%0/%1/%2/db.sqlite3")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                .arg(m_dbname)
                .arg(m_dbname);
    }

    inline QString indexDir_v() {
        return QString("%0/%1/%2")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                .arg(m_dbname)
                .arg("indices/value");
    }

    inline QString indexDir_t() {
        return QString("%0/%1/%2")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                .arg(m_dbname)
                .arg("indices/text");
    }

    inline QString indexFile_v(const QString& name) {
        return QString("%0/%1/%2/%3.json")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                .arg(m_dbname)
                .arg("indices/value")
                .arg(name);
    }

    inline QString indexFile_t(const QString& name) {
        return QString("%0/%1/%2/%3.json")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                .arg(m_dbname)
                .arg("indices/text")
                .arg(name);
    }

    inline QString queryDir() {
        return QString("%0/%1/%2")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                .arg(m_dbname)
                .arg("queries");
    }

    inline QString queryFile(const QString& name) {
        return QString("%0/%1/%2/%3.json")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                .arg(m_dbname)
                .arg("queries")
                .arg(name);
    }

    inline QString importDir() {
        return QString("%0/%1")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                .arg("import");
    }

    inline QString importFile(const QString& name) {
        return QString("%0/%1/%2.json")
                .arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                .arg("import")
                .arg(name);
    }

Q_SIGNALS:
    void dbnameChanged();
    void inputChanged();
    void messageChanged();
    void docItemsChanged();
    void databasesChanged();
    void importfilesChanged();
    void queriesChanged();
    void indicesChanged_v();
    void indicesChanged_t();
    void queryvarsChanged();
    void hasQueryVar1Changed();
    void queryVar1Changed();
    void hasQueryVar2Changed();
    void queryVar2Changed();
    void hasQueryVar3Changed();
    void queryVar3Changed();
    void queryText1Changed();
    void queryText2Changed();
    void queryText3Changed();
    void queryDefault1Changed();
    void queryDefault2Changed();
    void queryDefault3Changed();
    void queryI1Changed();
    void queryI2Changed();
    void queryI3Changed();
    void dbOpenSuccessChanged();
    void dbLoadedChanged();
    void webLoadSuccessChanged();
    void queryFulltextChanged();
    void queryRunningChanged();
    void queryTextRunningChanged();

    void indexRunningChanged_v();
    void indexRunningChanged_t();
    void importRunningChanged();


    void docsCountChanged();
    void docContentChanged(const QString& docId);


    void queryCountChanged();
    void dbFileSizeChanged();
    void searchItemsChanged();
    void repUrlChanged();
    void repProgressChanged();
    void repStatusChanged();
    void repBusyChanged();
    void impProgressChanged();
    void loadProgressChanged();
    void statusFlagChanged();
    void importProgress(int line, int count);
    void importPathChanged();
    void importNameChanged();
    void importFinished();

public Q_SLOTS:
    void shutDown();


private Q_SLOTS:
    void onImportFinished();
    void onQueryFinished();
    void onIndexCreationFinished();
    void onReady();
    void onProgress(qint64 curr, qint64 count);
    void onApplicationStateChanged(Qt::ApplicationState state);

private:

    static void searchItemAppend(QQmlListProperty<DocItem>* l, DocItem* g);
    static void searchItemClear(QQmlListProperty<DocItem>* l);
    static int searchItemCount(QQmlListProperty<DocItem>* l);
    static DocItem* searchItemAt(QQmlListProperty<DocItem>* l, int index);

    void clearDocItems(bool emitqml = true);
    static void itemAppend(QQmlListProperty<DocItem>* l, DocItem* g);
    static void itemClear(QQmlListProperty<DocItem>* l);
    static int itemCount(QQmlListProperty<DocItem>* l);
    static DocItem* itemAt(QQmlListProperty<DocItem>* l, int index);

    void updateDocItem(DocItem* docItem);

    DocItem* createDocItem(C4Document* docCbl);
    DocItem* createSearchItem(C4QueryEnumerator* qenum);

    void initCivetWeb();

    void getDatabases();
    void getImports();
    void getQueries();
    void getIndices_v();
    void getIndices_t();

    void destroyDependants();
    void dbObserverCalledPrivate();

    void docObserverCalled(DocItem* item, quint64 seq);

    C4DatabaseObserver* createDbObserver();
    C4DocumentObserver* createDocObserver(DocItem* docItem);


    void repStatusInfo(C4ReplicatorStatus* status);
    DocItem* findDocId(const QString& docId);

    bool checkDirectory(const QString& dir);
    bool writeJson(const QByteArray& data, int line);


    void replicatorStatusChanged(C4ReplicatorStatus status);
    bool checkWebLoadSuccess();



private:
    C4Database*         m_c4Database;
    C4Import*           m_c4Import;
    C4RunQuery*         m_c4Query;
    C4IndexCreator*     m_c4IndexCreator;
    C4Replicator*       m_replicator;
    C4DatabaseObserver* m_dbObserver;
    bool                m_newQueryRequest;
    int                 m_docsCount;
    bool                m_repContinuous;
    qreal               m_repProgress;
    qreal               m_impProgress;
    qreal               m_loadProgress;

    QStringList         m_message;
    QString             m_dbname;
    QList<DocItem*>     m_docItems;
    QList<DocItem*>     m_searchitems;
    QStringList         m_databasesItems;
    QStringList         m_importFileItems;
    QStringList         m_queryItems;
    QStringList         m_indexItems_v;
    QStringList         m_indexItems_t;
    QStringListModel*   m_databasesModel;
    QStringListModel*   m_importFileModel;
    QStringListModel*   m_queryItemsModel;
    QStringListModel*   m_queryVarsModel;
    QStringListModel*   m_indexItemsModel_v;
    QStringListModel*   m_indexItemsModel_t;
    QString             m_queryName;
    QVector<QString>    m_queryArgs;
    QVector<QString>    m_queryText;
    QVector<bool>       m_queryI;

    QMap<QString, QString> m_queryCache;
    QString             m_querycurr;
    QUrlLoader*         m_webLoader;
    bool                m_webLoadSuccess;



    bool                m_error;
    int                 m_queryCount;


    QString             m_repUrl;
    QString             m_repStatus;
    QString             m_importPath;
    bool                m_repBusy;
    QColor              m_statusFlag;



};

class C4IndexCreator: public QThread
{
    Q_OBJECT

public:
    explicit C4IndexCreator(QExplore* explore, bool fullText=false);
    ~C4IndexCreator();

    inline bool error() { return m_error; }
    inline bool fulltext() { return m_fulltext; }
    inline const QString& expJson() { return m_expJson ;}
    inline QString setExpJson(const QString& content) { return m_expJson = content ;}
    inline const QString& name() { return m_name ;}
    inline QString setName(const QString& name) { return m_name = name ;}
    inline qint64 elapsed () { return m_elapsed ;}

    inline bool isCanceled() const {
        return m_cancel ;
    }

signals:
    void indexCreationFinished();

public slots:
    void abort();

private:
    void runCreation();
    void run();

private:
    C4Database*                             m_c4DatabaseI;
    QString                                 m_expJson;
    QString                                 m_name;
    bool                                    m_cancel;
    qint64                                  m_elapsed;
    bool                                    m_error;
    bool                                    m_fulltext;
    QExplore*                               m_explore;
};


class C4RunQuery: public QThread
{
    Q_OBJECT

public:
    explicit C4RunQuery(QExplore* explore);
    ~C4RunQuery();

    inline bool error() { return m_error; }
    inline int count() { return m_count;}
    inline int row() { return m_rowsQueried; }
    inline const QString& query() { return m_query ;}
    void setQuery(const QString& query) { m_query = query; }

    inline bool isFulltext() { return m_isFulltext;}
    inline const QString& fullTextRes() { return m_fullTextRes;}
    inline qint64 elapsed () { return m_elapsed ;}
    inline bool isDocumentCount() { return m_isDocumentCount;}

    QString nameOfColumn(int col);
    C4QueryEnumerator* next();
    void reset();

    inline bool isCanceled() const {
        return m_cancel ;
    }

signals:
    void queryFinished();

public slots:
    void abort();

private:
    void runQueryCount();
    void runQuery();
    void run();
    static QString markResult(const C4FullTextTerm *ft, int ftCount, C4Slice c4res);

private:
    C4Database*                             m_c4DatabaseI;
    C4Query*                                m_c4query;
    C4QueryEnumerator*                      m_qenum;
    bool                                    m_isFulltext;
    int                                     m_rowsQueried;
    QString                                 m_query;
    bool                                    m_cancel;
    int                                     m_count;
    bool                                    m_error;
    QExplore*                               m_explore;
    qint64                                  m_elapsed;
    bool                                    m_isDocumentCount;
    QVector<QString>                        m_colNameCache;
    QString                                 m_fullTextRes;
    static C4Slice                          highLightMarkBefore;
    static C4Slice                          highLightMarkAfter;

};


class C4Import: public QThread
{
    Q_OBJECT

public:
    explicit C4Import(QExplore* explore);
    ~C4Import() {}

    inline bool isCanceled() const {
        return m_cancel ;
    }

signals:
    void importFinished();

public slots:
    void abort();

private:
    void run();
    void runImport();
    bool writeJson(const QByteArray& data, int line);

private:
    QExplore*                               m_explore;
    C4Database*                             m_c4DatabaseI;
    bool                                    m_cancel;
};

#endif // QEXPLORE_H
