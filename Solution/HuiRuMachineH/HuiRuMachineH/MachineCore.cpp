#include "MachineCore.h"
#include <QDebug>
#include <QStyle>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QCloseEvent>
#include <QMetaType>            // 注册元对象

#include <algorithm>            // 包含 std::min 和 std::max

#include "HuiRuLoggerH.h"
#include "WorkThread.h"
#include "VisionFrame.h"
#include "VisionThread.h"
#include "ResultView.h"
#include "Database.h"
#include "Library.h"
#include "TaskList.h"

#include "dialog/FileDialog.h"
#include "dialog/PCBDialog.h"
#include "dialog/ParaDialog.h"
#include "dialog/LibraryDialog.h"
#include "dialog/ToolDialog.h"
#include "dialog/VisionDialog.h"
#include "dialog/MotionDialog.h"
#include "dialog/DataDialog.h"

#include "../../../Include/VisionCore_Export.h"
#include "../../../Include/MotionCore_Export.h"

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\VisionCore.lib")
#pragma comment(lib,  LIB_PATH"\\MotionCore.lib")

#include "opencv2/imgproc.hpp"

#define WIN32_LEAD_AND_MEAN
#include <windows.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

MachineCore::MachineCore(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MachineCoreClass())
{
    ui->setupUi(this);

    // 设置mainWindow标题
    this->setWindowTitle("HuiRuStationH");

    // 设置dialog标题风格
    //this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // 设置图标
    this->setWindowIcon(QIcon(":/MachineCore/Icon/Linux.png"));

    // 设置窗口尺寸
    this->setFixedSize(this->size());

    // 初始化日志系统
    if (this->OnInitLoggerH() == MACHINECORE_RESULT_SUCCESS)
    {
        qDebug() << tr("=== [Core] 日志系统已启动 ===");
    }

    // 初始化机器
    int result = this->OnInitMachine();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine failed");
        ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 初始化失败！", L"提示", MB_OK | MB_ICONERROR);
        //return;
    }

    // 初始化界面表格样式
    this->OnInitTableStyle();

    // 将 cv::Mat 注册到 Qt 元对象系统中
    qRegisterMetaType<cv::Mat>("cv::Mat");
}

MachineCore::~MachineCore()
{
    // 卸载Motion设备
    qDebug() << "[MachineCore] Motion: 正在断开连接...";

    Motion_UninitDevice();                  // 销毁运控设备

    qDebug() << "[MachineCore] Motion: 断开连接完成";

    // 卸载Vision库
    qDebug() << "[MachineCore] Vision: 卸载中...";

    Vision_UnregisterImageCallback();       // 销毁视觉回调
    Vision_UninitDevice();                  // 销毁视觉设备

    qDebug() << "[MachineCore] Vision: 卸载完成";

    // 清理视觉图像对象
    if (VisionFrame::instance().m_visionFrame)
    {
        delete VisionFrame::instance().m_visionFrame;
        VisionFrame::instance().m_visionFrame = nullptr;
    }

    delete ui;
}

void MachineCore::SetDialogMessageOnLabel(QString message)
{
    ui->label_Top->setText(message);
}

void MachineCore::ImageCallbackFunc(unsigned char* pData,
                                    int nLen,
                                    int nWidth,
                                    int nHeight,
                                    int nPixelType,
                                    void* pUser)
{
    // void* 强制转换为 MachineCore指针
    MachineCore* core = static_cast<MachineCore*>(pUser);
    if (!core)
    {
        qDebug() << "[MachineCore] Vision 回调函数错误，pUser 为 NULL";
        return;
    }

    //if (core->m_newFrame)
    //{
    //    return;
    //}

    // 加锁保护，防止定时器正在读的时候，回调在写
    // QMutexLocker locker(&core->m_frameMutex);

    // SDK原始数据memcpy直接拷贝，比cv::Mat深拷贝更快更安全
    // memcpy(core->m_visionFrame->data, pData, nLen);

    // 更新视频帧数据
    VisionFrame::instance().UpdateVisionFrame(pData, nLen);

    // 设置新帧
    // core->m_newFrame = true;
}

