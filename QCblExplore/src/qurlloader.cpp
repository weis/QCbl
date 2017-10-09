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

#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFileDevice>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QEventLoop>
#include <limits>
#include <QDebug>

#include "qurlloader.h"
#include "httpstatus.h"


#define DEFAULT_TIMEOUT 10 // wait 10 seconds for timeout

int QUrlLoader::QINTMAX = std::numeric_limits<int>::max();


QUrlLoader::QUrlLoader(const QUrl& url,
                       const QString& filePath,
                       QObject *parent,
                       QNetworkAccessManager *nmanager)
    : QObject(parent)
    , m_url(url)
    , m_localFilePath(filePath)
    , m_replyTtimeout(DEFAULT_TIMEOUT * 1000)
    , m_error(false)
    , m_aborted(false)
    , m_networkError(QNetworkReply::NoError)
    , m_nmanager(nmanager)
    , m_reply(nullptr)
    , m_replyTimer(nullptr)
    , m_localFile(nullptr)
    , m_contentLength(-1)
    , m_maxContentSize(QINTMAX-1)
    , m_beginRead(true)
    , m_dwTime(0)

{
    init();
}


QUrlLoader::~QUrlLoader()
{
    finish();
}

void QUrlLoader::finish()
{
    if(m_replyTimer)
        delete m_replyTimer;
    if(m_localFile)
    {
        if(m_localFile->isOpen())
            m_localFile->close();
        delete m_localFile;
    }
    m_localFile = nullptr;
    m_replyTimer = nullptr;
}

void QUrlLoader::init()
{
    finish();
    m_error = !m_url.isValid();
    if(m_error)
    {
        m_errorMsg = "Invalid source URL";
        return;
    }
    QFileInfo fileInfo(m_url.path());
    m_localFileName = fileInfo.fileName();
    m_localFileName =  m_localFilePath +  "/" + m_localFileName;

    m_localFile = new QFile(m_localFileName);
    if(m_localFile->exists() && !m_localFile->remove())
    {
        m_error = true;
        m_errorMsg = QString("Unable to remove file %1").arg(m_localFilePath);
        return;
    }

    m_localFile = new QFile(m_localFileName + ".tmp");
    if(!m_localFile->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        m_error = true;
        m_errorMsg = QString("Unable to create file %1").arg(m_localFilePath);
        return;
    }
    m_replyTimer = new QTimer(this);


    if(!m_nmanager)
        m_nmanager = new QNetworkAccessManager(this);
    m_replyTimer->setInterval(m_replyTtimeout);
    m_replyTimer->setSingleShot(true);

    QObject::connect(m_replyTimer, SIGNAL(timeout()),
                     this, SLOT(onReplyTimeout()));

}

bool QUrlLoader::checkIntSize(quint64 value)
{
    m_error = value > (quint64) QINTMAX;
    if(m_error)
    {
        m_errorMsg = QString("Unable to handle size %0")
                .arg(value)
                .arg(QINTMAX);
    }
    return !m_error;
}

void QUrlLoader::run()
{ 
    if(m_error || busy())
        return;

    m_elapsedTimer.start();

    startNetworkReader();
}


void QUrlLoader::startNetworkReader()
{
    QNetworkRequest request(m_url);
    setRelevantHeaders(&request);
    m_reply = m_nmanager->get(request);
    m_error  = m_reply->error();
    m_errorMsg = m_reply->errorString();
    if(m_error)
        return;

    connectReply();
}

void QUrlLoader::connectReply()
{
    if(m_reply)
    {
        QObject::connect(m_reply, SIGNAL(readyRead()),
                         this, SLOT(onReadyRead()));
        QObject::connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)),
                         this, SLOT(onProgress(qint64,qint64)));
        QObject::connect(m_reply, SIGNAL(finished()),
                         this, SLOT(onFinished()));

        QObject::connect(m_reply, SIGNAL(sslErrors(QList<QSslError>)),
                         this, SLOT(onSslHandshakeFailure(QList<QSslError>)));

        m_replyTimer->start();
    }
}

void QUrlLoader::onSslHandshakeFailure(const QList<QSslError> &errors)
{
    m_reply->ignoreSslErrors(errors);
}

void QUrlLoader::setRelevantHeaders(QNetworkRequest *request)
{

    request->setHeader(QNetworkRequest::UserAgentHeader, "Loader V0.1");
    request->setRawHeader("Connection", "Keep-Alive");
}


const QUrl& QUrlLoader::url() const
{
    return m_url;
}


bool QUrlLoader::lastError() const
{
    return m_error;
}

const QString& QUrlLoader::lastErrorMsg() const
{
    return m_errorMsg;
}


void QUrlLoader::setTimeout(int timeout)
{
    m_replyTtimeout = timeout * 1000;
}

int QUrlLoader::timeout() const
{
    return m_replyTtimeout;
}


const QString&  QUrlLoader::localFileName() const
{
    return m_localFileName;
}

int QUrlLoader::errcode() const
{
    return m_networkError;
}


