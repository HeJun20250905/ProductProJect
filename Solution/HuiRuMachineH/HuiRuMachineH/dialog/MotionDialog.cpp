#include "MotionDialog.h"
#include <QDebug>

#include "../../../Include/MotionCore_Export.h"

#include "SpatialCube.h"
#include "VisionFrame.h"

#include <opencv2/core.hpp> 
#include "opencv2/imgproc.hpp"

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\MotionCore.lib")

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

MotionDialog::MotionDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::MotionDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("运控模块");

    this->setFixedSize(this->size());

    // 初始化运控定时器，绑定信号槽
    m_motionDialog_moveTimer = new QTimer(this);
    connect(m_motionDialog_moveTimer, &QTimer::timeout, this, &MotionDialog::motionDialog_moveTimer_timeout);

    // 初始化视觉定时器，绑定信号槽
    m_motionDialog_visionTimer = new QTimer(this);
    connect(m_motionDialog_visionTimer, &QTimer::timeout, this, &MotionDialog::motionDialog_visionTimer_timeout);
    m_motionDialog_visionTimer->start(33);
}

MotionDialog::~MotionDialog()
{
    delete ui;
}

void MotionDialog::on_pushButton_Cancel_clicked()
{
    ui->label_Info->setText("Cancel");
    this->close();
}

bool MotionDialog::MotionDialogAxisMove()
{
    //// 初始化轴参数（ATYPE, UNITS, SPEED等）
    //Motion_InitAxis(-1, 400.0, 10.0, 100.0, 100.0);
    
    // 轴使能，1使能, 0关闭使能
    Motion_Enable(0, 1);
    Motion_Enable(1, 1);
    Motion_Enable(2, 1);
    qDebug() << "[MotionDialog] 使能轴";
    
    // 组装轴号和目标位置数组
    int   axes[3] = {0, 1, 2};
    float targets[3] = { m_targetPosX, m_targetPosY, m_targetPosZ };

    // 轴绝对运动（单轴也用数组接口，方便统一）
    //qDebug() << "[MotionDialog] 移动轴" << m_axisIndex << "到位置" << m_targetPos << "，速度" << m_speed;

    int ret = Motion_MoveRelMulti(3, axes, targets);
    if (ret != 0)
    {
        qDebug() << "[MotionDialog] 运动失败，错误码:" << ret;
        ui->label_Info->setText("运动失败");
        return false;
    }
    
    ui->label_Info->setText("正在移动...");
    
    m_motionDialog_moveTimer->start(33);

    return true;
}

bool MotionDialog::SetPixmap(int index)
{
    if (index == 0)
    {
        ui->pushButton_Up->setIcon(QIcon(":/MachineCore/Icon/fa_0xf0d8.png"));
        ui->pushButton_Up->setIconSize(QSize(16, 16));
        ui->pushButton_Down->setIcon(QIcon(":/MachineCore/Icon/fa_0xf0dd.png"));
        ui->pushButton_Down->setIconSize(QSize(16, 16));
        ui->pushButton_Left->setIcon(QIcon(":/MachineCore/Icon/fa_0xf0d9.png"));
        ui->pushButton_Left->setIconSize(QSize(16, 16));
        ui->pushButton_Right->setIcon(QIcon(":/MachineCore/Icon/fa_0xf0da.png"));
        ui->pushButton_Right->setIconSize(QSize(16, 16));
    }
    else if (index == 1)
    {
        ui->pushButton_Up->setIcon(QIcon(":/MachineCore/Icon/fa_0xf077.png"));
        ui->pushButton_Up->setIconSize(QSize(16, 16));
        ui->pushButton_Down->setIcon(QIcon(":/MachineCore/Icon/fa_0xf078.png"));
        ui->pushButton_Down->setIconSize(QSize(16, 16));
        ui->pushButton_Left->setIcon(QIcon(":/MachineCore/Icon/fa_0xf0e2.png"));
        ui->pushButton_Left->setIconSize(QSize(16, 16));
        ui->pushButton_Right->setIcon(QIcon(":/MachineCore/Icon/fa_0xf01e.png"));
        ui->pushButton_Right->setIconSize(QSize(16, 16));
    }

    return true;
}

void MotionDialog::on_pushButton_OK_clicked()
{ 
    ui->label_Info->setText("OK");
    this->accept();
}

