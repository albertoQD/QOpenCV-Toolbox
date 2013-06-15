#include "imagebuffer.h"
#include "processingthread.h"
#include "mattoqimage.h"
#include "config.h"
#include <QDebug>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/nonfree/nonfree.hpp>

#define PI 3.14159265359

ProcessingThread::ProcessingThread(ImageBuffer *imageBuffer)
    : QThread()
    , outputBuffer(imageBuffer)
    , stopped(false)
    , paused(false)
{
    settings.saltPepperNoiseDensity = 0;
    settings.colorSpace = 0;
    settings.dilateIterations = 0;
    settings.erodeIterations = 0;
    settings.openIterations = 0;
    settings.closeIterations = 0;
    settings.blurSize = 1;
    settings.sobelDirection = 0;
    settings.sobelKernelSize = 1;
    settings.laplacianKernelSize = 1;
    settings.sharpKernelCenter = 5;
    settings.cannyLowThres = 100;
    settings.cannyHighThres = 300;
    settings.linesHoughVotes = 60;
    settings.circlesHoughMin = 25;
    settings.circlesHoughMax = 50;
    settings.contoursThres = 50;
    settings.boundingBoxThres = 50;
    settings.enclosingCircleThres = 50;
    settings.harrisCornerThres = 150;
    settings.fastThreshold = 40;
    settings.surfThreshold = 2500;
    settings.siftEdgeThres = 10;
    settings.siftContrastThres = 0.03;
    settings.blurSigma = 0.1;

    filters.flags = vector<bool>(23, false);
    currentFrame = cv::Mat();
    processedFrame = cv::Mat();
}

ProcessingThread::~ProcessingThread()
{
}

