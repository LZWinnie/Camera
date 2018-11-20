//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :guncamera.h
//        description :枪机类定义，包含各种球机操作
//
//        created by LZW at 2018/10/21
//
//======================================================================

#ifndef GUNCAMERA_H
#define GUNCAMERA_H

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QImage>

class gunCamera : public QObject
{
    Q_OBJECT

private:
    cv::VideoCapture *gunCap;                  //读取枪机的视频流类
    bool GUN;                                  //枪机能否读取标志
    cv::Mat gunSrc;                            //通过VideoCapture类读入进来的Mat类的一帧图像
    QImage gunImg;                             //通过Mat2QImage函数转换成的QImage类的一帧图像

public:
    gunCamera();
    ~gunCamera();

    void loginGunCamera();          //获取枪机画面

    //友元类
    friend class MainWindow;
    friend class gunImgPro;

};

#endif // GUNCAMERA_H
