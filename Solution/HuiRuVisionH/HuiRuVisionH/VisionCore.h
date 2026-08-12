#pragma once
#include <QtWidgets/QMainWindow>
#include <windows.h>  

QT_BEGIN_NAMESPACE
namespace Ui { class VisionCoreClass; };
QT_END_NAMESPACE

class VisionCore : public QMainWindow
{
    Q_OBJECT

public:
    VisionCore(QWidget* parent = nullptr);
    ~VisionCore();

    HWND m_hWndDisplay = nullptr;

    bool m_bGrabbing = false;
    void* m_hGrabThread = nullptr;

private:
    Ui::VisionCoreClass* ui;



private slots:
    void on_pushButton_Init_clicked();
    void on_pushButton_Start_clicked();
    void on_pushButton_Stop_clicked();
};