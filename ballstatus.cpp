//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :ballstatus.cpp
//        description :球机状态类定义,以及一些初始化
//
//        created by LZW at 2018/10/21
//
//======================================================================

#include "ballstatus.h"

ballStatus::ballStatus()
{
    horizontalPosition = 0;     //水平位置
    verticalPosition = 0;       //竖直位置
    zoomFactor = 6;             //缩放倍数
    horizontalSpeed = 0;        //水平移动速度
    verticalSpeed = 0;          //竖直移动速度
    zoomSpeed = 0;              //缩放速度
}
