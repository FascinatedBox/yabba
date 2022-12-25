#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QVBoxLayout>

#include "yinfodialog.h"

YInfoDialog::YInfoDialog(QWidget *parent, YSettings *settings)
    : QDialog(parent)
{
    _settings = settings;

    QVBoxLayout *vlayout = new QVBoxLayout;
    QFormLayout *layout = new QFormLayout;
    QDialogButtonBox *box = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            parent
    );

    _userLine = new QLineEdit;
    _passLine = new QLineEdit;
    _channelLine = new QLineEdit;
    _temp_ldLine = new QLineEdit;
    _searchButton = new QPushButton("Search");
    _status = new QStatusBar;

    _userLine->setText(_settings->username());
    _passLine->setText(_settings->password());
    _channelLine->setText(_settings->channel());
    _temp_ldLine->setText(_settings->temp_ld());

    _temp_ldLine->setReadOnly(true);
    _passLine->setEchoMode(QLineEdit::Password);
    _passLine->setInputMethodHints(
            Qt::ImhHiddenText |
            Qt::ImhNoPredictiveText |
            Qt::ImhNoAutoUppercase
    );

    layout->insertRow(0, "Username:", _userLine);
    layout->insertRow(1, "Password:", _passLine);
    layout->insertRow(2, "Channel:", _channelLine);
    layout->insertRow(3, "Baba temp.ld:", _temp_ldLine);
    layout->insertRow(4, "", _searchButton);
    layout->addWidget(box);

    connect(box, &QDialogButtonBox::accepted,
            this, &YInfoDialog::onAcceptClicked);

    connect(box, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    connect(_searchButton, &QPushButton::clicked,
            this, &YInfoDialog::onSearchClicked);

    vlayout->addLayout(layout);
    vlayout->addWidget(_status);

    setLayout(vlayout);
}

void YInfoDialog::onConnectFailed(QString message)
{
    _status->showMessage(message, 3000);
}

void YInfoDialog::onAcceptClicked()
{
    _settings->setUsername(_userLine->text().toLower());
    _settings->setPassword(_passLine->text().toLower());
    _settings->setChannel(_channelLine->text().toLower());
    _settings->setTemp_ld(_temp_ldLine->text());
    emit startConnect();
}

void YInfoDialog::onSearchClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File",
                                                    QDir::homePath(),
                                                    "Baba Temp File (temp.ld)");

    if (fileName == "")
        return;

    _temp_ldLine->setText(fileName);
}
