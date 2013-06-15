#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H
#include <QThread>
#include <QtGui>
#include <opencv/highgui.h>
#include "imagebuffer.h"
#include "structures.h"

class ProcessingThread : public QThread
{
    Q_OBJECT

public:
    ProcessingThread(ImageBuffer *imageBuffer);
    ~ProcessingThread();

    void stopProcessingThread();
    int  getCurrentSizeOfBuffer();
    void updateFlags(int, bool);

    void setSaltPepperDensity(int v)    { QMutexLocker locker(&updM); settings.saltPepperNoiseDensity = v; }
    void setColorSpace(int v)           { QMutexLocker locker(&updM); settings.colorSpace = v; }
    void setDilateIterations(int v)     { QMutexLocker locker(&updM); settings.dilateIterations = v; }
    void setErodeIterations(int v)      { QMutexLocker locker(&updM); settings.erodeIterations = v; }
    void setOpenIterations(int v)       { QMutexLocker locker(&updM); settings.openIterations = v; }
    void setCloseIterations(int v)      { QMutexLocker locker(&updM); settings.closeIterations = v; }
    void setBlurSize(int v)             { QMutexLocker locker(&updM); settings.blurSize = v; }
    void setBlurSigma(double v)         { QMutexLocker locker(&updM); settings.blurSigma = v; }
    void setSobelDirection(int v)       { QMutexLocker locker(&updM); settings.sobelDirection = v; }
    void setSobelKernelSize(int v)      { QMutexLocker locker(&updM); settings.sobelKernelSize = v; }
    void setLaplacianKernelSize(int v)  { QMutexLocker locker(&updM); settings.laplacianKernelSize = v; }
    void setsharpKernelCenter(int v)    { QMutexLocker locker(&updM); settings.sharpKernelCenter = v; }
    void setCannyLowThres(int v)        { QMutexLocker locker(&updM); settings.cannyLowThres = v; }
    void setCannyHighThres(int v)       { QMutexLocker locker(&updM); settings.cannyHighThres = v; }
    void setLinesHoughVotes(int v)      { QMutexLocker locker(&updM); settings.linesHoughVotes = v; }
    void setCirclesHoughMin(int v)      { QMutexLocker locker(&updM); settings.circlesHoughMin = v; }
    void setCirclesHoughMax(int v)      { QMutexLocker locker(&updM); settings.circlesHoughMax = v; }
    void setContoursThreshold(int v)    { QMutexLocker locker(&updM); settings.contoursThres = v; }
    void setBoundingBoxThres(int v)     { QMutexLocker locker(&updM); settings.boundingBoxThres = v; }
    void setEnclosingCircleThres(int v) { QMutexLocker locker(&updM); settings.enclosingCircleThres = v; }
    void setHarrisCornerThres(int v)    { QMutexLocker locker(&updM); settings.harrisCornerThres = v; }
    void setFastThres(int v)            { QMutexLocker locker(&updM); settings.fastThreshold = v; }
    void setSurfThres(int v)            { QMutexLocker locker(&updM); settings.surfThreshold = v; }
    void setSiftContrastThres(double v) { QMutexLocker locker(&updM); settings.siftContrastThres = v; }
    void setSiftEdgeThres(int v)        { QMutexLocker locker(&updM); settings.siftEdgeThres = v; }
    void setInputMode(int v)            { QMutexLocker locker(&inputMutex); inputMode = v; }
    void setCurrentImage(cv::Mat frame) { currentFrame = frame; }
    void pause()                        { QMutexLocker locker(&pauseMutex); paused = true; }
    void play()                         { QMutexLocker locker(&pauseMutex); paused = false; }
    bool isPaused()                     { return paused; }

    int getColorSpace()           const { return settings.colorSpace; }
    int getSaltPepperDensity()    const { return settings.saltPepperNoiseDensity; }
    int getDilateIterations()     const { return settings.dilateIterations; }
    int getErodeIterations()      const { return settings.erodeIterations; }
    int getOpenIterations()       const { return settings.openIterations; }
    int getCloseIterations()      const { return settings.closeIterations; }
    int getBlurSize()             const { return settings.blurSize; }
    int getSobelDirection()       const { return settings.sobelDirection; }
    int getSobelKernelSize()      const { return settings.sobelKernelSize; }
    int getLaplacianKernelSize()  const { return settings.laplacianKernelSize; }
    int getsharpKernelCenter()    const { return settings.sharpKernelCenter; }
    int getCannyLowThres()        const { return settings.cannyLowThres; }
    int getCannyHighThres()       const { return settings.cannyHighThres; }
    int getLinesHoughVotes()      const { return settings.linesHoughVotes; }
    int getCirclesHoughMin()      const { return settings.circlesHoughMin; }
    int getCirclesHoughMax()      const { return settings.circlesHoughMax; }
    int getContourThreshold()     const { return settings.contoursThres; }
    int getBoundingBoxThres()     const { return settings.boundingBoxThres; }
    int getEnclosingCircleThres() const { return settings.enclosingCircleThres; }
    int getHarrisCornerThres()    const { return settings.harrisCornerThres; }
    int getFastThres()            const { return settings.fastThreshold; }
    int getSurfThres()            const { return settings.surfThreshold; }
    int getSiftEdgeThres()        const { return settings.siftEdgeThres; }
    double getSiftContrastThres() const { return settings.siftContrastThres; }
    double getBlurSigma()         const { return settings.blurSigma; }
    cv::Mat getProcessedFrame()   const { return processedFrame; }
    bool getFilter(int index)     const { return filters.flags[index]; }

private:
    ImageBuffer   *outputBuffer;
    volatile bool stopped;
    bool          paused;
    int           currentSizeOfBuffer;
    int           inputMode;
    QMutex        inputMutex;
    QMutex        stoppedMutex;
    QMutex        pauseMutex;
    QMutex        updM; // Update Members Mutex
    // Image processing flags
    ImageProcessingFlags filters;
    // Image processing settings
    ImageProcessingSettings settings;
    cv::Mat currentFrame;
    cv::Mat processedFrame;

protected:
    void run();

signals:
    void newProcessedFrame(const QImage &frame);
    void newProcessedHistogram(const QImage &hist);

};

#endif // PROCESSINGTHREAD_H
