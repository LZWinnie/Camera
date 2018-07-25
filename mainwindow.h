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

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    //QImage Mat2QImage(const cv::Mat& mat);//——v1.0

protected:
    void closeEvent(QCloseEvent *);

signals:
    void startBallCamera();//——v1.0
    void startGunCamera();//——v1.3

private slots:
    //void startCameraSlot();//——v1.0
    //void getFrameSlot();//——v1.0

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

    //void getSizeSlot();//——v1.7
    void tabChangeSlot();//——v1.8

protected:
    void contextMenuEvent(QContextMenuEvent *event);//——v1.7

    void keyPressEvent(QKeyEvent *event);//——v1.9
    void keyReleaseEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);//——v1.9

private:
    Ui::MainWindow *ui;
    QTimer *timer1,*timer2;

};
//为了多线程后台处理图片，定义一个新类——v1.1
class ImgPro : public QObject
{
    Q_OBJECT

public:
    explicit ImgPro(QObject *parent = 0){}
    QImage Mat2QImage(const cv::Mat& mat);
signals:
    //void showball();//——v1.6
    //void showgun();
    void getBall();
    void getGun();

public slots:
    void readBallSlot();//——v1.6
    void readGunSlot();
    void getBallImageSlot();//——v1.1
    void getGunImageSlot();//——v1.3
};

#endif // MAINWINDOW_H
