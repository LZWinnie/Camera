//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :mainwindow.cpp
//        description :主窗口类实现，实现其函数
//
//        created by LZW at 2018/10/21
//
//======================================================================

#include "mainwindow.h"
#include "ui_mainwindow.h"

extern Global *global;

//======================================================================
//构造函数
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //定时器
    timer1 = new QTimer(this);
    timer2 = new QTimer(this);

    global = new Global;

    //登录登出
    QObject::connect(ui->loginButton,SIGNAL(clicked()),this,SLOT(loginSlot()));
    QObject::connect(ui->logoutButton,SIGNAL(clicked()),this,SLOT(logoutSlot()));

    //定时显示
    QObject::connect(timer1,SIGNAL(timeout()),this,SLOT(showBallSlot()));
    QObject::connect(timer2,SIGNAL(timeout()),this,SLOT(showGunSlot()));


    //读取视频流进程
    //球机
    ballImgPro *imgpro1 = new ballImgPro;
    imgpro1->moveToThread(&ballImgProThread);
    //ballImgProThread.start();
    QObject::connect(this,SIGNAL(startBallCamera(ballCamera*)),imgpro1,SLOT(readBallMatSlot(ballCamera*)));

    //枪机
    gunImgPro *imgpro2 = new gunImgPro;
    imgpro2->moveToThread(&gunImgProThread);
    //gunImgProThread.start();
    QObject::connect(this,SIGNAL(startGunCamera(gunCamera*)),imgpro2,SLOT(readGunMatSlot(gunCamera*)));


    //窗口和状态栏
    MainWindow::setWindowTitle("Camera  Control  System --- Ver3.0");
    QLabel *copyright = new QLabel(this);
    copyright->setText("Copyright   ©   LAB   369");
    ui->statusBar->addPermanentWidget(copyright);

    //速度滑块设置
    ui->verticalSpeedSlider->setMaximum(8);
    ui->verticalSpeedSlider->setMinimum(0);
    ui->verticalSpeedSlider->setTickPosition(QSlider::TicksAbove);
    ui->horizontalSpeedSlider->setMaximum(8);
    ui->horizontalSpeedSlider->setMinimum(0);
    ui->horizontalSpeedSlider->setTickPosition(QSlider::TicksAbove);

    //debug临时链接
    QObject::connect(global->ball,SIGNAL(loginFail()),this,SLOT(debugSlot()));
    //QObject::connect(global->ball,SIGNAL(loginSuccess()),this,SLOT(debugSlot()));
    QObject::connect(global->ball,SIGNAL(moveFail()),this,SLOT(debugSlot()));
    QObject::connect(global->ball,SIGNAL(zoomFail()),this,SLOT(debugSlot()));


    //速度控制
    QObject::connect(ui->verticalSpeedSlider,SIGNAL(valueChanged(int)),this,SLOT(verticalSpeedSlot()));
    QObject::connect(ui->horizontalSpeedSlider,SIGNAL(valueChanged(int)),this,SLOT(horizontalSpeedSlot()));

    //云台控制
    QObject::connect(ui->upButton,SIGNAL(pressed()),this,SLOT(upSlot()));
    QObject::connect(ui->downButton,SIGNAL(pressed()),this,SLOT(downSlot()));
    QObject::connect(ui->leftButton,SIGNAL(pressed()),this,SLOT(leftSlot()));
    QObject::connect(ui->rightButton,SIGNAL(pressed()),this,SLOT(rightSlot()));
    QObject::connect(ui->upButton,SIGNAL(released()),this,SLOT(upStopSlot()));
    QObject::connect(ui->downButton,SIGNAL(released()),this,SLOT(downStopSlot()));
    QObject::connect(ui->leftButton,SIGNAL(released()),this,SLOT(leftStopSlot()));
    QObject::connect(ui->rightButton,SIGNAL(released()),this,SLOT(rightStopSlot()));

    //变倍滑块设置
    ui->zoomSpeedSlider->setMaximum(1);
    ui->zoomSpeedSlider->setMinimum(0);
    ui->zoomSpeedSlider->setTickPosition(QSlider::TicksAbove);
    QObject::connect(ui->zoomSpeedSlider,SIGNAL(valueChanged(int)),this,SLOT(zoomSpeedSlot()));

    //变倍控制
    QObject::connect(ui->plusButton,SIGNAL(pressed()),this,SLOT(plusSlot()));
    QObject::connect(ui->minusButton,SIGNAL(pressed()),this,SLOT(minusSlot()));
    QObject::connect(ui->plusButton,SIGNAL(released()),this,SLOT(plusStopSlot()));
    QObject::connect(ui->minusButton,SIGNAL(released()),this,SLOT(minusStopSlot()));

    //控制坐标移动
    QObject::connect(ui->resetButton,SIGNAL(clicked()),this,SLOT(resetSlot()));
    QObject::connect(ui->goButton,SIGNAL(clicked()),this,SLOT(goSlot()));
    QObject::connect(ui->clearButton,SIGNAL(clicked()),this,SLOT(clearSlot()));
    QObject::connect(ui->rezoomButton,SIGNAL(clicked()),this,SLOT(rezoomSlot()));
    QObject::connect(ui->zoomButton,SIGNAL(clicked()),this,SLOT(zoomSlot()));
    QObject::connect(ui->clearButton2,SIGNAL(clicked()),this,SLOT(clear2Slot()));

    //控制GUN窗口隐藏
    QObject::connect(ui->gunCheckBox,SIGNAL(stateChanged(int)),this,SLOT(gunVisibleSlot()));

    //控制目标追踪窗口
    QObject::connect(ui->trackingButton,SIGNAL(clicked()),this,SLOT(trackSlot()));

    // 矫正
    //QObject::connect(ui->rectifyPushButton, SIGNAL(clicked()), this, SLOT(rectifySlot()));

    //菜单栏
    QObject::connect(ui->loginAction,SIGNAL(triggered(bool)),this,SLOT(loginSlot()));
    QObject::connect(ui->logoutAction,SIGNAL(triggered(bool)),this,SLOT(logoutSlot()));

    QObject::connect(ui->upAction,SIGNAL(triggered(bool)),this,SLOT(upActionSlot()));
    QObject::connect(ui->downAction,SIGNAL(triggered(bool)),this,SLOT(downActionSlot()));
    QObject::connect(ui->leftAction,SIGNAL(triggered(bool)),this,SLOT(leftActionSlot()));
    QObject::connect(ui->rightAction,SIGNAL(triggered(bool)),this,SLOT(rightActionSlot()));
    QObject::connect(ui->resetAction,SIGNAL(triggered(bool)),this,SLOT(resetSlot()));

    QObject::connect(ui->plusAction,SIGNAL(triggered(bool)),this,SLOT(plusActionSlot()));
    QObject::connect(ui->minusAction,SIGNAL(triggered(bool)),this,SLOT(minusActionSlot()));
    QObject::connect(ui->rezoomAction,SIGNAL(triggered(bool)),this,SLOT(rezoomSlot()));

    QObject::connect(ui->helpAction,SIGNAL(triggered(bool)),this,SLOT(helpSlot()));
}

