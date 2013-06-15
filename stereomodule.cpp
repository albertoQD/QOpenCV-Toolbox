#include "stereomodule.h"
#include "ui_stereomodule.h"
#include "mattoqimage.h"
#include <math.h>
#include <iostream>

using namespace std;

StereoModule::StereoModule(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StereoModule)
{
    ui->setupUi(this);

    leftImage      = cv::Mat();
    rightImage     = cv::Mat();
    resultingImage = cv::Mat();
    cameraMatrix   = Mat::zeros(3, 3, CV_32FC1);
    fundamental    = Mat::eye(3,3, CV_32FC1);
    homography     = Mat::eye(3,3, CV_32FC1);

    // draw first time the fundamental (identity)
    drawMatrix(0);

    points_im1 = vector<cv::Point2f>(0);
    points_im2 = vector<cv::Point2f>(0);
}

StereoModule::~StereoModule()
{
    delete ui;
}

void StereoModule::loadImage(int side)
{
    if (side == 0)
    { // left
        leftImage = cv::imread(leftFilename.toStdString());
        // resize the frame to fit in the UI frames (375x181)
        cv::resize(leftImage, leftImage, cv::Size(369, 181));
        ui->leftImageLabel->setPixmap(QPixmap::fromImage(MatToQImage(leftImage)));
    }
    else if (side == 1)
    { // right
        rightImage = cv::imread(rightFilename.toStdString());
        // resize the frame to fit in the UI frames (375x181)
        cv::resize(rightImage, rightImage, cv::Size(369, 181));
        ui->rightImageLabel->setPixmap(QPixmap::fromImage(MatToQImage(rightImage)));
    }
}

void StereoModule::on_actionLoad_left_image_triggered()
{
    QString filename = QFileDialog::getOpenFileName(
            this,
            tr("Select Image(s) to Open"),
            QDir::toNativeSeparators(QDir::homePath()),
            tr("Images (*.png *.bmp *.jpg *.jpeg *.tiff)"));

    if (!filename.isEmpty())
    {
        leftFilename = filename;
        loadImage(0);
    }
}

void StereoModule::on_actionLoad_right_image_triggered()
{
    QString filename = QFileDialog::getOpenFileName(
            this,
            tr("Select Image(s) to Open"),
            QDir::toNativeSeparators(QDir::homePath()),
            tr("Images (*.png *.bmp *.jpg *.jpeg *.tiff)"));

    if (!filename.isEmpty())
    {
        rightFilename = filename;
        loadImage(1);
    }
}

void StereoModule::drawMatrix(int type)
{
    cv::Mat temp;
    if (type == 0 && !fundamental.empty())
    { // fundamental
        temp = fundamental.clone();
    }
    else if (type == 1 && !homography.empty())
    { // homography
        temp = homography.clone();
    }
    else if (type == 2 && !cameraMatrix.empty())
    {
        temp = cameraMatrix.clone();
    }
    else {
        return;
    }

    ui->matrixLeftTopLabel->setText(     QString().setNum(ceil(temp.at<double>(0,0)*100)/100));
    ui->matrixCenterTopLabel->setText(   QString().setNum(ceil(temp.at<double>(0,1)*100)/100));
    ui->matrixRightTopLabel->setText(    QString().setNum(ceil(temp.at<double>(0,2)*100)/100));
    ui->matrixLeftMiddleLabel->setText(  QString().setNum(ceil(temp.at<double>(1,0)*100)/100));
    ui->matrixCenterMiddleLabel->setText(QString().setNum(ceil(temp.at<double>(1,1)*100)/100));
    ui->matrixRightMiddleLabel->setText( QString().setNum(ceil(temp.at<double>(1,2)*100)/100));
    ui->matrixLeftBottomLabel->setText(  QString().setNum(ceil(temp.at<double>(2,0)*100)/100));
    ui->matrixCenterBottomLabel->setText(QString().setNum(ceil(temp.at<double>(2,1)*100)/100));
    ui->matrixRightBottomLabel->setText( QString().setNum(ceil(temp.at<double>(2,2)*100)/100));

}

