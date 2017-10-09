#ifndef QURLLOADER_H
#define QURLLOADER_H

#include <QObject>
#include <QUrl>
#include <QFile>
#include <QByteArray>
#include <QSet>
#include <QElapsedTimer>
#include <QDataStream>
#include <QSslError>


class QTimer;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;



typedef struct HttpResponseHeaders
{
    HttpResponseHeaders()
        : httpStatusCode(0)
        , contentLength(-1)
    {}
    int         httpStatusCode;
    int         contentLength;
    QString     contentType;

} HttpResponseHeaders;

class QUrlLoader : public QObject
{
    Q_OBJECT
public:
    QUrlLoader(const QUrl &url,
               const QString& filePath,
               QObject* parent = 0,
               QNetworkAccessManager* nmanager = 0);
    ~QUrlLoader();

    const QUrl&         url() const;
    void                abort(const QString& abortMsg = QString());
    bool                busy() const;

    void                setTimeout(int timeout);
    int                 timeout() const;

    void                run();

    int                 maxContentLength() const;
    void                setMaxContentLength(int size);

    bool                fromLocalFile() const;
    const QString&      localFileName() const;
    int                 errcode() const;
    int                 contentSize() const;

    bool                lastError() const;
    const QString&      lastErrorMsg() const;

    qint64              elapsedTime() const;

signals:
    void                progress(qint64 bytesReceived, qint64 bytesTotal);
    void                ready();
    void                sourceInfoReceived(const QUrl& url, int contentSize);



private slots:
    void                onFinished();

    void                onReadyRead();
    void                onProgress(qint64 bytesReceived, qint64 bytesTotal);
    void                onSslHandshakeFailure(const QList<QSslError> &errors);
    void                onReplyTimeout();

private:
    friend class QFileLoader;

    void                init();
    void                finish();
    void                startNetworkReader();
    bool                initData(const HttpResponseHeaders& header);
    void                setRelevantHeaders(QNetworkRequest *request);
    void                getRelevantHeaders(HttpResponseHeaders &headers);
    int                 toIntValue(const QStringRef& refval);
    bool                checkIntSize(quint64 value);
    void                connectReply();
    bool                checkContentLength(qint64 contentLength);

    void                initTest();

private:
    QUrl                        m_url;
    QString                     m_localFilePath;
    QFile*                      m_localFile;
    //QDataStream*                m_data;
    QTimer*                     m_replyTimer;
    int                         m_replyTtimeout;
    bool                        m_error;
    QString                     m_errorMsg;
    bool                        m_aborted;
    QString                     m_abortMsg;
    int                         m_networkError;
    QNetworkAccessManager*      m_nmanager;
    QNetworkReply*              m_reply;
    QString                     m_localFileName;
    int                         m_contentLength;
    int                         m_maxContentSize;
    bool                        m_beginRead;
    QElapsedTimer               m_elapsedTimer;
    qint64                      m_dwTime;

    static int                  QINTMAX;


};

#endif // QURLLOADER_H
