#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

//进行图像操作的变量
Mat ballImg,gunImg;
VideoCapture *ballCap=new VideoCapture,*gunCap=new VideoCapture;
QImage ballImage,gunImage;

//预设登录信息
char szDevIp[64] = {"lab.zhuzhuguowang.cn"};
NET_DEVICEINFO stDevInfo = {0};
int nError = 0;
int nPort = 36956;
static LLONG lLoginHandle = 0L;

//摄像头状态
DH_PTZ_LOCATION_INFO status;
int statuslen=0;

//坐标和变倍
int horizon,vertical;
int zoom;

//帧率相关
double ballRate=10;
double gunRate=10;

//图像处理临时变量
QImage ballshow,gunshow;
Mat ball,gun;

//互斥锁
QMutex mutex1,mutex2,mutex3,mutex4,mutex5;

//线程循环控制变量
bool BALL,GUN;

//窗口大小
int lwidth=800;
int lheight=600;

//画框
bool mouseispressed = false;
QPoint beginp,endp;

//目标追踪
Ptr<Tracker> tracker = TrackerKCF::create();
Rect2d box;
Mat ballTrack,ballTrackImg;
bool isinit=false;
bool isSelect = false;  // 记录是否按下Select键

// 枪球矫正
double va = 0.239;
double vb = 49.653;
double ha = -0.3101;
double hb = 634.28;

/*************************************************************************************/

