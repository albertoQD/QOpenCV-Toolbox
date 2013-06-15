#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stereomodule.h"

#include "controller.h"
#include "config.h"
#include "mattoqimage.h"

#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QListWidgetItem>

using namespace cv;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isCameraConnected(false)
{
    ui->setupUi(this);

    // disabling the filter list
    ui->filtersList->setEnabled(false);
    // Create controller
    controller = new Controller;
    connectSignals();
    // Initialize flag
    isCameraConnected=false;
    // hide all parameters frames
    hideAllConfigFrames();

    for (int i=0; i<ui->filtersList->count(); i+=1)
    {
        ui->filtersList->item(i)->setCheckState(Qt::Unchecked);
    }

    // disable the play button
    ui->playBtn->setEnabled(false);
}

MainWindow::~MainWindow()
{
    controller->stopThreads();
    controller->deleteThreads();
    delete ui;
}

void MainWindow::connectToCamera()
{
    // Connect to camera
    if((isCameraConnected = controller->connectToCamera()))
    {
        controller->processingThread->setInputMode(INPUT_CAMERA);

        connect(controller, SIGNAL(newInputFrame(QImage)), this, SLOT(updateInputFrame(QImage)));
        connect(controller->processingThread, SIGNAL(newProcessedFrame(QImage)), this, SLOT(updateOutputFrame(QImage)));
        connect(controller->processingThread, SIGNAL(newProcessedHistogram(QImage)), this, SLOT(updateHistogramFrame(QImage)));

        // enabling filterlist
        ui->filtersList->setEnabled(true);
        // enable play btn
        ui->playBtn->setEnabled(true);
    }
    else
    {
        QMessageBox::warning(this,"ERROR:","Could not connect to camera.");
    }
}

void MainWindow::connectSignals()
{
    connect(ui->loadVideoAction, SIGNAL(triggered()), this, SLOT(loadVideo()));
    connect(ui->loadImgAction, SIGNAL(triggered()), this, SLOT(loadImage()));
    connect(ui->connectCamAction, SIGNAL(triggered()), this, SLOT(connectToCamera()));
    connect(ui->exitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->saveImgAction, SIGNAL(triggered()), this, SLOT(saveImageAs()));
    connect(ui->aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->stereoModuleAction, SIGNAL(triggered()), this, SLOT(OpenStereoModule()));

    connect(ui->selectROIBtn, SIGNAL(clicked()), ui->inputLabel, SLOT(selectROI()));
    connect(ui->inputLabel, SIGNAL(roiSelected(QRect, QPoint)), controller, SLOT(setLogoROI(QRect, QPoint)));
}

void MainWindow::OpenStereoModule()
{
    StereoModule * stereo = new StereoModule();
    stereo->show();
}


void MainWindow::saveImageAs()
{
    cv::Mat frame = controller->processingThread->getProcessedFrame();

    if (frame.empty())
    {
        statusBar()->showMessage(tr("Must generate a processed image first"));
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

    QImage generatedImg = QImage(MatToQImage(frame));

    // saving image
    if (generatedImg.save(filename))
    {
        statusBar()->showMessage(tr("Image successfully saved in ") + filename);
    }
    else
    {
        statusBar()->showMessage(tr("Error saving image"));
    }
}


void MainWindow::loadImage()
{
    QString filename = QFileDialog::getOpenFileName(
            this,
            tr("Select Image(s) to Open"),
            QDir::toNativeSeparators(QDir::homePath()),
            tr("Images (*.png *.bmp *.jpg *.jpeg *.tiff)"));

    if (!filename.isEmpty())
    {
        controller->readImage(filename);

        controller->processingThread->setInputMode(INPUT_VIDEO);

        connect(controller, SIGNAL(newInputFrame(QImage)), this, SLOT(updateInputFrame(QImage)));
        connect(controller->processingThread, SIGNAL(newProcessedFrame(QImage)), this, SLOT(updateOutputFrame(QImage)));
        connect(controller->processingThread, SIGNAL(newProcessedHistogram(QImage)), this, SLOT(updateHistogramFrame(QImage)));

        // enabling filterlist
        ui->filtersList->setEnabled(true);
        // enable play btn
        ui->playBtn->setEnabled(true);
    }
}


void MainWindow::on_selectLogoFileBtn_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
            this,
            tr("Select Image(s) to Open"),
            QDir::toNativeSeparators(QDir::homePath()),
            tr("Images (*.png *.bmp *.jpg *.jpeg *.tiff)"));

    if (!filename.isEmpty())
    {
        if (controller->loadLogo(filename))
        {
            ui->logoFilenameLabel->setText(
                        QFileInfo(filename).fileName());
        }
    }
}


