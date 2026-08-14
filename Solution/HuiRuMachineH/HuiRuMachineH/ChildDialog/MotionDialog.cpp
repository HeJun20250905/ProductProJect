#include "MotionDialog.h"
#include <QDebug>

#include "../../../Include/MotionCore_Export.h"

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

    m_moveTimer = new QTimer(this);
    connect(m_moveTimer, &QTimer::timeout, this, &MotionDialog::on_moveTimer_timeout);
}

MotionDialog::~MotionDialog()
{
    delete ui;
}

void MotionDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}

void MotionDialog::on_pushButton_OK_clicked()
{ 
    ui->label_Info->setText("OK");
}

void MotionDialog::on_pushButton_Move_clicked()
{
    // 获取轴索引
    m_axisIndex = ui->comboBox_Axis->currentIndex();

    // 初始化轴参数（ATYPE, UNITS, SPEED等）
    Motion_InitAxis(m_axisIndex);

    // 轴使能，1使能, 0关闭使能
    Motion_Enable(m_axisIndex, 1);
    qDebug() << "[MotionDialog] 使能轴";

    // 设定目标速度与位置
    m_speed = ui->lineEdit_Speed->text().toDouble();
    m_targetPos = ui->lineEdit_Pos->text().toDouble();

    // 轴绝对运动
    qDebug() << "[MotionDialog] 移动轴" << m_targetPos << "，速度" << m_speed;
    Motion_MoveAbs(m_axisIndex, m_targetPos, m_speed);

    ui->label_Info->setText("正在移动...");
    ui->pushButton_Move->setEnabled(false);

    m_moveTimer->start(50);
}

void MotionDialog::on_moveTimer_timeout()
{
    // 【核心修改】直接获取当前实际位置
    Motion_GetDpos(m_axisIndex, &m_currentPos);

    // 记录轴是否已经离开了初始位置（防止还没动就判定到位）
    if (std::abs(m_currentPos - 0.0) > 0.01)
    {
        qDebug() << "[MotionDialog] 检测到轴已起步，当前位置:" << m_currentPos;
    }

    // 【核心判断】如果 目标位置与实际位置的差值 小于 允许误差，说明到位了！
    if (std::abs(m_currentPos - m_targetPos) <= 0.05)
    {
        qDebug() << "[MotionDialog] 轴移动确认到位！最终位置:" << m_currentPos;
        ui->pushButton_Move->setEnabled(true);
        ui->pushButton_DecX->setEnabled(true);
        ui->pushButton_IncX->setEnabled(true);
        ui->pushButton_DecY->setEnabled(true);
        ui->pushButton_IncY->setEnabled(true);
        ui->pushButton_DecZ->setEnabled(true);
        ui->pushButton_IncZ->setEnabled(true);
        ui->label_Info->setText("确认移动到位");
        m_moveTimer->stop();
    }
}

void MotionDialog::on_pushButton_DecX_clicked()
{
    // 获取轴索引
    m_axisIndex = 0;

    // 初始化轴参数（ATYPE, UNITS, SPEED等）
    Motion_InitAxis(m_axisIndex);

    // 轴使能，1使能, 0关闭使能
    Motion_Enable(m_axisIndex, 1);
    qDebug() << "[MotionDialog] 使能轴";

    // 设定目标速度与位置
    m_speed = ui->lineEdit_Speed->text().toDouble();
    Motion_GetDpos(m_axisIndex, &m_currentPos);
    m_targetPos = m_currentPos - ui->comboBox_Step->currentText().toFloat();

    // 轴绝对运动
    qDebug() << "[MotionDialog] 移动轴" << m_targetPos << "，速度" << m_speed;
    Motion_MoveAbs(m_axisIndex, m_targetPos, m_speed);

    ui->label_Info->setText("正在移动...");
    ui->pushButton_DecX->setEnabled(false);

    m_moveTimer->start(50);
}

void MotionDialog::on_pushButton_IncX_clicked()
{
    // 获取轴索引
    m_axisIndex = 0;

    // 初始化轴参数（ATYPE, UNITS, SPEED等）
    Motion_InitAxis(m_axisIndex);

    // 轴使能，1使能, 0关闭使能
    Motion_Enable(m_axisIndex, 1);
    qDebug() << "[MotionDialog] 使能轴";

    // 设定目标速度与位置
    m_speed = ui->lineEdit_Speed->text().toDouble();
    Motion_GetDpos(m_axisIndex, &m_currentPos);
    m_targetPos = ui->comboBox_Step->currentText().toFloat() + m_currentPos;

    // 轴绝对运动
    qDebug() << "[MotionDialog] 移动轴" << m_targetPos << "，速度" << m_speed;
    Motion_MoveAbs(m_axisIndex, m_targetPos, m_speed);

    ui->label_Info->setText("正在移动...");
    ui->pushButton_IncX->setEnabled(false);

    m_moveTimer->start(50);
}

