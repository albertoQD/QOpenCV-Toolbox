#-------------------------------------------------
#
# Project created by QtCreator 2013-05-18T14:24:31
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QOpenCV
TEMPLATE = app


SOURCES += main.cpp \
    processingthread.cpp \
    mattoqimage.cpp \
    mainwindow.cpp \
    controller.cpp \
    capturethread.cpp \
    imagebuffer.cpp \
    FrameLabel.cpp \
    stereomodule.cpp

HEADERS  += \
    structures.h \
    processingthread.h \
    mattoqimage.h \
    mainwindow.h \
    controller.h \
    config.h \
    capturethread.h \
    imagebuffer.h \
    FrameLabel.h \
    stereomodule.h

FORMS    += \
    mainwindow.ui \
    stereomodule.ui

RESOURCES += \
    resources.qrc

LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann -lopencv_nonfree
