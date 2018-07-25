#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // 初始化SDK
    CLIENT_Init(NULL, 0);


    return a.exec();
}
