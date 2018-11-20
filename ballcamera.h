//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :ballcamera.h
//        description :球机类定义，包含各种球机操作
//
//        created by LZW at 2018/10/21
//
//======================================================================

#ifndef BALLCAMERA_H
#define BALLCAMERA_H

#include <opencv2/opencv.hpp>
#include <QObject>
#include <QImage>

//类声明
class dhParameter;
class ballStatus;

class ballCamera : public QObject
{
    Q_OBJECT

private:
    dhParameter *ballParameter;                 //球机需要的大华参数
    ballStatus *ballControl;                    //球机的坐标等控制参数
    cv::VideoCapture *ballCap;                      //读取球机的视频流类
    bool BALL;                                  //球机能否读取标志
    cv::Mat ballSrc;                                //通过VideoCapture类读入进来的Mat类的一帧图像
    QImage ballImg;                             //通过Mat2QImage函数转换成的QImage类的一帧图像

signals:
    void loginFail();                           //登录失败信号
    //void loginSuccess();                        //登录成功信号
    void moveFail();                            //移动失败信号
    void zoomFail();                            //缩放失败信号


public:
    ballCamera();                               //构造函数
    ~ballCamera();                              //析构函数

    //初始化及状态函数
    void ballStatusGet();                       //获得当前状态
    void loginBallCamera();                     //球机登录函数

    //到达当前内部参数位置
    void goNow();

    //云台移动函数
    void verticalSpeedSet(int speed);           //垂直速度设置
    void horizontalSpeedSet(int speed);         //水平速度设置

    void upStart();                             //开始向上移动
    void downStart();                           //开始向下移动
    void leftStart();                           //开始向左移动
    void rightStart();                          //开始向右移动

    void upStop();                              //停止向上移动
    void downStop();                            //停止向下移动
    void leftStop();                            //停止向左移动
    void rightStop();                           //停止向右移动

    void resetPosition();                       //恢复默认位置
    void goPosition(int horizon, int vertical); //精确定位

    //摄像头缩放函数
    void zoomSpeedSet(int speed);               //缩放速度设置

    void plusStart();                           //开始放大
    void minusStart();                          //开始缩小

    void plusStop();                            //停止放大
    void minusStop();                           //停止缩小

    void resetZoom();                           //恢复默认放大倍数
    void goZoom(int factor);                    //精确缩放

    //友元类
    friend class MainWindow;
    friend class ballImgPro;
    friend class trackImgPro;
    friend class Track;

};


#endif // BALLCAMERA_H