void MainWindow::loadVideo()
{
    QString filename = QFileDialog::getOpenFileName(
            this,
            tr("Select Video to Open"),
            QDir::toNativeSeparators(QDir::homePath()),
            tr("Videos (*.avi)"));

    if (!filename.isEmpty())
    {
        if (controller->readVideo(filename))
        {
            controller->processingThread->setInputMode(INPUT_VIDEO);

            connect(controller, SIGNAL(newInputFrame(QImage)), this, SLOT(updateInputFrame(QImage)));
            connect(controller->processingThread, SIGNAL(newProcessedFrame(QImage)), this, SLOT(updateOutputFrame(QImage)));
            connect(controller->processingThread, SIGNAL(newProcessedHistogram(QImage)), this, SLOT(updateHistogramFrame(QImage)));

            // enabling filterlist
            ui->filtersList->setEnabled(true);
            // enable play btn
            ui->playBtn->setEnabled(true);
        }
    }
}

void MainWindow::about()
{
    QMessageBox::information(this,"About",QString("Created by Alberto QUINTERO DELGADO\nMaster Student - 2013"));
}

void MainWindow::updateInputFrame(QImage input)
{
    // Display input image in inputlabel
    ui->inputLabel->setPixmap(QPixmap::fromImage(input));
}

void MainWindow::updateOutputFrame(QImage output)
{
    // Display output image in inputlabel
    ui->outputLabel->setPixmap(QPixmap::fromImage(output));
}

void MainWindow::updateHistogramFrame(QImage hist)
{
    ui->histogramLabel->setPixmap(QPixmap::fromImage(hist));
}


void MainWindow::on_filtersList_itemChanged(QListWidgetItem *item)
{
    if (ui->filtersList->isEnabled())
    {
        // update processing flags
        controller->processingThread->updateFlags(
                    ui->filtersList->row(item),
                    (item->checkState() == Qt::Checked));
    }
}

