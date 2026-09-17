#pragma once
#include <QDialog>
#include "ui_MotionDialog.h"

#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MotionDialog; };
QT_END_NAMESPACE

class MotionDialog : public QDialog
{
    Q_OBJECT
public:
    MotionDialog(QWidget* parent = nullptr);
    ~MotionDialog();

private:
    Ui::MotionDialog* ui;

    QTimer* m_motionDialog_moveTimer = nullptr;
    QTimer* m_motionDialog_visionTimer = nullptr;

    float m_speed = 0.01f;
    float m_targetPos = 0.0f;
    float m_currentPos = 0.0f;

    int m_axisIndex = -1;

    bool AxisMove();

    bool SetPixmap(int index);

private slots:
    void on_pushButton_Cancel_clicked();
    void on_pushButton_OK_clicked();

    void on_pushButton_Up_clicked();
    void on_pushButton_Down_clicked();
    void on_pushButton_Left_clicked();
    void on_pushButton_Right_clicked();

    void on_pushButton_WorldOrigin_clicked();
    void on_pushButton_PcbOrigin_clicked();
    void on_pushButton_FeederOrigin_clicked();

    void motionDialog_moveTimer_timeout();
    void motionDialog_visionTimer_timeout();

    void on_comboBox_Axis_currentIndexChanged(int index);
};
