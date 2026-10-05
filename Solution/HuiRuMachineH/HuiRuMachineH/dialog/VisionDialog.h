#pragma once
#include <QDialog>
#include "ui_VisionDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class VisionDialog; };
QT_END_NAMESPACE

namespace cv 
{
    class Mat;
}

#define PIXEL_TYPE_MONO8            0x01080001
#define PIXEL_TYPE_RGB8_PACKED      0x01080008

class VisionDialog : public QDialog
{
    Q_OBJECT
public:
    VisionDialog(QWidget* parent = nullptr);
    ~VisionDialog();

private:
    Ui::VisionDialog* ui;

    QTimer* m_visionDialog_visionTimer = nullptr;

    // 鼠标事件
    bool eventFilter(QObject* obj, QEvent* event);

    // 鼠标事件处理
    void handleDoubleClick(const QPoint& pos);

    double m_mmPerPixel = 0.05;      // 毫米/像素

    bool VisionDialogAxisMove(double xPoint, double yPoint);

    double m_notchAngle = 0.00;    // 角点角度

private slots:
    void on_pushButton_Cancel_clicked();
    void on_pushButton_OK_clicked();

    void on_pushButton_Start_clicked();
    void on_pushButton_Stop_clicked();

    void on_pushButton_WorkFlow_clicked();
    void on_pushButton_Computation_clicked();

    void visionDialog_visionTimer_timeout();

    void on_pushButton_Picture_clicked();

    void on_pushButton_CalibBoard_clicked();

    void on_pushButton_CalibScale_clicked();

    void on_pushButton_DetectAngle_clicked();

public slots:
    void displayCalculateResult(const cv::Mat& image);

signals:
    void vision_computation_signal();
};

