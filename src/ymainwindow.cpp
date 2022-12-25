#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "ymainwindow.h"

#define Y_PLAY_NAMES \
    QStringList() << "⌚" << "User" << "Code" << "Name"

#define Y_P_TIME 0
#define Y_P_USER 1
#define Y_P_CODE 2
#define Y_P_NAME 3
#define Y_P_AUTHOR 4

#define Y_QUEUE_NAMES \
    QStringList() << "User" << "Code" << "Name"

#define Y_Q_USER 0
#define Y_Q_CODE 1
#define Y_Q_NAME 2

YMainWindow::YMainWindow(YWebSock *webSock)
{
    _webSock = webSock;
    _tabWidget = new QTabWidget;
    _queueIsOpen = true;

    _tabWidget->addTab(buildQueueTab(), "Queue");
    _tabWidget->addTab(buildCommandsTab(), "Commands");
    setCentralWidget(_tabWidget);

    connect(_queueCopyButton, &QPushButton::clicked,
            this, &YMainWindow::onQueueCopyClicked);

    connect(_queueDeleteButton, &QPushButton::clicked,
            this, &YMainWindow::onQueueDeleteClicked);

    connect(_queueOpenCloseButton, &QPushButton::clicked,
            this, &YMainWindow::onQueueOpenCloseClicked);

    connect(_queueStartButton, &QPushButton::clicked,
            this, &YMainWindow::onQueueStartClicked);

    connect(_queueTree, &QTreeWidget::itemDoubleClicked,
            this, &YMainWindow::onQueueItemDoubleClicked);

    connect(_playTimer, &QTimer::timeout,
            this, &YMainWindow::onPlayTimerTick);

    connect(_pausePlayButton, &QPushButton::clicked,
            this, &YMainWindow::onPausePlayToggle);

    connect(_playGiveUpButton, &QPushButton::clicked,
            this, &YMainWindow::onPlayGiveUpClicked);

    connect(_playSuccessButton, &QPushButton::clicked,
            this, &YMainWindow::onPlaySuccessClicked);
}

//
// Helper functions
//

QTreeWidget *YMainWindow::createTree()
{
    QTreeWidget *w = new QTreeWidget;

    w->setIndentation(0);
    w->setSelectionBehavior(QAbstractItemView::SelectRows);
    w->setSelectionMode(QAbstractItemView::SingleSelection);
    w->setUniformRowHeights(true);

    return w;
}

void YMainWindow::removeLevelAt(int index)
{
    _queueTree->takeTopLevelItem(index);

    if (_queueTree->topLevelItemCount() == 0) {
        _queueStartButton->setEnabled(false);
        _queueDeleteButton->setEnabled(false);
    }

    sendQueueChange();
}

QTreeWidgetItem *YMainWindow::takeLevelAt(int index)
{
    QTreeWidgetItem *result = _queueTree->takeTopLevelItem(index);

    if (_queueTree->topLevelItemCount() == 0) {
        _queueCopyButton->setEnabled(false);
        _queueDeleteButton->setEnabled(false);
    }

    return result;
}

void YMainWindow::togglePlaySectionEnabled(bool enabled)
{
    _playGiveUpButton->setEnabled(enabled);
    _playSuccessButton->setEnabled(enabled);

    if (enabled == false) {
        _pausePlayButton->setText("Pause");
        _playPaused = false;
    }

   _pausePlayButton->setEnabled(enabled);
   _playTreeItem->setHidden(!enabled);
}

//
// Public API
//

int YMainWindow::queueAddLevel(QString user, QString code)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;

    item->setText(0, user);
    item->setText(1, code);
    item->setText(2, "");
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    _queueTree->addTopLevelItem(item);

    if (_queueTree->topLevelItemCount() == 1) {
        _queueTree->setCurrentItem(item);
        _queueCopyButton->setEnabled(true);
        _queueDeleteButton->setEnabled(true);
    }

    sendQueueChange();

    return _queueTree->topLevelItemCount();
}

bool YMainWindow::queueRemoveLevel(QString user)
{
    int position = userPositionInQueue(user);

    if (position != -1)
        removeLevelAt(position);

    return (position != -1);
}

QStringList YMainWindow::getQueueUsers()
{
    QStringList sl;

    for (int i;i < _queueTree->topLevelItemCount();i++) {
        QTreeWidgetItem *item = _queueTree->topLevelItem(i);
        QString user = item->text(Y_Q_USER);

        sl << user;
    }

    return sl;
}

