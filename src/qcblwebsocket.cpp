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

#include "qcblwebsocket.h"
#include <QUrl>
#include <QNetworkRequest>
#include "c4Replicator.h"

QCBLWebSocket::QCBLWebSocket(C4Socket *_c4socket, const C4Address *c4To, FLSlice _optionsFleece,QObject* parent)
    :QWebSocket (QString(),QWebSocketProtocol::VersionLatest, parent)
{
    c4socket = _c4socket;
    scheme = (QString)QSlString(c4To->scheme);
    host = (QString)QSlString(c4To->hostname);
    port =c4To->port;
    path = (QString)QSlString(c4To->path);
    options = AllocedDict((C4Slice)_optionsFleece);
    closed=false;
}

void QCBLWebSocket::socket_messageReceived(const QByteArray& message)
{
    alloc_slice sl(QSlice::c4FromQByteArray(message));
    //qDebug()<<"QCBLWebSocket:message received"<<host<<path<<message.size();
    c4socket_received(c4socket,sl);
}

void QCBLWebSocket::socket_disconnected()
{
    if (!closed)
    {
        qDebug()<<"QCBLWebSocket:disconnected"<<host<<path<< this->error()<<this->errorString();
        C4Error closestatus {};
        alloc_slice msg(QSlice::c4FromQByteArray(this->errorString().toUtf8()));
        closestatus = c4error_make(WebSocketDomain, QWebSocketProtocol::CloseCodeAbnormalDisconnection, msg);
        c4socket_closed(this->c4socket,closestatus);
    }
}

void QCBLWebSocket::socket_sslErrors(QList<QSslError> errors)
{
	foreach (const QSslError &error, errors)
		qWarning() << "QCBLWebSocket:sslerror"<<host<<path<<error.errorString();
}

void QCBLWebSocket::socket_connected()
{
    closed = false;
    qDebug()<<"QCBLWebSocket:connected"<<host<<path;
    c4socket_opened(this->c4socket);
}

void QCBLWebSocket::startSocket()
{
    QUrl url(QString("%0://%1:%2%3").arg(scheme,host).arg(port).arg(path));
    QNetworkRequest r(url);
    for (Dict::iterator header(options.get(kC4ReplicatorOptionExtraHeaders).asDict());
         header; ++header) {
         QSlice key(header.key().asString());
         QSlice value(header.value().asString());
         r.setRawHeader(QSlice::qslToQByteArray(key),QSlice::qslToQByteArray(value));
    }
    QSlice cookies(options.get(kC4ReplicatorOptionCookies).asString());
    if (!cookies.isEmpty())
         r.setRawHeader("Cookie",QSlice::qslToQByteArray(cookies));

    QSlice protocols(options.get(kC4SocketOptionWSProtocols).asString());
    if (!protocols.isEmpty())
        r.setRawHeader("Sec-WebSocket-Protocol",QSlice::qslToQByteArray(protocols));
    //qDebug()<<"QCBLWebSocket:starting websocket"<<url;
    open(r);
}

void QCBLWebSocket::sendData(const QByteArray &data)
{
    qint64 count = sendBinaryMessage(data);
    c4socket_completedWrite(this->c4socket, count);
    //qDebug()<<"QCBLWebSocket:senddata"<<host<<path<<data.length()<<count;
}

void QCBLWebSocket::closeSocket(int status,const QByteArray& data)
{
    closed = true;
    //qDebug()<<"QCBLWebSocket::closeSocket"<<host<<path<<status<<data;
    this->close((QWebSocketProtocol::CloseCode)status,QString(data));

    C4Error closestatus {};
    if (status != QWebSocketProtocol::CloseCodeNormal)
    {
        alloc_slice msg(QSlice::c4FromQByteArray(data));
        closestatus = c4error_make(WebSocketDomain, status, msg);
    }
    c4socket_closed(this->c4socket,closestatus);
}

QCBLWebSocketController::QCBLWebSocketController(C4Socket *c4sock, const C4Address *c4To, FLSlice optionsFleece, QObject *parent) : QObject(parent)
{
    socket = new QCBLWebSocket(c4sock,c4To,optionsFleece);
    socket->moveToThread(&websocketthread);

    connect(&websocketthread, SIGNAL(started()), socket, SLOT(startSocket()));
    connect(socket, SIGNAL(binaryMessageReceived(const QByteArray&)), socket, SLOT(socket_messageReceived(const QByteArray&)));
    connect(socket, SIGNAL(disconnected()), socket, SLOT(socket_disconnected()));
    connect(socket, SIGNAL(sslErrors(QList<QSslError>)), socket, SLOT(socket_sslErrors(QList<QSslError>)));
    connect(socket, SIGNAL(connected()), socket, SLOT(socket_connected()));

    websocketthread.start();

}

void QCBLWebSocketController::stop()
{
    QMetaObject::invokeMethod(socket, "close", Qt::QueuedConnection);
    socket->deleteLater();
    websocketthread.quit();
    websocketthread.wait();
}


void QCBLWebSocketController::send(C4SliceResult allocatedData)
{
    QByteArray b = QSlice::c4ToQByteArray((C4Slice)allocatedData);
    QMetaObject::invokeMethod(socket, "sendData", Qt::QueuedConnection,
                              Q_ARG(const QByteArray&, b));
}

void QCBLWebSocketController::completedReceive(size_t byteCount)
{
    //qDebug()<<"QCBLWebSocketController:completedReceive"<<byteCount;
}

void QCBLWebSocketController::close(int status, C4String message)
{
    QByteArray b = QSlice::c4ToQByteArray(message);
    QMetaObject::invokeMethod(socket, "closeSocket", Qt::QueuedConnection,
                              Q_ARG(int, status),
                              Q_ARG(const QByteArray&, b));
}
