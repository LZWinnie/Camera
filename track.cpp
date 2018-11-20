//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :track.cpp
//        description :目标追踪窗口类实现，实现其函数
//
//        created by LZW at 2018/10/23
//
//======================================================================

#include "track.h"
#include "ui_track.h"

#include "ballcamera.h"
#include "imgpro.h"
#include "global.h"
#include <QPixmap>


extern Global *global;

trackImgPro *balltrack;

Track::Track(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Track)
{
    ui->setupUi(this);

    //窗口设置
    Track::setWindowTitle("Target  Tracking");

    timer3 = new QTimer(this);  //定时显示
    QObject::connect(timer3,SIGNAL(timeout()),this,SLOT(showTrackSlot()));

    balltrack = new trackImgPro;    //目标追踪处理线程
    balltrack->moveToThread(&trackImgProThread);
    trackImgProThread.start();
    QObject::connect(this,SIGNAL(startBallTrack(ballCamera*)),balltrack,SLOT(targetTrack(ballCamera*)));

    QObject::connect(ui->selectBoxButton,SIGNAL(clicked()),this,SLOT(selectSlot()));
    QObject::connect(ui->startTrackButton,SIGNAL(clicked()),this,SLOT(startSlot()));
    QObject::connect(ui->stopTrackButton,SIGNAL(clicked()),this,SLOT(stopSlot()));

}

Track::~Track()
{
    delete ui;
}


//Select键开始选择目标
void Track::selectSlot()
{
    this->setMouseTracking(true);
    timer3->start(1000/global->ballRate);//开启定时器
    balltrack->isInit = false;
    isSelect = true;
}

//Start键开始追踪
void Track::startSlot()
{
    isStart = true;
    emit startBallTrack(global->ball);
}

//Stop键停止追踪
void Track::stopSlot()
{
    isStart = false;
    balltrack->isInit = false;
}

//显示
void Track::showTrackSlot()
{
    if(isStart && balltrack->isInit)        //显示追踪图像
    {
        global->mutex3.lock();
        balltrack->ballTrackerImg = global->Mat2QImage(balltrack->ballTracker);
        global->mutex3.unlock();

        ui->trackWindowLabel->setPixmap(QPixmap::fromImage(balltrack->ballTrackerImg));
    }
    else        //显示原始图像
    {
        ui->trackWindowLabel->setPixmap(QPixmap::fromImage(global->ball->ballImg));
    }
}

//目标追踪的鼠标事件
void Track::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)   //左键
    {
        if(isSelect)    //如果已经按下Select
        {
            //获得起始点
            beginp=event->globalPos();
            beginp=ui->trackWindowLabel->mapFromGlobal(beginp);
        }
    }
}

void Track::mouseReleaseEvent(QMouseEvent *event)
{
    //获得终止点
    endp=event->globalPos();
    endp=ui->trackWindowLabel->mapFromGlobal(endp);

    isSelect = false;       //恢复标志位

    //将取到的两点构成一个矩形
    balltrack->box = cv::Rect2d(beginp.x(), beginp.y(), endp.x() - beginp.x(), endp.y() - beginp.y());

}

//关闭事件
void Track::closeEvent(QCloseEvent *event)
{
    isStart = false;        //清空标志位使线程循环退出
    balltrack->isInit = false;

    trackImgProThread.quit();   //退出线程
    timer3->stop();     //停止定时

    delete balltrack;       //删除指针

    event->accept();//接收事件
}
