//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :help.h
//        description :帮助类定义，用于产生帮助子窗口
//
//        created by LZW at 2018/10/21
//
//======================================================================

#ifndef HELP_H
#define HELP_H

#include <QWidget>

namespace Ui {
class Help;
}

class Help : public QWidget
{
    Q_OBJECT

public:
    explicit Help(QWidget *parent = 0);
    ~Help();

private:
    Ui::Help *ui;

private slots:
    void helpPreviousSlot();
    void helpNextSlot();

};

#endif // HELP_H
