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

private slots:
    void on_pushButton_Cancel_clicked();
    void on_pushButton_OK_clicked();

    void on_pushButton_Start_clicked();
    void on_pushButton_Stop_clicked();

    void on_pushButton_WorkFlow_clicked();
    void on_pushButton_Computation_clicked();

    void visionDialog_visionTimer_timeout();

public slots:
    void displayCalculateResult(const cv::Mat& image);

signals:
    void vision_computation_signal();
};

