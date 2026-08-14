#include "DataDialog.h"
#include <QDebug>
#include <cstring> // 必须包含，用于 strncpy

//#include "../DataBase.h"
//#include "../Library.h"

#include "../../../Include/LibDatabaseCore_Export.h"

#ifdef _DEBUG
    #define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
    #define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\LibDatabase.lib")

#define WIN32_LEAD_AND_MEAN
#include <windows.h>


#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

DataDialog::DataDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DataDialog)
{
    ui->setupUi(this);

    // 设置dialog标题风格
    this->setWindowFlags(
        Qt::Dialog |
        Qt::CustomizeWindowHint |
        Qt::WindowTitleHint |
        Qt::WindowCloseButtonHint);

    this->setWindowTitle("数据库");

    // 【关键】：在界面加载时，初始化数据库 DLL
    LibDatabase_InitDevice();
}

DataDialog::~DataDialog()
{
    LibDatabase_UninitDevice();

    delete ui;
}

void DataDialog::on_pushButton_Cancel_clicked()
{
    this->close();
}

void DataDialog::on_pushButton_OK_clicked()
{
    qDebug() << "[DataDialog] OK";
    this->accept();
}

void DataDialog::on_pushButton_Insert_clicked()
{
    qDebug() << "[DataDialog] Insert";

    // 从 UI 读取数据到结构体
    // 【核心修改】：使用纯 C 结构体，name 是 char 数组，不能直接用 QString 赋值
    LibraryItem item;
    memset(&item, 0, sizeof(LibraryItem)); // 安全初始化

    // 安全拷贝字符串到 char 数组
    QByteArray nameBytes = ui->lineEdit_Name->text().trimmed().toLocal8Bit();
    strncpy_s(item.name, nameBytes.constData(), sizeof(item.name) - 1);
    item.name[sizeof(item.name) - 1] = '\0';
    item.width = ui->lineEdit_Width->text().toFloat();
    item.height = ui->lineEdit_Height->text().toFloat();
    item.length = ui->lineEdit_Length->text().toFloat();
    item.pickZ = ui->lineEdit_PickZ->text().toFloat();
    item.placeZ = ui->lineEdit_PlaceZ->text().toFloat();
    item.speed = ui->lineEdit_Speed->text().toFloat();
    item.nozzleId = ui->lineEdit_NozzleId->text().toInt();

    // 工业安全校验
    if (strlen(item.name) == 0)
    {
        ::MessageBoxW((HWND)this->winId(), L"元件型号名称不能为空！", L"警告", MB_OK | MB_ICONWARNING);
        return;
    }

    // 【核心修改】：直接调用纯 C 的 DLL 导出函数，传入指针 &item
    int ret = LibDatabase_InsertLibraryItem(&item);

    // 根据返回值给出相应的弹窗提示
    if (ret == 0)
    {
        ::MessageBoxW((HWND)this->winId(), L"[DataDialog] 插入成功！", L"提示", MB_OK | MB_ICONINFORMATION);
    }
    else if (ret == -1)
    {
        ::MessageBoxW((HWND)this->winId(), L"错误：数据库未打开！", L"失败", MB_OK | MB_ICONERROR);
    }
    else
    {
        // 因为 name 是 UNIQUE 约束，如果已存在，SQLite 会报错，这里提示用户
        ::MessageBoxW((HWND)this->winId(), L"插入失败：该元件型号可能已存在！", L"失败", MB_OK | MB_ICONERROR);
    }
}

void DataDialog::on_pushButton_Delete_clicked()
{
    qDebug() << "[DataDialog] Delete";

    // 获取界面上输入的型号名称
    QString libraryName = ui->lineEdit_Name->text().trimmed();
    if (libraryName.isEmpty())
    {
        ::MessageBoxW((HWND)this->winId(), L"请输入要删除的元件型号！", L"警告", MB_OK | MB_ICONWARNING);
        return;
    }

    // 【核心修改】：将 QString 转为 const char* 传给纯 C 接口
    QByteArray nameBytes = libraryName.toLocal8Bit();
    int ret = LibDatabase_DeleteLibraryItem(nameBytes.constData());
    
    // 根据返回值处理结果
    if (ret == 0)
    {
        ::MessageBoxW((HWND)this->winId(), L"[DataDialog] 删除成功！", L"提示", MB_OK | MB_ICONINFORMATION);
        // 删除成功后，清空表格和输入框
        ui->tableWidget->clearContents();
        ui->lineEdit_Name->clear();
    }
    else if (ret == 1)
    {
        ::MessageBoxW((HWND)this->winId(), L"未找到该元件，删除失败！", L"警告", MB_OK | MB_ICONWARNING);
    }
    else
    {
        ::MessageBoxW((HWND)this->winId(), L"错误：数据库删除失败！", L"失败", MB_OK | MB_ICONERROR);
    }
}

