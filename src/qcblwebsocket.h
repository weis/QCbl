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

#ifndef QCBLWEBSOCKET_H
#define QCBLWEBSOCKET_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QThread>
#include "qcbl.h"
#include "c4Socket.h"

class QCBLWebSocket : public QWebSocket
{
    Q_OBJECT
private:
    C4Socket* c4socket;
    QString scheme;
    QString host;
    int port;
    QString path;
    fleece::AllocedDict options;
    bool closed;
public:
    explicit QCBLWebSocket(C4Socket* _c4socket, const C4Address *c4To, FLSlice _optionsFleece,QObject* parent = nullptr);
private slots:
    void socket_messageReceived(const QByteArray &message);
    void socket_disconnected();
    void socket_sslErrors(QList<QSslError> errors);
    void socket_connected();
public slots:
    void startSocket();
    void sendData(const QByteArray& data);
    void closeSocket(int status,const QByteArray& data);
};
class QCBLWebSocketController : QObject
{
    Q_OBJECT
private:
    QThread websocketthread;
    QCBLWebSocket* socket;
public:
    explicit QCBLWebSocketController(C4Socket* c4sock,const C4Address *c4To, FLSlice optionsFleece, QObject *parent = nullptr);
    void stop();
    void send(C4SliceResult allocatedData);
    void completedReceive(size_t byteCount);
    void close(int status, C4String message);


    static inline QCBLWebSocketController* internal(C4Socket *sock) {
        return ((QCBLWebSocketController*)sock->nativeHandle);
    }
    static void sock_open(C4Socket *sock, const C4Address *c4To, FLSlice optionsFleece, void*) {
        auto self = new QCBLWebSocketController(sock,c4To, optionsFleece);
        sock->nativeHandle = self;
    }
    static void sock_write(C4Socket *sock, C4SliceResult allocatedData) {
        if (internal(sock))
            internal(sock)->send(allocatedData);
    }
    static void sock_completedReceive(C4Socket *sock, size_t byteCount) {
        if (internal(sock))
            internal(sock)->completedReceive(byteCount);
    }
    static void sock_requestClose(C4Socket *sock, int status, C4String message) {
        if (internal(sock))
            internal(sock)->close(status, message);
    }
    static void sock_dispose(C4Socket *sock) {
        if (internal(sock)) {
            internal(sock)->stop();
            internal(sock)->deleteLater();
        }
        sock->nativeHandle = nullptr;
    }

};

const C4SocketFactory C4QtSocketFactory {
    kC4NoFraming,
    nullptr,
    &QCBLWebSocketController::sock_open,
    &QCBLWebSocketController::sock_write,
    &QCBLWebSocketController::sock_completedReceive,
    nullptr,
    &QCBLWebSocketController::sock_requestClose,
    &QCBLWebSocketController::sock_dispose
};

#endif // QCBLWEBSOCKET_H
