#pragma once
#include <QDialog>
#include "ui_ToolDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ToolDialog; };
QT_END_NAMESPACE

class ToolDialog : public QDialog
{
    Q_OBJECT
public:
    ToolDialog(QWidget* parent = nullptr);
    ~ToolDialog();

private:
    Ui::ToolDialog* ui;

private slots:
    void on_pushButton_Cancel_clicked();
};