void DataDialog::on_pushButton_Update_clicked()
{
    qDebug() << "[DataDialog] Update";

    // 从 UI 读取数据到结构体
    LibraryItem item;
    memset(&item, 0, sizeof(LibraryItem));

    QByteArray nameBytes = ui->lineEdit_Name->text().trimmed().toLocal8Bit();
    strncpy_s(item.name, nameBytes.constData(), sizeof(item.name) - 1);
    item.name[sizeof(item.name) - 1] = '\0';

    item.width = ui->lineEdit_Width->text().toFloat();
    item.height = ui->lineEdit_Height->text().toFloat();
    item.length = ui->lineEdit_Length->text().toFloat();
    item.pickZ = ui->lineEdit_PickZ->text().toFloat();
    item.placeZ = ui->lineEdit_PlaceZ->text().toFloat();
    item.speed = ui->lineEdit_Speed->text().toFloat();
    item.nozzleId = ui->lineEdit_NozzleId->text().toInt();

    // 工业安全校验：名称不能为空
    if (strlen(item.name) == 0)
    {
        ::MessageBoxW((HWND)this->winId(), L"元件型号名称不能为空！", L"警告", MB_OK | MB_ICONWARNING);
        return;
    }

    // 【核心修改】：直接调用纯 C 的 DLL 导出函数
    int ret = LibDatabase_UpdateLibraryItem(&item);

    // 根据返回值给出相应的弹窗提示
    if (ret == 0)
    {
        ::MessageBoxW((HWND)this->winId(), L"[DataDialog] 更新成功！", L"提示", MB_OK | MB_ICONINFORMATION);
    }
    else if (ret == 1)
    {
        ::MessageBoxW((HWND)this->winId(), L"未找到该元件，请先插入再更新！", L"警告", MB_OK | MB_ICONWARNING);
    }
    else
    {
        ::MessageBoxW((HWND)this->winId(), L"错误：元件数据更新失败！", L"失败", MB_OK | MB_ICONERROR);
    }
}

void DataDialog::on_pushButton_Query_clicked()
{
    qDebug() << "[DataDialog] Query";

    // 获取界面上输入的型号名称
    QString libraryName = ui->lineEdit_Name->text().trimmed();
    if (libraryName.isEmpty())
    {
        ::MessageBoxW((HWND)this->winId(), L"请输入元件型号后再查询！", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }

    // 准备接收数据的结构体并初始化
    LibraryItem item;
    LibDatabase_OnInitDataStruct(&item); // 注意：传指针

    // 调用数据库查询方法
    int ret = LibDatabase_QueryLibraryItem(&item, libraryName.toLocal8Bit().constData());

    // 根据返回值处理结果
    if (ret == 0)
    {
        // 确保 tableWidget 至少有 1 行、8 列
        if (ui->tableWidget->rowCount() < 1) ui->tableWidget->insertRow(0);
        if (ui->tableWidget->columnCount() < 8) ui->tableWidget->setColumnCount(8);

        // 辅助 lambda：创建居中的 Item
        auto createCenterItem = [](const QString& text) -> QTableWidgetItem* 
        {
            QTableWidgetItem* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter); // 核心：设置水平和垂直居中
            return item;
        };

        // 将查到的数据回显到 tableWidget 的第一行
        ui->tableWidget->setItem(0, 0, createCenterItem(item.name));
        ui->tableWidget->setItem(0, 1, createCenterItem(QString::number(item.length)));
        ui->tableWidget->setItem(0, 2, createCenterItem(QString::number(item.width)));
        ui->tableWidget->setItem(0, 3, createCenterItem(QString::number(item.height)));
        ui->tableWidget->setItem(0, 4, createCenterItem(QString::number(item.pickZ)));
        ui->tableWidget->setItem(0, 5, createCenterItem(QString::number(item.placeZ)));
        ui->tableWidget->setItem(0, 6, createCenterItem(QString::number(item.nozzleId)));
        ui->tableWidget->setItem(0, 7, createCenterItem(QString::number(item.speed)));

        ::MessageBoxW((HWND)this->winId(), L"[DataDialog] 查询成功！", L"提示", MB_OK | MB_ICONINFORMATION);
    }
    else if (ret == 1)
    {
        ::MessageBoxW((HWND)this->winId(), L"未查询到该元件型号！", L"提示", MB_OK | MB_ICONWARNING);
    }
    else
    {
        ::MessageBoxW((HWND)this->winId(), L"错误：数据库查询失败！", L"失败", MB_OK | MB_ICONERROR);
    }
}
