#include "MachineCore.h"
#include <QtWidgets/QApplication>

#include "Database.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MachineCore window;
    window.show();

    // 启动事件循环，程序会在这里阻塞，直到窗口关闭或退出
    int ret = app.exec();

    // 程序退出事件循环后，再安全地关闭数据库
    Database::instance().Database_Uninit();

    return ret;
}
