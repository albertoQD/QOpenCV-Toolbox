#ifndef IMAGEBUFFER_H
#define IMAGEBUFFER_H

#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QSemaphore>
#include <QImage>
#include <QObject>
#include <opencv/highgui.h>

class ImageBuffer : public QObject
{
    Q_OBJECT
public:
    explicit ImageBuffer(QObject *parent = 0, int s = 0, bool fd = false);

    void addFrame(const cv::Mat& frame);
    cv::Mat getFrame();
    void clearBuffer();
    int getSizeOfImageBuffer();

    
signals:
    void newFrame();

public slots:

private:
    QMutex imageQueueProtect;
    QQueue<cv::Mat> imageQueue;
    QSemaphore *freeSlots;
    QSemaphore *usedSlots;
    QSemaphore *clearBuffer1;
    QSemaphore *clearBuffer2;
    int bufferSize;
    bool dropFrame;
    
};

#endif // IMAGEBUFFER_H
