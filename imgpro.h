//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :imgpro.h
//        description :图像处理类定义，包含各种处理
//
//        created by LZW at 2018/10/21
//
//======================================================================

#ifndef IMGPRO_H
#define IMGPRO_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <QImage>
#include <opencv2/tracking.hpp>

//类声明
class ballCamera;
class gunCamera;

class imgPro : public QObject
{
    Q_OBJECT

public:
    explicit imgPro(QObject *parent = 0){}

//public slots:
    //virtual void targetTrack(ballCamera* ballc);     //目标跟踪
    //virtual void faceRecognition(const cv::Mat& mat);     //人脸识别
};


//球机处理类
class ballImgPro: public QObject
{
    Q_OBJECT

public slots:
    void readBallMatSlot(ballCamera* ballc);     //读取球机视频流

public:
    explicit ballImgPro(QObject *parent = 0){}
    //friend class MainWindow;
};

//枪击处理类
class gunImgPro: public QObject
{
    Q_OBJECT

public slots:
    void readGunMatSlot(gunCamera* gunc);     //读取枪机视频流

public:
    explicit gunImgPro(QObject *parent = 0){}
    //friend class MainWindow;
};


//目标追踪类
class trackImgPro: public QObject
{
    Q_OBJECT

private:
    cv::Ptr<cv::Tracker> tracker = cv::TrackerKCF::create();
    cv::Rect2d box;
    bool isInit = false;        //初始化标记
    cv::Mat ballTracker;        //目标追踪后的处理图像
    QImage ballTrackerImg;

public slots:
    void targetTrack(ballCamera* ballc);

public:
    friend class Track;
};

#endif // IMGPRO_H
