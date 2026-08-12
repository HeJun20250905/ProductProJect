#include "MotionCore.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>

// Windows API
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif

#include <windows.h>

#include "../../../Include/zauxdll2.h"

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\zauxdll.lib")
#pragma comment(lib,  LIB_PATH"\\zmotion.lib") 

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

MotionCore& MotionCore::instance()
{
    // 全局单例对象
    static MotionCore instance;

    return instance;
}

int MotionCore::MotionCore_Init(const QString& ipAddress)
{
    if (m_isConnected && m_handle != nullptr) 
    {
        qDebug() << "[MotionCore] 设备已连接，跳过重复初始化";
        return 0;
    }

    // 将 QString 转换为正运动需要的 char* 格式
    QByteArray ipBytes = ipAddress.toLatin1();
    char* ipStr = ipBytes.data();

    qDebug() << "[MotionCore] 正在尝试连接控制器，IP:" << ipAddress;

    // 调用正运动官方以太网连接API
    int32 ret = ZAux_OpenEth(const_cast<char*>(ipStr), reinterpret_cast<ZMC_HANDLE*>(&m_handle));

    if (ret == 0 && m_handle != nullptr) 
    {
        m_isConnected = true;
        qDebug() << "[MotionCore] 控制器连接成功! Handle:" << (void*)m_handle;

        return 0;
    }
    else {
        qDebug() << "[MotionCore] 控制器连接失败，错误码:" << ret;
        m_handle = nullptr;
        return -1;
    }
}

int MotionCore::MotionCore_InitAxis(int axis)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接，无法初始化轴";
        return -1;
    }

    int32 ret = 0;

    // 1. 设置轴类型为 1 (脉冲轴)
    ret = ZAux_Direct_SetAtype(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, 1);
    if (ret != 0) { qDebug() << "[MotionCore] 设置 ATYPE 失败:" << ret; return (int)ret; }

    // 2. 设置脉冲当量 (例如 1.0)
    ret = ZAux_Direct_SetUnits(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, 1.0);
    if (ret != 0) { qDebug() << "[MotionCore] 设置 UNITS 失败:" << ret; return (int)ret; }

    // 3. 设置速度 (例如 100)
    ret = ZAux_Direct_SetSpeed(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, 100.0);
    if (ret != 0) { qDebug() << "[MotionCore] 设置 SPEED 失败:" << ret; return (int)ret; }

    // 4. 设置加速度 (例如 3000)
    ret = ZAux_Direct_SetAccel(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, 3000.0);
    if (ret != 0) { qDebug() << "[MotionCore] 设置 ACCEL 失败:" << ret; return (int)ret; }

    // 5. 设置减速度 (例如 3000)
    ret = ZAux_Direct_SetDecel(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, 3000.0);
    if (ret != 0) { qDebug() << "[MotionCore] 设置 DECEL 失败:" << ret; return (int)ret; }

    // 6. 设置S曲线时间 (例如 10)
    ret = ZAux_Direct_SetSramp(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, 10);
    if (ret != 0) { qDebug() << "[MotionCore] 设置 SRAMP 失败:" << ret; return (int)ret; }

    qDebug() << "[MotionCore] 轴" << axis << " 初始化完成";

    return 0;
}

