#include "Library.h"

#include <QDebug>

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

Library& Library::instance()
{
    // 全局单例对象
    static Library instance;

    return instance;
}

int Library::Library_Init()
{

    return 0;
}


int Library::Library_Uninit()
{
    qDebug() << "[Library] 开始安全停止连接...";


    return 0;
}

Library::Library()
{
    qDebug() << "[Library] 开始初始化";

}

Library::~Library()
{
    Library_Uninit();
}

