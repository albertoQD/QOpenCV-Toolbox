#ifndef STEREOMODULE_H
#define STEREOMODULE_H

#include <QWidget>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>

using namespace std;

namespace Ui {
class StereoModule;
}

class StereoModule : public QWidget
{
    Q_OBJECT
    
public:
    explicit StereoModule(QWidget *parent = 0);
    ~StereoModule();
    
private slots:
    void on_actionLoad_left_image_triggered();

    void on_actionLoad_right_image_triggered();

    void on_comboBox_currentIndexChanged(int index);

    void on_action7_Points_triggered();

    void on_actionRANSAC_triggered();

    void on_action8_Points_Method_triggered();

    void on_actionDraw_Epipolar_Lines_triggered();

    void on_actionHomography_Matrix_triggered();

    void on_actionGenerate_Mosaic_triggered();

    void on_actionSave_as_triggered();

    void on_actionCalibrate_Camera_triggered();

private:
    void drawMatrix(int);
    void computeFundamental(int);
    void computeHomography();
    void loadImage(int);

    Ui::StereoModule *ui;
    QString leftFilename;
    QString rightFilename;
    cv::Mat leftImage;
    cv::Mat rightImage;
    cv::Mat resultingImage;
    cv::Mat fundamental;
    cv::Mat homography;
    cv::Mat cameraMatrix;
    vector< cv::Point2f > points_im1;
    vector< cv::Point2f > points_im2;
};

#endif // STEREOMODULE_H
