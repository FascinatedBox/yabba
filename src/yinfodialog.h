#ifndef YINFODIALOG_H
# define YINFODIALOG_H
# include <QDialog>
# include <QLineEdit>
# include <QPushButton>
# include <QStatusBar>

# include "ysettings.h"

class YInfoDialog : public QDialog
{
    Q_OBJECT

public:
    YInfoDialog(QWidget *, YSettings *);

    void showError(QString);

signals:
    void startConnect();

public slots:
    void onConnectFailed(QString);

private slots:
    void onAcceptClicked();
    void onSearchClicked();

private:
    YSettings *_settings;
    QLineEdit *_userLine;
    QLineEdit *_passLine;
    QLineEdit *_temp_ldLine;
    QLineEdit *_channelLine;
    QPushButton *_searchButton;
    QStatusBar *_status;
};

#endif
