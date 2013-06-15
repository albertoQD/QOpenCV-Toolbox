#ifndef CONFIG_H
#define CONFIG_H

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QtGui>

// Camera device number
#define DEFAULT_CAMERA_DEV_NO -1
// Image buffer size
#define DEFAULT_IMAGE_BUFFER_SIZE 1
// Drop frame if image/frame buffer is full
#define DEFAULT_DROP_FRAMES false
// Thread priorities
#define DEFAULT_CAP_THREAD_PRIO QThread::NormalPriority
#define DEFAULT_PROC_THREAD_PRIO QThread::HighPriority

// Input mode
#define INPUT_CAMERA 0x0001
#define INPUT_VIDEO  0x0002
#define INPUT_IMAGE  0x0003

#endif // CONFIG_H
