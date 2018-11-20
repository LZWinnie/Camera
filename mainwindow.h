//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :mainwindow.h
//        description :主窗口类定义，包含各种整体操作，及ui界面所需要的槽
//
//        created by LZW at 2018/10/21
//
//======================================================================

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>

#include "avglobal.h"
#include "dhconfigsdk.h"
#include "dhnetsdk.h"
#include <opencv2/opencv.hpp>
#include "ballcamera.h"
#include "ballstatus.h"
#include "dhparameter.h"
#include "guncamera.h"
#include "imgpro.h"
#include "global.h"
#include "help.h"
#include "track.h"

#include <QThread>
#include <QImage>
#include <QDebug>
#include <QTimer>
#include <QPixmap>
#include <QSlider>
#include <QString>
#include <QLabel>
#include <QList>
#include <QMutex>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;

    QTimer *timer1,*timer2;         //定时器

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QThread ballImgProThread;   //用于后台读取的线程
    QThread gunImgProThread;

    Help *help;     //帮助窗口
    Track *track;   //目标追踪窗口

    void showBallStatus();      //显示当前球机状态在界面上

signals:
    void startBallCamera(ballCamera* ballc);
    void startGunCamera(gunCamera* gunc);
    //void startBallTrack();

public slots:
    void loginSlot();
    void logoutSlot();

    void showBallSlot();
    void showGunSlot();

    void debugSlot();

    void verticalSpeedSlot();
    void horizontalSpeedSlot();

    void upSlot();
    void downSlot();
    void leftSlot();
    void rightSlot();

    void upStopSlot();
    void downStopSlot();
    void leftStopSlot();
    void rightStopSlot();   

    void resetSlot();
    void goSlot();
    void clearSlot();

    void zoomSpeedSlot();

    void plusSlot();
    void plusStopSlot();
    void minusSlot();
    void minusStopSlot();   

    void zoomSlot();
    void rezoomSlot();
    void clear2Slot();

    void gunVisibleSlot();

    //void rectifySlot();

    void upActionSlot();
    void downActionSlot();
    void leftActionSlot();
    void rightActionSlot();

    void plusActionSlot();
    void minusActionSlot();

    void helpSlot();
    void trackSlot();


protected:
    void closeEvent(QCloseEvent *);

    void contextMenuEvent(QContextMenuEvent *);

    void mousePressEvent(QMouseEvent *);

    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

};

#endif // MAINWINDOW_H
