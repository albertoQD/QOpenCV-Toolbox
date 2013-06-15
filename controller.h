#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "capturethread.h"
#include "imagebuffer.h"
#include "processingthread.h"
#include "structures.h"
#include <QtGui>
#include <opencv/highgui.h>


class Controller : public QObject
{
    Q_OBJECT

public:
    Controller();
    ~Controller();

    ImageBuffer      *inputBuffer;
    ImageBuffer      *outputBuffer;
    ProcessingThread *processingThread;
    CaptureThread    *captureThread;

    bool connectToCamera();
    void stopThreads();
    void deleteThreads();
    void clearImageBuffers();
    void deleteImageBuffers();
    void setInputMode(int);
    int  getInputMode();
    bool readVideo(QString);
    bool loadLogo(QString);
    bool readImage(QString);

public slots:
    void processFrame();
    void setLogoROI(QRect, QPoint);

signals:
    void newInputFrame(QImage);

private:
    int imageBufferSize;
    int inputMode;
    cv::Mat logo;
    QRect logoROI;
    QPoint logoOrigen;
};

#endif // CONTROLLER_H
