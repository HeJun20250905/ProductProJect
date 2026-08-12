#include "MachineCore.h"
#include <QDebug>
#include <QStyle>
#include <QTimer>
#include <QFileInfo>
#include <QDir>

#include "HuiRuLoggerH.h"
#include "WorkThread.h"
#include "ResultView.h"
#include "Database.h"
#include "Library.h"
#include "TaskList.h"

#include "ChildDialog/FileDialog.h"
#include "ChildDialog/PCBDialog.h"
#include "ChildDialog/ParaDialog.h"
#include "ChildDialog/LibraryDialog.h"
#include "ChildDialog/ToolDialog.h"
#include "ChildDialog/VisionDialog.h"
#include "ChildDialog/MotionDialog.h"
#include "ChildDialog/DataDialog.h"

#include "../../../Include/MotionCore_Export.h"

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\MotionCore.lib")

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
        return;
    }
}

MachineCore::~MachineCore()
{
    // 卸载设备
    qDebug() << "[MachineCore] Motion: 正在断开连接...";
    Motion_UninitDevice();
    qDebug() << "[MachineCore] Motion: 断开连接完成";

    delete ui;
}

void MachineCore::SetDialogMessageOnLabel(QString message)
{
    ui->label_Top->setText(message);
}

void MachineCore::on_toolButton_Start_clicked()
{
    qDebug() << "[MachineCore] Start";
    
    int titleBarHeight = style()->pixelMetric(QStyle::PM_TitleBarHeight);
    qDebug() << "[MachineCore] 当前标题栏高度为:" << titleBarHeight;

    qDebug() << "[MachineCore] 当前窗口尺寸为:" << this->geometry();

    ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 开始运行！", L"提示", MB_OK | MB_ICONINFORMATION);

    // ###################### 动态创建子线程和工作对象 ######################
    
    // 防止重复点击
    if (workerThread && workerThread->isRunning()) 
    {
        qDebug() << "[MachineCore] 线程正在运行，请勿重复点击！";
        ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 线程正在运行，请勿重复点击！", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }

    // 创建工作对象（Worker不能指定父对象）
    workerThread = new QThread();
    WorkThread* worker = new WorkThread();

    // 将工作对象移动到子线程
    worker->moveToThread(workerThread);

    // 当线程启动时，自动调用worker的doWork槽函数
    connect(workerThread, &QThread::started, worker, &WorkThread::doWork);

    // 当工作对象完成工作时，自动调用WorkThreadWorkFinished槽函数
    connect(worker, &WorkThread::workFinished, this, &MachineCore::WorkThreadWorkFinished);

    // 当任务完成时，清理资源（可选，根据业务需求决定）
    connect(worker, &WorkThread::workFinished, workerThread, &QThread::quit);
    connect(worker, &WorkThread::workFinished, worker, &WorkThread::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    // 启动子线程
    workerThread->start();

    ui->label_Top->setText("WorkThreadWorking");
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

    // 构造日志文件路径：<EXE目录>/Log Files/HttpEngineLogger.log
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
        logFilePath = QDir(logDirPath).filePath("HttpEngineLogger.log");
    }
    else
    {
        // 回退到当前工作目录下的 Log Files
        QString logDirPath = QDir::current().filePath("Log Files");
        if (!QDir(logDirPath).exists())
        {
            QDir().mkpath(logDirPath);
        }
        logFilePath = QDir(logDirPath).filePath("HttpEngineLogger.log");
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
        return MACHINECORE_RESULT_ERROR_ENVIRONMENT;
    }

    result = this->OnInitDatabase();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine: OnInitDatabase failed");
        return MACHINECORE_RESULT_ERROR_DATABASE;
    }

    result = this->OnInitVision();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine: OnInitVision failed");
        return MACHINECORE_RESULT_ERROR_VISION;
    }

    result = this->OnInitMotion();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine: OnInitMotion failed");
        return MACHINECORE_RESULT_ERROR_MOTION;
    }

    result = this->OnInitLibrary();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine: OnInitLibrary failed");
        return MACHINECORE_RESULT_ERROR_LIBRARY;
    }

    result = this->OnInitParameter();
    if (result != MACHINECORE_RESULT_SUCCESS)
    {
        qDebug("[MachineCore] OnInitMachine: OnInitParameter failed");
        return MACHINECORE_RESULT_ERROR_PARAMETER;
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
    return MACHINECORE_RESULT_SUCCESS;
}

int MachineCore::OnInitMotion()
{
    qDebug() << "[MachineCore] InitMotion: Motion_InitDevice";

    // 初始化设备 (连接仿真器)
    int ret = Motion_InitDevice("127.0.0.1");
    if (ret != MOTION_RESULT_OK) 
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

void MachineCore::on_toolButton_Quit_clicked()
{
    qDebug() << "[MachineCore] Stop";

    if (workerThread && workerThread->isRunning()) 
    {
        qDebug() << "[MachineCore] 正在安全停止子线程...";

        workerThread->quit();
        workerThread->wait(); // 阻塞等待线程结束

        // 关键修改：线程结束后，将指针置空，防止后续误用或重复释放
        workerThread = nullptr;

        qDebug() << "[MachineCore] 子线程已安全停止";
        ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 子线程已安全停止", L"提示", MB_OK | MB_ICONINFORMATION);
    }
    else 
    {
        qDebug() << "[MachineCore] 当前没有正在运行的子线程";
        ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 当前没有正在运行的子线程", L"提示", MB_OK | MB_ICONINFORMATION);
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
    ::MessageBoxW((HWND)this->winId(), L"[MachineCore] 子线程任务完成！", L"提示", MB_OK | MB_ICONINFORMATION);
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
    visionDialog.exec();
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

