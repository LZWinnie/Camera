//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :global.h
//        description :全局变量和函数类定义，包含要用到的所有全局变量
//
//        created by LZW at 2018/10/24
//
//======================================================================

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QMutex>
#include <QImage>
#include <opencv2/opencv.hpp>

//类声明
class ballCamera;
class gunCamera;

class Global
{
public:
    Global();

    //枪球变量
    ballCamera *ball;      //球机类成员
    gunCamera *gun;         //枪机类成员

    int lwidth=800;     //图像大小
    int lheight=600;

    //帧率相关
    double ballRate=10;
    double gunRate=10;

    // 枪球矫正
    double verticalSlope = 0.239;
    double verticalIntercept = 49.653;
    double horizonSlope = -0.3101;
    double horizonIntercept = 634.28;

    //互斥锁
    QMutex mutex1,mutex2,mutex3,mutex4,mutex5;

    //Mat转QImage类的函数
    QImage Mat2QImage(const cv::Mat& mat);

    //恢复函数
    //void globalClear();

};

#endif // GLOBAL_H
