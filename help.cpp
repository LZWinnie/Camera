//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :help.cpp
//        description :帮助类实现，实现其函数
//
//        created by LZW at 2018/10/21
//
//======================================================================

#include "help.h"
#include "ui_help.h"

int index = 0;

Help::Help(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Help)
{
    ui->setupUi(this);

    Help::setWindowTitle("Help ---- Ver2.4");

    QObject::connect(ui->helpPreviousButton,SIGNAL(clicked()),this,SLOT(helpPreviousSlot()));
    QObject::connect(ui->helpNextButton,SIGNAL(clicked()),this,SLOT(helpNextSlot()));
}

Help::~Help()
{
    delete ui;
}

void Help::helpPreviousSlot()
{
    if(index>0)
    {
        index--;
        ui->stackedWidget->setCurrentIndex(index);
    }
}

void Help::helpNextSlot()
{
    if(index<5)
    {
        index++;
        ui->stackedWidget->setCurrentIndex(index);
    }
}
