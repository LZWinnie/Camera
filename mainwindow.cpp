#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;
using namespace cv;

//进行图像操作的变量——v1.0
//增加枪机变量——v1.3
Mat ballImg,gunImg;
VideoCapture *ballCap=new VideoCapture,*gunCap=new VideoCapture;
QImage ballImage,gunImage;
//QList<QImage> *list=new QList<QImage>;

//预设登录信息——v1.2
char szDevIp[64] = {"lab.zhuzhuguowang.cn"};
NET_DEVICEINFO stDevInfo = {0};
int nError = 0;
int nPort = 36956;
static LLONG lLoginHandle = 0L;

//摄像头状态——v1.5
DH_PTZ_LOCATION_INFO status;
int statuslen=0;

//坐标和变倍——v1.5
int horizon,vertical;
int zoom;

//帧率相关——v1.6
double ballRate=15;
double gunRate=30;
//图像处理临时变量
QImage ballshow,gunshow;
Mat ball,gun;

//互斥锁——v1.6
QMutex mutex1,mutex2,mutex3,mutex4;
QMutex mutex01,mutex02;

//线程循环控制变量——v1.8
bool BALL,GUN;

//窗口大小——v1.7
int lwidth=800;
int lheight=600;

//构造函数——v1.0
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer1 = new QTimer(this);
    timer2 = new QTimer(this);
    //QObject::connect(ui->loginButton,SIGNAL(clicked()),this,SLOT(startCameraSlot()));//——v1.0打开摄像头
    QObject::connect(ui->loginButton,SIGNAL(clicked()),this,SLOT(loginSlot()));//——v1.2
    QObject::connect(ui->logoutButton,SIGNAL(clicked()),this,SLOT(logoutSlot()));//——v1.2

    //QObject::connect(timer,SIGNAL(timeout()),this,SLOT(getFrameSlot()));//——v1.0
    QObject::connect(timer1,SIGNAL(timeout()),this,SLOT(showBallSlot()));//——v1.1
    QObject::connect(timer2,SIGNAL(timeout()),this,SLOT(showGunSlot()));//——v1.1

    //读取视频流进程——v1.6
    ImgPro *imgpro1 = new ImgPro;
    imgpro1->moveToThread(&imgproThread1);
    imgproThread1.start();
    QObject::connect(this,SIGNAL(startBallCamera()),imgpro1,SLOT(readBallSlot()));
    ImgPro *imgpro2 = new ImgPro;
    imgpro2->moveToThread(&imgproThread2);
    imgproThread2.start();
    QObject::connect(this,SIGNAL(startGunCamera()),imgpro2,SLOT(readGunSlot()));

    //处理图片进程——v1.6
    ImgPro *imgpro3 = new ImgPro;
    imgpro3->moveToThread(&imgproThread3);// 调用moveToThread将该任务交割imgproThread——v1.1
    imgproThread3.start();//启动线程——v1.1
    //QObject::connect(this,SIGNAL(startBallCamera()),imgpro1,SLOT(getBallImageSlot()));//——v1.1
    QObject::connect(imgpro1,SIGNAL(getBall()),imgpro3,SLOT(getBallImageSlot()));
    ImgPro *imgpro4 = new ImgPro;
    imgpro4->moveToThread(&imgproThread4);// 调用moveToThread将该任务交割imgproThread——v1.1
    imgproThread4.start();//启动线程——v1.1
    //QObject::connect(this,SIGNAL(startGunCamera()),imgpro2,SLOT(getGunImageSlot()));//——v1.1
    QObject::connect(imgpro2,SIGNAL(getGun()),imgpro4,SLOT(getGunImageSlot()));

    /*//定时器控制处理，发射一个信号驱动现实——v1.6
    QObject::connect(timer1,SIGNAL(timeout()),imgpro1,SLOT(getBallImageSlot()));
    QObject::connect(timer2,SIGNAL(timeout()),imgpro2,SLOT(getGunImageSlot()));
    QObject::connect(imgpro1,SIGNAL(showball()),this,SLOT(showBallSlot()));
    QObject::connect(imgpro2,SIGNAL(showgun()),this,SLOT(showGunSlot()));*/

    //窗口和状态栏——v1.5
    MainWindow::setWindowTitle("Camera  Control  System");
    QLabel *copyright = new QLabel(this);
    copyright->setText("Copyright   ©   LAB   369");
    ui->statusBar->addPermanentWidget(copyright);
    //速度滑块设置——v1.2
    ui->verticalSpeedSlider->setMaximum(8);
    ui->verticalSpeedSlider->setMinimum(0);
    ui->verticalSpeedSlider->setTickPosition(QSlider::TicksAbove);
    ui->horizontalSpeedSlider->setMaximum(8);
    ui->horizontalSpeedSlider->setMinimum(0);
    ui->horizontalSpeedSlider->setTickPosition(QSlider::TicksAbove);
    //速度控制——v1.2
    QObject::connect(ui->verticalSpeedSlider,SIGNAL(valueChanged(int)),this,SLOT(verticalSpeedSlot()));
    QObject::connect(ui->horizontalSpeedSlider,SIGNAL(valueChanged(int)),this,SLOT(horizontalSpeedSlot()));
    //云台控制——v1.2
    QObject::connect(ui->upButton,SIGNAL(pressed()),this,SLOT(upSlot()));
    QObject::connect(ui->downButton,SIGNAL(pressed()),this,SLOT(downSlot()));
    QObject::connect(ui->leftButton,SIGNAL(pressed()),this,SLOT(leftSlot()));
    QObject::connect(ui->rightButton,SIGNAL(pressed()),this,SLOT(rightSlot()));
    QObject::connect(ui->upButton,SIGNAL(released()),this,SLOT(upStopSlot()));
    QObject::connect(ui->downButton,SIGNAL(released()),this,SLOT(downStopSlot()));
    QObject::connect(ui->leftButton,SIGNAL(released()),this,SLOT(leftStopSlot()));
    QObject::connect(ui->rightButton,SIGNAL(released()),this,SLOT(rightStopSlot()));
    //变倍滑块设置——v1.4
    ui->zoomSpeedSlider->setMaximum(1);
    ui->zoomSpeedSlider->setMinimum(0);
    ui->zoomSpeedSlider->setTickPosition(QSlider::TicksAbove);
    QObject::connect(ui->zoomSpeedSlider,SIGNAL(valueChanged(int)),this,SLOT(zoomSpeedSlot()));
    //变倍控制——v1.4
    QObject::connect(ui->plusButton,SIGNAL(pressed()),this,SLOT(plusSlot()));
    QObject::connect(ui->minusButton,SIGNAL(pressed()),this,SLOT(minusSlot()));
    QObject::connect(ui->plusButton,SIGNAL(released()),this,SLOT(plusStopSlot()));
    QObject::connect(ui->minusButton,SIGNAL(released()),this,SLOT(minusStopSlot()));
    //控制坐标移动——v1.5
    QObject::connect(ui->resetButton,SIGNAL(clicked()),this,SLOT(resetSlot()));
    QObject::connect(ui->goButton,SIGNAL(clicked()),this,SLOT(goSlot()));
    QObject::connect(ui->clearButton,SIGNAL(clicked()),this,SLOT(clearSlot()));
    QObject::connect(ui->rezoomButton,SIGNAL(clicked()),this,SLOT(rezoomSlot()));
    QObject::connect(ui->zoomButton,SIGNAL(clicked()),this,SLOT(zoomSlot()));
    QObject::connect(ui->clearButton2,SIGNAL(clicked()),this,SLOT(clear2Slot()));

    //获得图片大小——v1.7_——会卡死
    //QObject::connect(this,SIGNAL(startBallCamera()),this,SLOT(getSizeSlot()));

    //标签页切换——v1.8
    QObject::connect(ui->tabWidget,SIGNAL(tabBarClicked(int)),this,SLOT(tabChangeSlot()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
/*//打开摄像头——v1.0
//更改为登录loginSlot——v1.2
void MainWindow::startCameraSlot()
{
    cap->open("rtsp://admin:123456@192.168.73.110:36955/cam/realmonitor?channel=1&subtype=0");//连接摄像头
    emit startCamera();//发射一个摄像头开启信号
    timer->start(1);//开启定时器
    ui->statusBar->showMessage("Login!", 5000);//状态栏显示
}*/
//登陆函数——v1.2
void MainWindow::loginSlot()
{
    lLoginHandle = CLIENT_LoginEx2(szDevIp, nPort, "admin", "123456", EM_LOGIN_SPEC_CAP_TCP,NULL,NULL, &nError);
    if(!lLoginHandle)//登录失败
    {
        ui->statusBar->showMessage("Login Fail!",2000);
    }
    else  // 登录成功
    {
        ui->statusBar->showMessage("Login Success!",1000);
        ui->tabWidget->setCurrentIndex(0);//首先显示球机窗口
        ballCap->open("rtsp://admin:123456@lab.zhuzhuguowang.cn:36955/cam/realmonitor?channel=1&subtype=0");//连接摄像头
        gunCap->open("rtsp://admin:123456@lab.zhuzhuguowang.cn:36958/cam/realmonitor?channel=1&subtype=0");//连接枪机摄像头
        BALL=true;
        GUN=false;
        emit startBallCamera();//发射一个摄像头开启信号
        //mutex02.lock();
        //emit startGunCamera();//发射一个枪机摄像头开启信号
        //ballRate=ballCap->get(CV_CAP_PROP_FPS);
        //gunRate=gunCap->get(CV_CAP_PROP_FPS);
        //qDebug()<<gunRate;
        timer1->start(1000/ballRate);//开启定时器
        timer2->start(1000/gunRate);//开启定时器
        char szBuffer[2048] = "";
        int nError = 0;
        char p[]="ptz.getCurrentProtocolCaps";//将const char*转换为char*，否则下面的函数会报错
        if (FALSE==CLIENT_QueryNewSystemInfo(lLoginHandle, p, 0, szBuffer, (DWORD)sizeof(szBuffer), &nError))
        {
            ui->statusBar->showMessage("CLIENT_QueryNewSystemInfo Failed!",3000);
            int nErrorNum1 = CLIENT_GetLastError()&(0x7fffffff);
            qDebug()<< "ErrorNum1:"<<nErrorNum1;
            return;
        }
        //ui->statusBar->showMessage("CLIENT_QueryNewSystemInfo Success!",5000);
        CFG_PTZ_PROTOCOL_CAPS_INFO stuPtzCapsInfo = {sizeof(CFG_PTZ_PROTOCOL_CAPS_INFO)};
        if (FALSE==CLIENT_ParseData(p, szBuffer, &stuPtzCapsInfo, sizeof(stuPtzCapsInfo), NULL))
        {
            ui->statusBar->showMessage("CLIENT_ParseData Failed!",3000);
            int nErrorNum2 = CLIENT_GetLastError()&(0x7fffffff);
            qDebug()<< "ErrorNum2:"<<nErrorNum2;
            return;
        }
        //ui->statusBar->showMessage("CLIENT_ParseData Success!",1000);
        //显示当前坐标和变倍——v1.5
        CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
        vertical=status.nPTZTilt;
        horizon=status.nPTZPan;
        zoom=status.nPTZZoom;
        ui->verticalLineEdit->setText(QString::number(vertical,10));
        ui->horizontalLineEdit->setText(QString::number(horizon,10));
        ui->zoomLineEdit->setText(QString::number(zoom,10));
    }
}
//登出函数——v1.2
void MainWindow::logoutSlot()
{
    //为了防止程序意外停止（在读视频流的地方），所以要检测当所有进程都解锁了之后，也就是可以被logout锁住之后，再release视频流
    //while(!mutex1.tryLock());
    //while(!mutex2.tryLock());
    //while(!mutex3.tryLock());
    //while(!mutex4.tryLock());
    BALL=false;
    //ballCap->release();//释放cap以便于停止线程
    imgproThread1.quit();
    //imgproThread1.wait();听说只quit停不下来，要wait一下，但是wait之后就卡死了
    timer1->stop();//退出登录
    GUN=false;
    //gunCap->release();//释放cap以便于停止线程
    imgproThread2.quit();
    //imgproThread2.wait();
    timer2->stop();//退出登录
    imgproThread3.quit();
    //imgproThread3.wait();
    imgproThread4.quit();
    //imgproThread4.wait();
    CLIENT_Logout(lLoginHandle);
    CLIENT_Cleanup();
    ui->statusBar->showMessage("Logout!",2000);
}

/*//在界面上显示图像——v1.0
//由于会影响速度，所以将其通过QThread移动至其他线程后台处理——v1.1
void MainWindow::getFrameSlot()
{
    if(cap->isOpened())//如果连接成功
    {
        cap->read(img);//读一帧
        cv::resize(img,img,Size(800,590),0,0,INTER_AREA);
        image=Mat2QImage(img);//转换为QImage类
        ui->showWindowLabel->setPixmap(QPixmap::fromImage(image));//在界面上显示
    }
    else//如果连接失败
    {
        qDebug()<<"ERROR: Can't open the camera";
        return;
    }
}
*/
//显示图像——v1.1
void MainWindow::showBallSlot()
{
    mutex3.lock();
    ballshow=ballImage.copy();
    mutex3.unlock();
    ui->ballWindowLabel->setPixmap(QPixmap::fromImage(ballshow));//在界面上显示
    return;
}
//显示枪机图像——v1.3
void MainWindow::showGunSlot()
{
    mutex4.lock();
    gunshow=gunImage.copy();
    mutex4.unlock();
    ui->gunWindowLabel->setPixmap(QPixmap::fromImage(gunshow));//在界面上显示
    return;
}

//关闭事件——v1.1
void MainWindow::closeEvent(QCloseEvent *event)
{
    ballCap->release();//释放cap以便于停止线程
    gunCap->release();//释放cap以便于停止线程
    if(imgproThread1.isRunning())
    {
        imgproThread1.quit();//退出线程
        qDebug()<<"Thread1 is running!";
    }
    if(imgproThread2.isRunning())
    {
        imgproThread2.quit();//退出线程
        qDebug()<<"Thread2 is running!";
    }
    if(imgproThread3.isRunning())
    {
        imgproThread3.quit();//退出线程
        qDebug()<<"Thread3 is running!";
    }
    if(imgproThread4.isRunning())
    {
        imgproThread4.quit();//退出线程
        qDebug()<<"Thread4 is running!";
    }
    //目前的情况是所有的线程都没有被停止，所以会报错Runtime XXX，但是不会意外停止了
    event->accept();//接收事件
}

//控制摄像头移动——v1.2
int verticalSpeed,horizontalSpeed;
//垂直速度和水平速度——v1.2
void MainWindow::verticalSpeedSlot()
{
    verticalSpeed=ui->verticalSpeedSlider->value();
    ui->statusBar->showMessage("Vertical speed changed!",3000);
}
void MainWindow::horizontalSpeedSlot()
{
    horizontalSpeed=ui->horizontalSpeedSlider->value();
    ui->statusBar->showMessage("Horizontal speed changed!",3000);
}
//上下左右的启动和停止——v1.2
void MainWindow::upSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_UP_CONTROL,0,verticalSpeed,0,FALSE,NULL))
    {
        ui->statusBar->showMessage("Up Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
void MainWindow::downSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_DOWN_CONTROL,0,verticalSpeed,0,FALSE,NULL))
    {
        ui->statusBar->showMessage("Down Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
void MainWindow::leftSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_LEFT_CONTROL,0,horizontalSpeed,0,FALSE,NULL))
    {
        ui->statusBar->showMessage("Left Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
void MainWindow::rightSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_RIGHT_CONTROL,0,horizontalSpeed,0,FALSE,NULL))
    {
        ui->statusBar->showMessage("Right Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
void MainWindow::upStopSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_UP_CONTROL,0,verticalSpeed,0,TRUE,NULL))
    {
        ui->statusBar->showMessage("Up Stop Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
void MainWindow::downStopSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_DOWN_CONTROL,0,verticalSpeed,0,TRUE,NULL))
    {
        ui->statusBar->showMessage("Down Stop Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
void MainWindow::leftStopSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_LEFT_CONTROL,0,horizontalSpeed,0,TRUE,NULL))
    {
        ui->statusBar->showMessage("Left Stop Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
void MainWindow::rightStopSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_RIGHT_CONTROL,0,horizontalSpeed,0,TRUE,NULL))
    {
        ui->statusBar->showMessage("Right Stop Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}

//控制变倍——v1.4
int zoomSpeed;
//变倍速度——v1.4
void MainWindow::zoomSpeedSlot()
{
    zoomSpeed=ui->zoomSpeedSlider->value();
    ui->statusBar->showMessage("Zoom speed changed!",3000);
}
//变倍的启动和停止——v1.4
void MainWindow::plusSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_ZOOM_ADD_CONTROL,0,zoomSpeed,0,FALSE,NULL))
    {
        ui->statusBar->showMessage("Plus Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
void MainWindow::minusSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_ZOOM_DEC_CONTROL,0,zoomSpeed,0,FALSE,NULL))
    {
        ui->statusBar->showMessage("Minus Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
void MainWindow::plusStopSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_ZOOM_ADD_CONTROL,0,zoomSpeed,0,TRUE,NULL))
    {
        ui->statusBar->showMessage("Plus Stop Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
void MainWindow::minusStopSlot()
{
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_PTZ_ZOOM_DEC_CONTROL,0,zoomSpeed,0,TRUE,NULL))
    {
        ui->statusBar->showMessage("Minus Stop Fail!",2000);
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}

//控制坐标移动——v1.5
//坐标复位
void MainWindow::resetSlot()
{
    vertical=0;//坐标复位为0
    horizon=0;
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_EXTPTZ_EXACTGOTO,horizon,vertical,zoom,FALSE,NULL))
    {
        ui->statusBar->showMessage("Reset Fail!",2000);
    }
    //显示当前坐标和变倍——v1.5
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
//精确定位
void MainWindow::goSlot()
{
    vertical=ui->verticalLineEdit->text().toInt();//读取坐标
    horizon=ui->horizontalLineEdit->text().toInt();
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_EXTPTZ_EXACTGOTO,horizon,vertical,zoom,FALSE,NULL))
    {
        ui->statusBar->showMessage("Go Fail!",2000);
    }
    //显示当前坐标和变倍——v1.5
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
//清空对话框
void MainWindow::clearSlot()
{
    ui->verticalLineEdit->clear();
    ui->horizontalLineEdit->clear();
}
//变倍复位
void MainWindow::rezoomSlot()
{
    zoom=6;
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_EXTPTZ_EXACTGOTO,horizon,vertical,zoom,FALSE,NULL))
    {
        ui->statusBar->showMessage("Rezoom Fail!",2000);
    }
    //显示当前坐标和变倍——v1.5
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
//控制变倍
void MainWindow::zoomSlot()
{
    zoom=ui->zoomLineEdit->text().toInt();
    if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_EXTPTZ_EXACTGOTO,horizon,vertical,zoom,FALSE,NULL))
    {
        ui->statusBar->showMessage("Zoom Fail!",2000);
    }
    //显示当前坐标和变倍——v1.5
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}
//清空对话框2
void MainWindow::clear2Slot()
{
    ui->zoomLineEdit->clear();
}

