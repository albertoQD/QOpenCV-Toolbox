#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QtGui>
#include <vector>

using namespace std;

// ImageProcessingSettings structure definition
struct ImageProcessingSettings{
    int saltPepperNoiseDensity;
    int colorSpace;
    int dilateIterations;
    int erodeIterations;
    int openIterations;
    int closeIterations;
    int blurSize;
    int sobelDirection;
    int sobelKernelSize;
    int laplacianKernelSize;
    int sharpKernelCenter;
    int cannyLowThres;
    int cannyHighThres;
    int linesHoughVotes;
    int circlesHoughMin;
    int circlesHoughMax;
    int contoursThres;
    int boundingBoxThres;
    int enclosingCircleThres;
    int harrisCornerThres;
    int fastThreshold;
    int surfThreshold;
    int siftEdgeThres;
    double siftContrastThres;
    double blurSigma;
};

// ImageProcessingFlags structure definition
struct ImageProcessingFlags{
    vector<bool> flags;
    enum filters {
        SaltPepperNoise,
        ShowLogo,
        ConvertColorspace,
        ComputeHistogram,
        EqualizeHistogram,
        Dilate,
        Erode,
        Open,
        Close,
        Blur,
        Sobel,
        Laplacian,
        SharpByKernel,
        EdgeDetection,
        LinesHough,
        CirclesHough,
        Countours,
        BoundingBox,
        enclosingCircle,
        harris,
        FAST,
        SURF,
        SIFT
    };
};

#endif // STRUCTURES_H
