#include "controller.h"
#include "imagebuffer.h"
#include "mattoqimage.h"
#include "structures.h"
#include <QtGui>
#include "config.h"

Controller::Controller()
{
    imageBufferSize = DEFAULT_IMAGE_BUFFER_SIZE;
    inputBuffer = new ImageBuffer(this, imageBufferSize);
    outputBuffer = new ImageBuffer(this, imageBufferSize);

    captureThread    = new CaptureThread(inputBuffer);
    processingThread = new ProcessingThread(outputBuffer);
    connect(inputBuffer, SIGNAL(newFrame()), this, SLOT(processFrame()));

    logo = cv::Mat();
    logoROI = QRect();
    logoOrigen = QPoint();
}

Controller::~Controller()
{
}

bool Controller::readImage(QString filename)
{
    bool res = false;
    inputMode = INPUT_IMAGE;

    if (captureThread->isRunning())
    {
        captureThread->pause();
        processingThread->pause();
    }

    if ((res = captureThread->readImage(filename)))
    {
        processingThread->setInputMode(inputMode);
        if (captureThread->isPaused())
        {
            captureThread->play();
            processingThread->play();
        }
        else
        {
            captureThread->start(DEFAULT_CAP_THREAD_PRIO);
            processingThread->start(DEFAULT_PROC_THREAD_PRIO);
        }
    }

    return res;
}

bool Controller::readVideo(QString filename)
{
    bool res = false;
    inputMode = INPUT_VIDEO;

    if (captureThread->isRunning())
    {
        captureThread->pause();
        processingThread->pause();
    }

    if ((res = captureThread->readVideo(filename)))
    {
        processingThread->setInputMode(inputMode);
        if (captureThread->isPaused())
        {
            captureThread->play();
            processingThread->play();
        }
        else
        {
            captureThread->start(DEFAULT_CAP_THREAD_PRIO);
            processingThread->start(DEFAULT_PROC_THREAD_PRIO);
        }
    }

    return res;
}

bool Controller::connectToCamera()
{
    bool isOpened    = false;
    inputMode = INPUT_CAMERA;

    if (captureThread->isRunning())
    {
        captureThread->pause();
        processingThread->pause();
    }

    if((isOpened = captureThread->connectToCamera(DEFAULT_CAMERA_DEV_NO)))
    {
        processingThread->setInputMode(inputMode);
        if (captureThread->isPaused())
        {
            captureThread->play();
            processingThread->play();
        }
        else
        {
            captureThread->start(DEFAULT_CAP_THREAD_PRIO);
            processingThread->start(DEFAULT_PROC_THREAD_PRIO);
        }
    }

    return isOpened;
}


void Controller::stopThreads()
{
    captureThread->stopCaptureThread();

    // Take one frame off a FULL queue to allow the capture thread to finish
    if(inputBuffer->getSizeOfImageBuffer() == imageBufferSize)
        inputBuffer->getFrame();

    if (!captureThread->wait(2000))
    {
        captureThread->terminate();
    }

    processingThread->stopProcessingThread();
    if (!processingThread->wait(2000))
    {
        processingThread->terminate();
    }
}


void Controller::deleteThreads()
{
    delete captureThread;
    delete processingThread;
}


void Controller::clearImageBuffers()
{
    inputBuffer->clearBuffer();
    outputBuffer->clearBuffer();
}

void Controller::deleteImageBuffers()
{
    delete inputBuffer;
    delete outputBuffer;
}

void Controller::setInputMode(int mode)
{
    inputMode = mode;
}

int Controller::getInputMode()
{
    return inputMode;
}

bool Controller::loadLogo(QString filename)
{
    logo = cv::imread(filename.toStdString());
    return true;
}

void Controller::setLogoROI(QRect roi, QPoint origen)
{
    logoROI = roi;
    logoOrigen = origen;
}

void Controller::processFrame()
{
    // get the frame from inputbuffer
    cv::Mat frame = inputBuffer->getFrame();

    // check if it's necessary to insert a logo
    if (processingThread->getFilter(ImageProcessingFlags::ShowLogo) &&
        !logo.empty() &&
        !logoROI.isEmpty())
    {
        cv::Mat logoResized;
        cv::Mat imageROI = frame(cv::Rect(
                                     logoOrigen.x(),
                                     logoOrigen.y(),
                                     logoROI.width(),
                                     logoROI.height()));
        cv::resize(logo, logoResized, imageROI.size());
        cv::addWeighted(imageROI, 1.0, logoResized, 0.3, 0., imageROI);
    }

    // add the new frame to the outputbuffer, so the processingThread can take it
    outputBuffer->addFrame(frame);
    // send signal to update the inputlabel in the UI
    emit newInputFrame(MatToQImage(frame));
}
