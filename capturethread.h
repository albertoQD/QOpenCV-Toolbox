#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>
#include <QtGui>
#include "opencv/highgui.h"
#include "imagebuffer.h"
#include "config.h"

class CaptureThread : public QThread
{
    Q_OBJECT

public:
    CaptureThread(ImageBuffer *buffer);

    void setInputMode(int m)    { QMutexLocker locker(&inputMutex); inputMode = m; }
    int  getInputMode()         { return inputMode; }
    bool isCameraConnected()    { return cap.isOpened(); }
    int  getInputSourceWidth()  { return cap.get(CV_CAP_PROP_FRAME_WIDTH); }
    int  getInputSourceHeight() { return cap.get(CV_CAP_PROP_FRAME_HEIGHT); }
    void pause()                { QMutexLocker locker(&pauseMutex); paused = true; }
    void play()                 { QMutexLocker locker(&pauseMutex); paused = false; }
    bool isPaused()             { return paused; }

    bool readVideo(QString fn);
    bool readImage(QString fn);
    bool connectToCamera(int c);
    void disconnectCamera();
    void stopCaptureThread();

private:
    ImageBuffer *inputBuffer;
    cv::VideoCapture cap;
    cv::Mat grabbedFrame;
    QMutex stoppedMutex;
    QMutex inputMutex;
    QMutex capMutex;
    QMutex pauseMutex;

    volatile bool stopped;
    bool paused;
    int inputMode;
protected:
    void run();
};

#endif // CAPTURETHREAD_H