void MotionDialog::on_pushButton_DecY_clicked()
{
    // 获取轴索引
    m_axisIndex = 1;

    // 初始化轴参数（ATYPE, UNITS, SPEED等）
    Motion_InitAxis(m_axisIndex);

    // 轴使能，1使能, 0关闭使能
    Motion_Enable(m_axisIndex, 1);
    qDebug() << "[MotionDialog] 使能轴";

    // 设定目标速度与位置
    m_speed = ui->lineEdit_Speed->text().toDouble();
    Motion_GetDpos(m_axisIndex, &m_currentPos);
    m_targetPos = m_currentPos - ui->comboBox_Step->currentText().toFloat();

    // 轴绝对运动
    qDebug() << "[MotionDialog] 移动轴" << m_targetPos << "，速度" << m_speed;
    Motion_MoveAbs(m_axisIndex, m_targetPos, m_speed);

    ui->label_Info->setText("正在移动...");
    ui->pushButton_DecY->setEnabled(false);

    m_moveTimer->start(50);
}

void MotionDialog::on_pushButton_IncY_clicked()
{
    // 获取轴索引
    m_axisIndex = 1;

    // 初始化轴参数（ATYPE, UNITS, SPEED等）
    Motion_InitAxis(m_axisIndex);

    // 轴使能，1使能, 0关闭使能
    Motion_Enable(m_axisIndex, 1);
    qDebug() << "[MotionDialog] 使能轴";

    // 设定目标速度与位置
    m_speed = ui->lineEdit_Speed->text().toDouble();
    Motion_GetDpos(m_axisIndex, &m_currentPos);
    m_targetPos = ui->comboBox_Step->currentText().toFloat() + m_currentPos;

    // 轴绝对运动
    qDebug() << "[MotionDialog] 移动轴" << m_targetPos << "，速度" << m_speed;
    Motion_MoveAbs(m_axisIndex, m_targetPos, m_speed);

    ui->label_Info->setText("正在移动...");
    ui->pushButton_IncY->setEnabled(false);

    m_moveTimer->start(50);
}

void MotionDialog::on_pushButton_IncZ_clicked()
{
    // 获取轴索引
    m_axisIndex = 2;

    // 初始化轴参数（ATYPE, UNITS, SPEED等）
    Motion_InitAxis(m_axisIndex);

    // 轴使能，1使能, 0关闭使能
    Motion_Enable(m_axisIndex, 1);
    qDebug() << "[MotionDialog] 使能轴";

    // 设定目标速度与位置
    m_speed = ui->lineEdit_Speed->text().toDouble();
    Motion_GetDpos(m_axisIndex, &m_currentPos);
    m_targetPos = ui->comboBox_Step->currentText().toFloat() + m_currentPos;

    // 轴绝对运动
    qDebug() << "[MotionDialog] 移动轴" << m_targetPos << "，速度" << m_speed;
    Motion_MoveAbs(m_axisIndex, m_targetPos, m_speed);

    ui->label_Info->setText("正在移动...");
    ui->pushButton_IncZ->setEnabled(false);

    m_moveTimer->start(50);
}

void MotionDialog::on_pushButton_DecZ_clicked()
{
    // 获取轴索引
    m_axisIndex = 2;

    // 初始化轴参数（ATYPE, UNITS, SPEED等）
    Motion_InitAxis(m_axisIndex);

    // 轴使能，1使能, 0关闭使能
    Motion_Enable(m_axisIndex, 1);
    qDebug() << "[MotionDialog] 使能轴";

    // 设定目标速度与位置
    m_speed = ui->lineEdit_Speed->text().toDouble();
    Motion_GetDpos(m_axisIndex, &m_currentPos);
    m_targetPos = m_currentPos - ui->comboBox_Step->currentText().toFloat();

    // 轴绝对运动
    qDebug() << "[MotionDialog] 移动轴" << m_targetPos << "，速度" << m_speed;
    Motion_MoveAbs(m_axisIndex, m_targetPos, m_speed);

    ui->label_Info->setText("正在移动...");
    ui->pushButton_DecZ->setEnabled(false);

    m_moveTimer->start(50);
}