//构造函数
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //定时器
    timer1 = new QTimer(this);
    timer2 = new QTimer(this);

    //登录登出
    QObject::connect(ui->loginButton,SIGNAL(clicked()),this,SLOT(loginSlot()));
    QObject::connect(ui->logoutButton,SIGNAL(clicked()),this,SLOT(logoutSlot()));

    //定时显示
    QObject::connect(timer1,SIGNAL(timeout()),this,SLOT(showBallSlot()));
    QObject::connect(timer2,SIGNAL(timeout()),this,SLOT(showGunSlot()));


    //读取视频流进程
    //球机
    ImgPro *imgpro1 = new ImgPro;
    imgpro1->moveToThread(&imgproThread1);
    imgproThread1.start();
    QObject::connect(this,SIGNAL(startBallCamera()),imgpro1,SLOT(readBallSlot()));

    //枪机
    ImgPro *imgpro2 = new ImgPro;
    imgpro2->moveToThread(&imgproThread2);
    imgproThread2.start();
    QObject::connect(this,SIGNAL(startGunCamera()),imgpro2,SLOT(readGunSlot()));


    //窗口和状态栏
    MainWindow::setWindowTitle("Camera  Control  System --- Ver2.4");
    QLabel *copyright = new QLabel(this);
    copyright->setText("Copyright   ©   LAB   369");
    ui->statusBar->addPermanentWidget(copyright);

    //速度滑块设置
    ui->verticalSpeedSlider->setMaximum(8);
    ui->verticalSpeedSlider->setMinimum(0);
    ui->verticalSpeedSlider->setTickPosition(QSlider::TicksAbove);
    ui->horizontalSpeedSlider->setMaximum(8);
    ui->horizontalSpeedSlider->setMinimum(0);
    ui->horizontalSpeedSlider->setTickPosition(QSlider::TicksAbove);

    //速度控制
    QObject::connect(ui->verticalSpeedSlider,SIGNAL(valueChanged(int)),this,SLOT(verticalSpeedSlot()));
    QObject::connect(ui->horizontalSpeedSlider,SIGNAL(valueChanged(int)),this,SLOT(horizontalSpeedSlot()));

    //云台控制
    QObject::connect(ui->upButton,SIGNAL(pressed()),this,SLOT(upSlot()));
    QObject::connect(ui->downButton,SIGNAL(pressed()),this,SLOT(downSlot()));
    QObject::connect(ui->leftButton,SIGNAL(pressed()),this,SLOT(leftSlot()));
    QObject::connect(ui->rightButton,SIGNAL(pressed()),this,SLOT(rightSlot()));
    QObject::connect(ui->upButton,SIGNAL(released()),this,SLOT(upStopSlot()));
    QObject::connect(ui->downButton,SIGNAL(released()),this,SLOT(downStopSlot()));
    QObject::connect(ui->leftButton,SIGNAL(released()),this,SLOT(leftStopSlot()));
    QObject::connect(ui->rightButton,SIGNAL(released()),this,SLOT(rightStopSlot()));

    //变倍滑块设置
    ui->zoomSpeedSlider->setMaximum(1);
    ui->zoomSpeedSlider->setMinimum(0);
    ui->zoomSpeedSlider->setTickPosition(QSlider::TicksAbove);
    QObject::connect(ui->zoomSpeedSlider,SIGNAL(valueChanged(int)),this,SLOT(zoomSpeedSlot()));

    //变倍控制
    QObject::connect(ui->plusButton,SIGNAL(pressed()),this,SLOT(plusSlot()));
    QObject::connect(ui->minusButton,SIGNAL(pressed()),this,SLOT(minusSlot()));
    QObject::connect(ui->plusButton,SIGNAL(released()),this,SLOT(plusStopSlot()));
    QObject::connect(ui->minusButton,SIGNAL(released()),this,SLOT(minusStopSlot()));

    //控制坐标移动
    QObject::connect(ui->resetButton,SIGNAL(clicked()),this,SLOT(resetSlot()));
    QObject::connect(ui->goButton,SIGNAL(clicked()),this,SLOT(goSlot()));
    QObject::connect(ui->clearButton,SIGNAL(clicked()),this,SLOT(clearSlot()));
    QObject::connect(ui->rezoomButton,SIGNAL(clicked()),this,SLOT(rezoomSlot()));
    QObject::connect(ui->zoomButton,SIGNAL(clicked()),this,SLOT(zoomSlot()));
    QObject::connect(ui->clearButton2,SIGNAL(clicked()),this,SLOT(clear2Slot()));

    //控制GUN窗口隐藏
    QObject::connect(ui->gunCheckBox,SIGNAL(stateChanged(int)),this,SLOT(gunVisibleSlot()));

    //select按键相关
    QObject::connect(ui->selectPushButton,SIGNAL(clicked()),this,SLOT(ballSelectSlot()));

    //目标追踪线程
    ImgPro *imgpro3 = new ImgPro;
    imgpro3->moveToThread(&imgtracker);
    imgtracker.start();
    QObject::connect(this,SIGNAL(startBallTrack()),imgpro3,SLOT(ballTrackSlot()));

    // 矫正
    QObject::connect(ui->rectifyPushButton, SIGNAL(clicked()), this, SLOT(rectifySlot()));

    //菜单栏
    QObject::connect(ui->loginAction,SIGNAL(triggered(bool)),this,SLOT(loginSlot()));
    QObject::connect(ui->logoutAction,SIGNAL(triggered(bool)),this,SLOT(logoutSlot()));

    QObject::connect(ui->upAction,SIGNAL(triggered(bool)),this,SLOT(upActionSlot()));
    QObject::connect(ui->downAction,SIGNAL(triggered(bool)),this,SLOT(downActionSlot()));
    QObject::connect(ui->leftAction,SIGNAL(triggered(bool)),this,SLOT(leftActionSlot()));
    QObject::connect(ui->rightAction,SIGNAL(triggered(bool)),this,SLOT(rightActionSlot()));
    QObject::connect(ui->resetAction,SIGNAL(triggered(bool)),this,SLOT(resetSlot()));

    QObject::connect(ui->plusAction,SIGNAL(triggered(bool)),this,SLOT(plusActionSlot()));
    QObject::connect(ui->minusAction,SIGNAL(triggered(bool)),this,SLOT(minusActionSlot()));
    QObject::connect(ui->rezoomAction,SIGNAL(triggered(bool)),this,SLOT(rezoomSlot()));

    QObject::connect(ui->helpAction,SIGNAL(triggered(bool)),this,SLOT(helpSlot()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//登陆函数
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

        ballCap->open("rtsp://admin:123456@lab.zhuzhuguowang.cn:36955/cam/realmonitor?channel=1&subtype=0");//连接摄像头
        gunCap->open("rtsp://admin:123456@lab.zhuzhuguowang.cn:36958/cam/realmonitor?channel=1&subtype=0");//连接枪机摄像头
        BALL=true;
        GUN=true;
        emit startBallCamera();//发射一个摄像头开启信号
        emit startGunCamera();//发射一个枪机摄像头开启信号
        timer1->start(1000/ballRate);//开启定时器
        timer2->start(1000/gunRate);//开启定时器
        ui->gunCheckBox->setChecked(true);//初始GUN选中，显示GUN窗口
        ui->gunWindowLabel->setVisible(true);

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

        //显示当前坐标和变倍
        CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
        vertical=status.nPTZTilt;
        horizon=status.nPTZPan;
        zoom=status.nPTZZoom;
        ui->verticalLineEdit->setText(QString::number(vertical,10));
        ui->horizontalLineEdit->setText(QString::number(horizon,10));
        ui->zoomLineEdit->setText(QString::number(zoom,10));
    }
}
//登出函数
void MainWindow::logoutSlot()
{
    BALL=false;
    imgproThread1.quit();
    timer1->stop();//退出登录
    GUN=false;
    imgproThread2.quit();
    timer2->stop();//退出登录
    isinit=false;
    imgtracker.quit();
    CLIENT_Logout(lLoginHandle);
    CLIENT_Cleanup();
    ui->statusBar->showMessage("Logout!",2000);
}

