#include <QApplication>

#include "ycontroller.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    YController *yc = new YController;

    yc->start();
    app.exec();
}
