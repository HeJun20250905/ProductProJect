#include "ParaDialog.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

ParaDialog::ParaDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ParaDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("参数设置");
}

ParaDialog::~ParaDialog()
{
    delete ui;
}

void ParaDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}