//关闭事件
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
    if(imgtracker.isRunning())
    {
        imgtracker.quit();
        qDebug()<<"imgtracker is running!";
    }
    event->accept();//接收事件
}

/*************************************************************************************/

//显示球机图像
void MainWindow::showBallSlot()
{
    if(ui->sourceComboBox->currentIndex()==0)
    {
        mutex1.lock();//互斥锁
        ball=ballImg.clone();//复制一个mat用于处理  // 如果这里不复制呢 //慢啊
        mutex1.unlock();//解锁
        cv::resize(ball,ball,Size(lwidth,lheight),0,0,INTER_AREA);

        ballImage=MainWindow::Mat2QImage(ball);
        ui->ballWindowLabel->setPixmap(QPixmap::fromImage(ballImage));
    }
    else
    {
        mutex5.lock();//互斥锁
        ball=ballTrack.clone();//复制一个mat用于处理
        mutex5.unlock();//解锁
        ballImage=MainWindow::Mat2QImage(ball);
        ui->ballWindowLabel->setPixmap(QPixmap::fromImage(ballImage));
    }

    return;
}
//显示枪机图像
void MainWindow::showGunSlot()
{
    mutex2.lock();
    gun=gunImg.clone();
    mutex2.unlock();

    cv::resize(gun,gun,Size(lwidth/2,lheight/2),0,0,INTER_AREA);

    gunImage=Mat2QImage(gun);
    ui->gunWindowLabel->setPixmap(QPixmap::fromImage(gunImage));

    return;
}

/*************************************************************************************/

//控制摄像头移动
int verticalSpeed,horizontalSpeed;
//垂直速度和水平速度
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

//上下左右的启动和停止
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

/*************************************************************************************/

//控制变倍
int zoomSpeed;
//变倍速度
void MainWindow::zoomSpeedSlot()
{
    zoomSpeed=ui->zoomSpeedSlider->value();
    ui->statusBar->showMessage("Zoom speed changed!",3000);
}

//变倍的启动和停止
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

//控制坐标移动
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

/*************************************************************************************/

//Mat转QImage类的函数
QImage MainWindow::Mat2QImage(const cv::Mat& mat)
{
    if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

//后台处理读取视频流
void ImgPro::readBallSlot()
{
    while(BALL)
    {
        if(ballCap->isOpened())
        {
            mutex1.lock();//互斥锁
            if(!ballCap->read(ballImg))
                break;
            mutex1.unlock();

            mutex3.lock();
            ballTrackImg=ballImg.clone();//追踪用原图
            mutex3.unlock();

            waitKey(30);
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
            mutex2.lock();
            if(!gunCap->read(gunImg))
                break;
            mutex2.unlock();

            waitKey(30);
        }
    }
    return;
}


/*************************************************************************************/

//右键显示坐标
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu=new QMenu(this);
    QAction *action=new QAction(this);
    QPoint point = event->globalPos();
    point = ui->gunWindowLabel->mapFromGlobal(point);

    if(point.x()>=0 && point.x()<=lwidth/2 && point.y()>=0 && point.y()<=lheight/2 && ui->gunCheckBox->isChecked())
    {
        action->setText("x="+QString::number(point.x())+"\ty="+QString::number(point.y()));
        menu->addAction(action);
        menu->exec(QCursor::pos());
    }
}

//按键控制摄像头移动
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


//隐藏枪机画面
void MainWindow::gunVisibleSlot()
{
    if(ui->gunCheckBox->isChecked())
    {
        ui->gunWindowLabel->setVisible(true);
    }
    else
    {
        ui->gunWindowLabel->setVisible(false);
    }
}

