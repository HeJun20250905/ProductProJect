#include "VisionDialog.h"
#include <QDebug>
#include <QMetaType>        // 注册元对象

#include "../../../Include/VisionCore_Export.h"

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\VisionCore.lib")

#include "opencv2/core/core_c.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

VisionDialog::VisionDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::VisionDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("视觉模块");

    // 强制创建原生窗口句柄
    ui->label_Video->setAttribute(Qt::WA_NativeWindow);
    
    // 将 cv::Mat 注册到 Qt 元对象系统中
    qRegisterMetaType<cv::Mat>("cv::Mat");
}

VisionDialog::~VisionDialog()
{
    delete ui;
}

void VisionDialog::on_pushButton_Init_clicked()
{
    WId nativeHandle = ui->label_Video->winId();
    int result = Vision_InitDevice(reinterpret_cast<void*>(nativeHandle));
    if (result == 0) 
    {
        qDebug() << "[VisionDialog] 初始化成功，显示窗口已绑定";

        // 初始化成功后，立刻注册回调，传递当前this指针
        Vision_RegisterImageCallback(ImageCallbackFunc, this);
    }
    else 
    {
        qDebug() << "[VisionDialog] 初始化失败，错误码:" << result;
    }
}

void VisionDialog::on_pushButton_Start_clicked()
{
    Vision_StartGrabbing();
}

void VisionDialog::on_pushButton_Stop_clicked()
{
    Vision_StopGrabbing();
}

void VisionDialog::on_pushButton_Uninit_clicked()
{
    // 销毁回调
    Vision_UnregisterImageCallback();

    // 销毁设备
    Vision_UninitDevice();
}

void VisionDialog::on_pushButton_MathThread_clicked()
{
    emit sigMathThread();
}

void VisionDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}

void VisionDialog::ImageCallbackFunc(unsigned char* pData, 
                                                int nLen, 
                                                int nWidth, 
                                                int nHeight, 
                                                int nPixelType, 
                                                void* pUser)
{
    // void* 强制转换为 VisionDialog指针
    VisionDialog* dialog = static_cast<VisionDialog*>(pUser);
    if (!dialog)
    {
        qDebug() << "[VisionDialog] 回调函数错误，pUser 为 NULL";
        return;
    }

    // 原始数据转换为 cv::Mat 指针
    cv::Mat img;
    if (nPixelType == PIXEL_TYPE_MONO8)
    {
        img = cv::Mat(nHeight, nWidth, CV_8UC1, pData);
    }
    else if (nPixelType == PIXEL_TYPE_RGB8_PACKED)
    {
        img = cv::Mat(nHeight, nWidth, CV_8UC3, pData);
    }

    // 深拷贝
    cv::Mat signalImg = img.clone();

    // 发射信号
    emit dialog->sigImageReady(signalImg);
}

void VisionDialog::on_pushButton_OK_clicked()
{
}