int MachineCore::OnInitLoggerH()
{
    // --- 初始化日志系统 ---

    // 获取宿主EXE路径
    QString hostExePath;
    {
        wchar_t exeBuf[MAX_PATH] = { 0 };

        if (GetModuleFileNameW(nullptr, exeBuf, MAX_PATH) != 0)
        {
            hostExePath = QString::fromWCharArray(exeBuf);
        }
    }
    qDebug() << tr("[MachineCore] Host EXE 路径:") << hostExePath;

    // 构造日志文件路径：<EXE目录>/Log Files/HuiRuMachineH.log
    QString logFilePath;
    if (!hostExePath.isEmpty())
    {
        QFileInfo fi(hostExePath);
        QDir exeDir = fi.absoluteDir();
        QString logDirPath = exeDir.filePath("Log Files");

        // 确保目录存在
        if (!QDir(logDirPath).exists())
        {
            QDir().mkpath(logDirPath);
        }
        logFilePath = QDir(logDirPath).filePath("HuiRuMachineH.log");
    }
    else
    {
        // 回退到当前工作目录下的 Log Files
        QString logDirPath = QDir::current().filePath("Log Files");
        if (!QDir(logDirPath).exists())
        {
            QDir().mkpath(logDirPath);
        }
        logFilePath = QDir(logDirPath).filePath("HuiRuMachineH.log");
    }

    // 只要单例被创建，日志就会启动
    LoggerH::instance().Init(logFilePath);

    // 安装全局钩子
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& context, const QString& msg)
    {
        LoggerH::instance().WriteLog(type, msg);
    });

    return MACHINECORE_RESULT_SUCCESS;
}

int MachineCore::OnInitMachine()
{
    qDebug() << "[MachineCore] InitMachine";

    int result = this->OnInitEnvironment();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine: OnInitEnvironment failed");
        // return MACHINECORE_RESULT_ERROR_ENVIRONMENT;
    }

    result = this->OnInitDatabase();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine: OnInitDatabase failed");
        // return MACHINECORE_RESULT_ERROR_DATABASE;
    }

    result = this->OnInitVision();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine: OnInitVision failed");
        // return MACHINECORE_RESULT_ERROR_VISION;
    }

    result = this->OnInitMotion();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine: OnInitMotion failed");
        // return MACHINECORE_RESULT_ERROR_MOTION;
    }

    result = this->OnInitLibrary();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine: OnInitLibrary failed");
        // return MACHINECORE_RESULT_ERROR_LIBRARY;
    }

    result = this->OnInitParameter();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine: OnInitParameter failed");
        // return MACHINECORE_RESULT_ERROR_PARAMETER;
    }

    qDebug("[MachineCore] InitMachine: success");

    return MACHINECORE_RESULT_SUCCESS;
}

int MachineCore::OnInitEnvironment()
{
    return MACHINECORE_RESULT_SUCCESS;
}

int MachineCore::OnInitDatabase()
{
    //// 初始化数据库
    //Database::instance().Database_Init();

    return MACHINECORE_RESULT_SUCCESS;
}

int MachineCore::OnInitVision()
{
    qDebug() << "[MachineCore] OnInitVision InitVision";

    // 初始化视觉成员变量：高1024宽1280，单通道灰度图
    // m_visionFrame = new cv::Mat(1024, 1280, CV_8UC1);
    VisionFrame::instance().m_visionFrame = new cv::Mat(1024, 1280, CV_8UC1);

    WId nativeHandle = ui->label_Video->winId();
    int result = Vision_InitDevice(reinterpret_cast<void*>(nativeHandle));
    if (result != VISION_RESULT_OK)
    {
        qDebug() << "[MachineCore] OnInitVision 初始化失败，错误码:" << result;
        return MACHINECORE_RESULT_ERROR_VISION;
    }

    // 初始化成功后，立刻注册回调，传递当前this指针
    Vision_RegisterImageCallback(ImageCallbackFunc, this);

    qDebug() << "[MachineCore] OnInitVision 初始化成功，显示窗口已绑定";

    result = Vision_StartGrabbing();
    if (result != VISION_RESULT_OK)
    {
        qDebug() << "[MachineCore] OnInitVision 启动采集失败，错误码:" << result;
        
        return MACHINECORE_RESULT_ERROR_VISION;
    }

    qDebug() << "[MachineCore] OnInitVision 启动采集成功";

    // m_frameTimer = new QTimer(this);
    // connect(m_frameTimer, &QTimer::timeout, this, &MachineCore::OnFrameTimer);
    // m_frameTimer->start(33);

    return MACHINECORE_RESULT_SUCCESS;
}

