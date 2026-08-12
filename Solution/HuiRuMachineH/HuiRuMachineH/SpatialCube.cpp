#include "SpatialCube.h"
#include <QDebug>

// Windows API
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <cmath> // 用于 sin, cos 等数学函数

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

SpatialCube& SpatialCube::instance()
{
    // 全局单例对象
    static SpatialCube instance;

    return instance;
}

int SpatialCube::SpatialCube_Init()
{
    QMutexLocker locker(&m_mutex); // 加锁保护

    // 电机回零操作
    
    // 计算世界坐标原点坐标补偿


    return 0;
}

int SpatialCube::TransformPcbToWorld(float& pcb_x, float& pcb_y, float& pcb_z, float& pcb_r)
{
    QMutexLocker locker(&m_mutex); // 加锁保护

    return 0;
}

int SpatialCube::TransformFeederToWorld(float& feeder_x, float& feeder_y, float& feeder_z, float& feeder_r)
{
    QMutexLocker locker(&m_mutex); // 加锁保护

    return 0;
}


int SpatialCube::SpatialCube_Uninit()
{
    QMutexLocker locker(&m_mutex); // 加锁保护

    qDebug() << "[SpatialCube] 开始安全停止连接...";

    return 0;
}

SpatialCube::SpatialCube()
{
    qDebug() << "[SpatialCube] 开始初始化";

}

SpatialCube::~SpatialCube()
{
    SpatialCube_Uninit();
}

