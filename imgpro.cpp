//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :imgpro.cpp
//        description :图像处理类实现，包含多个子类各种处理
//
//        created by LZW at 2018/10/21
//
//======================================================================

#include "imgpro.h"
#include "global.h"
#include "ballcamera.h"
#include "guncamera.h"

//互斥锁
extern Global *global;


void ballImgPro::readBallMatSlot(ballCamera* ballc)
{
    //#pragma omp parallel for
    while(ballc->BALL)
    {
        if(ballc->ballCap->isOpened())
        {
            global->mutex1.lock();//互斥锁
            if(!ballc->ballCap->read(ballc->ballSrc))
                break;

            //ballc->ballCap->grab();

            global->mutex1.unlock();//解锁

            cv::waitKey(30);
        }
    }

}

void gunImgPro::readGunMatSlot(gunCamera* gunc)
{
    //#pragma omp parallel for
    while(gunc->GUN)
    {
        if(gunc->gunCap->isOpened())
        {
            global->mutex2.lock();//互斥锁
            if(!gunc->gunCap->read(gunc->gunSrc))
                break;

            //gunc->gunCap->grab();

            global->mutex2.unlock();//解锁

            cv::waitKey(30);
        }
    }

}

void trackImgPro::targetTrack(ballCamera* ballc)
{
    global->mutex3.lock();//balltracker锁
    global->mutex1.lock();//互斥锁
    ballTracker = ballc->ballSrc.clone();
    global->mutex1.unlock();//解锁

    cv::resize(ballTracker,ballTracker,cv::Size(global->lwidth,global->lheight),0,0,cv::INTER_AREA);

    tracker = cv::TrackerKCF::create();//清空
    isInit = tracker->init(ballTracker, box);
    global->mutex3.unlock();

    //#pragma omp parallel for
    while(isInit)
    {
        global->mutex3.lock();//balltracker锁
        global->mutex1.lock();//互斥锁
        ballTracker = ballc->ballSrc.clone();
        global->mutex1.unlock();//解锁

        cv::resize(ballTracker,ballTracker,cv::Size(global->lwidth,global->lheight),0,0,cv::INTER_AREA);

        tracker->update(ballTracker, box);
        cv::rectangle(ballTracker, box, cv::Scalar(255, 0, 0), 2, 1);
        global->mutex3.unlock();//balltracker锁
    }
}