void StereoModule::on_comboBox_currentIndexChanged(int index)
{
    drawMatrix(index);
}

void StereoModule::computeFundamental(int type)
{
    loadImage(0);
    loadImage(1);

    int type_;
    if (type == 0)
    { // 7 points
        type_ = FM_7POINT;
    }
    else if (type == 1)
    { // 8 points
        type_ = FM_8POINT;
    }
    else if (type == 2)
    { // ransac
        type_ = FM_RANSAC;
    }

    cv::Mat tLeft;
    cv::Mat tRight;

    cv::cvtColor(rightImage, tRight, CV_RGB2GRAY);
    cv::cvtColor(leftImage, tLeft, CV_RGB2GRAY);

    vector<cv::KeyPoint> keypoints1;
    vector<cv::KeyPoint> keypoints2;
    cv::Mat descriptors1;
    cv::Mat descriptors2;

    points_im1 = vector<cv::Point2f>(0);
    points_im2 = vector<cv::Point2f>(0);

    // Construct the SURF feature detector object
    cv::SurfFeatureDetector surf( 2500 ); // threshold

    // Construction of the SURF descriptor extractor
    cv::SurfDescriptorExtractor surfDesc;

    // Detect the SURF features
    surf.detect(tLeft,keypoints1);
    surf.detect(tRight,keypoints2);

    // Extraction of the SURF descriptors
    surfDesc.compute(tLeft,keypoints1,descriptors1);
    surfDesc.compute(tRight,keypoints2,descriptors2);

    // Construction of the matcher
    FlannBasedMatcher matcher;
    // Match the two image descriptors
    vector<cv::DMatch> matches;
    matcher.match(descriptors1,descriptors2, matches);

    for( unsigned int i = 0; i < matches.size(); i++ )
    {
        points_im1.push_back( keypoints1[ matches[i].queryIdx ].pt );
        points_im2.push_back( keypoints2[ matches[i].trainIdx ].pt );
    }

    fundamental = cv::findFundamentalMat(points_im1,points_im2, type_, 5);

}

void StereoModule::on_action7_Points_triggered()
{
    computeFundamental(0);
    if (ui->comboBox->currentIndex() == 0)
        drawMatrix(0);
}

void StereoModule::on_actionRANSAC_triggered()
{
    computeFundamental(2);
    if (ui->comboBox->currentIndex() == 0)
        drawMatrix(0);
}

void StereoModule::on_action8_Points_Method_triggered()
{
    computeFundamental(1);
    if (ui->comboBox->currentIndex() == 0)
        drawMatrix(0);
}

void StereoModule::on_actionDraw_Epipolar_Lines_triggered()
{
    loadImage(0);
    loadImage(1);

    vector<cv::Vec3f> lines1, lines2;
    cv::computeCorrespondEpilines(points_im1, 1, fundamental, lines1);
    cv::computeCorrespondEpilines(points_im2, 2, fundamental, lines2);
    // vector of epipolar lines
    // for all epipolar lines
    vector<cv::Vec3f>::const_iterator it= lines1.begin();
    for (it; it!=lines1.end(); ++it)
    {
        // draw the line between first and last column
        cv::line(rightImage,
            cv::Point(0,-(*it)[2]/(*it)[1]),
            cv::Point(rightImage.cols,-((*it)[2]+
            (*it)[0]*rightImage.cols)/(*it)[1]),
            cv::Scalar(255,255,255));
    }

    it= lines2.begin();
    for (it; it!=lines2.end(); ++it)
    {
        // draw the line between first and last column
        cv::line(leftImage,
            cv::Point(0,-(*it)[2]/(*it)[1]),
            cv::Point(leftImage.cols,-((*it)[2]+
            (*it)[0]*leftImage.cols)/(*it)[1]),
            cv::Scalar(255,255,255));
    }

    // update images
    ui->rightImageLabel->setPixmap(QPixmap::fromImage(MatToQImage(rightImage)));
    ui->leftImageLabel->setPixmap(QPixmap::fromImage(MatToQImage(leftImage)));
}

