//======================================================================
//
//        Copyright © 2018 LAB369
//        All rights reserved
//
//        filename :track.h
//        description :目标追踪窗口类定义，实现其函数
//
//        created by LZW at 2018/10/23
//
//======================================================================

#ifndef TRACK_H
#define TRACK_H

#include <QWidget>
#include <QThread>
#include <QTimer>
#include <QPoint>
#include <QMouseEvent>

class ballCamera;

namespace Ui {
class Track;
}

class Track : public QWidget
{
    Q_OBJECT

    QThread trackImgProThread;  //用于追踪处理

public:
    explicit Track(QWidget *parent = 0);
    ~Track();

private:
    Ui::Track *ui;

    QTimer *timer3;

    bool isSelect = false;      //是否开始画框
    bool isStart = false;       //是否开始追踪

    QPoint beginp,endp;     //画框用点

signals:
    void startBallTrack(ballCamera*);

private slots:
    void selectSlot();
    void startSlot();
    void stopSlot();

    void showTrackSlot();

protected:
    void closeEvent(QCloseEvent *);

    void mousePressEvent(QMouseEvent *);

    void mouseReleaseEvent(QMouseEvent *);

};

#endif // TRACK_H
