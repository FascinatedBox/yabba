#ifndef YBOTCONNECTION_H
# define YBOTCONNECTION_H
# include <QTcpSocket>

# include "ysettings.h"

class YBotConnection : public QObject
{
    Q_OBJECT

public:
    YBotConnection(YSettings *);

public:
    void sendMessage(QString);
    bool tryToConnect();

signals:
    void socketNotice(QString);
    void socketRecv(QString);
    void socketSend(QString);

private slots:
    void onReadyRead();

private:
    void write(QByteArray);

    YSettings *_settings;
    QTcpSocket *_socket;
    QByteArray _channel;
};

#endif