int MachineCore::OnInitMotion()
{
    qDebug() << "[MachineCore] InitMotion: Motion_InitDevice";

    // 初始化设备 (连接仿真器)
    int result = Motion_InitDevice("127.0.0.1");
    if (result != MOTION_RESULT_OK)
    {
        qDebug() << "[MachineCore] InitMotion: Motion_InitDevice failed";

        return MACHINECORE_RESULT_ERROR_MOTION;
    }

    qDebug() << "[MachineCore] InitMotion: Motion_InitDevice: success";

    return MACHINECORE_RESULT_SUCCESS;
}

int MachineCore::OnInitLibrary()
{
    return MACHINECORE_RESULT_SUCCESS;
}

int MachineCore::OnInitParameter()
{
    return MACHINECORE_RESULT_SUCCESS;
}

int MachineCore::OnInitTableStyle()
{
    // 列标高度设置
    ui->tableWidget->horizontalHeader()->setFixedHeight(25);

    // 设置第一列为固定模式，固定宽度
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableWidget->setColumnWidth(0, 40);

    return MACHINECORE_RESULT_SUCCESS;
}

void MachineCore::closeEvent(QCloseEvent* event)
{
    qDebug() << "[MachineCore] closeEvent called";

    event->accept();
}

void MachineCore::on_toolButton_Start_clicked()
{
    qDebug() << "[MachineCore] Start";

    int titleBarHeight = style()->pixelMetric(QStyle::PM_TitleBarHeight);
    qDebug() << "[MachineCore] 当前标题栏高度为:" << titleBarHeight;

    qDebug() << "[MachineCore] 当前窗口尺寸为:" << this->geometry();

    ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 开始运行！", L"提示", MB_OK | MB_ICONINFORMATION);

    // ###################### 动态创建运控线程和工作对象 ######################

    // 防止重复点击
    if (m_workerThread && m_workerThread->isRunning())
    {
        qDebug() << "[MachineCore] 运控线程正在运行，请勿重复点击！";
        ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 运控线程正在运行，请勿重复点击！", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }

    // 创建运控工作对象（Worker不能指定父对象）
    m_workerThread = new QThread();
    m_worker = new WorkThread();

    //m_workerThread->setObjectName("WorkThread");

    // 将工作对象移动到运控线程
    m_worker->moveToThread(m_workerThread);

    // 当运控线程启动时，自动调用worker的doWork槽函数
    connect(m_workerThread, &QThread::started, m_worker, &WorkThread::doWork);

    // 当运控工作对象完成工作时，自动调用WorkThreadWorkFinished槽函数
    connect(m_worker, &WorkThread::workFinished, this, &MachineCore::WorkThreadWorkFinished);

    // 当任务完成时，清理资源（可选，根据业务需求决定）
    connect(m_worker, &WorkThread::workFinished, m_workerThread, &QThread::quit);
    connect(m_worker, &WorkThread::workFinished, m_worker, &WorkThread::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);

    // 启动运控线程
    m_workerThread->start();

    ui->label_Top->setText("WorkThreadWorking");
}

void MachineCore::on_toolButton_Quit_clicked()
{
    qDebug() << "[MachineCore] Stop";

    if (m_workerThread && m_workerThread->isRunning())
    {
        qDebug() << "[MachineCore] 正在安全停止运控线程...";

        m_workerThread->quit();
        m_workerThread->wait();         // 阻塞等待线程结束

        qDebug() << "[MachineCore] 运控线程已安全停止";
        ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 运控线程已安全停止", L"提示", MB_OK | MB_ICONINFORMATION);
    }
    else 
    {
        qDebug() << "[MachineCore] 当前没有正在运行的运控线程";
        ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 当前没有正在运行的运控线程", L"提示", MB_OK | MB_ICONINFORMATION);
    }

    if (m_visionThread && m_visionThread->isRunning())
    {
        qDebug() << "[MachineCore] 正在安全停止视觉线程...";

        m_visionThread->quit();
        m_visionThread->wait();         // 阻塞等待线程结束

        qDebug() << "[MachineCore] 视觉线程已安全停止";
        ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 视觉线程已安全停止", L"提示", MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        qDebug() << "[MachineCore] 当前没有正在运行的视觉线程";
        ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 当前没有正在运行的视觉线程", L"提示", MB_OK | MB_ICONINFORMATION);
    }

    // 更新 UI 状态
    ui->label_Top->setText("任务已手动停止");
    ui->toolButton_Start->setEnabled(true);

    // 等待1秒
    QTimer::singleShot(1000, [this]() 
    {
        qDebug() << "[MachineCore] 1秒后退出程序...";

        // 退出程序
        qApp->quit();
    });
}

void MachineCore::on_pushButton_VisionDetect_clicked()
{
    qDebug() << "[MachineCore] VisionDetect";
    //::MessageBoxW((HWND)this->winId(), L"[MachineCore] 操作成功！", L"提示", MB_OK | MB_ICONINFORMATION);

    // 启动视觉线程
    this->VisionMathThreadStartWork();
}

void MachineCore::on_pushButton_Result_clicked()
{
    qDebug() << "[MachineCore] Result";
    ResultView resultView(this);
    if (resultView.exec() == QDialog::Accepted)
    {
        qDebug() << "[MachineCore] ResultView::Accepted";
        QString message = resultView.GetResult();
        ui->label_Top->setText("子窗口返回Result: " + message);
    }
    else 
    {
        qDebug() << "[MachineCore] ResultView::Rejected";
    }
    ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 操作成功！", L"提示", MB_OK | MB_ICONINFORMATION);
}

void MachineCore::WorkThreadWorkFinished()
{
    ui->label_Top->setText("WorkThreadWorkFinished");
    ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 运控线程任务完成！", L"提示", MB_OK | MB_ICONINFORMATION);
}

void MachineCore::on_pushButton_File_clicked()
{
    FileDialog fileDialog(this);
    fileDialog.exec();
}

void MachineCore::on_pushButton_PCB_clicked()
{
    PCBDialog pcbDialog(this);
    pcbDialog.exec();
}

void MachineCore::on_pushButton_Para_clicked()
{
    ParaDialog paraDialog(this);
    paraDialog.exec();
}

void MachineCore::on_pushButton_Lib_clicked()
{
    LibraryDialog libraryDialog(this);
    libraryDialog.exec();
}

void MachineCore::on_pushButton_Tool_clicked()
{
    ToolDialog toolDialog(this);
    toolDialog.exec();
}

void MachineCore::on_pushButton_Vision_clicked()
{
    VisionDialog visionDialog(this);

    connect(&visionDialog, &VisionDialog::vision_computation_signal, this, &MachineCore::VisionMathThreadStartWork);
    connect(this, &MachineCore::sigToVisionDialog, &visionDialog, &VisionDialog::displayCalculateResult);

    m_isUi = false;

    visionDialog.exec();

    m_isUi = true;
}

void MachineCore::on_pushButton_Motion_clicked()
{
    MotionDialog motionDialog(this);
    motionDialog.exec();
}

void MachineCore::on_pushButton_Data_clicked()
{
    DataDialog dataDialog(this);
    dataDialog.exec();
}

//void MachineCore::OnFrameTimer()
//{
//    if (!m_newFrame)
//    {
//        return;
//    }
//    
//    cv::Mat displayImg;
//    {
//        // 安全加锁
//        QMutexLocker locker(&m_frameMutex);
//
//        // 深拷贝
//        displayImg = m_visionFrame->clone();
//
//        // 重置标志位
//        m_newFrame = false;
//    }
//
//    // 无锁显示
//    if (ui->label_Video && !displayImg.empty())
//    {
//        // 用OpenCV进行等比例缩放
//        double scale = (std::min)(
//            (double)ui->label_Video->width() / displayImg.cols,
//            (double)ui->label_Video->height() / displayImg.rows
//        );
//        cv::Size targetSize(displayImg.cols * scale, displayImg.rows * scale);
//
//        cv::Mat resizedImg;
//        cv::resize(displayImg, resizedImg, targetSize, 0, 0, cv::INTER_LINEAR);
//
//        // 用缩放后的数据构造QImage
//        QImage qImg(resizedImg.data, 
//                    resizedImg.cols, 
//                    resizedImg.rows, 
//                    resizedImg.step, 
//                    QImage::Format_Grayscale8);
//
//        // 显示图片
//        ui->label_Video->setPixmap(QPixmap::fromImage(qImg));
//    }
//}

void MachineCore::VisionMathThreadStartWork()
{
    // ###################### 动态创建视觉线程和工作对象 ######################

    // 防止重复点击
    if (m_visionThread && m_visionThread->isRunning())
    {
        qDebug() << "[MachineCore] 视觉线程正在运行，请勿重复点击！";
        ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 视觉线程正在运行，请勿重复点击！", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }

    // 创建视觉工作对象（Worker不能指定父对象）
    m_visionThread = new QThread();
    m_vision = new VisionThread();

    // 将视觉工作对象移动到子线程
    m_vision->moveToThread(m_visionThread);

    // 连接数据信号
    connect(this, &MachineCore::sigStartVision, m_vision, &VisionThread::doWork);

    // 连接启动信号
    connect(m_visionThread, &QThread::started, this, [this]() 
    {
        // 线程启动，发射加锁信号
        // QMutexLocker locker(&m_frameMutex);
        // emit sigStartVision(m_visionFrame->clone());

        cv::Mat frame = VisionFrame::instance().GetVisionFrame();
        emit sigStartVision(frame);
    });
    
    // 消息弹窗
    //connect(m_vision, &VisionThread::visionFinished, this, [this]()
    //{
    //    ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 视觉线程已完成！", L"提示", MB_OK | MB_ICONINFORMATION);
    //});

    // 结果图像显示到 UI
    connect(m_vision, &VisionThread::visionFinished, this, [this](const cv::Mat& resultImage)
    {
        // 安全检查
        if (resultImage.empty())
        {
            qDebug() << "[MachineCore] 视觉线程已完成，结果图像为空";
            return;
        }

        // 若当前显示的不是Ui界面
        if (!m_isUi)
        {
            emit sigToVisionDialog(resultImage);
            return;
        }

        // cv::Mat 转换为 QImage
        QImage qImg(resultImage.data, 
                    resultImage.cols,
                    resultImage.rows,
                    resultImage.step,
                    QImage::Format_Grayscale8);

        qImg = qImg.copy();

        // QImage 阶段等比例缩放
        QImage scaledImage = qImg.scaled(ui->label_Result->size(), 
                                        Qt::KeepAspectRatio, 
                                        Qt::SmoothTransformation);

        // 将已经缩放好的 QImage 转换为 QPixmap
        QPixmap pixmap = QPixmap::fromImage(scaledImage);

        // 显示到界面上
        ui->label_Result->setPixmap(pixmap);
    });

    // 当视觉任务完成时，清理资源
    connect(m_vision, &VisionThread::visionFinished, m_visionThread, &QThread::quit);
    connect(m_vision, &VisionThread::visionFinished, m_vision, &VisionThread::deleteLater);
    connect(m_visionThread, &QThread::finished, m_visionThread, &QObject::deleteLater);

    // 启动视觉线程
    m_visionThread->start();
}

