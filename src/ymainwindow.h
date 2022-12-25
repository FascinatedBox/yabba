#ifndef YMAINWINDOW_H
# define YMAINWINDOW_H
# include <QMainWindow>
# include <QPushButton>
# include <QTabWidget>
# include <QTimer>
# include <QTreeWidget>

# include "ywebsock.h"

class YMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    YMainWindow(YWebSock *);

    int queueAddLevel(QString code, QString user);
    bool queueRemoveLevel(QString user);
    bool getQueueIsOpen() { return _queueIsOpen; }
    QStringList getQueueUsers();
    void startCurrentQueueLevel(QString name, QString author);
    void sendSyncReplyTo(int id);
    int userPositionInQueue(QString user);

signals:
    void readLevelFile();

private slots:
    void onPausePlayToggle();
    void onPlayGiveUpClicked();
    void onPlaySuccessClicked();
    void onPlayTimerTick();
    void onQueueCopyClicked();
    void onQueueDeleteClicked();
    void onQueueItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onQueueOpenCloseClicked();
    void onQueueStartClicked();

private:
    void removeLevelAt(int index);
    void sendPlayStart();
    void sendQueueChange();
    QTreeWidgetItem *takeLevelAt(int index);
    void togglePlaySectionEnabled(bool enabled);

    QTreeWidget *createTree();
    QWidget *buildQueueTab();
    QWidget *buildCommandsTab();

    QPushButton *_pausePlayButton;
    QPushButton *_playGiveUpButton;
    QPushButton *_playSuccessButton;
    QPushButton *_queueCopyButton;
    QPushButton *_queueDeleteButton;
    QPushButton *_queueOpenCloseButton;
    QPushButton *_queueStartButton;
    QTabWidget *_tabWidget;
    QTimer *_playTimer;
    QTreeWidgetItem *_playTreeItem;
    QTreeWidget *_playingTree;
    QTreeWidget *_queueTree;
    YWebSock *_webSock;
    bool _playPaused;
    bool _queueIsOpen;
    int _playTimerMinutes;
    int _playTimerSeconds;
};

#endif
