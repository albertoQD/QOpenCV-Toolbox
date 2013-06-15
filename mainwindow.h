#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QListWidgetItem>

namespace Ui {
    class MainWindow;
}

class Controller;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void loadImage();
    void loadVideo();
    void saveImageAs();
    void OpenStereoModule();

    void connectToCamera();
    void about();

private:
    void connectSignals();
    void hideAllConfigFrames();
    Ui::MainWindow *ui;

    Controller *controller;
    bool isCameraConnected;
    int sourceWidth;
    int sourceHeight;
    int deviceNumber;
    int imageBufferSize;

private slots:
    void updateInputFrame(QImage);
    void updateOutputFrame(QImage);
    void updateHistogramFrame(QImage);
    void on_filtersList_clicked(const QModelIndex &index);
    void on_filtersList_itemChanged(QListWidgetItem *item);
    void on_noiseDensityThresSL_valueChanged(int value);
    void on_colorSpaceCB_currentIndexChanged(int index);
    void on_dilateSB_valueChanged(int arg1);
    void on_erodeSB_valueChanged(int arg1);
    void on_openSB_valueChanged(int arg1);
    void on_closeSB_valueChanged(int arg1);
    void on_blurSizeSB_valueChanged(int value);
    void on_blurSigmaSB_valueChanged(double arg1);
    void on_sobelDirectionCB_currentIndexChanged(int index);
    void on_sobelKernelSizeSB_valueChanged(int arg1);
    void on_laplacianKernelSizeSB_valueChanged(int arg1);
    void on_sharpKernelCenterSB_valueChanged(int arg1);
    void on_cannyLowThresSB_valueChanged(int arg1);
    void on_cannyHighThresSB_valueChanged(int arg1);
    void on_linesHoughVotesSB_valueChanged(int arg1);
    void on_circlesHoughMinSB_valueChanged(int arg1);
    void on_circlesHoughMaxSB_valueChanged(int arg1);
    void on_contourThresSL_valueChanged(int value);
    void on_boundingBoxThresSL_valueChanged(int value);
    void on_enclosingCirclesThresSL_valueChanged(int value);
    void on_harrisCornerThresSL_valueChanged(int value);
    void on_fastThresSL_valueChanged(int value);
    void on_surfThresSL_valueChanged(int value);
    void on_siftContrastThresSB_valueChanged(double arg1);
    void on_siftEdgeThresSL_valueChanged(int value);
    void on_selectLogoFileBtn_clicked();
    void on_playBtn_clicked();
};

#endif // MAINWINDOW_H
