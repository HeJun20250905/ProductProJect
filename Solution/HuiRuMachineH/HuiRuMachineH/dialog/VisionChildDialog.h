#pragma once
#include <QDialog>
#include "ui_VisionChildDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class VisionChildDialog; };
QT_END_NAMESPACE

class VisionChildDialog : public QDialog
{
    Q_OBJECT
public:
    VisionChildDialog(QWidget* parent = nullptr);
    ~VisionChildDialog();

private:
    Ui::VisionChildDialog* ui;

private slots:
    void on_pushButton_Cancel_clicked();
    void on_pushButton_OK_clicked();

    void on_listWidget_Left_currentRowChanged(int currentRow);
};