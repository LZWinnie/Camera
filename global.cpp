//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :global.cpp
//        description :全局变量和函数类实现，包含要用到的所有全局变量
//
//        created by LZW at 2018/10/24
//
//======================================================================

#include "global.h"
#include "ballcamera.h"
#include "guncamera.h"
#include <QDebug>

Global::Global()
{
    ball = new ballCamera;
    gun = new gunCamera;
}

QImage Global::Mat2QImage(const cv::Mat& mat)
{
    if(mat.type() == CV_8UC3)
    {
        const uchar *pSrc = (const uchar*)mat.data;//复制输入
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);//创造QImage
        return image.rgbSwapped();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}
/*
void Global::globalClear()
{
    ball = new ballCamera;
    gun = new gunCamera;
}*/

Global *global;
