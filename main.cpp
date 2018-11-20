//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :main.cpp
//        description :主程序，打开应用并显示主窗口
//
//        created by LZW at 2018/10/21
//
//======================================================================

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