/*************************************************************************************/

//枪球联动和目标追踪的鼠标事件
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {

        if(isSelect && !ui->gunCheckBox->isChecked())
        {
            //获得起始点
            beginp=event->globalPos();
            beginp=ui->ballWindowLabel->mapFromGlobal(beginp);
            //更改鼠标状态
            mouseispressed=true;
        }
        else
        {
            //获得坐标
            QPoint point = event->globalPos();
            point = ui->gunWindowLabel->mapFromGlobal(point);
            if(point.x()>=0 && point.x()<=lwidth/2 && point.y()>=0 && point.y()<=lheight/2 && ui->gunCheckBox->isChecked())
            {

                vertical=va*point.y()*2 + vb;
                horizon=ha*point.x()*2 + hb;
                zoom=44;
                if(FALSE==CLIENT_DHPTZControlEx2(lLoginHandle,0,DH_EXTPTZ_EXACTGOTO,horizon,vertical,zoom,FALSE,NULL))
                {
                    ui->statusBar->showMessage("Go to Point Fail!",2000);
                }

                //显示当前坐标和变倍
                CLIENT_QueryDevState(lLoginHandle,DH_DEVSTATE_PTZ_LOCATION,(char *)&status,sizeof(status),&statuslen,3000);
                ui->verticalLineEdit->setText(QString::number(vertical,10));
                ui->horizontalLineEdit->setText(QString::number(horizon,10));
                ui->zoomLineEdit->setText(QString::number(zoom,10));
            }
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    endp=event->globalPos();
    endp=ui->ballWindowLabel->mapFromGlobal(endp);

    //更改鼠标状态
    mouseispressed = false;

    if (isSelect)  // 加入判断条件，防止在任意时间按下鼠标时触发tracker导致闪退
    {
        isSelect = false;
        box = Rect2d(beginp.x(), beginp.y(), endp.x() - beginp.x(), endp.y() - beginp.y());

        //mutex3.lock();
        mutex3.lock();
        ballTrack = ballTrackImg.clone();
        mutex3.unlock();

        cv::resize(ballTrack, ballTrack, Size(lwidth, lheight), 0, 0, INTER_AREA);
        rectangle(ballTrack, box, Scalar(0, 255, 0), 2, 1);
        tracker = TrackerKCF::create();//清空
        isinit = tracker->init(ballTrack, box);
        //mutex3.unlock();
        emit startBallTrack();
    }
}


//select键
void MainWindow::ballSelectSlot()
{
    this->setMouseTracking(true);
    isinit = false;
	isSelect = true;

}

//目标追踪
void ImgPro::ballTrackSlot()
{
    while(isinit)//循环处理防止跟丢，而不是定时处理
    {
        mutex5.lock();//锁住ballTrack
        mutex3.lock();//锁住ballTrackImg
        ballTrack=ballTrackImg.clone();
        mutex3.unlock();

        cv::resize(ballTrack,ballTrack,Size(lwidth,lheight),0,0,INTER_AREA);
        tracker->update(ballTrack, box);
        rectangle(ballTrack, box, Scalar(255, 0, 0), 2, 1);
        mutex5.unlock();
    }
}

/*************************************************************************************/

// 枪球矫正
double calc_a(vector<x_y> x_y_in) // 最小二乘法计算斜率
{
    double s1, s2, s3, s4;  // s1-x, s2-y, s3-xy, s4-x^2
    int n;
    if (x_y_in.size() <2 )
    {
        return 0;
    }
    else
    {
        s1 = 0;
        s2 = 0;
        s3 = 0;
        s4 = 0;
        n = 0;
        vector<x_y>::iterator it;
        for (it = x_y_in.begin(); it != x_y_in.end(); it++)
        {
            s1 += it->x;
            s2 += it->y;
            s3 += it->x * it->y;
            s4 += it->x * it->x;
            n++;
        }
        return (s1*s2 - n * s3) / (s1*s1 - n * s4);

    }
}

double calc_b(vector<x_y> x_y_in) // 最小二乘法计算截距
{
    double s1, s2, s3, s4;  // s1-x, s2-y, s3-xy, s4-x^2
    int n;
    if (x_y_in.begin() == x_y_in.end())
    {
        return 0;
    }
    else
    {
        s1 = 0;
        s2 = 0;
        s3 = 0;
        s4 = 0;
        n = 0;
        vector<x_y>::iterator it;
        for (it = x_y_in.begin(); it != x_y_in.end(); it++)
        {
            s1 += it->x;
            s2 += it->y;
            s3 += it->x * it->y;
            s4 += it->x * it->x;
            n++;
        }
        return (s4*s2 - s1 * s3) / (n*s4 - s1 * s1);

    }
}

void MainWindow::rectifySlot()
{


    mutex1.lock();
    Mat ballSrc = ballImg.clone();
    mutex1.unlock();

    mutex2.lock();
    Mat gunSrc = gunImg.clone();
    mutex2.unlock();

    cv::resize(ballSrc, ballSrc, Size(800, 600));
    cv::resize(gunSrc, gunSrc, Size(800, 600));

    int minHessian = 1500;
    Ptr<SurfFeatureDetector> detector = SurfFeatureDetector::create(minHessian);  // 特征检测器

    vector<cv::KeyPoint> key_points_1, key_points_2;

    Mat dstImage1, dstImage2;
	Mat V;
    detector->detectAndCompute(ballSrc, noArray(), key_points_1, dstImage1);
    detector->detectAndCompute(gunSrc, noArray(), key_points_2, dstImage2);

    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");
    vector<DMatch>mach;

    matcher->match(dstImage1, dstImage2, mach);

    double Max_dist = 0;
    double Min_dist = 100;

    for (int i = 0; i < dstImage1.rows; i++)
    {
        double dist = mach[i].distance;
        if (dist < Min_dist)Min_dist = dist;
        if (dist > Max_dist)Max_dist = dist;
    }

    vector<DMatch>goodmaches;  // 筛选匹配特征点
    for (int i = 0; i < dstImage1.rows; i++)
    {
        if (mach[i].distance < 1.3* Min_dist)  // 这个数可以改，越小越准，但匹配的点对越少
            goodmaches.push_back(mach[i]);  // 把符合条件的mach中的点压入栈中
    }

    Mat img_maches;
    drawMatches(ballSrc, key_points_1, gunSrc, key_points_2, goodmaches, img_maches, Scalar::all(-1), Scalar(0xff, 0xff, 0xff));
    cv::resize(img_maches,img_maches,Size(800, 300));
    imshow("match result", img_maches);
    vector<x_y> x_h, y_v;
    double l_ha, l_hb, l_va, l_vb;

    for (int i = 0; i < goodmaches.size(); i++)
    {
        double h, v;
        h = horizon - (key_points_1[goodmaches[i].queryIdx].pt.x - 400)*0.135;
        v = vertical + (key_points_1[goodmaches[i].queryIdx].pt.y - 300)*0.1;

        x_h.push_back(x_y(key_points_2[goodmaches[i].trainIdx].pt.x, h));
        y_v.push_back(x_y(key_points_2[goodmaches[i].trainIdx].pt.y, v));

        l_ha = calc_a(x_h);
        l_hb = calc_b(x_h);
        l_va = calc_a(y_v);
        l_vb = calc_b(y_v);

        if ((l_ha!=0) && (l_hb!=0) && (l_va!=0) && (l_vb!=0))
        {
            qDebug() << "h = " << l_ha << " * x + " << l_hb << endl;
            qDebug() << "v = " << l_va << " * y + " << l_vb << endl;
            ha = l_ha;
            hb = l_hb;
            va = l_va;
            vb = l_vb;

        }
        else
        {
            qDebug() << "Error: Less than two points! \n";
        }

    }


    return ;
}

/*************************************************************************************/

//菜单栏
void MainWindow::upActionSlot()
{
    upSlot();
    waitKey(1000);
    upStopSlot();
}

void MainWindow::downActionSlot()
{
    downSlot();
    waitKey(1000);
    downStopSlot();
}

void MainWindow::leftActionSlot()
{
    leftSlot();
    waitKey(1000);
    leftStopSlot();
}

void MainWindow::rightActionSlot()
{
    rightSlot();
    waitKey(1000);
    rightStopSlot();
}

void MainWindow::plusActionSlot()
{
    plusSlot();
    waitKey(1000);
    plusStopSlot();
}

void MainWindow::minusActionSlot()
{
    minusSlot();
    waitKey(1000);
    minusStopSlot();
}

void MainWindow::helpSlot()
{
    help = new Help;
    help->show();
}
