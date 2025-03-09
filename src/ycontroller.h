#ifndef YMAINCONTROLLER_H
# define YMAINCONTROLLER_H
# include <QRegularExpression>

# include "ybotconnection.h"
# include "ymainwindow.h"
# include "ysettings.h"
# include "ywebsock.h"

class YController : public QObject
{
    Q_OBJECT

public:
    YController();

    void start();

signals:
    void connectOk();
    void connectFailed(QString);

private slots:
    void onReadLevelFile();
    void onSocketRecv(QString);
    void onWebSockMessageInput(QJsonObject o);
    void onStartConnect();

private:
    void cmd_addlevel(QString username, QString cmd);
    void cmd_commands(QString username, QString cmd);
    void cmd_list(QString username, QString cmd);
    void cmd_leave(QString username, QString cmd);

    QRegularExpression _levelcodeRegExp;
    YBotConnection *_bot;
    YMainWindow *_mw;
    YSettings *_settings;
    YWebSock *_webSock;
};

#endif
