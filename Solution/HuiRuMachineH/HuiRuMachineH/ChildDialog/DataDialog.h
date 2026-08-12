#pragma once
#include <QDialog>
#include "ui_DataDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DataDialog; };
QT_END_NAMESPACE

class DataDialog : public QDialog
{
    Q_OBJECT
public:
    DataDialog(QWidget* parent = nullptr);
    ~DataDialog();

private:
    Ui::DataDialog* ui;

private slots:
    void on_pushButton_Cancel_clicked();
    void on_pushButton_OK_clicked();

    void on_pushButton_Insert_clicked();    // 插入
    void on_pushButton_Delete_clicked();    // 删除
    void on_pushButton_Update_clicked();    // 更新
    void on_pushButton_Query_clicked();     // 查询
};