void MainWindow::on_filtersList_clicked(const QModelIndex &index)
{
    // hide all config frames first
    hideAllConfigFrames();
    // show config params
    switch (index.row())
    { // from 0 till 22

    case 0:
    { // Salt and Pepper Noise
        ui->configFrameMainLayout->addWidget(ui->saltPepperFrame);
        ui->saltPepperFrame->setHidden(false);
        ui->saltPepperFrame->setEnabled(true);
    } break;
    case 1:
    { // Show logo
        ui->configFrameMainLayout->addWidget(ui->logoFrame);
        ui->logoFrame->setHidden(false);
        ui->logoFrame->setEnabled(true);
    } break;
    case 2:
    { // Convert to Colorspace
        ui->configFrameMainLayout->addWidget(ui->colorSpaceFrame);
        ui->colorSpaceFrame->setHidden(false);
        ui->colorSpaceFrame->setEnabled(true);
    } break;
    case 3:
    { // Compute Histogram
        // no frame to show for this
    } break;
    case 4:
    { // Equalize Histogram
        // no frame to show for this
    } break;
    case 5:
    { // Dilate
        ui->configFrameMainLayout->addWidget(ui->dilateFrame);
        ui->dilateFrame->setHidden(false);
        ui->dilateFrame->setEnabled(true);
    } break;
    case 6:
    { // Erode
        ui->configFrameMainLayout->addWidget(ui->erodeFrame);
        ui->erodeFrame->setHidden(false);
        ui->erodeFrame->setEnabled(true);
    } break;
    case 7:
    { // Open (Morphological Op.)
        ui->configFrameMainLayout->addWidget(ui->openFrame);
        ui->openFrame->setHidden(false);
        ui->openFrame->setEnabled(true);
    } break;
    case 8:
    { // Close (Morphological Op.)
        ui->configFrameMainLayout->addWidget(ui->closeFrame);
        ui->closeFrame->setHidden(false);
        ui->closeFrame->setEnabled(true);
    } break;
    case 9:
    { // Blur
        ui->configFrameMainLayout->addWidget(ui->blurFrame);
        ui->blurFrame->setHidden(false);
        ui->blurFrame->setEnabled(true);
    } break;
    case 10:
    { // Sobel Operator
        ui->configFrameMainLayout->addWidget(ui->sobelFrame);
        ui->sobelFrame->setHidden(false);
        ui->sobelFrame->setEnabled(true);
    } break;
    case 11:
    { // Laplacian Operator
        ui->configFrameMainLayout->addWidget(ui->laplacianFrame);
        ui->laplacianFrame->setHidden(false);
        ui->laplacianFrame->setEnabled(true);
    } break;
    case 12:
    { // Sharp by Kernel
        ui->configFrameMainLayout->addWidget(ui->sharpKernelFrame);
        ui->sharpKernelFrame->setHidden(false);
        ui->sharpKernelFrame->setEnabled(true);
    } break;
    case 13:
    { // Edge Detection (Canny)
        ui->configFrameMainLayout->addWidget(ui->cannyFrame);
        ui->cannyFrame->setHidden(false);
        ui->cannyFrame->setEnabled(true);
    } break;
    case 14:
    { // Extract lines (Hough)
        ui->configFrameMainLayout->addWidget(ui->linesHoughFrame);
        ui->linesHoughFrame->setHidden(false);
        ui->linesHoughFrame->setEnabled(true);
    } break;
    case 15:
    { // Extract circles (Hough)
        ui->configFrameMainLayout->addWidget(ui->circlesHoughFrame);
        ui->circlesHoughFrame->setHidden(false);
        ui->circlesHoughFrame->setEnabled(true);
    } break;
    case 16:
    { // Find Countours
        ui->configFrameMainLayout->addWidget(ui->contoursFrame);
        ui->contoursFrame->setHidden(false);
        ui->contoursFrame->setEnabled(true);
    } break;
    case 17:
    { // Apply Bounding Box
        ui->configFrameMainLayout->addWidget(ui->boundingBoxFrame);
        ui->boundingBoxFrame->setHidden(false);
        ui->boundingBoxFrame->setEnabled(true);
    } break;
    case 18:
    { // Apply minimum enclosing circle
        ui->configFrameMainLayout->addWidget(ui->enclosingCirclesFrame);
        ui->enclosingCirclesFrame->setHidden(false);
        ui->enclosingCirclesFrame->setEnabled(true);
    } break;
    case 19:
    { // Extract Corners (Harris)
        ui->configFrameMainLayout->addWidget(ui->harrisCornerFrame);
        ui->harrisCornerFrame->setHidden(false);
        ui->harrisCornerFrame->setEnabled(true);
    } break;
    case 20:
    { // Extract FAST
        ui->configFrameMainLayout->addWidget(ui->fastFrame);
        ui->fastFrame->setHidden(false);
        ui->fastFrame->setEnabled(true);
    } break;
    case 21:
    { // Extract SURF
        ui->configFrameMainLayout->addWidget(ui->surfFrame);
        ui->surfFrame->setHidden(false);
        ui->surfFrame->setEnabled(true);
    } break;
    case 22:
    { // Extract SIFT
        ui->configFrameMainLayout->addWidget(ui->siftFrame);
        ui->siftFrame->setHidden(false);
        ui->siftFrame->setEnabled(true);
    } break;
    default:
    { // nothing to do here

    } break;

    } // end of switch
}


