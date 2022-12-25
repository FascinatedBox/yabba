#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWebSocket>

#include "ywebsock.h"

#define SERVER_ADDR "baba server"
#define SERVER_MODE QWebSocketServer::NonSecureMode

YWebSock::YWebSock(int port)
{
    ok = true;
    _server = new QWebSocketServer(SERVER_ADDR, SERVER_MODE, nullptr);

    if (_server->listen(QHostAddress::LocalHost, port) == false) {
        ok = false;
        return;
    }

    connect(_server, &QWebSocketServer::newConnection,
            this, &YWebSock::onNewConnection);
}

YWebSock::~YWebSock()
{
    _server->close();
    qDeleteAll(_allSocks.begin(), _allSocks.end());
}

void YWebSock::onNewConnection()
{
    QWebSocket *sock = _server->nextPendingConnection();
    QJsonObject o;

    connect(sock, &QWebSocket::textMessageReceived,
            this, &YWebSock::onTextMessage);
    connect(sock, &QWebSocket::binaryMessageReceived,
            this, &YWebSock::onBinaryMessage);
    connect(sock, &QWebSocket::disconnected,
            this, &YWebSock::onDisconnect);

    _allSocks << sock;

    o["op"] = "init";
    o["id"] = _allSocks.size();
    sock->sendTextMessage(QJsonDocument(o).toJson());
}

void YWebSock::broadcastFrom(QJsonObject o, QWebSocket *source)
{
    QString message = QJsonDocument(o).toJson();

    for (int i = 0;i < _allSocks.size();i++) {
        QWebSocket *s = _allSocks[i];

        if (s != source)
            s->sendTextMessage(message);
    }
}

void YWebSock::onMessageOutput(QJsonObject o)
{
    broadcastFrom(o, nullptr);
}

void YWebSock::onTextMessage(QString message)
{
    QWebSocket *sock = qobject_cast<QWebSocket *>(sender());

    if (sock == nullptr) return;

    QJsonObject o = QJsonDocument::fromJson(message.toLatin1()).object();

    broadcastFrom(o, sock);
    emit messageInput(o);
}

void YWebSock::onBinaryMessage(QByteArray)
{
    QWebSocket *sock = qobject_cast<QWebSocket *>(sender());

    if (sock == nullptr) return;

    sock->close();
}

void YWebSock::onDisconnect()
{
    QWebSocket *sock = qobject_cast<QWebSocket *>(sender());

    if (sock == nullptr) return;

    _allSocks.removeAll(sock);
    sock->deleteLater();
}
