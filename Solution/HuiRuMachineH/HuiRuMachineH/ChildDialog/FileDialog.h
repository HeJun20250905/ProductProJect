#pragma once
#include <QDialog>
#include "ui_FileDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FileDialog; };
QT_END_NAMESPACE

class FileDialog : public QDialog
{
    Q_OBJECT
public:
    FileDialog(QWidget* parent = nullptr);
    ~FileDialog();

private:
    Ui::FileDialog* ui;

    int ShowDataOnLineEdit();

private slots:
    void on_pushButton_Cancel_clicked();
    void on_pushButton_LoadFile_clicked();
};