//析构函数
MainWindow::~MainWindow()
{
    delete ui;
}


//======================================================================

//显示当前状态函数
void MainWindow::showBallStatus()
{
    //显示当前位置
    ui->horizontalLineEdit->setText(QString::number(global->ball->ballControl->horizontalPosition,10));
    ui->verticalLineEdit->setText(QString::number(global->ball->ballControl->verticalPosition,10));
    ui->zoomLineEdit->setText(QString::number(global->ball->ballControl->zoomFactor,10));
}

//登录登出函数
void MainWindow::loginSlot()
{
    //判断是否已经登陆
    if(global->ball->BALL && global->gun->GUN)
    {
        QMessageBox::information(this,"login","You have already login!");
        return;
    }

    ballImgProThread.start();
    gunImgProThread.start();

    //调用两个登录函数
    global->ball->loginBallCamera();
    global->gun->loginGunCamera();

    if(global->ball->BALL)
    {
        emit startBallCamera(global->ball);//发射一个摄像头开启信号
        timer1->start(1000/global->ballRate);//开启定时器
        //ui->statusBar->showMessage("LoginBall!",500);

        showBallStatus();
    }
    else
    {
        QMessageBox::warning(this,"login","Login ball failed!");
        return;
    }


    if(global->gun->GUN)
    {
        emit startGunCamera(global->gun);//发射一个枪机摄像头开启信号
        timer2->start(1000/global->gunRate);//开启定时器
        //ui->statusBar->showMessage("LoginGun!",500);     

        ui->gunCheckBox->setChecked(true);//初始GUN选中，显示GUN窗口
        ui->gunWindowLabel->setVisible(true);
    }
    else
    {
        QMessageBox::warning(this,"login","Login gun failed!");
        return;
    }


    if(global->ball->BALL && global->gun->GUN)
        ui->statusBar->showMessage("Login!",2000);

}