int MotionCore::MotionCore_SetEnable(int axis, int enableState)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接，无法设置使能";
        return -1;
    }

    // 【修正】使用正运动官方的使能 API
    // enableState: 1 = 开启使能, 0 = 关闭使能
    int32 ret = ZAux_Direct_SetAxisEnable(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, enableState);

    if (ret == 0) 
    {
        qDebug() << "[MotionCore] 轴" << axis << " 使能状态已设置为:" << (enableState == 1 ? "开启" : "关闭");
    }
    else 
    {
        qDebug() << "[MotionCore] 设置使能失败，错误码:" << ret;
    }

    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_MoveAbs(int axis, float position, float speed)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接，请先调用 MotionCore_Init() 初始化设备";
        return -1;
    }

    // 设置目标速度
    ZAux_Direct_SetSpeed(m_handle, axis, speed);

    // 执行绝对定位
    int32 ret = ZAux_Direct_Single_MoveAbs(m_handle, axis, position);

    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_MoveRel(int axis, float distance, float speed)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接，请先调用 MotionCore_Init() 初始化设备";
        return -1;
    }

    ZAux_Direct_SetSpeed(m_handle, axis, speed);
    int32 ret = ZAux_Direct_Single_Move(m_handle, axis, distance);

    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_MoveAbsXY(int x, int y, float targetX, float targetY, float speed)
{
    if (!m_isConnected || m_handle == nullptr) 
    {
        qDebug() << "[MotionCore] 设备未连接，无法执行XY插补";
        return -1;
    }

    // 1. 设置合成速度（插补时的速度是合成速度，即XY合成矢量的速度）
    ZAux_Direct_SetSpeed(m_handle, x, speed);
    ZAux_Direct_SetSpeed(m_handle, y, speed);

    // 2. 定义轴列表和目标位置列表
    int32 axes[2] = { x, y };
    float targetPos[2] = { targetX, targetY };

    // 3. 调用绝对多轴直线插补
    int32 ret = ZAux_Direct_MoveAbs(m_handle, 2, axes, targetPos);

    if (ret != 0) 
    {
        qDebug() << "[MotionCore] XY绝对插补失败，错误码:" << ret;
    }
    else 
    {
        qDebug() << "[MotionCore] XY绝对插补启动 -> X:" << targetX << ", Y:" << targetY;
    }

    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_MoveRelXY(int x, int y, float distX, float distY, float speed)
{
    if (!m_isConnected || m_handle == nullptr) 
    {
        qDebug() << "[MotionCore] 设备未连接";
        return -1;
    }

    ZAux_Direct_SetSpeed(m_handle, x, speed);
    ZAux_Direct_SetSpeed(m_handle, y, speed);

    int32 axes[2] = { x, y };
    float distPos[2] = { distX, distY };

    int32 ret = ZAux_Direct_Move(m_handle, 2, axes, distPos);

    if (ret != 0) {
        qDebug() << "[MotionCore] XY相对插补失败，错误码:" << ret;
    }
    else {
        qDebug() << "[MotionCore] XY相对插补启动 -> dX:" << distX << ", dY:" << distY;
    }

    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_Stop(int axis)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接，请先调用 MotionCore_Init() 初始化设备";
        return -1;
    }

    // 单轴减速停止 (0=减速停, 1=急停)
    int32 ret = ZAux_Direct_Single_Cancel(m_handle, axis, 0);
    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_GetDpos(int axis, float* outPosition)
{
    if (!m_isConnected || m_handle == nullptr || outPosition == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接，请先调用 MotionCore_Init() 初始化设备";
        return -1;
    }

    // 获取轴的目标位置 (Dpos)
    int32 ret = ZAux_Direct_GetDpos(m_handle, axis, outPosition);
    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_GetAxisStatus(int axis, int* outStatus)
{
    if (!m_isConnected || m_handle == nullptr || outStatus == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接，请先调用 MotionCore_Init() 初始化设备";
        return -1;
    }

    // 获取轴的状态寄存器
    int32 ret = ZAux_Direct_GetAxisStatus(m_handle, axis, outStatus);
    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_UninitDevice()
{
    qDebug() << "[MotionCore] 开始安全停止运控卡连接...";

    if (m_handle != nullptr) 
    {
        // 关闭与运控卡的连接
        ZAux_Close(m_handle);
        m_handle = nullptr;
        m_isConnected = false;
        qDebug() << "[MotionCore] 运控卡连接已安全断开";
    }
    else 
    {
        qDebug() << "[MotionCore] 当前无有效连接，跳过断开操作";
    }

    return 0;
}

MotionCore::MotionCore()
{
    qDebug() << "[MotionCore] 开始初始化";

}

MotionCore::~MotionCore()
{
    if (m_isConnected) 
    {
        MotionCore_UninitDevice();
    }
}

