//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :guncamera.cpp
//        description :枪机类实现，包含各种球机操作
//
//        created by LZW at 2018/10/21
//
//======================================================================

#include "guncamera.h"

gunCamera::gunCamera()      //构造函数
{
    gunCap = new cv::VideoCapture;
    GUN = false;
    gunSrc = cv::Mat::Mat();
    gunImg = QImage();
}

gunCamera::~gunCamera()     //析构函数
{
    delete gunCap;
}

void gunCamera::loginGunCamera()         //枪机图像获取
{
    gunCap->open("rtsp://admin:123456@lab.zhuzhuguowang.cn:36958/cam/realmonitor?channel=1&subtype=0");    //连接枪机
    if(gunCap->isOpened())
        GUN = true;            //读取成功
    else
        return;
}
