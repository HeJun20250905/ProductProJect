#include "ResultView.h"
#include "MachineCore.h"

ResultView::ResultView(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ResultView)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog | 
        Qt::CustomizeWindowHint | 
        Qt::WindowTitleHint | 
        Qt::WindowCloseButtonHint);

    m_Parent = qobject_cast<MachineCore*>(parent);
    
}

ResultView::~ResultView()
{
    delete ui;
}

QString ResultView::GetResult() const
{
    return ui->lineEdit->text();
}

void ResultView::on_pushButton_OK_clicked()
{
    this->accept();
}

void ResultView::on_pushButton_Cancel_clicked()
{
    this->close();
}

void ResultView::on_pushButton_Ptr_clicked()
{
    if (m_Parent)
    {
        QString message = ui->lineEdit->text();
        m_Parent->SetDialogMessageOnLabel(message);
    }
}