/*//获得窗口大小——v1.7会卡死还是算了
void MainWindow::getSizeSlot()
{
    while(lLoginHandle)
    {
        lwidth=ui->ballWindowLabel->width();
        lheight=ui->ballWindowLabel->height();
    }
    return;
}*/

/*************************************************************************************/
//Mat转QImage类的函数——v1.0
//移动到ImgPro类中——v1.1
QImage ImgPro::Mat2QImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

//后台处理读取视频流——v1.6
void ImgPro::readBallSlot()
{
    while(BALL)
    {
        if(ballCap->isOpened())
        {
            //mutex01.lock();
            mutex1.lock();//互斥锁
            if(!ballCap->read(ballImg))
                break;
            emit getBall();
            mutex1.unlock();
            waitKey(1000/ballRate);
            //mutex01.unlock();
        }
    }
    return;
}
void ImgPro::readGunSlot()
{
    while(GUN)
    {
        if(gunCap->isOpened())
        {
            //mutex02.lock();
            mutex2.lock();
            if(!gunCap->read(gunImg))
                break;
            emit getGun();
            mutex2.unlock();
            waitKey(1000/gunRate);
            //mutex02.unlock();
        }
    }
    return;
}

//后台循环处理得到ballImage图像——v1.1
void ImgPro::getBallImageSlot()
{
    /*while(ballCap->isOpened())//如果连接成功
    {
        ballCap->read(ballImg);//读一帧
        cv::resize(ballImg,ballImg,Size(800,576),0,0,INTER_AREA);
        ballImage=Mat2QImage(ballImg);//转换为QImage类
        //emit showball();
    }*/

    mutex1.lock();//互斥锁
    ball=ballImg.clone();//复制一个mat用于处理
    mutex1.unlock();//解锁
    cv::resize(ball,ball,Size(lwidth,lheight),0,0,INTER_AREA);
    mutex3.lock();
    ballImage=Mat2QImage(ball);
    mutex3.unlock();

    //qDebug()<<"ERROR: Can't open the camera";
    return;
}
//后台循环处理得到gunImage图像——v1.3
void ImgPro::getGunImageSlot()
{
   /* while(gunCap->isOpened())//如果连接成功
    {
        gunCap->read(gunImg);//读一帧
        cv::resize(gunImg,gunImg,Size(800,576),0,0,INTER_AREA);
        gunImage=Mat2QImage(gunImg);//转换为QImage类
        //emit showgun();
    }*/

    mutex2.lock();
    gun=gunImg.clone();
    mutex2.unlock();
    cv::resize(gun,gun,Size(lwidth,lheight),0,0,INTER_AREA);
    mutex4.lock();
    gunImage=Mat2QImage(gun);
    mutex4.unlock();

    //qDebug()<<"ERROR: Can't open the camera";
    return;
}