void MotionDialog::motionDialog_moveTimer_timeout()
{
    // 读取三个轴当前位置
    Motion_GetDpos(AXIS_X, &m_currentPosX);
    Motion_GetDpos(AXIS_Y, &m_currentPosY);
    Motion_GetDpos(AXIS_Z, &m_currentPosZ);

    // 三个轴是否全部到位
    bool xDone = std::abs(m_currentPosX - m_targetPosX) <= 10.0f;
    bool yDone = std::abs(m_currentPosY - m_targetPosY) <= 10.0f;
    bool zDone = std::abs(m_currentPosZ - m_targetPosZ) <= 10.0f;

    if (xDone && yDone && zDone)
    {
        qDebug() << "[MotionDialog] 三轴移动确认到位！最终位置:" << m_currentPosX << m_currentPosY << m_currentPosZ;

        ui->label_Info->setText("确认移动到位");
        m_motionDialog_moveTimer->stop();
    }
    else
    {
        QString newText = QString("正在运行 ： X = %1, Y = %2, Z = %3")
            .arg(m_currentPosX, 0, 'f', 2)
            .arg(m_currentPosY, 0, 'f', 2)
            .arg(m_currentPosZ, 0, 'f', 2);

        // 显示结果
        if (ui->label_Info->text() != newText)
        {
            ui->label_Info->setText(newText);
        }
    }
}

void MotionDialog::motionDialog_visionTimer_timeout()
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

void MotionDialog::on_pushButton_Up_clicked()
{
    //// 获取轴索引
    //int index = ui->comboBox_Axis->currentIndex();
    //switch (index)
    //{
    //case 0:
    //    m_axisIndex = AXIS_Y;
    //    break;
    //case 1:
    //    m_axisIndex = AXIS_Z;
    //    break;
    //default:
    //    qDebug() << "[MotionDialog] 轴索引错误！";
    //    break;
    //}

    //// 设定目标速度
    //m_speed = ui->comboBox_Speed->currentText().toDouble();

    //// 获取当前实际位置
    //Motion_GetDpos(m_axisIndex, &m_currentPos);

    //// 设定目标位置
    //m_targetPos = m_currentPos + ui->comboBox_Step->currentText().toDouble();

    //this->AxisMove();
}

void MotionDialog::on_pushButton_Down_clicked()
{
    //// 获取轴索引
    //int index = ui->comboBox_Axis->currentIndex();
    //switch (index)
    //{
    //case 0:
    //    m_axisIndex = AXIS_Y;
    //    break;
    //case 1:
    //    m_axisIndex = AXIS_Z;
    //    break;
    //default:
    //    qDebug() << "[MotionDialog] 轴索引错误！";
    //    break;
    //}

    //// 设定目标速度
    //m_speed = ui->comboBox_Speed->currentText().toDouble();

    //// 获取当前实际位置
    //Motion_GetDpos(m_axisIndex, &m_currentPos);

    //// 设定目标位置
    //m_targetPos = m_currentPos + ui->comboBox_Step->currentText().toDouble();

    //this->AxisMove();
}

void MotionDialog::on_pushButton_Left_clicked()
{
    //// 获取轴索引
    //int index = ui->comboBox_Axis->currentIndex();
    //switch (index)
    //{
    //case 0:
    //    m_axisIndex = AXIS_X;
    //    break;
    //case 1:
    //    m_axisIndex = AXIS_R;
    //    break;
    //default:
    //    qDebug() << "[MotionDialog] 轴索引错误！";
    //    break;
    //}

    //// 设定目标速度
    //m_speed = ui->comboBox_Speed->currentText().toDouble();

    //// 获取当前实际位置
    //Motion_GetDpos(m_axisIndex, &m_currentPos);

    //// 设定目标位置
    //m_targetPos = m_currentPos + ui->comboBox_Step->currentText().toDouble();

    this->MotionDialogAxisMove();
}

void MotionDialog::on_pushButton_Right_clicked()
{
    //// 获取轴索引
    //int index = ui->comboBox_Axis->currentIndex();
    //switch (index)
    //{
    //case 0:
    //    m_axisIndex = AXIS_X;
    //    break;
    //case 1:
    //    m_axisIndex = AXIS_R;
    //    break;
    //default:
    //    qDebug() << "[MotionDialog] 轴索引错误！";
    //    break;
    //}

    //// 设定目标速度
    //m_speed = ui->comboBox_Speed->currentText().toDouble();

    //// 获取当前实际位置
    //Motion_GetDpos(m_axisIndex, &m_currentPos);

    //// 设定目标位置
    //m_targetPos = m_currentPos + ui->comboBox_Step->currentText().toDouble();

    //this->AxisMove();
}

