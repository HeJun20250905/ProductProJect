#include "FileDialog.h"
#include <QDebug>

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

int FileDialog::ShowDataOnLineEdit()
{
    // 创建局部QQueue副本
    QQueue<PickPlace> localQueueCopy = TaskList::instance().TaskList_GetQueue();

    // 清空表格旧数据
    ui->tableWidget_Center->setRowCount(0);

    // 根据局部副本的大小设置行数
    int rowCount = localQueueCopy.size();
    ui->tableWidget_Center->setRowCount(rowCount);

    // 遍历局部副本，将数据填入表格
    for (int row = 0; row < rowCount; ++row)
    {
        // 循环遍历取出数据取出数据
        PickPlace coord = localQueueCopy.dequeue();

        QTableWidgetItem* itemX = new QTableWidgetItem(QString::number(coord.PickX, 'f', 2));
        QTableWidgetItem* itemY = new QTableWidgetItem(QString::number(coord.PickY, 'f', 2));
        QTableWidgetItem* itemZ = new QTableWidgetItem(QString::number(coord.PickZ, 'f', 2));
        QTableWidgetItem* itemR = new QTableWidgetItem(QString::number(coord.PickR, 'f', 2));

        ui->tableWidget_Center->setItem(row, 0, itemX);
        ui->tableWidget_Center->setItem(row, 1, itemY);
        ui->tableWidget_Center->setItem(row, 2, itemZ);
        ui->tableWidget_Center->setItem(row, 3, itemR);
    }
    return 0;
}

void FileDialog::on_pushButton_LoadFile_clicked()
{
    QString selectedPath;
    int ret = TaskList::instance().TaskList_Init(selectedPath);

    if (ret == 0) 
    {
        qDebug() << "文件加载成功，路径为:" << selectedPath;
        ui->lineEdit_FilePath->setText(selectedPath);
        this->ShowDataOnLineEdit();
    }
    else if (ret == 1) 
    {
        qDebug() << "用户取消了选择";
    }
    else 
    {
        qDebug() << "文件打开失败";
    }
}

void FileDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}