void StereoModule::on_actionHomography_Matrix_triggered()
{
    computeHomography();

    if (ui->comboBox->currentIndex() == 1)
        drawMatrix(1);

}

void StereoModule::computeHomography()
{
    loadImage(0);
    loadImage(1);

    if (points_im1.size() == 0) computeFundamental(3);

    homography = cv::findHomography(points_im2, points_im1, CV_RANSAC, 5);
}

void StereoModule::on_actionGenerate_Mosaic_triggered()
{
    loadImage(0);
    loadImage(1);

    if (points_im1.size() == 0) computeHomography();

    cv::warpPerspective(rightImage, resultingImage, homography,cv::Size(4*rightImage.cols, rightImage.rows));

    cv::Mat half(resultingImage,cv::Rect(0,0,leftImage.cols,leftImage.rows));
    leftImage.copyTo(half);

    // showing resultingImage
    ui->outputImageLabel->setPixmap(QPixmap::fromImage(MatToQImage(resultingImage)));
}

void StereoModule::on_actionSave_as_triggered()
{
    if (resultingImage.empty())
    {
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
            this,
            tr("Save Image"),
            QDir::toNativeSeparators(QDir::homePath()),
            tr("PNG Files (*.png)") );

    if (filename.isEmpty())
        return;

    if (filename.mid(filename.size()-4) != ".png")
        filename += ".png";

    QImage generatedImg = QImage(MatToQImage(resultingImage));

    // saving image
    generatedImg.save(filename);
}

void StereoModule::on_actionCalibrate_Camera_triggered()
{
    // preguntar en que dir estan las imagenes
    QString dirStr = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                     QDir::toNativeSeparators(QDir::homePath()),
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);

    if (!dirStr.isEmpty())
    {
        QDir dir(dirStr);
        if (!dir.exists())
        {
            return;
        }

        QStringList formats;
        formats << "*.JPG" << "*.JPE" << ".JPEG";
        formats << "*.TIFF" << "*.TIF";
        formats << "*.PNG";

        QStringList files = dir.entryList(formats,
                      QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable);
        if (!files.isEmpty())
        {
            cv::Size boardSize(6,4);
            vector< cv::Mat > images;
            vector< vector<cv::Point2f> > imagePoints;
            vector< vector<cv::Point3f> > objectPoints;

            for (int t=0; t<files.size(); t+=1)
            { // dirStr+'/'+files[i]
                vector<cv::Point2f> corn;
                vector<cv::Point3f> objectCorners;
                cv::Mat image = cv::imread(QString(dirStr+'/'+files[t]).toStdString());
                cv::Mat imGray;
                cv::cvtColor(image, imGray, CV_RGB2GRAY);

                bool found = cv::findChessboardCorners(image, boardSize, corn);

                if (found)
                {
                    cv::drawChessboardCorners(image, boardSize, corn, found); // corners have been found
                    // Get subpixel accuracy on the corners
                    cornerSubPix( imGray, corn, Size(11,11),
                                        Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
                }

                //If we have a good board, add it to our data

                if (corn.size() == boardSize.area())
                {
                    for (int i=0; i<boardSize.height; i++)
                    {
                        for (int j=0; j<boardSize.width; j++)
                        {
                            objectCorners.push_back(cv::Point3f(i, j, 0.0f));
                        }
                    }
                    images.push_back(image);
                    imagePoints.push_back(corn);
                    objectPoints.push_back(objectCorners);
                }
            }
            cv::Mat distCoeffs;
            cv::Size imgSize(images[0].rows, images[0].cols);
            vector<cv::Mat> rvecs, tvecs;
            cv::calibrateCamera(objectPoints, // the 3D points
                            imagePoints,
                            imgSize,
                            cameraMatrix,
                            distCoeffs,
                            rvecs, tvecs);
            if (ui->comboBox->currentIndex() == 2)
                drawMatrix(2);

            // show the first two images
            if (images.size() > 1)
                ui->leftImageLabel->setPixmap(QPixmap::fromImage(MatToQImage(images[0])));

            if (images.size() > 2)
                ui->rightImageLabel->setPixmap(QPixmap::fromImage(MatToQImage(images[1])));

        }
    }
}
