#pragma once
#include <QDialog>
#include "ui_LibraryDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LibraryDialog; };
QT_END_NAMESPACE

class LibraryDialog : public QDialog
{
    Q_OBJECT
public:
    LibraryDialog(QWidget* parent = nullptr);
    ~LibraryDialog();

private:
    Ui::LibraryDialog* ui;

private slots:
    void on_pushButton_Cancel_clicked();
};