void MainWindow::logoutSlot()
{
    //清除球相关
    global->ball->BALL = false;     //恢复循环条件以退出线程
    ballImgProThread.quit();

    //清除枪相关
    global->gun->GUN = false;     //恢复循环条件以退出线程
    gunImgProThread.quit();

    timer1->stop();//停止定时
    timer2->stop();//停止定时

    CLIENT_Logout(global->ball->ballParameter->lLoginHandle);

    ui->statusBar->showMessage("Logout!",2000);

    //global->globalClear();
}

//显示函数
void MainWindow::showBallSlot()
{
    global->mutex1.lock();//互斥锁
    //global->ball->ballCap->retrieve(global->ball->ballSrc);
    cv::Mat ballSrcCopy = global->ball->ballSrc.clone();//复制一个mat用于处理
    global->mutex1.unlock();//解锁

    cv::resize(ballSrcCopy,ballSrcCopy,cv::Size(global->lwidth,global->lheight),0,0,cv::INTER_AREA);//调整大小

    global->ball->ballImg = global->Mat2QImage(ballSrcCopy);

    ui->ballWindowLabel->setPixmap(QPixmap::fromImage(global->ball->ballImg));

    return;
}

void MainWindow::showGunSlot()
{
    global->mutex2.lock();//互斥锁
    //global->gun->gunCap->retrieve(global->gun->gunSrc);
    cv::Mat gunSrcCopy = global->gun->gunSrc.clone();//复制一个mat用于处理
    global->mutex2.unlock();//解锁

    cv::resize(gunSrcCopy,gunSrcCopy,cv::Size(global->lwidth,global->lheight),0,0,cv::INTER_AREA);//调整大小
    global->gun->gunImg = global->Mat2QImage(gunSrcCopy);

    ui->gunWindowLabel->setPixmap(QPixmap::fromImage(global->gun->gunImg));

    return;
}

//debug临时槽
void MainWindow::debugSlot()
{
    qDebug()<<"ERROR!";
}

//隐藏枪机画面
void MainWindow::gunVisibleSlot()
{
    if(ui->gunCheckBox->isChecked())
    {
        ui->gunWindowLabel->setVisible(true);
    }
    else
    {
        ui->gunWindowLabel->setVisible(false);
    }
}


//==================================================================================

//坐标部分：

//垂直速度和水平速度
void MainWindow::verticalSpeedSlot()
{
    global->ball->verticalSpeedSet(ui->verticalSpeedSlider->value());
    ui->statusBar->showMessage("Vertical speed changed!",3000);
}

void MainWindow::horizontalSpeedSlot()
{
    global->ball->horizontalSpeedSet(ui->horizontalSpeedSlider->value());
    ui->statusBar->showMessage("Horizontal speed changed!",3000);
}

//上下左右的启动和停止
void MainWindow::upSlot()
{
    global->ball->upStart();

    showBallStatus();
}

void MainWindow::downSlot()
{
    global->ball->downStart();

    showBallStatus();
}

void MainWindow::leftSlot()
{
    global->ball->leftStart();

    showBallStatus();
}

void MainWindow::rightSlot()
{
    global->ball->rightStart();

    showBallStatus();
}

void MainWindow::upStopSlot()
{
    global->ball->upStop();

    showBallStatus();
}

void MainWindow::downStopSlot()
{
    global->ball->downStop();

    showBallStatus();
}

void MainWindow::leftStopSlot()
{
    global->ball->leftStop();

    showBallStatus();
}

void MainWindow::rightStopSlot()
{
    global->ball->rightStop();

    showBallStatus();
}


//坐标复位
void MainWindow::resetSlot()
{
    global->ball->resetPosition();

    showBallStatus();
}

//精确定位
void MainWindow::goSlot()
{
    global->ball->goPosition(ui->horizontalLineEdit->text().toInt(),ui->verticalLineEdit->text().toInt());

    showBallStatus();
}

//清空对话框
void MainWindow::clearSlot()
{
    ui->verticalLineEdit->clear();
    ui->horizontalLineEdit->clear();
}


//==================================================================================

//缩放部分：

//变倍速度
void MainWindow::zoomSpeedSlot()
{
    global->ball->zoomSpeedSet(ui->zoomSpeedSlider->value());

    ui->statusBar->showMessage("Zoom speed changed!",3000);
}