void ProcessingThread::run()
{
    while(1)
    {
        // Check if paused
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
            stopped = false;
            stoppedMutex.unlock();
            break;
        }
        stoppedMutex.unlock();
        /////////////////////////////////
        /////////////////////////////////

        inputMutex.lock();
        if (inputMode != INPUT_IMAGE)
        {
            inputMutex.unlock();
            currentFrame = outputBuffer->getFrame();
        }
        else
        {
            inputMutex.unlock();
            if (outputBuffer->getSizeOfImageBuffer() > 0)
            {
                currentFrame = outputBuffer->getFrame();
            }
            msleep(50);
        }
        inputMutex.unlock();

        updM.lock();
        ////////////////////////////////////
        // PERFORM IMAGE PROCESSING BELOW //
        ////////////////////////////////////

        cv::Mat outputIm = currentFrame.clone();

        if (filters.flags[ImageProcessingFlags::ConvertColorspace])
        {
            switch (settings.colorSpace)
            {
            case 0:
            { // Gray
                cv::cvtColor(currentFrame,outputIm, CV_RGB2GRAY);
            } break;
            case 1:
            { // HSV
                cv::cvtColor(currentFrame,outputIm, CV_RGB2HSV);
            } break;
            case 3:
            { // Lba
                cv::cvtColor(currentFrame,outputIm, CV_RGB2Lab);
            } break;
            }
        }

        if (filters.flags[ImageProcessingFlags::SaltPepperNoise])
        {
            for (int i=0; i<settings.saltPepperNoiseDensity; i+=1)
            { // adding noise
                // generate randomly the col and row
                int m = qrand() % outputIm.rows;
                int n = qrand() % outputIm.cols;

                // generate randomly the value {black, white}
                int color_ = ((qrand() % 100) > 50) ? 255 : 0;

                if (outputIm.channels() == 1)
                { // gray-level image
                    outputIm.at<uchar>(m, n)= color_;
                }
                else if (outputIm.channels() == 3)
                { // color image
                    outputIm.at<cv::Vec3b>(m, n)[0]= color_;
                    outputIm.at<cv::Vec3b>(m, n)[1]= color_;
                    outputIm.at<cv::Vec3b>(m, n)[2]= color_;
                }
            }
        }

        if (filters.flags[ImageProcessingFlags::Dilate])
        {
            cv::dilate(outputIm,
                       outputIm,
                       cv::Mat(),
                       cv::Point(-1, -1),
                       settings.dilateIterations);
        }

        if (filters.flags[ImageProcessingFlags::Erode])
        {
            cv::erode(outputIm,
                      outputIm,
                      cv::Mat(),
                      cv::Point(-1, -1),
                      settings.erodeIterations);
        }

        if (filters.flags[ImageProcessingFlags::Open])
        {
            cv::morphologyEx(outputIm,
                             outputIm,
                             cv::MORPH_OPEN,
                             cv::Mat(),
                             cv::Point(-1, -1),
                             settings.openIterations);
        }

        if (filters.flags[ImageProcessingFlags::Close])
        {
            cv::morphologyEx(outputIm,
                             outputIm,
                             cv::MORPH_CLOSE,
                             cv::Mat(),
                             cv::Point(-1, -1),
                             settings.openIterations);
        }

        if (filters.flags[ImageProcessingFlags::Blur])
        {
            cv::GaussianBlur(outputIm,
                             outputIm,
                             cv::Size(settings.blurSize, settings.blurSize),
                             settings.blurSigma);
        }

        if (filters.flags[ImageProcessingFlags::Sobel])
        {
            int scale = 1;
            int delta = 0;
            int ddepth = CV_16S;

            // check the direction
            switch (settings.sobelDirection)
            {
            case 0:
            { // horizontal
                cv::Mat grad_x;
                cv::Sobel( outputIm, grad_x, ddepth, 1, 0, settings.sobelKernelSize, scale, delta, BORDER_DEFAULT );
                cv::convertScaleAbs( grad_x, outputIm );
            } break;
            case 1:
            { // vertical
                cv::Mat grad_y;
                cv::Sobel( outputIm, grad_y, ddepth, 0, 1, settings.sobelKernelSize, scale, delta, BORDER_DEFAULT );
                cv::convertScaleAbs( grad_y, outputIm );
            } break;
            case 2:
            { // both directions
                cv::Mat grad_x;
                cv::Mat grad_y;
                cv::Mat abs_grad_x;
                cv::Mat abs_grad_y;
                cv::Sobel( outputIm, grad_x, ddepth, 1, 0, settings.sobelKernelSize, scale, delta, BORDER_DEFAULT );
                cv::Sobel( outputIm, grad_y, ddepth, 0, 1, settings.sobelKernelSize, scale, delta, BORDER_DEFAULT );
                cv::convertScaleAbs( grad_x, abs_grad_x );
                cv::convertScaleAbs( grad_y, abs_grad_y );

                cv::addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, outputIm );
            } break;
            }
        }

        if (filters.flags[ImageProcessingFlags::Laplacian])
        {
            int scale = 1;
            int delta = 0;
            int ddepth = CV_16S;

            cv::Laplacian( outputIm, outputIm, ddepth, settings.laplacianKernelSize, scale, delta, BORDER_DEFAULT );
            cv::convertScaleAbs( outputIm, outputIm );
        }

        if (filters.flags[ImageProcessingFlags::SharpByKernel])
        {
            cv::Mat kernel(3,3,CV_32F,cv::Scalar(0));// init the kernel with zeros
            // assigns kernel values
            kernel.at<float>(1,1)= settings.sharpKernelCenter;
            kernel.at<float>(0,1)= -1.0;
            kernel.at<float>(2,1)= -1.0;
            kernel.at<float>(1,0)= -1.0;
            kernel.at<float>(1,2)= -1.0;
            //filter the image
            cv::filter2D(outputIm,outputIm,outputIm.depth(),kernel);
        }

        if (filters.flags[ImageProcessingFlags::EdgeDetection])
        { // with canny
            cv::Canny(outputIm, outputIm, settings.cannyLowThres, settings.cannyHighThres);
        }

        if (filters.flags[ImageProcessingFlags::LinesHough])
        {
            // Apply Canny algorithm
            cv::Mat contours;
            cv::Canny(outputIm,contours,125,350);

            // Hough tranform for line detection
            vector<cv::Vec2f> lines;
            cv::HoughLines(contours,lines, 1, PI/180, settings.linesHoughVotes);

            vector<cv::Vec2f>::const_iterator it= lines.begin();

            while (it!=lines.end())
            {
                float rho = (*it)[0]; // first element is distance rho
                float theta = (*it)[1]; // second element is angle theta
                if (theta < PI/4. || theta > 3.*PI/4.)
                {// ~vertical line
                    // point of intersection of the line with first row
                    cv::Point pt1(rho/cos(theta),0);
                    // point of intersection of the line with last row
                    cv::Point pt2((rho-contours.rows*sin(theta))/cos(theta),contours.rows);
                    // draw a white line
                    cv::line( outputIm, pt1, pt2, cv::Scalar(255), 1);
                }
                else
                { // ~horizontal line
                    // point of intersection of the line with first column
                    cv::Point pt1(0,rho/sin(theta));
                    // point of intersection of the line with last column
                    cv::Point pt2(contours.cols, (rho-contours.cols*cos(theta))/sin(theta));
                    // draw a white line
                    cv::line(outputIm, pt1, pt2, cv::Scalar(255), 1);
                }
                ++it;
            }
        }

        if (filters.flags[ImageProcessingFlags::CirclesHough])
        {
            cv::Mat temp;
            if (outputIm.channels() > 1)
            {
                cv::cvtColor(outputIm, temp, CV_RGB2GRAY);
            }
            else
            {
                temp = outputIm;
            }

            cv::GaussianBlur(temp, temp, cv::Size(5,5), 1.5);
            vector<cv::Vec3f> circles;

            cv::HoughCircles(temp, circles, CV_HOUGH_GRADIENT,
                            2,    // accumulator resolution (size of the image / 2)
                            50,   // minimum distance between two circles
                            200,  // Canny high threshold
                            60,   // minimum number of votes
                            settings.circlesHoughMin,
                            settings.circlesHoughMax);

            std::vector<cv::Vec3f>::const_iterator itc= circles.begin();
            while (itc!=circles.end())
            {
                cv::circle(outputIm,
                        cv::Point((*itc)[0], (*itc)[1]), // circle centre
                                (*itc)[2],               // circle radius
                                cv::Scalar(255),         // color
                                2);                      // thickness
                ++itc;
            }
        }

        if (filters.flags[ImageProcessingFlags::Countours])
        {
            cv::Mat temp;
            if (outputIm.channels() > 1)
            {
                cv::cvtColor(outputIm, temp, CV_RGB2GRAY);
            }
            else
            {
                temp = outputIm;
            }

            cv::blur(temp, temp, Size(3,3));
            cv::Canny(temp, temp, settings.contoursThres, settings.contoursThres+30);

            vector< vector<cv::Point> > contours;
            cv::findContours(temp,
                            contours,              // a vector of contours
                            CV_RETR_TREE,          // retrieve all contours, reconstructs a full hierarchy
                            CV_CHAIN_APPROX_NONE); // all pixels of each contours

            cv::drawContours(outputIm,contours,
                            -1,                        // draw all contours
                            cv::Scalar(255, 255, 255), // in white
                            1);                        // with a thickness of 1
        }

        if (filters.flags[ImageProcessingFlags::BoundingBox])
        {
            cv::Mat temp;
            if (outputIm.channels() > 1)
            {
                cv::cvtColor(outputIm, temp, CV_RGB2GRAY);
            }
            else
            {
                temp = outputIm;
            }

            cv::blur(temp, temp, Size(3,3));
            cv::Canny(temp, temp, settings.boundingBoxThres, settings.boundingBoxThres*2);

            vector< vector<cv::Point> > contours;
            cv::findContours(temp,
                            contours,              // a vector of contours
                            CV_RETR_TREE,          // retrieve all contours, reconstructs a full hierarchy
                            CV_CHAIN_APPROX_NONE); // all pixels of each contours

            vector< vector<cv::Point> >::iterator itc = contours.begin();
            while (itc != contours.end())
            {
                cv::Rect r0 = cv::boundingRect(cv::Mat(*itc));
                cv::rectangle(outputIm,r0,cv::Scalar(255, 0, 0), 2);
                ++itc;
            }
        }

        if (filters.flags[ImageProcessingFlags::enclosingCircle])
        {
            cv::Mat temp;
            if (outputIm.channels() > 1)
            {
                cv::cvtColor(outputIm, temp, CV_RGB2GRAY);
            }
            else
            {
                temp = outputIm;
            }

            cv::blur(temp, temp, Size(3,3));
            cv::Canny(temp, temp, settings.enclosingCircleThres, settings.enclosingCircleThres*2);

            vector< vector<cv::Point> > contours;
            cv::findContours(temp,
                            contours,              // a vector of contours
                            CV_RETR_TREE,          // retrieve all contours, reconstructs a full hierarchy
                            CV_CHAIN_APPROX_NONE); // all pixels of each contours

            vector< vector<cv::Point> >::iterator itc = contours.begin();
            while (itc != contours.end())
            {
                float radius;
                cv::Point2f center;
                cv::minEnclosingCircle(cv::Mat(*itc),center,radius);
                cv::circle(outputIm, center,
                        static_cast<int>(radius),
                        cv::Scalar(0, 255, 0),
                        2);
                ++itc;
            }
        }

        if (filters.flags[ImageProcessingFlags::harris])
        {
            cv::Mat temp;
            if (outputIm.channels() > 1)
            {
                cv::cvtColor(outputIm, temp, CV_RGB2GRAY);
            }
            else
            {
                temp = outputIm;
            }

            // Detector parameters
            int blockSize = 2;
            int apertureSize = 3;
            double k = 0.04;

            // Detecting corners
            cv::cornerHarris(temp, temp, blockSize, apertureSize, k, BORDER_DEFAULT);

            // Normalizing
            normalize(temp,temp, 0, 255, NORM_MINMAX, CV_32FC1, Mat());

            // Drawing a circle around corners
            for( int j = 0; j < temp.rows ; j++ )
            {
                for( int i = 0; i < temp.cols; i++ )
                {
                    if( (int) temp.at<float>(j,i) > settings.harrisCornerThres)
                    {
                        circle(outputIm, Point( i, j ), 5,  Scalar(0, 0 , 255), 2, 8, 0);
                    }
                }
            }
        }

        if (filters.flags[ImageProcessingFlags::FAST])
        {
            // vector of keypoints
            vector<cv::KeyPoint> keypoints;
            // Construction of the Fast feature detector object
            cv::FastFeatureDetector fast(settings.fastThreshold); // threshold for detection
            // feature point detection
            fast.detect(outputIm,keypoints);

            cv::drawKeypoints(outputIm, keypoints, outputIm, cv::Scalar(255,255,255), cv::DrawMatchesFlags::DRAW_OVER_OUTIMG);
        }

        if (filters.flags[ImageProcessingFlags::SURF])
        {
            // vector of keypoints
            vector<cv::KeyPoint> keypoints;
            // Construct the SURF feature detector object
            cv::SurfFeatureDetector surf((double) settings.surfThreshold); // threshold
            // Detect the SURF features
            surf.detect(outputIm,keypoints);

            // Draw the keypoints with scale and orientation information
            cv::drawKeypoints(outputIm, keypoints, outputIm, cv::Scalar(255,255,255),cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        }

        if (filters.flags[ImageProcessingFlags::SIFT])
        {

            vector<cv::KeyPoint> keypoints;
            // Construct the SURF feature detector object
            cv::SiftFeatureDetector sift( settings.siftContrastThres,        // feature threshold
                                          (double) settings.siftEdgeThres); // threshold to reduce sens. to lines

            sift.detect(outputIm,keypoints);
            // Draw the keypoints with scale and orientation information
            cv::drawKeypoints(outputIm, keypoints, outputIm, cv::Scalar(255,255,255),cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        }

        if (filters.flags[ImageProcessingFlags::EqualizeHistogram])
        {
            // converting the image to gray
            if (outputIm.channels() == 3)
            {
                vector<Mat> bgr_planes;
                split( outputIm, bgr_planes );
                equalizeHist( bgr_planes[0], bgr_planes[0] );
                equalizeHist( bgr_planes[1], bgr_planes[1] );
                equalizeHist( bgr_planes[2], bgr_planes[2] );
                merge( bgr_planes, outputIm );
            }
            else
            {
                equalizeHist( outputIm, outputIm );
            }
        }

        // Computing histogram
        if (filters.flags[ImageProcessingFlags::ComputeHistogram])
        {
            cv::Mat grayIm;
            cv::Mat hist;
            // converting the image to gray
            if (outputIm.channels() == 3)
            {
                 cv::cvtColor(outputIm,grayIm, CV_RGB2GRAY);
            }
            else
            {
                grayIm = outputIm;
            }

            int histSize = 256;           // number of bins
            float range [] = {0, 256};    // ranges
            const float* histRange = { range };
            bool uniform = true, accumulate = false;

            // compute histogram
            cv::calcHist(&grayIm,
                         1,  // using just one image
                         0,  // using just one layer
                         cv::Mat(),
                         hist,
                         1,
                         &histSize,
                         &histRange,
                         uniform,
                         accumulate);


            int hist_w = 691; int hist_h =161;
            Mat result( hist_h, hist_w, CV_8UC3, Scalar( 255,255,255) );
            int bin_w = cvRound( (double) hist_w/histSize );
            normalize(hist, hist, 0, result.rows, NORM_MINMAX, -1, Mat());

            /// Draw for each channel
            for( int i = 1; i < histSize; i++ )
            {
                line(result,
                     Point( bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ),
                     Point( bin_w*(i), hist_h - cvRound(hist.at<float>(i)) ),
                     Scalar( 0, 0, 0), 2, 8, 0  );
            }
            // emit signal
            emit newProcessedHistogram(MatToQImage(result));
        }

        updM.unlock();

        processedFrame =  outputIm;
        // Inform GUI thread of new frame (QImage)
        emit newProcessedFrame(MatToQImage(outputIm));
    }
}

void ProcessingThread::stopProcessingThread()
{
    stoppedMutex.lock();
    stopped = true;
    stoppedMutex.unlock();
}


int ProcessingThread::getCurrentSizeOfBuffer()
{
    return outputBuffer->getSizeOfImageBuffer();
}

void ProcessingThread::updateFlags(int index, bool status)
{
    QMutexLocker locker(&updM);
    filters.flags[index] = status;
}
