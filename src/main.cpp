#include <QApplication>
#include <QFile>

#include "ycontroller.h"

QString loadFile(QString path)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QString s = f.readAll();
    f.close();

    return s;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    YController *yc = new YController;

    app.setStyleSheet(loadFile(":/style.qss"));
    yc->start();
    app.exec();
}
