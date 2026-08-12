#include "VisionDialog.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

VisionDialog::VisionDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::VisionDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("视觉");
}

VisionDialog::~VisionDialog()
{
    delete ui;
}

void VisionDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}