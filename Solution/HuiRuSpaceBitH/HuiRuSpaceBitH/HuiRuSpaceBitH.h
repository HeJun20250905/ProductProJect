#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_HuiRuSpaceBitH.h"

QT_BEGIN_NAMESPACE
namespace Ui { class HuiRuSpaceBitHClass; };
QT_END_NAMESPACE

class HuiRuSpaceBitH : public QMainWindow
{
    Q_OBJECT

public:
    HuiRuSpaceBitH(QWidget *parent = nullptr);
    ~HuiRuSpaceBitH();

private:
    Ui::HuiRuSpaceBitHClass *ui;

    QVector<QLabel*> m_cvMatLabels;
    QVector<QLabel*> m_windowLabels;

    int m_cvMatIndex = -1;
    int m_windowIndex = -1;

    QVector<QStringList> m_recordString = { {}, {}, {}, {}, {}, {} };

    int m_recordIndex = -1;

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private slots:
    void on_pushButton_LoadSource_clicked();
};