void MainWindow::hideAllConfigFrames()
{
    ui->saltPepperFrame->setEnabled(false);
    ui->saltPepperFrame->setHidden(true);
    ui->colorSpaceFrame->setEnabled(false);
    ui->colorSpaceFrame->setHidden(true);
    ui->dilateFrame->setEnabled(false);
    ui->dilateFrame->setHidden(true);
    ui->erodeFrame->setEnabled(false);
    ui->erodeFrame->setHidden(true);
    ui->openFrame->setEnabled(false);
    ui->openFrame->setHidden(true);
    ui->closeFrame->setEnabled(false);
    ui->closeFrame->setHidden(true);
    ui->blurFrame->setEnabled(false);
    ui->blurFrame->setHidden(true);
    ui->sobelFrame->setEnabled(false);
    ui->sobelFrame->setHidden(true);
    ui->laplacianFrame->setEnabled(false);
    ui->laplacianFrame->setHidden(true);
    ui->sharpKernelFrame->setEnabled(false);
    ui->sharpKernelFrame->setHidden(true);
    ui->cannyFrame->setEnabled(false);
    ui->cannyFrame->setHidden(true);
    ui->linesHoughFrame->setEnabled(false);
    ui->linesHoughFrame->setHidden(true);
    ui->circlesHoughFrame->setEnabled(false);
    ui->circlesHoughFrame->setHidden(true);
    ui->contoursFrame->setEnabled(false);
    ui->contoursFrame->setHidden(true);
    ui->boundingBoxFrame->setEnabled(false);
    ui->boundingBoxFrame->setHidden(true);
    ui->enclosingCirclesFrame->setEnabled(false);
    ui->enclosingCirclesFrame->setHidden(true);
    ui->harrisCornerFrame->setEnabled(false);
    ui->harrisCornerFrame->setHidden(true);
    ui->fastFrame->setEnabled(false);
    ui->fastFrame->setHidden(true);
    ui->surfFrame->setEnabled(false);
    ui->surfFrame->setHidden(true);
    ui->siftFrame->setEnabled(false);
    ui->siftFrame->setHidden(true);
    ui->logoFrame->setEnabled(false);
    ui->logoFrame->setHidden(true);
}

void MainWindow::on_noiseDensityThresSL_valueChanged(int value)
{
    controller->processingThread->setSaltPepperDensity(value);
    ui->noiseDensityThresValLabel->setText(QString().setNum(value));
}

void MainWindow::on_colorSpaceCB_currentIndexChanged(int index)
{
    controller->processingThread->setColorSpace(index);
}

void MainWindow::on_dilateSB_valueChanged(int arg1)
{
    controller->processingThread->setDilateIterations(arg1);
}

void MainWindow::on_erodeSB_valueChanged(int arg1)
{
    controller->processingThread->setErodeIterations(arg1);
}

void MainWindow::on_openSB_valueChanged(int arg1)
{
    controller->processingThread->setOpenIterations(arg1);
}

void MainWindow::on_closeSB_valueChanged(int arg1)
{
    controller->processingThread->setCloseIterations(arg1);
}

void MainWindow::on_blurSizeSB_valueChanged(int value)
{
    int val = (value % 2 == 1) ? value : value+1;
    controller->processingThread->setBlurSize(val);
}

void MainWindow::on_blurSigmaSB_valueChanged(double arg1)
{
    controller->processingThread->setBlurSigma(arg1);
}

void MainWindow::on_sobelDirectionCB_currentIndexChanged(int index)
{
    controller->processingThread->setSobelDirection(index);
}

void MainWindow::on_sobelKernelSizeSB_valueChanged(int arg1)
{
    controller->processingThread->setSobelKernelSize(arg1);
}

void MainWindow::on_laplacianKernelSizeSB_valueChanged(int arg1)
{
    controller->processingThread->setLaplacianKernelSize(arg1);
}

void MainWindow::on_sharpKernelCenterSB_valueChanged(int arg1)
{
    controller->processingThread->setsharpKernelCenter(arg1);
}

void MainWindow::on_cannyLowThresSB_valueChanged(int arg1)
{
    controller->processingThread->setCannyLowThres(arg1);
}

void MainWindow::on_cannyHighThresSB_valueChanged(int arg1)
{
    controller->processingThread->setCannyHighThres(arg1);
}

