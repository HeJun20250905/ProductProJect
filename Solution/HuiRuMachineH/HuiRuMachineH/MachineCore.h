#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_MachineCore.h"

#include <QPointer>                 // 智能指针
#include <QThread>                  // 线程
#include <QMutex>                   // 互斥锁
#include <QTimer>                   // 定时器

QT_BEGIN_NAMESPACE
namespace Ui { class MachineCoreClass; };
QT_END_NAMESPACE

// C风格返回结果枚举
typedef enum MACHINECORE_RESULT
{
    MACHINECORE_RESULT_SUCCESS              = 0,            // 成功
    MACHINECORE_RESULT_FAIL                 = 1,            // 失败
    MACHINECORE_RESULT_ERROR_ENVIRONMENT    = 2,            // 环境错误
    MACHINECORE_RESULT_ERROR_DATABASE       = 3,            // 数据库错误
    MACHINECORE_RESULT_ERROR_VISION         = 4,            // 视觉错误
    MACHINECORE_RESULT_ERROR_MOTION         = 5,            // 运控错误
    MACHINECORE_RESULT_ERROR_LIBRARY        = 6,            // 元件库错误
    MACHINECORE_RESULT_ERROR_PARAMETER      = 7,            // 参数错误
    MACHINECORE_RESULT_ERROR_UNKNOWN        = 99            // 未知错误
}MACHINECORE_RESULT;

// 强类型状态枚举
enum class MachineCore_State 
{
    Init = 0,
    Start,
    Quit
};

class WorkThread;
class VisionThread;

namespace cv 
{ class Mat; }

class MachineCore : public QMainWindow
{
    Q_OBJECT

public:
    MachineCore(QWidget *parent = nullptr);
    ~MachineCore();

    // 显示对话框信息
    void SetDialogMessageOnLabel(QString message);

private:
    Ui::MachineCoreClass *ui;

    QPointer<QThread> m_workerThread;               // 工作线程
    WorkThread* m_worker = nullptr;                 // 工作线程对象

    // cv::Mat* m_visionFrame = nullptr;               // 视觉算法源图
    // bool m_visionThreadSign = false;                // 视觉算法线程启动信号

    QPointer<QThread> m_visionThread;               // 视觉算法线程
    VisionThread* m_vision = nullptr;               // 视觉线程对象

    // QMutex m_frameMutex;                            // 视觉安全锁                          
    // QTimer* m_frameTimer = nullptr;                 // 视频帧定时器
    // bool m_newFrame = false;                        // 新视频帧信号

    // 这个是用于VisionCore模块的静态回调函数，对接DLL的纯C接口
    static void ImageCallbackFunc(unsigned char* pData,
        int nLen,
        int nWidth,
        int nHeight,
        int nPixelType,
        void* pUser
    );

    // 当前显示是否是UI界面
    bool m_isUi = true;

private:
    int OnInitLoggerH();                            // 初始化日志系统
    int OnInitMachine();                            // 初始化机器
    int OnInitEnvironment();                        // 初始化PC环境
    int OnInitDatabase();                           // 数据库初始化
    int OnInitVision();                             // 视觉库初始化
    int OnInitMotion();                             // 运控模组初始化
    int OnInitLibrary();                            // 元件库初始化
    int OnInitParameter();                          // 参数表初始化
    int OnInitTableStyle();                         // 表格初始化

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void on_toolButton_Start_clicked();             // 启动
    void on_toolButton_Quit_clicked();              // 退出

    void on_pushButton_VisionDetect_clicked();      // 视觉算法
    void on_pushButton_Result_clicked();            // 结果

    void WorkThreadWorkFinished();                  // 工作线程工作完成

    void on_pushButton_File_clicked();              // 文件
    void on_pushButton_PCB_clicked();               // PCB
    void on_pushButton_Para_clicked();              // 参数
    void on_pushButton_Lib_clicked();               // 元件库
    void on_pushButton_Tool_clicked();              // 工具
    void on_pushButton_Vision_clicked();            // 视觉
    void on_pushButton_Motion_clicked();            // 运控
    void on_pushButton_Data_clicked();              // 数据

    // void ShowImageFromVisiondialogToMachineCore(const cv::Mat& image);
    // void VisionMathThreadStartSign();
    void VisionMathThreadStartWork();

    // void OnFrameTimer();                            // 帧定时器

signals:
    // void VisionThreadStartWork();
    void sigStartVision(const cv::Mat& image);
    void sigToVisionDialog(const cv::Mat& image);
};

