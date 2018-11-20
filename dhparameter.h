//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :dhparameter.h
//        description :大华参数类定义，包含一些变量的初始化
//
//        created by LZW at 2018/10/21
//
//======================================================================

#ifndef DHPARAMETER_H
#define DHPARAMETER_H

#include "dhnetsdk.h"
#include "dhconfigsdk.h"
#include <cstring>

using namespace std;

class dhParameter
{

private:
    //预设登录信息
    char szDevIp[64] = {"lab.zhuzhuguowang.cn"};
    char szBuffer[2048] = "";
    char szCommand[32] = "ptz.getCurrentProtocolCaps";

    CFG_PTZ_PROTOCOL_CAPS_INFO stuPtzCapsInfo = {sizeof(CFG_PTZ_PROTOCOL_CAPS_INFO)};
    int nError = 0;
    int nPort = 36956;
    LLONG lLoginHandle = 0L;

    //摄像头状态
    DH_PTZ_LOCATION_INFO status;
    int statuslen = 0;

public:
    friend class ballCamera;    //友元类
    friend class MainWindow;

};

#endif // DHPARAMETER_H