void YMainWindow::startCurrentQueueLevel(QString name, QString author)
{
    QTreeWidgetItem *queueItem = _queueTree->currentItem();

    if (queueItem == NULL)
        return;

    int index = _queueTree->indexOfTopLevelItem(queueItem);

    queueItem = takeLevelAt(index);
    _queueStartButton->setEnabled(false);

    _playTreeItem->setText(Y_P_TIME, "0:00");
    _playTreeItem->setText(Y_P_USER, queueItem->text(Y_Q_USER));
    _playTreeItem->setText(Y_P_CODE, queueItem->text(Y_Q_CODE));
    _playTreeItem->setText(Y_P_NAME, name);
    _playTreeItem->setText(Y_P_AUTHOR, author);
    _playTreeItem->setHidden(false);
    _playTimerSeconds = 0;
    _playTimerMinutes = 0;

    sendPlayStart();
    _playTimer->start(1000);

    _pausePlayButton->setEnabled(true);
    _playSuccessButton->setEnabled(true);
    _playGiveUpButton->setEnabled(true);
}

int YMainWindow::userPositionInQueue(QString user)
{
    int result = -1;

    for (int i = 0;i < _queueTree->topLevelItemCount();i++) {
        QTreeWidgetItem *item = _queueTree->topLevelItem(i);
        QString itemUser = item->text(Y_Q_USER);

        if (itemUser == user) {
            result = i;
            break;
        }
    }

    return result;
}

//
// Tab builders
//

QWidget *YMainWindow::buildQueueTab()
{
    QWidget *queueTab = new QWidget;
    QVBoxLayout *queueLayout = new QVBoxLayout;

    {
        QStringList sl = Y_PLAY_NAMES;

        _playingTree = createTree();
        _playingTree->setHeaderLabels(sl);
        _playingTree->header()->setMinimumSectionSize(100);
        _playTreeItem = new QTreeWidgetItem;
        _playingTree->addTopLevelItem(_playTreeItem);
        // Now that it's in the tree, hide it until there's data to put in it.
        _playTreeItem->setHidden(true);
        _playTimer = new QTimer(this);
        _playPaused = false;
        queueLayout->addWidget(_playingTree);
    }

    {
        QHBoxLayout *l = new QHBoxLayout;

        _pausePlayButton = new QPushButton("Pause");
        _playSuccessButton = new QPushButton("Success!");
        _playGiveUpButton = new QPushButton("Give up");
        _pausePlayButton->setEnabled(false);
        _playSuccessButton->setEnabled(false);
        _playGiveUpButton->setEnabled(false);

        l->addWidget(_pausePlayButton);
        l->addWidget(_playSuccessButton);
        l->addWidget(_playGiveUpButton, 2, Qt::AlignRight);

        queueLayout->addLayout(l);
    }

    {
        QStringList sl = Y_QUEUE_NAMES;

        _queueTree = createTree();
        _queueTree->setEditTriggers(QTreeWidget::NoEditTriggers);
        _queueTree->setHeaderLabels(sl);
        _queueTree->header()->setMinimumSectionSize(100);
        queueLayout->addWidget(_queueTree);
    }

    {
        QHBoxLayout *l = new QHBoxLayout;

        _queueCopyButton = new QPushButton("Copy level");
        _queueStartButton = new QPushButton("Start level");
        _queueDeleteButton = new QPushButton("Delete level");
        _queueOpenCloseButton = new QPushButton("Close Queue");

        _queueCopyButton->setEnabled(false);
        _queueStartButton->setEnabled(false);
        _queueDeleteButton->setEnabled(false);

        l->addWidget(_queueCopyButton);
        l->addWidget(_queueStartButton);
        l->addWidget(_queueDeleteButton, 2, Qt::AlignRight);
        l->addWidget(_queueOpenCloseButton, 3, Qt::AlignRight);
        queueLayout->addLayout(l);
    }

    queueTab->setLayout(queueLayout);
    return queueTab;
}

QWidget *YMainWindow::buildCommandsTab()
{
    QWidget *commandsTab = new QWidget;
    QVBoxLayout *commandsLayout = new QVBoxLayout;
    QLabel *label = new QLabel(
R"(<b>!addlevel &lt;code&gt;</b><br>
Add &lt;code&gt; to the queue.<br>
<b>!list</b><br>
Show users in queue.<br>
<b>!queue</b><br>
Alias for !list.<br>
<b>!leave</b><br>
Remove levelcode from queue.<br>
)");

    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    commandsLayout->addWidget(label);
    commandsTab->setLayout(commandsLayout);

    return commandsTab;
}

//
// WebSock message senders
//