void MainWindow::on_linesHoughVotesSB_valueChanged(int arg1)
{
    controller->processingThread->setLinesHoughVotes(arg1);
}

void MainWindow::on_circlesHoughMinSB_valueChanged(int arg1)
{
    controller->processingThread->setCirclesHoughMin(arg1);
}

void MainWindow::on_circlesHoughMaxSB_valueChanged(int arg1)
{
    controller->processingThread->setCirclesHoughMax(arg1);
}

void MainWindow::on_contourThresSL_valueChanged(int value)
{
    controller->processingThread->setContoursThreshold(value);
    ui->contoursThresValLabel->setText(QString().setNum(value));
}

void MainWindow::on_boundingBoxThresSL_valueChanged(int value)
{
    controller->processingThread->setBoundingBoxThres(value);
    ui->boundingBoxThresValLabel->setText(QString().setNum(value));
}

void MainWindow::on_enclosingCirclesThresSL_valueChanged(int value)
{
    controller->processingThread->setEnclosingCircleThres(value);
    ui->enclosingCirclesThresValLabel->setText(QString().setNum(value));
}

void MainWindow::on_harrisCornerThresSL_valueChanged(int value)
{
    controller->processingThread->setHarrisCornerThres(value);
    ui->harrisCornerThresValLabel->setText(QString().setNum(value));
}


void MainWindow::on_fastThresSL_valueChanged(int value)
{
    controller->processingThread->setFastThres(value);
    ui->fastThresValLabel->setText(QString().setNum(value));
}

void MainWindow::on_surfThresSL_valueChanged(int value)
{
    controller->processingThread->setSurfThres(value);
    ui->surfThresValLabel->setText(QString().setNum(value));
}

void MainWindow::on_siftContrastThresSB_valueChanged(double arg1)
{
    controller->processingThread->setSiftContrastThres(arg1);
}

void MainWindow::on_siftEdgeThresSL_valueChanged(int value)
{
    int val = (value % 2 == 1) ? value : value+1;
    controller->processingThread->setSiftEdgeThres(val);
    ui->siftEdgeThresValLabel->setText(QString().setNum(val));
}


void MainWindow::on_playBtn_clicked()
{
    if (!controller->captureThread->isPaused())
    { // pause it
        controller->captureThread->pause();
        controller->processingThread->pause();
        ui->playBtn->setStyleSheet(QString("QPushButton#playBtn {"
                                           "border: none;"
                                           "background: url(:/imgs/pause.png) no-repeat transparent;"
                                           "background-position: center center;"
                                           "background-color: qlineargradient(spread:reflect, x1:0, y1:1, x2:1, y2:0, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(192, 192, 192, 255));"
                                           "}"

                                           "QPushButton#playBtn:pressed {"
                                           "border: none;"
                                           "background: url(:/imgs/pause.png) no-repeat transparent;"
                                           "background-position: center center;"
                                           "background-color: qlineargradient(spread:reflect, x1:1, y1:0, x2:0, y2:1, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(255, 255, 255, 255));"
                                           "}"
                                           "QPushButton#playBtn:focus {"
                                           "border: none;"
                                           "outline: none;"
                                           "}"));
    }
    else
    { // run it
        controller->captureThread->play();
        controller->processingThread->play();
        ui->playBtn->setStyleSheet(QString("QPushButton#playBtn {"
                                           "border: none;"
                                           "background: url(:/imgs/play.png) no-repeat transparent;"
                                           "background-position: center center;"
                                           "background-color: qlineargradient(spread:reflect, x1:0, y1:1, x2:1, y2:0, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(192, 192, 192, 255));"
                                           "}"

                                           "QPushButton#playBtn:pressed {"
                                           "border: none;"
                                           "background: url(:/imgs/play.png) no-repeat transparent;"
                                           "background-position: center center;"
                                           "background-color: qlineargradient(spread:reflect, x1:1, y1:0, x2:0, y2:1, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(255, 255, 255, 255));"
                                           "}"
                                           "QPushButton#playBtn:focus {"
                                           "border: none;"
                                           "outline: none;"
                                           "}"));
    }
}