int QUrlLoader::contentSize() const
{
    return m_contentLength;
}

bool QUrlLoader::busy() const
{
    return m_reply ? m_reply->isRunning() : false;
}


void  QUrlLoader::abort(const QString& abortMsg)
{
    m_aborted = true;
    m_abortMsg = abortMsg;
    if(m_reply && m_reply->isRunning())
        m_reply->abort();
    else
        onFinished();

}

int QUrlLoader::toIntValue(const QStringRef& refval)
{
    bool ok = true;
    int intval = -1;
    quint64 value = refval.isEmpty() ? -1 : refval.toULongLong(&ok);
    if(ok && checkIntSize(value))
        intval = (int) value;

    return intval;
}

bool QUrlLoader::checkContentLength(qint64 contentLength)
{
    if(!m_error)
        m_error = contentLength > (qint64) m_maxContentSize;
    if(m_error)
    {
        m_errorMsg = QString("Content-length %0 exceeds limit of %1").arg(contentLength).arg(m_maxContentSize);
    }
    return !m_error;
}

void QUrlLoader::getRelevantHeaders(HttpResponseHeaders& headers)
{
    const QList<QNetworkReply::RawHeaderPair> rawHeaderPairs = m_reply->rawHeaderPairs();
    headers.httpStatusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    for (QList<QNetworkReply::RawHeaderPair>::ConstIterator it = rawHeaderPairs.constBegin();
         it != rawHeaderPairs.constEnd();
         it++)
    {
        if(m_error)
            break;

        QByteArray headerName = (*it).first.toLower();
        if(headerName == "content-type")
        {
            headers.contentType = QString((*it).second);
            continue;
        }
        if(headerName == "content-length")
        {
            if(headers.httpStatusCode != HttpPartialContent)
            {
                quint64 contentSize =  (*it).second.toULongLong();
                if(checkIntSize(contentSize))
                    headers.contentLength = (int) contentSize;
                checkContentLength(headers.contentLength);
            }
            continue;
        }

    }
}


void QUrlLoader::onReplyTimeout()
{
    m_aborted = true;
    m_abortMsg = "Connection timed out";
    m_reply->close();
}


void QUrlLoader::onReadyRead()
{
    if(m_aborted)
        return;
    m_replyTimer->stop();
    if(m_reply->error() == QNetworkReply::NoError)
    {
        if(m_beginRead)
        {
            m_beginRead = false;

            HttpResponseHeaders header;
            getRelevantHeaders(header);
            if(!initData(header))
                return;

        }
        m_localFile->write(m_reply->readAll());
        m_replyTimer->start();
    }
}


bool QUrlLoader::initData(const HttpResponseHeaders& header)
{
    if(m_error)
    {
        m_aborted = true;
        m_abortMsg = m_errorMsg;
        m_reply->abort();
        return false;
    }
    else
    {
        // We handle only HttpOk.
        if(header.httpStatusCode != HttpOk)
        {
            return false;
        }
        m_contentLength = header.contentLength;

        // if contentLength is availlable, we can send the source infos.
        // if not, me must wait until the whole content is readed.
        if (m_contentLength > 0)
            emit sourceInfoReceived(m_url, m_contentLength);

        return true;
    }
}

void QUrlLoader::onProgress(qint64 bytesReceived, qint64 bytesTotal)
{

    if(m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != HttpOk)
        return;

    if(!m_contentLength)
    {
        // At this place we check the the total received bytes
        // when content-size is unknown (m_segManager is null)
        m_error = !checkContentLength(bytesTotal);
        if(m_error)
        {
            m_abortMsg = m_errorMsg;
            m_reply->abort();
            return;
        }
    }

    int received = bytesReceived;
    int total = bytesTotal;
    emit progress(received, total);
}

void QUrlLoader::onFinished()
{

    m_dwTime = m_elapsedTimer.elapsed();

    m_replyTimer->stop();


    if(m_reply)
    {
        m_networkError = m_reply->error();
        m_error = m_networkError != QNetworkReply::NoError;
        // If not aborted we use the Qt-error string on network-errors,
        if(m_error && !m_aborted)
            m_errorMsg = m_reply->errorString();

        m_reply->deleteLater();
        m_reply = 0;
    }

    // If aborted we use the abort message as error string.
    if(m_aborted)
    {
        m_error = true;
        m_errorMsg = m_abortMsg;
    }

    if(m_error)
        m_localFile->remove();
    else
        if(!m_localFile->rename(m_localFileName))
        {
            m_error = true;
            m_errorMsg = QString("Unable to rename file %1.tmp to %2").arg(m_localFileName, m_localFileName);
        }


    finish();

    emit ready();
}

int QUrlLoader::maxContentLength() const
{
    return m_maxContentSize;
}

void QUrlLoader::setMaxContentLength(int size)
{
    m_maxContentSize = size;
}

qint64 QUrlLoader::elapsedTime() const
{
    return m_elapsedTimer.isValid() ? m_elapsedTimer.elapsed() : m_dwTime;
}