//变倍的启动和停止
void MainWindow::plusSlot()
{
    global->ball->plusStart();

    showBallStatus();
}

void MainWindow::minusSlot()
{
    global->ball->minusStart();

    showBallStatus();

}

void MainWindow::plusStopSlot()
{
    global->ball->plusStop();

    showBallStatus();

}

void MainWindow::minusStopSlot()
{
    global->ball->minusStop();

    showBallStatus();
}


//变倍复位
void MainWindow::rezoomSlot()
{
    global->ball->resetZoom();

    showBallStatus();
}

//精确缩放
void MainWindow::zoomSlot()
{
    global->ball->goZoom(ui->zoomLineEdit->text().toInt());

    showBallStatus();
}

//清空对话框2
void MainWindow::clear2Slot()
{
    ui->zoomLineEdit->clear();
}

//==================================================================================

//菜单栏
void MainWindow::upActionSlot()
{
    upSlot();
    cv::waitKey(1000);
    upStopSlot();
}

void MainWindow::downActionSlot()
{
    downSlot();
    cv::waitKey(1000);
    downStopSlot();
}

void MainWindow::leftActionSlot()
{
    leftSlot();
    cv::waitKey(1000);
    leftStopSlot();
}

void MainWindow::rightActionSlot()
{
    rightSlot();
    cv::waitKey(1000);
    rightStopSlot();
}

void MainWindow::plusActionSlot()
{
    plusSlot();
    cv::waitKey(1000);
    plusStopSlot();
}

void MainWindow::minusActionSlot()
{
    minusSlot();
    cv::waitKey(1000);
    minusStopSlot();
}

//==================================================================================

//关闭事件
void MainWindow::closeEvent(QCloseEvent *event)
{
    delete global;

    if(ballImgProThread.isRunning())
    {
        ballImgProThread.quit();//退出线程
        qDebug()<<"ballThread is running!";
    }

    if(gunImgProThread.isRunning())
    {
        gunImgProThread.quit();//退出线程
        qDebug()<<"gunThread2 is running!";
    }

    CLIENT_Cleanup();

    event->accept();//接收事件
}

//右键显示坐标
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu=new QMenu(this);
    QAction *action=new QAction(this);
    QPoint point = event->globalPos();
    point = ui->gunWindowLabel->mapFromGlobal(point);

    if(point.x()>=0 && point.x()<=global->lwidth/2 && point.y()>=0 && point.y()<=global->lheight/2 && ui->gunCheckBox->isChecked())
    {
        action->setText("x="+QString::number(point.x())+"\ty="+QString::number(point.y()));
        menu->addAction(action);
        menu->exec(QCursor::pos());
    }
}

//枪球联动和目标追踪的鼠标事件
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        //获得坐标
        QPoint point = event->globalPos();
        point = ui->gunWindowLabel->mapFromGlobal(point);
        if(point.x()>=0 && point.x()<=global->lwidth/2 && point.y()>=0 && point.y()<=global->lheight/2 && ui->gunCheckBox->isChecked())
        {
            global->ball->ballControl->horizontalPosition = global->horizonSlope*point.x()*2 + global->horizonIntercept;
            global->ball->ballControl->verticalPosition = global->verticalSlope*point.y()*2 + global->verticalIntercept;
            global->ball->ballControl->zoomFactor = 44;
            global->ball->goNow();
        }

        showBallStatus();
    }
}



//按键控制摄像头移动
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(!event->isAutoRepeat())
    switch(event->key())
    {

    case Qt::Key_W: global->ball->upStart();
                    break;
    case Qt::Key_S: global->ball->downStart();
                    break;
    case Qt::Key_A: global->ball->leftStart();
                    break;
    case Qt::Key_D: global->ball->rightStart();
                    break;
    }

    showBallStatus();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(!event->isAutoRepeat())
    switch(event->key())
    {

    case Qt::Key_W: global->ball->upStop();
                    break;
    case Qt::Key_S: global->ball->downStop();
                    break;
    case Qt::Key_A: global->ball->leftStop();
                    break;
    case Qt::Key_D: global->ball->rightStop();
                    break;
    }

    showBallStatus();
}


//==================================================================================

//子窗口：

//帮助窗口
void MainWindow::helpSlot()
{
    help = new Help;
    help->show();
}

//目标追踪窗口
void MainWindow::trackSlot()
{
    track = new Track;
    track->show();
}
