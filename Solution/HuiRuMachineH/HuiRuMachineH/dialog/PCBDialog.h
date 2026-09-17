#pragma once
#include <QDialog>
#include "ui_PCBDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PCBDialog; };
QT_END_NAMESPACE

class PCBDialog : public QDialog
{
    Q_OBJECT
public:
    PCBDialog(QWidget* parent = nullptr);
    ~PCBDialog();

private:
    Ui::PCBDialog* ui;

private slots:
    void on_pushButton_Cancel_clicked();
};