/*************************************************************************************/
//右键显示坐标——v1.8
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    if(ui->tabWidget->currentIndex())
    {
        QMenu *menu=new QMenu(this);
        QAction *action=new QAction(this);
        QPoint eventpoint = event->globalPos();
        eventpoint = ui->gunWindowLabel->mapFromGlobal(eventpoint);
        action->setText("x="+QString::number(eventpoint.x())+"\ty="+QString::number(eventpoint.y()));
        menu->addAction(action);
        menu->exec(QCursor::pos());
    }
}

//切换标签使按键无效——v1.8
void MainWindow::tabChangeSlot()
{
    switch (ui->tabWidget->currentIndex()) {
    case 1://ball
        //qDebug()<<"Case 1:Tab:"+QString::number(ui->tabWidget->currentIndex());
        ui->groupBox_3->setEnabled(true);
        ui->groupBox_4->setEnabled(true);
        ui->groupBox_5->setEnabled(true);

        //ballCap->open("rtsp://admin:123456@lab.zhuzhuguowang.cn:36955/cam/realmonitor?channel=1&subtype=0");//连接摄像头
        //mutex1.unlock();
        //mutex3.unlock();
        BALL=true;
        emit startBallCamera();//发射一个摄像头开启信号
        timer1->start(1000/ballRate);//开启定时器
        //mutex01.unlock();

        //while(!mutex2.tryLock());
        //while(!mutex4.tryLock());
        //gunCap->release();//释放cap以便于停止线程
        //mutex02.lock();//锁住gun的变量使gun暂停
        GUN=false;
        timer2->stop();

        break;
    case 0://gun
        //qDebug()<<"Case 0:Tab:"+QString::number(ui->tabWidget->currentIndex());
        ui->groupBox_3->setEnabled(false);
        ui->groupBox_4->setEnabled(false);
        ui->groupBox_5->setEnabled(false);

        //gunCap->open("rtsp://admin:123456@lab.zhuzhuguowang.cn:36958/cam/realmonitor?channel=1&subtype=0");//连接枪机摄像头
        //mutex2.unlock();
        //mutex4.unlock();
        GUN=true;
        emit startGunCamera();//发射一个枪机摄像头开启信号
        timer2->start(1000/gunRate);//开启定时器
        //mutex02.unlock();

        //while(!mutex1.tryLock());
        //while(!mutex3.tryLock());
        //ballCap->release();//释放cap以便于停止线程
        //mutex01.lock();
        BALL=false;
        timer1->stop();

        break;
    default:
        break;
    }
}

