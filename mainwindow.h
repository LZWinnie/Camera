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
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <iostream>
#include <cstring>
#include <QRect>
#include "help.h"

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)
#define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str();

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QThread imgproThread1;//用于后台读取
    QThread imgproThread2;
    QThread imgtracker;//用于追踪处理

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QImage Mat2QImage(const cv::Mat& mat);
    Help *help;

signals:
    void startBallCamera();
    void startGunCamera();

    void startBallTrack();

private slots:
    void loginSlot();
    void logoutSlot();

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

    void showBallSlot();
    void showGunSlot();

    void plusSlot();
    void plusStopSlot();
    void minusSlot();
    void minusStopSlot();
    void zoomSpeedSlot();

    void resetSlot();
    void goSlot();
    void clearSlot();

    void zoomSlot();
    void rezoomSlot();
    void clear2Slot();

    void gunVisibleSlot();

    void ballSelectSlot();

    void rectifySlot();

    void upActionSlot();
    void downActionSlot();
    void leftActionSlot();
    void rightActionSlot();

    void plusActionSlot();
    void minusActionSlot();

    void helpSlot();


protected:
    void closeEvent(QCloseEvent *);

    void contextMenuEvent(QContextMenuEvent *);

    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

    void mousePressEvent(QMouseEvent *);

    void mouseReleaseEvent(QMouseEvent *);

private:
    Ui::MainWindow *ui;
    QTimer *timer1,*timer2;

};


//为了多线程后台处理图片，定义一个新类
class ImgPro : public QObject
{
    Q_OBJECT

public:
    explicit ImgPro(QObject *parent = 0){}

signals:
    void getBall();
    void getGun();

public slots:
    void readBallSlot();
    void readGunSlot();

    void ballTrackSlot();
};

//用于存储用于枪球矫正的点
class x_y
{
public:
    x_y(double x, double y) :x(x), y(y) {}
    x_y() :x(0), y(0) {}
    double x;
    double y;
};

#endif // MAINWINDOW_H
