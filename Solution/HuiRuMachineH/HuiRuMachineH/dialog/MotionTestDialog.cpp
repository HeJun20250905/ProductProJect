#include "MotionTestDialog.h"

#include <QDebug>

#include "../../../../Include/MotionCore_Export.h"

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\MotionCore.lib")

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

#define WIN32_LEAD_AND_MEAN
#include <windows.h>

MotionTestDialog::MotionTestDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::MotionTestDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("MotionTest");
}

MotionTestDialog::~MotionTestDialog()
{
    delete ui;
}

void MotionTestDialog::on_pushButton_OK_clicked()
{
    this->close();
}

void MotionTestDialog::StartSingleWorkThread(Step step)
{
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

    Step startStep = step;

    // 当运控线程启动时，自动调用worker的doWork槽函数
    // connect(m_workerThread, &QThread::started, m_worker, &WorkThread::doWork);
    connect(m_workerThread, &QThread::started, m_worker, [this, startStep]()
    {
        m_worker->doWork(startStep);
    });

    // 当运控工作对象完成工作时，自动调用WorkThreadWorkFinished槽函数
    // connect(m_worker, &WorkThread::workFinished, this, &MachineCore::WorkThreadWorkFinished);

    // 当任务完成时，清理资源（可选，根据业务需求决定）
    connect(m_worker, &WorkThread::workFinished, m_workerThread, &QThread::quit);
    connect(m_worker, &WorkThread::workFinished, m_worker, &WorkThread::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);

    // 启动运控线程
    m_workerThread->start();

    // ui->label_Top->setText("WorkThreadWorking");
}

void MotionTestDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}

void MotionTestDialog::on_pushButton_Stacking_clicked()
{
    this->StartSingleWorkThread(Step::Stacking);
}

void MotionTestDialog::on_pushButton_VisionPCB_clicked()
{
    this->StartSingleWorkThread(Step::VisionPCB);
}

void MotionTestDialog::on_pushButton_FetchShell_clicked()
{
    this->StartSingleWorkThread(Step::FetchShell);
}

void MotionTestDialog::on_pushButton_VisionShell_clicked()
{
    this->StartSingleWorkThread(Step::VisionShell);
}

void MotionTestDialog::on_pushButton_PickPCB1_clicked()
{
    this->StartSingleWorkThread(Step::PickPCB1);
}

void MotionTestDialog::on_pushButton_PickPCB2_clicked()
{
    this->StartSingleWorkThread(Step::PickPCB2);
}

void MotionTestDialog::on_pushButton_PickPCB3_clicked()
{
    this->StartSingleWorkThread(Step::PickPCB3);
}

void MotionTestDialog::on_pushButton_PlacePCB1_clicked()
{
    this->StartSingleWorkThread(Step::PlacePCB1);
}

void MotionTestDialog::on_pushButton_PlacePCB2_clicked()
{
    this->StartSingleWorkThread(Step::PlacePCB2);
}

void MotionTestDialog::on_pushButton_PlacePCB3_clicked()
{
    this->StartSingleWorkThread(Step::PlacePCB3);
}

void MotionTestDialog::on_pushButton_LaserConfirm1_clicked()
{
    this->StartSingleWorkThread(Step::LaserConfirm1);
}

void MotionTestDialog::on_pushButton_LaserConfirm2_clicked()
{
    this->StartSingleWorkThread(Step::LaserConfirm2);
}

void MotionTestDialog::on_pushButton_LaserConfirm3_clicked()
{
    this->StartSingleWorkThread(Step::LaserConfirm3);
}

void MotionTestDialog::on_pushButton_LaserConfirm4_clicked()
{
    this->StartSingleWorkThread(Step::LaserConfirm4);
}

void MotionTestDialog::on_pushButton_ShellPlace_clicked()
{
    this->StartSingleWorkThread(Step::ShellPlace);
}
