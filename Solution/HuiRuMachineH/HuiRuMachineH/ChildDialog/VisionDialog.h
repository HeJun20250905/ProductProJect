#pragma once
#include <QDialog>
#include "ui_VisionDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class VisionDialog; };
QT_END_NAMESPACE

namespace cv 
{
    class Mat;
}

#define PIXEL_TYPE_MONO8            0x01080001
#define PIXEL_TYPE_RGB8_PACKED      0x01080008

class VisionDialog : public QDialog
{
    Q_OBJECT
public:
    VisionDialog(QWidget* parent = nullptr);
    ~VisionDialog();

private:
    Ui::VisionDialog* ui;

    //// 声明一个静态回调函数，用来对接 DLL 的纯 C 接口
    //static void ImageCallbackFunc(unsigned char* pData, 
    //                                        int nLen,
    //                                        int nWidth, 
    //                                        int nHeight,
    //                                        int nPixelType, 
    //                                        void* pUser
    //);

private slots:
    void on_pushButton_Cancel_clicked();
    void on_pushButton_OK_clicked();

    void on_pushButton_Init_clicked();
    void on_pushButton_Start_clicked();
    void on_pushButton_Stop_clicked();
    void on_pushButton_Uninit_clicked();

    void on_pushButton_MathThread_clicked();

signals:
    // void sigImageReady(const cv::Mat& image);           // cv::Mat传递，Qt信号槽底层会自动进行深拷贝
    // void sigMathThread();                               // 发送开启算法信号到Ui线程
};

