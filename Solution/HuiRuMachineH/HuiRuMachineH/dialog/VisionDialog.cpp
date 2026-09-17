#include "VisionDialog.h"
#include <QDebug>
#include <QTimer>

#include "../../../Include/VisionCore_Export.h"
#include "VisionFrame.h"
#include "VisionChildDialog.h"

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\VisionCore.lib")

#include <opencv2/core.hpp> 
#include "opencv2/imgproc.hpp"

#define WIN32_LEAD_AND_MEAN
#include <windows.h>

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

    m_visionDialog_visionTimer = new QTimer(this);
    connect(m_visionDialog_visionTimer, &QTimer::timeout, this, &VisionDialog::visionDialog_visionTimer_timeout);
    m_visionDialog_visionTimer->start(33);
}

VisionDialog::~VisionDialog()
{
    delete ui;
}

void VisionDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}

void VisionDialog::on_pushButton_OK_clicked()
{
    this->accept();
}

void VisionDialog::on_pushButton_Start_clicked()
{
    if (!m_visionDialog_visionTimer->isActive())
    {
        m_visionDialog_visionTimer->start(33);
    }
}

void VisionDialog::on_pushButton_Stop_clicked()
{
    if (m_visionDialog_visionTimer->isActive())
    {
        m_visionDialog_visionTimer->stop();
    }
}

void VisionDialog::on_pushButton_WorkFlow_clicked()
{
    VisionChildDialog visionChildDialog;
    visionChildDialog.exec();
}

// 执行算法工作流
bool DoVisionWorkFlow(VisionWorkFlow step)
{
    return true;
}

void VisionDialog::on_pushButton_Computation_clicked()
{
    //// 读取工作流
    //QVector<VisionWorkFlow> workFlow = VisionFrame::instance().GetVisionWorkFlow();

    //// 非空判定
    //if (workFlow.isEmpty())
    //{
    //    qDebug() << "[MotionDialog] 工作流为空";
    //    return;
    //}

    //// 遍历工作流，依次执行
    //for (int index = 0; index < workFlow.size(); index++)
    //{
    //    DoVisionWorkFlow(workFlow[index]);
    //}

    //// 弹窗检查：打印工作流
    //QString msg = QString("即将应用 %1 个算法步骤：\n\n").arg(workFlow.size());
    //for (int i = 0; i < workFlow.size(); ++i) 
    //{
    //    msg += QString("步骤 %1: %2\n").arg(i + 1).arg(static_cast<int>(workFlow[i]));
    //}

    //// 使用 Win32 API 弹窗
    //::MessageBox(
    //    (HWND)this->winId(),                // 当前窗口的句柄（让弹窗居中显示）
    //    (LPCWSTR)msg.utf16(),               // 将 QString 转为宽字符指针
    //    (LPCWSTR)L"工作流检查",             // 弹窗标题
    //    MB_OK | MB_ICONINFORMATION          // 样式：OK按钮 + 信息图标
    //);

    //// 读取当前参数
    //VisionWorkPara para = VisionFrame::instance().GetVisionWorkPara();

    //// 拼接参数信息字符串
    //// QString msg;
    //msg += "--- 预处理参数 ---\n";
    //msg += QString("高斯核大小: %1 x %2\n").arg(para.m_gaussCoreX).arg(para.m_gaussCoreY);
    //msg += QString("高斯标准差: %1, %2\n").arg(para.m_gaussSigmaX).arg(para.m_gaussSigmaY);
    //msg += QString("二值化阈值: %1\n\n").arg(para.m_thresholdValue);

    //msg += "--- 轮廓筛选参数 ---\n";
    //msg += QString("Mark点最小面积: %1\n").arg(para.m_minMarkArea);
    //msg += QString("Mark点最大面积: %1\n\n").arg(para.m_maxMarkArea);

    //msg += "--- 绘制参数 ---\n";
    //msg += QString("十字线长度: %1 x %2\n").arg(para.m_lineLenX).arg(para.m_lineLenY);
    //msg += QString("线宽: %1 x %2\n").arg(para.m_lineWidX).arg(para.m_lineWidY);
    //msg += QString("线颜色(RGB): (%1, %2, %3)").arg(para.m_lineColorR).arg(para.m_lineColorG).arg(para.m_lineColorB);

    //// 使用 Win32 API 弹窗展示
    //::MessageBox(
    //    (HWND)this->winId(),                // 当前窗口的句柄（让弹窗居中显示）
    //    (LPCWSTR)msg.utf16(),               // 将 QString 转为宽字符指针
    //    (LPCWSTR)L"算法参数检查",           // 弹窗标题
    //    MB_OK | MB_ICONINFORMATION          // 样式：OK按钮 + 信息图标
    //);

    emit vision_computation_signal();
}

void VisionDialog::visionDialog_visionTimer_timeout()
{
    cv::Mat frame = VisionFrame::instance().GetVisionFrame();

    // 无锁显示
    if (!ui->label_Video || frame.empty())
    {
        qDebug() << "[MotionDialog] 显示图像异常";
        return;
    }

    // 用OpenCV进行等比例缩放
    double scale = (std::min)(
        (double)ui->label_Video->width() / frame.cols,
        (double)ui->label_Video->height() / frame.rows
        );
    cv::Size targetSize(frame.cols * scale, frame.rows * scale);

    cv::Mat resizedImg;
    cv::resize(frame, resizedImg, targetSize, 0, 0, cv::INTER_LINEAR);

    // 用缩放后的数据构造QImage
    QImage qImg(resizedImg.data,
        resizedImg.cols,
        resizedImg.rows,
        resizedImg.step,
        QImage::Format_Grayscale8);

    // 显示图片
    ui->label_Video->setPixmap(QPixmap::fromImage(qImg));
}

void VisionDialog::displayCalculateResult(const cv::Mat& resultImage)
{
    m_visionDialog_visionTimer->stop();

    // cv::Mat 转换为 QImage
    QImage qImg(resultImage.data,
        resultImage.cols,
        resultImage.rows,
        resultImage.step,
        QImage::Format_Grayscale8);

    qImg = qImg.copy();

    // QImage 阶段等比例缩放
    QImage scaledImage = qImg.scaled(ui->label_Video->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);

    // 将已经缩放好的 QImage 转换为 QPixmap
    QPixmap pixmap = QPixmap::fromImage(scaledImage);

    // 显示到界面上
    ui->label_Video->setPixmap(pixmap);
}

