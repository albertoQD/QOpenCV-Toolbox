#include "capturethread.h"
#include "imagebuffer.h"
#include <QDebug>
#include "config.h"

CaptureThread::CaptureThread(ImageBuffer *imageBuffer)
    : QThread()
    , inputBuffer(imageBuffer)
    , stopped(false)
    , paused(false)
{
}

void CaptureThread::run()
{
    while(1)
    {
        // Check if it is paused
        pauseMutex.lock();
        if (paused)
        {
            pauseMutex.unlock();
            sleep(3);
            continue;
        }
        pauseMutex.unlock();

        /////////////////////////////////
        // Stop thread if stopped=TRUE //
        /////////////////////////////////
        stoppedMutex.lock();
        if (stopped)
        {
            stopped=false;
            stoppedMutex.unlock();
            break;
        }
        stoppedMutex.unlock();

        inputMutex.lock();
        if (inputMode != INPUT_IMAGE)
        {
            inputMutex.unlock();
            // Capture a frame
            capMutex.lock();
                cap >> grabbedFrame;
            capMutex.unlock();
            // resize the frame to fit in the UI frames
            cv::resize(grabbedFrame, grabbedFrame, cv::Size(332, 232));
        }
        inputMutex.unlock();

        // add the frame to the buffer
        inputBuffer->addFrame(grabbedFrame.clone());

        inputMutex.lock();
        if (inputMode == INPUT_VIDEO)
        {
            inputMutex.unlock();
            capMutex.lock();
            if (cap.get(CV_CAP_PROP_FRAME_COUNT)-1 == cap.get(CV_CAP_PROP_POS_FRAMES))
            {
                cap.set(CV_CAP_PROP_POS_FRAMES, 0);
            }
            capMutex.unlock();
            msleep(1000/cap.get(CV_CAP_PROP_FPS));
        }
        else if(inputMode == INPUT_IMAGE)
        {
            inputMutex.unlock();
            msleep(50);
        }
        inputMutex.unlock();
    }
}


void CaptureThread::disconnectCamera()
{
    if(cap.isOpened())
    {
        cap.release();
    }
}

void CaptureThread::stopCaptureThread()
{
    stoppedMutex.lock();
    stopped=true;
    stoppedMutex.unlock();
}

bool CaptureThread::readVideo(QString fn)
{
    setInputMode(INPUT_VIDEO);
    bool res = false;

    capMutex.lock();
    if (cap.isOpened())
        cap.release();
        res = cap.open(fn.toStdString());
    capMutex.unlock();

    return res;
}

bool CaptureThread::readImage(QString fn)
{
    setInputMode(INPUT_IMAGE);

    capMutex.lock();
    if (cap.isOpened())
        cap.release();
    capMutex.unlock();

    grabbedFrame = cv::imread(fn.toStdString());
    cv::resize(grabbedFrame, grabbedFrame, cv::Size(332, 232));

    return true;
}

bool CaptureThread::connectToCamera(int c)
{
    setInputMode(INPUT_CAMERA);
    bool res = false;

    capMutex.lock();
    if (cap.isOpened())
        cap.release();
        res = cap.open(c);
    capMutex.unlock();

    return res;
}
