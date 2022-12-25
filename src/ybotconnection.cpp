#include "ybotconnection.h"

YBotConnection::YBotConnection(YSettings *settings)
{
    _settings = settings;
    _socket = new QTcpSocket;

    connect(_socket, &QIODevice::readyRead,
            this, &YBotConnection::onReadyRead);
}

void YBotConnection::write(QByteArray ba)
{
    _socket->write(ba + "\n");
}

void YBotConnection::onReadyRead()
{
    do {
        QString data = _socket->readLine();

        if (data.startsWith(QString("PING :tmi.twitch.tv")))
            write("PONG :tmi.twitch.tv");
        else
            emit socketRecv(data);
    } while (_socket->canReadLine());
}

bool YBotConnection::tryToConnect()
{
    QString serverName = "irc.chat.twitch.tv";

    _channel = _settings->channel().toLocal8Bit();
    QByteArray password = _settings->password().toLocal8Bit();
    QByteArray username = _settings->username().toLocal8Bit();

    _socket->connectToHost(serverName, 6667);

    if (_socket->waitForConnected(3000) == false)
        return false;

    emit socketNotice("Connection established.");
    _socket->write("PASS oauth:" + password + "\n");

    write("NICK " + username);
    write("JOIN #" + _channel);

    return true;
}

void YBotConnection::sendMessage(QString m)
{
     write("PRIVMSG #" + _channel + " :" + m.toLocal8Bit());
}
