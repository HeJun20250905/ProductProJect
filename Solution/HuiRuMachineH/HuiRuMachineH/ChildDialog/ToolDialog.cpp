#include "ToolDialog.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

ToolDialog::ToolDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ToolDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("其他工具");
}

ToolDialog::~ToolDialog()
{
    delete ui;
}

void ToolDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}