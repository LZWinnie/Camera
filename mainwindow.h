#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <QDebug>
#include <QTimer>
#include <QPixmap>
#include <QThread>
#include <QCloseEvent>
#include "dhnetsdk.h"
#include "dhconfigsdk.h"
#include <QSlider>
#include <QString>
#include <QLabel>
#include <QList>
#include <QMutex>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QPoint>
#include <QKeyEvent>
#include <QMouseEvent>
#include <opencv2/tracking.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <cstring>
#include <QPainter>
#include <QRect>
#include <QPaintDevice>

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)
#define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str();

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QThread imgproThread1;//用于后台读取——v1.6
    QThread imgproThread2;

    QThread imgproThread3;//用于后台处理——v1.1
    QThread imgproThread4;//用于后台处理枪机——v1.3

    QThread imgRect;//用于画框
    QThread imgtracker;//用于追踪处理

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QImage Mat2QImage(const cv::Mat& mat);

signals:
    void startBallCamera();//——v1.0
    void startGunCamera();//——v1.3

    //void paintBall();

    //void startBallTrack();

private slots:
    void loginSlot();//——v1.2
    void logoutSlot();//——v1.2

    void upSlot();
    void downSlot();
    void leftSlot();
    void rightSlot();

    void upStopSlot();
    void downStopSlot();
    void leftStopSlot();
    void rightStopSlot();

    void verticalSpeedSlot();
    void horizontalSpeedSlot();

    void showBallSlot();//——v1.1
    void showGunSlot();//——v1.3

    void plusSlot();//——v1.4
    void plusStopSlot();
    void minusSlot();
    void minusStopSlot();
    void zoomSpeedSlot();

    void resetSlot();//——v1.5
    void goSlot();
    void clearSlot();

    void zoomSlot();
    void rezoomSlot();
    void clear2Slot();

    void gunVisibleSlot();//——v1.10

    void ballSelectSlot();


protected:
    void closeEvent(QCloseEvent *);

    void contextMenuEvent(QContextMenuEvent *);//——v1.7

    void keyPressEvent(QKeyEvent *);//——v1.9
    void keyReleaseEvent(QKeyEvent *);

    void mousePressEvent(QMouseEvent *);//——v1.9
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    //void paintEvent(QPaintEvent *);

private:
    Ui::MainWindow *ui;
    QTimer *timer1,*timer2;
    QPainter painter;

};
//为了多线程后台处理图片，定义一个新类——v1.1
class ImgPro : public QObject
{
    Q_OBJECT

public:
    explicit ImgPro(QObject *parent = 0){}

signals:
    //void showball();//——v1.6
    //void showgun();
    void getBall();
    void getGun();

public slots:
    void readBallSlot();//——v1.6
    void readGunSlot();

    //void getBallImageSlot();//——v1.1
    //void getGunImageSlot();//——v1.3

    //void paintBallSlot();
    void ballTrackSlot();
};

#endif // MAINWINDOW_H
