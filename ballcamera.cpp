//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :ballcamera.cpp
//        description :球机类实现，包含各种球机操作
//
//        created by LZW at 2018/10/21
//
//======================================================================

#include "ballcamera.h"
#include "ballstatus.h"
#include "dhparameter.h"
#include <QDebug>

//构造函数

ballCamera::ballCamera()       //构造函数
{
    ballParameter = new dhParameter;
    ballControl = new ballStatus;
    ballCap = new cv::VideoCapture;
    BALL = false;
    ballSrc = cv::Mat::Mat();
    ballImg = QImage();
}

ballCamera::~ballCamera()      //析构函数
{
    delete ballParameter;
    delete ballControl;
    delete ballCap;
}


//初始化部分

void ballCamera::ballStatusGet()        //获得当前状态
{
    CLIENT_QueryDevState(ballParameter->lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&ballParameter->status,sizeof(ballParameter->status),&ballParameter->statuslen,3000);
    //赋值给相应变量
    ballControl->horizontalPosition = ballParameter->status.nPTZPan;
    ballControl->verticalPosition = ballParameter->status.nPTZTilt;
    ballControl->zoomFactor = ballParameter->status.nPTZZoom;
}

void ballCamera::loginBallCamera()      //登录函数
{
    ballParameter->lLoginHandle = CLIENT_LoginEx2(ballParameter->szDevIp, ballParameter->nPort, "admin", "123456", EM_LOGIN_SPEC_CAP_TCP,NULL,NULL, &ballParameter->nError);

    if(!ballParameter->lLoginHandle)
    {
        emit loginFail();           //登录失败
        return;
    }
    else
    {
        //emit loginSuccess();        //登录成功
        ballCap->open("rtsp://admin:123456@lab.zhuzhuguowang.cn:36955/cam/realmonitor?channel=1&subtype=0");    //连接球机
        if(ballCap->isOpened())
            BALL = true;            //读取成功
        else
            return;

        ballParameter->nError = 0;
        //查询系统能力信息
        if (FALSE==CLIENT_QueryNewSystemInfo(ballParameter->lLoginHandle, ballParameter->szCommand, 0, ballParameter->szBuffer, (DWORD)sizeof(ballParameter->szBuffer), &ballParameter->nError))
        {
            int nErrorNum1 = CLIENT_GetLastError()&(0x7fffffff);
            qDebug()<< "ErrorNum1:"<<nErrorNum1;
            return;
        }

        //解析查询到的配置信息
        if (FALSE==CLIENT_ParseData(ballParameter->szCommand, ballParameter->szBuffer, &ballParameter->stuPtzCapsInfo, sizeof(ballParameter->stuPtzCapsInfo), NULL))
        {
            int nErrorNum2 = CLIENT_GetLastError()&(0x7fffffff);
            qDebug()<< "ErrorNum2:"<<nErrorNum2;
            return;
        }
        ballStatusGet();
    }
}

//到达当前参数位置
void ballCamera::goNow()
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_EXTPTZ_EXACTGOTO,ballControl->horizontalPosition,ballControl->verticalPosition,ballControl->zoomFactor,FALSE,NULL))
    {
        emit moveFail();
    }
    ballStatusGet();
}

//云台移动函数

void ballCamera::verticalSpeedSet(int speed)    //垂直速度设置
{
    ballControl->verticalSpeed = speed;
}

void ballCamera::horizontalSpeedSet(int speed)   //水平速度设置
{
    ballControl->horizontalSpeed = speed;
}

void ballCamera::upStart()  //开始向上移动
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_UP_CONTROL,0,ballControl->verticalSpeed,0,FALSE,NULL))
    {
        emit moveFail();
    }

    ballStatusGet();
}

void ballCamera::downStart()    //开始向下移动
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_DOWN_CONTROL,0,ballControl->verticalSpeed,0,FALSE,NULL))
    {
        emit moveFail();
    }

    ballStatusGet();
}

void ballCamera::leftStart()    //开始向左移动
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_LEFT_CONTROL,0,ballControl->horizontalSpeed,0,FALSE,NULL))
    {
        emit moveFail();
    }

    ballStatusGet();
}

void ballCamera::rightStart()   //开始向右移动
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_RIGHT_CONTROL,0,ballControl->horizontalSpeed,0,FALSE,NULL))
    {
        emit moveFail();
    }

    ballStatusGet();
}


void ballCamera::upStop()   //停止向上移动
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_UP_CONTROL,0,ballControl->verticalSpeed,0,TRUE,NULL))
    {
        emit moveFail();
    }

    ballStatusGet();
}

void ballCamera::downStop()  //停止向下移动
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_DOWN_CONTROL,0,ballControl->verticalSpeed,0,TRUE,NULL))
    {
        emit moveFail();
    }

    ballStatusGet();
}

void ballCamera::leftStop()     //停止向左移动
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_LEFT_CONTROL,0,ballControl->horizontalSpeed,0,TRUE,NULL))
    {
        emit moveFail();
    }

    ballStatusGet();
}

void ballCamera::rightStop()    //停止向右移动
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_RIGHT_CONTROL,0,ballControl->horizontalSpeed,0,TRUE,NULL))
    {
        emit moveFail();
    }

    ballStatusGet();
}


void ballCamera::resetPosition()    //恢复默认位置
{
    ballControl->horizontalPosition = 0;
    ballControl->verticalPosition = 0;

    goNow();
}

void ballCamera::goPosition(int horizon, int vertical)  //精确定位
{
    ballControl->horizontalPosition = horizon;
    ballControl->verticalPosition = vertical;

    goNow();
}


//摄像头缩放函数
void ballCamera::zoomSpeedSet(int speed)    //缩放速度设置
{
    ballControl->zoomSpeed = speed;
}

void ballCamera::plusStart()    //开始放大
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_ZOOM_ADD_CONTROL,0,ballControl->zoomSpeed,0,FALSE,NULL))
    {
        emit zoomFail();
    }

    ballStatusGet();
}

void ballCamera::minusStart()   //开始缩小
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_ZOOM_DEC_CONTROL,0,ballControl->zoomSpeed,0,FALSE,NULL))
    {
        emit zoomFail();
    }

    ballStatusGet();
}


void ballCamera::plusStop()     //停止放大
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_ZOOM_ADD_CONTROL,0,ballControl->zoomSpeed,0,TRUE,NULL))
    {
        emit zoomFail();
    }

    ballStatusGet();
}

void ballCamera::minusStop()    //停止缩小
{
    if(FALSE==CLIENT_DHPTZControlEx2(ballParameter->lLoginHandle,0,DH_PTZ_ZOOM_DEC_CONTROL,0,ballControl->zoomSpeed,0,TRUE,NULL))
    {
        emit zoomFail();
    }

    ballStatusGet();
}

void ballCamera::resetZoom()    //恢复默认放大倍数
{
    ballControl->zoomFactor = 6;

    goNow();
}

void ballCamera::goZoom(int factor) //精确缩放
{
    ballControl->zoomFactor = factor;

    goNow();
}
