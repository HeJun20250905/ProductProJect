#include "FileDialog.h"

#include "../TaskList.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

FileDialog::FileDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::FileDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("文件选项");
}

FileDialog::~FileDialog()
{
    delete ui;
}

void FileDialog::on_pushButton_AddFile_clicked()
{
    TaskList::instance().TaskList_Init();
}

void FileDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}