void YMainWindow::sendPlayStart()
{
    QJsonObject o;

    o["op"] = "playStart";
    o["playAuthor"] = _playTreeItem->text(Y_P_AUTHOR);
    o["playCode"] = _playTreeItem->text(Y_P_CODE);
    o["playName"] = _playTreeItem->text(Y_P_NAME);
    o["playUser"] = _playTreeItem->text(Y_P_USER);

    _webSock->onMessageOutput(o);
}

void YMainWindow::sendQueueChange()
{
    QJsonObject o;

    o["op"] = "queueChange";
    o["size"] = _queueTree->topLevelItemCount();

    _webSock->onMessageOutput(o);
}

void YMainWindow::sendSyncReplyTo(int id)
{
    bool playing = (_playTreeItem->isHidden() == false);
    QJsonObject o;

    o["op"] = "syncReply";
    o["id"] = id;
    o["playing"] = playing;
    o["queueOpen"] = true;
    o["queueSize"] = _queueTree->topLevelItemCount();

    if (playing) {
        QStringList playTime = _playTreeItem->text(Y_P_TIME).split(":");
        int minutes = playTime[0].toInt(0);
        int seconds = playTime[1].toInt(0);

        o["playAuthor"] = _playTreeItem->text(Y_P_AUTHOR);
        o["playCode"] = _playTreeItem->text(Y_P_CODE);
        o["playMinutes"] = minutes;
        o["playName"] = _playTreeItem->text(Y_P_NAME);
        o["playPaused"] = _playPaused;
        o["playSeconds"] = seconds;
        o["playUser"] = _playTreeItem->text(Y_P_USER);
    }

    _webSock->onMessageOutput(o);
}

//
// Queue + Play slots
//

void YMainWindow::onPausePlayToggle()
{
    QJsonObject o;

    _playPaused = !_playPaused;

    if (_playPaused) {
        o["op"] = "timerPause";
        _playTimer->stop();
        _pausePlayButton->setText("Play");
    }
    else {
        o["op"] = "timerPlay";
        _playTimer->start(1000);
        _pausePlayButton->setText("Pause");
    }

    _webSock->onMessageOutput(o);
}

void YMainWindow::onPlayGiveUpClicked()
{
    QJsonObject o;

    o["op"] = "playGiveUp";
    _webSock->onMessageOutput(o);

    _playTimer->stop();
    togglePlaySectionEnabled(false);
    // Blank the clipboard so this level can't accidentally be used twice.

    QGuiApplication::clipboard()->setText(" ");
}

void YMainWindow::onPlaySuccessClicked()
{
    QJsonObject o;

    o["op"] = "playSuccess";
    _webSock->onMessageOutput(o);

    _playTimer->stop();
    togglePlaySectionEnabled(false);

    // Blank the clipboard so this level can't accidentally be used twice.
    QGuiApplication::clipboard()->setText(" ");
}

void YMainWindow::onPlayTimerTick()
{
    char buffer[32];

    _playTimerSeconds++;

    if (_playTimerSeconds == 60) {
        _playTimerMinutes++;
        _playTimerSeconds = 0;
    }

    snprintf(buffer, 32, "%d:%02d", _playTimerMinutes, _playTimerSeconds);

    _playTreeItem->setText(Y_P_TIME, buffer);
}

void YMainWindow::onQueueOpenCloseClicked()
{
    QJsonObject o;

    _queueIsOpen = !_queueIsOpen;

    if (_queueIsOpen) {
        o["op"] = "queueOpen";
        _queueOpenCloseButton->setText("Close Queue");
    }
    else {
        o["op"] = "queueClose";
        _queueOpenCloseButton->setText("Open Queue");
    }

    _webSock->onMessageOutput(o);
}

void YMainWindow::onQueueStartClicked()
{
    readLevelFile();
}

void YMainWindow::onQueueCopyClicked()
{
    QTreeWidgetItem *item = _queueTree->currentItem();

    if (item == NULL)
        // Should not happen.
        return;

    QClipboard *clipboard = QGuiApplication::clipboard();
    QString code = item->text(Y_Q_CODE);

    clipboard->setText(code);
    _queueStartButton->setEnabled(true);
}

void YMainWindow::onQueueDeleteClicked()
{
    QTreeWidgetItem *item = _queueTree->currentItem();

    if (item == NULL)
        // Should not happen.
        return;

    removeLevelAt(_queueTree->indexOfTopLevelItem(item));
}

void YMainWindow::onQueueItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (column == Y_Q_CODE)
        _queueTree->editItem(item, column);
}
