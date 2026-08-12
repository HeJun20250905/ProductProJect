#pragma once
#include <QDialog>
#include "ui_ResultView.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ResultView; };
QT_END_NAMESPACE

class MachineCore;

class ResultView : public QDialog
{
    Q_OBJECT
public:
    ResultView(QWidget *parent = nullptr);
    ~ResultView();

    MachineCore* m_Parent = nullptr;

    QString GetResult() const;

private:
    Ui::ResultView *ui;
    
private slots:
    void on_pushButton_OK_clicked();
    void on_pushButton_Cancel_clicked();

    void on_pushButton_Ptr_clicked();
};

