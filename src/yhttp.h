#ifndef YHTTP_H
#define YHTTP_H

#include <QTcpServer>
#include <QObject>

class YHttp : public QTcpServer
{
    Q_OBJECT

public:
    YHttp(int port, QObject *parent = nullptr);

private slots:
    void discardClient();
    void onNewClient();
    void readClient();

private:
    QByteArray _payload;
};

#endif