//按键控制摄像头移动——v1.9
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(!event->isAutoRepeat())
    switch(event->key())
    {

    case Qt::Key_W: CLIENT_DHPTZControlEx2(lLoginHandle, 0, DH_PTZ_UP_CONTROL, 0, verticalSpeed, 0, FALSE, NULL);
                    break;

    case Qt::Key_S: CLIENT_DHPTZControlEx2(lLoginHandle, 0, DH_PTZ_DOWN_CONTROL, 0, verticalSpeed, 0, FALSE, NULL);
                    break;

    case Qt::Key_A: CLIENT_DHPTZControlEx2(lLoginHandle, 0, DH_PTZ_LEFT_CONTROL, 0, horizontalSpeed, 0, FALSE, NULL);
                    break;

    case Qt::Key_D: CLIENT_DHPTZControlEx2(lLoginHandle, 0, DH_PTZ_RIGHT_CONTROL, 0, horizontalSpeed, 0, FALSE, NULL);
                    break;
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(!event->isAutoRepeat())
    switch(event->key())
    {

    case Qt::Key_W: CLIENT_DHPTZControlEx2(lLoginHandle, 0, DH_PTZ_UP_CONTROL, 0, verticalSpeed, 0, TRUE, NULL);
                    break;

    case Qt::Key_S: CLIENT_DHPTZControlEx2(lLoginHandle, 0, DH_PTZ_DOWN_CONTROL, 0, verticalSpeed, 0, TRUE, NULL);
                    break;

    case Qt::Key_A: CLIENT_DHPTZControlEx2(lLoginHandle, 0, DH_PTZ_LEFT_CONTROL, 0, horizontalSpeed, 0, TRUE, NULL);
                    break;

    case Qt::Key_D: CLIENT_DHPTZControlEx2(lLoginHandle, 0, DH_PTZ_RIGHT_CONTROL, 0, horizontalSpeed, 0, TRUE, NULL);
                    break;
    }
    //显示当前坐标和变倍
    CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
    vertical=status.nPTZTilt;
    horizon=status.nPTZPan;
    zoom=status.nPTZZoom;
    ui->verticalLineEdit->setText(QString::number(vertical,10));
    ui->horizontalLineEdit->setText(QString::number(horizon,10));
    ui->zoomLineEdit->setText(QString::number(zoom,10));
}

//枪球联动——v1.9
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(ui->tabWidget->currentIndex())
    {
        if(event->button() == Qt::LeftButton)
        {
            //获得坐标
            QPoint eventpoint = event->globalPos();
            eventpoint = ui->gunWindowLabel->mapFromGlobal(eventpoint);
            //进行坐标到角度的变换
            vertical=0.239*eventpoint.y() + 49.653;
            horizon=-0.3101*eventpoint.x() + 634.28;
            zoom=128;
            if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_EXTPTZ_EXACTGOTO,horizon,vertical,zoom,FALSE,NULL))
            {
                ui->statusBar->showMessage("Go Fail!",2000);
            }
            //显示当前坐标和变倍——v1.5
            CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
            ui->verticalLineEdit->setText(QString::number(vertical,10));
            ui->horizontalLineEdit->setText(QString::number(horizon,10));
            ui->zoomLineEdit->setText(QString::number(zoom,10));
        }
    }
}
