#include "LibraryDialog.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

LibraryDialog::LibraryDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LibraryDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("元件库");
}

LibraryDialog::~LibraryDialog()
{
    delete ui;
}

void LibraryDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}