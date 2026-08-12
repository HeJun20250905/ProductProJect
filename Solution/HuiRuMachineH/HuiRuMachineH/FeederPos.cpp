#include "FeederPos.h"

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

FeederPos& FeederPos::instance()
{
    // 全局单例对象
    static FeederPos instance;

    return instance;
}

int FeederPos::FeederPos_Init()
{

    return 0;
}

int FeederPos::FeederPos_GetFeederPos(float& x, float& y, float& z, float& r)
{
    qDebug() << "[FeederPos] 获取 feeder pos ";
    
    x = m_feederPos_x;
    y = m_feederPos_y;
    z = m_feederPos_z;
    r = m_feederPos_r;

    return 0;
}


int FeederPos::FeederPos_Uninit()
{
    qDebug() << "[FeederPos] 开始安全停止连接...";

    return 0;
}

FeederPos::FeederPos()
{
    qDebug() << "[FeederPos] 开始初始化";
}

FeederPos::~FeederPos()
{
    FeederPos_Uninit();
}

