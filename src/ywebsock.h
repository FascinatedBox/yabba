#ifndef YWEBSOCK_H
#define YWEBSOCK_H

#include <QJsonObject>
#include <QWebSocket>
#include <QWebSocketServer>

class YWebSock : public QObject
{
    Q_OBJECT

public:
    explicit YWebSock(int port);
    ~YWebSock();

    bool ok;

public slots:
    void onMessageOutput(QJsonObject);

signals:
    void messageInput(QJsonObject);

private:
    void dispatchInit();
    void broadcastFrom(QJsonObject, QWebSocket *);

    QList<QWebSocket *> _allSocks;
    QWebSocketServer *_server;

private Q_SLOTS:
    void onNewConnection();
    void onTextMessage(QString message);
    void onBinaryMessage(QByteArray message);
    void onDisconnect();
};

#endif
