#include "VisionDialog.h"
#include <QDebug>

#include "../../../Include/VisionCore_Export.h"

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\VisionCore.lib")

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

    ui->label_Video->setAttribute(Qt::WA_NativeWindow);                 // 强制创建原生窗口句柄
    //ui->label_Video->setAttribute(Qt::WA_DontCreateNativeAncestors);    // 不强制父窗口创建原生句柄
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
    Vision_UninitDevice();
}

void VisionDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}

void VisionDialog::on_pushButton_OK_clicked()
{
}