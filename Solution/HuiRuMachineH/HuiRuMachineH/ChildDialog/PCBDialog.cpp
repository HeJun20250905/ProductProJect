#include "PCBDialog.h"
#include "DataDialog.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

PCBDialog::PCBDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PCBDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("PCB");
}

PCBDialog::~PCBDialog()
{
    delete ui;
}

void PCBDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}