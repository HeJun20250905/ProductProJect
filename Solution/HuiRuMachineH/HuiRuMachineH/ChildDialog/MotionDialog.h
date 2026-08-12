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

    QTimer* m_moveTimer = nullptr;

    float m_speed = 0.01f;
    float m_targetPos = 0.0f;
    float m_currentPos = 0.0f;

    int m_axisIndex = -1;

private slots:
    void on_pushButton_Cancel_clicked();
    void on_pushButton_OK_clicked();

    void on_pushButton_Move_clicked();

    void on_moveTimer_timeout();

    void on_pushButton_DecX_clicked();
    void on_pushButton_IncX_clicked();
    void on_pushButton_DecY_clicked();
    void on_pushButton_IncY_clicked();
    void on_pushButton_IncZ_clicked();
    void on_pushButton_DecZ_clicked();
};
