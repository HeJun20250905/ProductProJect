#include "TaskList.h"
#include <QDebug>
#include <QFileDialog>
#include <QTextStream>
#include <QApplication> // 确保包含了这个头文件，以使用 qApp

// Windows API
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

TaskList& TaskList::instance()
{
    // 全局单例对象
    static TaskList instance;

    return instance;
}

int TaskList::TaskList_Init()
{
    // 弹出文件选择对话框，默认打开当前目录，过滤txt和csv文件
    //QString filePath = QFileDialog::getOpenFileName(
    //    nullptr,
    //    "选择SMT坐标文件",
    //    "",
    //    "坐标文件 (*.txt *.csv);;所有文件 (*)"
    //);

    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "选择SMT坐标文件",
        qApp->applicationDirPath(),  // 【修改点】：强制使用 .exe 所在的目录作为默认路径
        "坐标文件 (*.txt *.csv);;所有文件 (*)"
    );

    // 如果用户取消了选择，直接返回
    if (filePath.isEmpty()) 
    {
        return 1;
    }

    QFile file(filePath);

    // 打开失败返回 -1
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
    {
        qDebug() << "无法打开文件:" << filePath;
        return -1;
    }

    QTextStream in(&file);

    QString line;

    int lineNum = 0;

    while (!in.atEnd()) 
    {
        line = in.readLine().trimmed();

        // 跳过空行
        if (line.isEmpty())
        {
            continue;
        }

        lineNum++;

        // 假设第一行是表头（如 "No X Y Z R"），跳过
        if (lineNum == 1)
        {
            continue;
        }

        // 使用 QTextStream 解析这一行
        QTextStream stream(&line);

        int no;             // 临时变量，用来吃掉第一列的序号
        PickPlace coord;    // 你的目标结构体

        // 按顺序读取：序号(丢弃)、X、Y、Z、R
        stream >> no >> coord.PickX >> coord.PickY >> coord.PickZ >> coord.PickR;

        // 检查流的状态，如果不是 Ok，说明解析遇到了非数字字符等错误
        if (stream.status() != QTextStream::Ok)
        {
            qDebug() << "[TaskList] 警告: 第" << lineNum << "行解析失败，已跳过";
            continue;
        }

        // 解析成功，加入任务队列
        m_gTaskQueue.enqueue(coord);
    }

    file.close();

    qDebug() << "[TaskList] 文件解析完成，共加载" << m_gTaskQueue.size() << "个坐标";

    // 遍历队列并输出每一项的数据
    int index = 1;
    for (const PickPlace& coord : m_gTaskQueue) 
    {
        qDebug() << "[任务" << index << "]"
            << "X:" << coord.PickX
            << "Y:" << coord.PickY
            << "Z:" << coord.PickZ
            << "R:" << coord.PickR;
        index++;
    }

    return 0; // 成功返回 0
}

int TaskList::TaskList_AddTask(PickPlace task)
{
    return 0;
}

int TaskList::TaskList_GetTask(PickPlace* task)
{
    QMutexLocker locker(&m_mutex); // 自动加锁，出作用域自动解锁
    
    if (m_gTaskQueue.isEmpty()) 
    {
        return 1;
    }

    *task = m_gTaskQueue.dequeue();

    return 0;
}

int TaskList::TaskList_Uninit()
{
    qDebug() << "[TaskList] 开始安全停止连接...";


    return 0;
}

TaskList::TaskList()
{
    qDebug() << "[TaskList] 开始初始化";

}

TaskList::~TaskList()
{
    TaskList_Uninit();
}

