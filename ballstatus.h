//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :ballstatus.h
//        description :球机状态类定义
//
//        created by LZW at 2018/10/21
//
//======================================================================

#ifndef BALLSTATUS_H
#define BALLSTATUS_H

class ballStatus
{
private:
    int horizontalPosition;     //水平位置
    int verticalPosition;       //竖直位置
    int zoomFactor;             //缩放倍数
    int horizontalSpeed;        //水平移动速度
    int verticalSpeed;          //竖直移动速度
    int zoomSpeed;              //缩放速度

public:
    ballStatus();               //构造函数
    friend class ballCamera;    //友元类
    friend class MainWindow;
};

#endif // BALLSTATUS_H
