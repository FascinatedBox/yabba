#include <QApplication>
#include <QDir>
#include <QFile>

#include "ycontroller.h"
#include "yinfodialog.h"

#define Y_COMMAND_LIST "!addlevel, !commands, !leave, !list, !queue"

YController::YController()
{
    _webSock = new YWebSock(8002);
    _settings = YSettings::loadSettings();
    _bot = new YBotConnection(_settings);
    _mw = new YMainWindow(_webSock);
    _levelcodeRegExp = QRegularExpression("^\\s+[A-Za-z0-9]{4}\\-[A-Za-z0-9]{4}\\s*$");

    connect(_webSock, &YWebSock::messageInput,
            this, &YController::onWebSockMessageInput);

    connect(_bot, &YBotConnection::socketRecv,
            this, &YController::onSocketRecv);

    connect(_mw, &YMainWindow::readLevelFile,
            this, &YController::onReadLevelFile);
}

void YController::start()
{
    YInfoDialog *dialog = new YInfoDialog(_mw, _settings);

    dialog->resize(350, 200);
    _mw->resize(500, 200);
    _mw->show();

    connect(dialog, &YInfoDialog::startConnect,
            this, &YController::onStartConnect);

    connect(this, &YController::connectOk,
            dialog, &YInfoDialog::accept);

    connect(this, &YController::connectFailed,
            dialog, &YInfoDialog::onConnectFailed);

    if (dialog->exec() == QDialog::Rejected)
        exit(EXIT_FAILURE);

    disconnect(dialog, &YInfoDialog::startConnect,
               this, &YController::onStartConnect);

    disconnect(this, &YController::connectOk,
               dialog, &YInfoDialog::accept);

    disconnect(this, &YController::connectFailed,
               dialog, &YInfoDialog::onConnectFailed);

    _settings->saveSettingsIfChanged();
}

void YController::onStartConnect()
{
    bool ok = _bot->tryToConnect();

    if (ok == false)
        emit connectFailed("Unable to connect to Twitch.");

    // On successful connection, onSocketRecv handles the Twitch auth reply.
}

#include <QDebug>

void YController::cmd_addlevel(QString username, QString cmd)
{
    QString level = cmd.mid(9);
    QRegularExpressionMatch match = _levelcodeRegExp.match(level);

    if (match.hasMatch() == false) {
        QString reply = QString("@%1: Invalid levelcode or the regexp I'm using is wrong.")
                .arg(username);

    qDebug() << reply;
        _bot->sendMessage(reply);
        return;
    }

    level = match.captured(0)
                 .trimmed()
                 .toUpper();

    if (_mw->getQueueIsOpen() == false) {
        QString reply = QString("@%1: box is tired (queue is closed).")
                    .arg(username);

    qDebug() << reply;
        _bot->sendMessage(reply);
        return;
    }

    int position = _mw->userPositionInQueue(username);
    QString reply;

    if (position == -1) {
        position = _mw->queueAddLevel(username, level);
        reply = QString("@%1: Added %2 to the queue at position %3.")
                    .arg(username)
                    .arg(level)
                    .arg(position);
    }
    else
        reply = QString("@%1: Error: Already have a level in the queue.")
                .arg(username);

    qDebug() << reply;

    _bot->sendMessage(reply);
}

void YController::cmd_commands(QString username, QString cmd)
{
    if (cmd != "!commands")
        return;

    QString reply = QString("@%1: " Y_COMMAND_LIST ".\n")
            .arg(username);

    _bot->sendMessage(reply);
}

void YController::cmd_list(QString username, QString cmd)
{
    if (cmd != "!list")
        return;

    QStringList users = _mw->getQueueUsers();

    if (users.size() == 0) {
        QString reply = QString("@%1: No users currently in queue.")
                .arg(username);

        _bot->sendMessage(reply);
        return;
    }

    QString joinedUsers = users.join(", ");
    QString reply = QString("@%1: Users in queue: %2.")
            .arg(username).arg(joinedUsers);

    _bot->sendMessage(reply);
}

void YController::cmd_leave(QString username, QString cmd)
{
    if (cmd != "!leave")
        return;

    bool removed = _mw->queueRemoveLevel(username);
    QString reply;

    if (removed)
        reply = QString("@%1: Removed from queue.")
                .arg(username);
    else
        reply = QString("@%1: You are not in the queue.")
                .arg(username);

    _bot->sendMessage(reply);
}

void YController::onWebSockMessageInput(QJsonObject o)
{
    int id = o["id"].toInt(0);

    _mw->sendSyncReplyTo(id);
}

// Example message:
// :fascinatedbox!fascinatedbox@fascinatedbox.tmi.twitch.tv PRIVMSG #fascinatedbox :!addlevel asdf-1234
void YController::onSocketRecv(QString data)
{
    // Ignore twitch tmi intro payload.
    if (data.startsWith(":tmi.twitch.tv")) {
        if (data.contains("authentication failed"))
            emit connectFailed("Invalid oauth token.");
        else if (data.contains("Welcome, GLHF"))
            emit connectOk();

        return;
    }

    QString action = data.section(" ", 1, 1);

    // No idea what this is, but it's not a command for the bot.
    if (action != "PRIVMSG")
        return;

    QString username = data.section("@", 0, 0).section("!", 1);
    QString command = data.section(":", 2);

    if (command.startsWith("!addlevel ")) {
        cmd_addlevel(username, command);
        return;
    }

    command = command.trimmed();

    if (command == "!list")
        cmd_list(username, command);
    else if (command == "!commands")
        cmd_commands(username, command);
    else if (command == "!leave")
        cmd_leave(username, command);
}

void YController::onReadLevelFile()
{
    QFile f(_settings->temp_ld());

    if (f.open(QFile::ReadOnly | QFile::Text) == false) {
        qDebug() << "Cannot read temp file, so will not have info.";
        return;
    }

    QTextStream in(&f);
    QString author, name;

    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.startsWith("[general")) {
            while (!in.atEnd()) {
                line = in.readLine();

                if (line.startsWith("name"))
                    name = line.split('=')[1].trimmed();
                else if (line.startsWith("author"))
                    author = line.split('=')[1].trimmed();
                else if (line.startsWith("["))
                    break;
            }
        }
    }

    name = QString("%1 by %2").arg(name).arg(author);
    f.close();

    _mw->startCurrentQueueLevel(name, author);
}
