#pragma once
#include <QDialog>
#include "ui_MotionTestDialog.h"

#include <QPointer>                 // 智能指针
#include <QThread>                  // 线程

#include "WorkThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MotionTestDialog; };
QT_END_NAMESPACE

class WorkThread;

class MotionTestDialog : public QDialog
{
    Q_OBJECT
public:
    MotionTestDialog(QWidget* parent = nullptr);
    ~MotionTestDialog();

private:
    Ui::MotionTestDialog* ui;

    QPointer<QThread> m_workerThread;               // 工作线程
    WorkThread* m_worker = nullptr;                 // 工作线程对象

    void StartSingleWorkThread(Step step);

private slots:
    void on_pushButton_OK_clicked();
    void on_pushButton_Cancel_clicked();

    void on_pushButton_Stacking_clicked();
    void on_pushButton_VisionPCB_clicked();
    void on_pushButton_FetchShell_clicked();
    void on_pushButton_VisionShell_clicked();
    void on_pushButton_PickPCB1_clicked();
    void on_pushButton_PickPCB2_clicked();
    void on_pushButton_PickPCB3_clicked();
    void on_pushButton_PlacePCB1_clicked();
    void on_pushButton_PlacePCB2_clicked();
    void on_pushButton_PlacePCB3_clicked();
    void on_pushButton_LaserConfirm1_clicked();
    void on_pushButton_LaserConfirm2_clicked();
    void on_pushButton_LaserConfirm3_clicked();
    void on_pushButton_LaserConfirm4_clicked();
    void on_pushButton_ShellPlace_clicked();
};