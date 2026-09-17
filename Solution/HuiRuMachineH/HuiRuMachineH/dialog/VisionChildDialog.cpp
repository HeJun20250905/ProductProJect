#include "VisionChildDialog.h"
#include "VisionFrame.h"
#include <QDebug>

#define WIN32_LEAD_AND_MEAN
#include <windows.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

VisionChildDialog::VisionChildDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::VisionChildDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("视觉算法");

    // 左侧：允许拖出，不接受拖入
    ui->listWidget_Left->setDragEnabled(true);
    ui->listWidget_Left->setAcceptDrops(false);
    ui->listWidget_Left->setDropIndicatorShown(false);
    ui->listWidget_Left->setDragDropMode(QAbstractItemView::DragOnly);
    ui->listWidget_Left->setDefaultDropAction(Qt::CopyAction);

    // 右侧：允许拖入，也允许内部调整顺序
    ui->listWidget_Right->setDragEnabled(true);
    ui->listWidget_Right->setAcceptDrops(true);
    ui->listWidget_Right->setDropIndicatorShown(true);
    ui->listWidget_Right->setDragDropOverwriteMode(false);
    ui->listWidget_Right->setDragDropMode(QAbstractItemView::DragDrop);
    ui->listWidget_Right->setDefaultDropAction(Qt::MoveAction);

    // 遍历左侧列表，给每个 Item 绑定对应的枚举值
    for (int i = 0; i < ui->listWidget_Left->count(); ++i) 
    {
        // 把当前的行号（0, 1, 2...）直接作为枚举值塞进去
        ui->listWidget_Left->item(i)->setData(Qt::UserRole, i);
    }
}

VisionChildDialog::~VisionChildDialog()
{
    delete ui;
}

void VisionChildDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}

void VisionChildDialog::on_pushButton_OK_clicked()
{ 
    // 工作流
    QVector<VisionWorkFlow> workFlow;

    for (int i = 0; i < ui->listWidget_Right->count(); ++i) 
    {
        QListWidgetItem* item = ui->listWidget_Right->item(i);

        // 之前用 UserRole 存枚举的 int 值
        int stepInt = item->data(Qt::UserRole).toInt();

        // 直接强转为枚举类型，存入容器
        workFlow.append(static_cast<VisionWorkFlow>(stepInt));
    }

    // 弹窗检查：直接提取右侧列表的文本内容
    QString msg = QString("即将应用 %1 个算法步骤：\n\n").arg(workFlow.size());
    for (int i = 0; i < ui->listWidget_Right->count(); ++i)
    {
        // 直接获取 UI 列表上显示的文字（比如 "预处理：高斯滤波"）
        QString stepText = ui->listWidget_Right->item(i)->text();

        msg += QString("步骤 %1: %2\n").arg(i + 1).arg(stepText);
    }

    // 使用 Win32 API 弹窗
    ::MessageBox(
        (HWND)this->winId(),
        (LPCWSTR)msg.utf16(),
        (LPCWSTR)L"工作流检查",
        MB_OK | MB_ICONINFORMATION
    );

    // 更新容器
    VisionFrame::instance().UpdateVisionWorkFlow(workFlow);

    // 参数
    VisionWorkPara visionWorkPara;

    visionWorkPara.m_gaussCoreX = ui->lineEdit_GaussCoreX->text().toInt();
    visionWorkPara.m_gaussCoreY = ui->lineEdit_GaussCoreY->text().toInt();
    visionWorkPara.m_gaussSigmaX = ui->lineEdit_GaussSigmaX->text().toDouble();
    visionWorkPara.m_gaussSigmaY = ui->lineEdit_GaussSigmaY->text().toDouble();
    visionWorkPara.m_thresholdValue = ui->lineEdit_ThresholdValue->text().toInt();
    visionWorkPara.m_minMarkArea = ui->lineEdit_MinMarkArea->text().toDouble();
    visionWorkPara.m_maxMarkArea = ui->lineEdit_MaxMarkArea->text().toDouble();
    visionWorkPara.m_lineWidX = ui->lineEdit_LineWidX->text().toInt();
    visionWorkPara.m_lineWidY = ui->lineEdit_LineWidY->text().toInt();
    visionWorkPara.m_lineLenX = ui->lineEdit_LineLenX->text().toInt();
    visionWorkPara.m_lineLenY = ui->lineEdit_LineLenY->text().toInt();
    visionWorkPara.m_lineColorR = ui->lineEdit_LineColorR->text().toInt();
    visionWorkPara.m_lineColorG = ui->lineEdit_LineColorG->text().toInt();
    visionWorkPara.m_lineColorB = ui->lineEdit_LineColorB->text().toInt();

    // 更新参数
    VisionFrame::instance().UpdateVisionWorkPara(visionWorkPara);

    this->accept();
}

void VisionChildDialog::on_listWidget_Left_currentRowChanged(int currentRow)
{
    // 安全检查，防止越界
    if (currentRow >= 0 && currentRow < ui->stackedWidget->count()) 
    {
        ui->stackedWidget->setCurrentIndex(currentRow);

        // 你可以在这里加其他逻辑
        qDebug() << "切换到了第" << currentRow << "页";
    }
}