void MotionDialog::on_pushButton_Move_clicked()
{
    // 读取三个 lineEdit 的文本并转为 float
    m_targetPosX = ui->lineEdit_X->text().toFloat();
    m_targetPosY = ui->lineEdit_Y->text().toFloat();
    m_targetPosZ = ui->lineEdit_Z->text().toFloat();

    qDebug() << "[MotionDialog] 目标位置: X=" << m_targetPosX
        << " Y=" << m_targetPosY
        << " Z=" << m_targetPosZ;

    this->MotionDialogAxisMove();
}

void MotionDialog::on_pushButton_HomeX_clicked()
{
    Motion_Home(0, 10, 2, 100, 100);
}

void MotionDialog::on_pushButton_HomeY_clicked()
{
    Motion_Home(1, 10, 2, 100, 100);
}

void MotionDialog::on_pushButton_HomeZ_clicked()
{
    Motion_Home(2, 10, 2, 100, 100);
}

void MotionDialog::on_pushButton_WorldOrigin_clicked()
{
    // 获取世界坐标偏移量的指针
    float& x = SpatialCube::instance().m_worldZeroOffsetX;
    float& y = SpatialCube::instance().m_worldZeroOffsetY;
    float& z = SpatialCube::instance().m_worldZeroOffsetZ;

    // 读取当前电机位置
    Motion_GetDpos(AXIS_X, &x);
    Motion_GetDpos(AXIS_Y, &y);
    Motion_GetDpos(AXIS_Z, &z);

    // 界面显示标定结果（保留2位小数）
    ui->label_Info->setText(QString("标定世界坐标原点: X=%1, Y=%2, Z=%3")
        .arg(x, 0, 'f', 2)
        .arg(y, 0, 'f', 2)
        .arg(z, 0, 'f', 2));

    qDebug() << "[MotionDialog] 标定世界坐标原点: X=" << x << ", Y=" << y << ", Z=" << z;
}

void MotionDialog::on_pushButton_PcbOrigin_clicked()
{
    // 获取PCB坐标偏移量的指针
    float& x = SpatialCube::instance().m_pcbZeroOffsetX;
    float& y = SpatialCube::instance().m_pcbZeroOffsetY;
    float& z = SpatialCube::instance().m_pcbZeroOffsetZ;

    // 读取当前电机位置
    Motion_GetDpos(AXIS_X, &x);
    Motion_GetDpos(AXIS_Y, &y);
    Motion_GetDpos(AXIS_Z, &z);

    // 界面显示标定结果（保留2位小数）
    ui->label_Info->setText(QString("标定PCB坐标原点: X=%1, Y=%2, Z=%3")
        .arg(x, 0, 'f', 2)
        .arg(y, 0, 'f', 2)
        .arg(z, 0, 'f', 2));

    qDebug() << "[MotionDialog] 标定PCB坐标原点: X=" << x << ", Y=" << y << ", Z=" << z;
}

void MotionDialog::on_pushButton_FeederOrigin_clicked()
{
    // 获取Feeder坐标偏移量的指针
    float& x = SpatialCube::instance().m_feederZeroOffsetX;
    float& y = SpatialCube::instance().m_feederZeroOffsetY;
    float& z = SpatialCube::instance().m_feederZeroOffsetZ;

    // 读取当前电机位置
    Motion_GetDpos(AXIS_X, &x);
    Motion_GetDpos(AXIS_Y, &y);
    Motion_GetDpos(AXIS_Z, &z);

    // 界面显示标定结果（保留2位小数）
    ui->label_Info->setText(QString("标定Feeder坐标原点: X=%1, Y=%2, Z=%3")
        .arg(x, 0, 'f', 2)
        .arg(y, 0, 'f', 2)
        .arg(z, 0, 'f', 2));

    qDebug() << "[MotionDialog] 标定Feeder坐标原点: X=" << x << ", Y=" << y << ", Z=" << z;
}

void MotionDialog::on_comboBox_Axis_currentIndexChanged(int index)
{
    qDebug() << "[MotionDialog] 轴索引改变：" << index;

    if (index == 0)
    {
        this->SetPixmap(index);
    }
    else if (index == 1)
    {
        this->SetPixmap(index);
    }
}
