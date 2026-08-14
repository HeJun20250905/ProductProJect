#pragma once
#include <QDialog>
#include "ui_VisionDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class VisionDialog; };
QT_END_NAMESPACE

class VisionDialog : public QDialog
{
    Q_OBJECT
public:
    VisionDialog(QWidget* parent = nullptr);
    ~VisionDialog();

private:
    Ui::VisionDialog* ui;

private slots:
    void on_pushButton_Cancel_clicked();
    void on_pushButton_OK_clicked();

    void on_pushButton_Init_clicked();
    void on_pushButton_Start_clicked();
    void on_pushButton_Stop_clicked();
    void on_pushButton_Uninit_clicked();
};


