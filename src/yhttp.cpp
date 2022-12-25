#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTcpSocket>

#include "yhttp.h"

YHttp::YHttp(int port, QObject *parent)
    : QTcpServer(parent)
{
    listen(QHostAddress::LocalHost, port);

    connect(this, &QTcpServer::newConnection,
            this, &YHttp::onNewClient);

    QDir baseDir = QDir(QDir::currentPath());
    QFile f(baseDir.absoluteFilePath("baba.html"));

    if (f.open(QFile::ReadOnly | QFile::Text))
        _payload = f.readAll();
    else
        /// todo stop the server if this fails
        qDebug() << "YHttp: Cannot open payload file!";

    f.close();
}

void YHttp::onNewClient()
{
    QTcpSocket *s = nextPendingConnection();

    connect(s, &QTcpSocket::readyRead,    this, &YHttp::readClient);
    connect(s, &QTcpSocket::disconnected, this, &YHttp::discardClient);
}

void YHttp::readClient()
{
    QTcpSocket* socket = (QTcpSocket *)sender();

    if (socket->canReadLine()) {
        QString data = QString(socket->readAll());
        QStringList tokens = data.split("\r\n");

        if (tokens[0] == "GET /baba.html HTTP/1.1") {
            socket->write(_payload);
            socket->close();
        }
    }
}

void YHttp::discardClient()
{
    QTcpSocket *socket = (QTcpSocket *)sender();

    socket->deleteLater();
}
