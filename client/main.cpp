
#include "mainwindow.h"
#include "settings.h"
#include "equeue.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Для быстрой отладки
    //MainWindow w;
    //w.show();

    EQueue e;
    e.show();
    return a.exec();
}
