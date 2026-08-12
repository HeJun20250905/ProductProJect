#pragma once
#include <QDialog>
#include "ui_ParaDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ParaDialog; };
QT_END_NAMESPACE

class ParaDialog : public QDialog
{
    Q_OBJECT
public:
    ParaDialog(QWidget* parent = nullptr);
    ~ParaDialog();

private:
    Ui::ParaDialog* ui;

private slots:
    void on_pushButton_Cancel_clicked();
};
