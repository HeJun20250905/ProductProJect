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

int MotionCore::MotionCore_InitAxis(int axis, double unit, double speed, double accel, double decel)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] MotionCore_InitAxis 设备未连接，无法初始化轴";
        return -1;
    }

    if (unit <= 0.0 || speed <= 0.0 || accel <= 0.0 || decel <= 0.0)
    {
        qDebug() << "[MotionCore] MotionCore_InitAxis 非法参数";
        return -1;
    }

    int32 ret = 0;
    int axisNum = 4;

    // 批量初始化所有轴
    if (axis == -1)
    {
        for (int i = 0; i < axisNum; i++)
        {
            // 设置轴类型为 1 (脉冲轴)
            ret = ZAux_Direct_SetAtype(reinterpret_cast<ZMC_HANDLE>(m_handle), i, 1);
            if (ret != 0) { qDebug() << "[MotionCore] ZAux_Direct_SetAtype 设置 ATYPE 失败:" << ret; return (int)ret; }

            // 设置脉冲当量 unit
            ret = ZAux_Direct_SetUnits(reinterpret_cast<ZMC_HANDLE>(m_handle), i, unit);
            if (ret != 0) { qDebug() << "[MotionCore] ZAux_Direct_SetUnits 设置 UNITS 失败:" << ret; return (int)ret; }

            // 设置速度 speed
            ret = ZAux_Direct_SetSpeed(reinterpret_cast<ZMC_HANDLE>(m_handle), i, speed);
            if (ret != 0) { qDebug() << "[MotionCore] ZAux_Direct_SetSpeed 设置 SPEED 失败:" << ret; return (int)ret; }

            // 设置加速度 accel
            ret = ZAux_Direct_SetAccel(reinterpret_cast<ZMC_HANDLE>(m_handle), i, accel);
            if (ret != 0) { qDebug() << "[MotionCore] ZAux_Direct_SetAccel 设置 ACCEL 失败:" << ret; return (int)ret; }

            // 设置减速度 decel
            ret = ZAux_Direct_SetDecel(reinterpret_cast<ZMC_HANDLE>(m_handle), i, decel);
            if (ret != 0) { qDebug() << "[MotionCore] ZAux_Direct_SetDecel 设置 DECEL 失败:" << ret; return (int)ret; }

            // 设置S曲线时间 (例如 10)
            ret = ZAux_Direct_SetSramp(reinterpret_cast<ZMC_HANDLE>(m_handle), i, 10);
            if (ret != 0) { qDebug() << "[MotionCore] ZAux_Direct_SetSramp 设置 SRAMP 失败:" << ret; return (int)ret; }
        }
    }
    else
    {
        // 初始化指定轴
        // 设置轴类型为 1 (脉冲轴)
        ret = ZAux_Direct_SetAtype(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, 1);
        if (ret != 0) { qDebug() << "[MotionCore] 设置 ATYPE 失败:" << ret; return (int)ret; }

        // 设置脉冲当量 unit
        ret = ZAux_Direct_SetUnits(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, unit);
        if (ret != 0) { qDebug() << "[MotionCore] 设置 UNITS 失败:" << ret; return (int)ret; }

        // 设置速度 speed
        ret = ZAux_Direct_SetSpeed(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, speed);
        if (ret != 0) { qDebug() << "[MotionCore] 设置 SPEED 失败:" << ret; return (int)ret; }

        // 设置加速度 accel
        ret = ZAux_Direct_SetAccel(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, accel);
        if (ret != 0) { qDebug() << "[MotionCore] 设置 ACCEL 失败:" << ret; return (int)ret; }

        // 设置减速度 decel
        ret = ZAux_Direct_SetDecel(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, decel);
        if (ret != 0) { qDebug() << "[MotionCore] 设置 DECEL 失败:" << ret; return (int)ret; }

        // 设置S曲线时间 (例如 10)
        ret = ZAux_Direct_SetSramp(reinterpret_cast<ZMC_HANDLE>(m_handle), axis, 10);
        if (ret != 0) { qDebug() << "[MotionCore] 设置 SRAMP 失败:" << ret; return (int)ret; }

        qDebug() << "[MotionCore] 轴" << axis << " 初始化完成";
    }

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

int MotionCore::MotionCore_Home(int axis, float speed, float creep, float accel, float decel)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接";
        return -1;
    }

    // 检查轴号合法性
    if (axis < 0 || axis > 2)
    {
        qDebug() << "[MotionCore] 无效轴号:" << axis;
        return -1;
    }

    // 判断轴是否空闲
    int32 idle = 0;
    ZAux_Direct_GetIfIdle(m_handle, axis, &idle);
    if (idle == 0)
    {
        qDebug() << "[MotionCore] 轴" << axis << "正在运动中，无法回零";
        return -1;
    }

    // 脉冲+方向模式
    ZAux_Direct_SetInvertStep(m_handle, axis, 0);

    // 轴类型：1=脉冲轴
    ZAux_Direct_SetAtype(m_handle, axis, 1);

    // 脉冲当量：统一
    ZAux_Direct_SetUnits(m_handle, axis, 400);

    // 速度参数
    ZAux_Direct_SetSpeed(m_handle, axis, speed);
    ZAux_Direct_SetCreep(m_handle, axis, creep);
    ZAux_Direct_SetAccel(m_handle, axis, accel);
    ZAux_Direct_SetDecel(m_handle, axis, decel);

    // 根据轴号自动选择原点输入口：X→IN0, Y→IN1, Z→IN2
    int datumIn = axis;   // 0/1/2 正好对应 IN0/IN1/IN2
    ZAux_Direct_SetDatumIn(m_handle, axis, datumIn);
    ZAux_Direct_SetInvertIn(m_handle, datumIn, 0);   // ECI 系列默认高电平触发

    // 启动回零：模式4 = 反向找开关 → 正向爬行离开 → 离开瞬间清零
    int32 ret = ZAux_Direct_Single_Datum(m_handle, axis, 4);
    if (ret != 0)
    {
        qDebug() << "[MotionCore] 轴" << axis << "回零失败，错误码:" << ret;
        return (int)ret;
    }

    qDebug() << "[MotionCore] 轴" << axis << "开始回零（模式4：反向找+缓慢爬行）";
    return 0;
}

int MotionCore::MotionCore_MoveAbsSingle(int axis, float position)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接，请先调用 MotionCore_Init() 初始化设备";
        return -1;
    }

    // 执行绝对定位
    int32 ret = ZAux_Direct_Single_MoveAbs(m_handle, axis, position);

    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_MoveRelSingle(int axis, float distance)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接，请先调用 MotionCore_Init() 初始化设备";
        return -1;
    }

    int32 ret = ZAux_Direct_Single_Move(m_handle, axis, distance);

    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_MoveAbsMulti(int axisCount, const int* axes, const float* targets)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接";
        return -1;
    }

    // 参数校验
    if (axes == nullptr || targets == nullptr)
    {
        qDebug() << "[MotionCore] 无效参数: 轴列表或目标位置为空";
        return -1;
    }

    // 拷贝
    int32 localAxes[MAX_INTERP_AXES];
    float localPos[MAX_INTERP_AXES];

    // 拷贝
    for (int i = 0; i < axisCount; ++i)
    {
        if (axes[i] < 0 || axes[i] > 3)
        {
            qDebug() << "[MotionCore] 无效轴号:" << axes[i];
            return -1;
        }
        localAxes[i] = axes[i];
        localPos[i] = targets[i];
    }

    // 执行绝对插补
    int32 ret = ZAux_Direct_MoveAbs(m_handle, axisCount, localAxes, localPos);
    if (ret != 0)
    {
        qDebug() << "[MotionCore] 多轴绝对插补失败，错误码:" << ret
            << "轴数:" << axisCount;
    }
    else
    {
        qDebug() << "[MotionCore] 多轴绝对插补启动，轴数:" << axisCount;
    }

    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_MoveRelMulti(int axisCount, const int* axes, const float* distances)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接";
        return -1;
    }

    // 参数校验
    if (axes == nullptr || distances == nullptr)
    {
        qDebug() << "[MotionCore] 无效参数: 轴列表或相对距离为空";
        return -1;
    }

    if (axisCount <= 0 || axisCount > MAX_INTERP_AXES)
    {
        qDebug() << "[MotionCore] 无效轴数:" << axisCount;
        return -1;
    }

    // 拷贝
    int32 localAxes[MAX_INTERP_AXES];
    float localDist[MAX_INTERP_AXES];

    for (int i = 0; i < axisCount; ++i)
    {
        if (axes[i] < 0 || axes[i] > 3)
        {
            qDebug() << "[MotionCore] 无效轴号:" << axes[i];
            return -1;
        }
        localAxes[i] = axes[i];
        localDist[i] = distances[i];
    }
    
    // 执行相对插补
    int32 ret = ZAux_Direct_Move(m_handle, axisCount, localAxes, localDist);
    if (ret != 0)
    {
        qDebug() << "[MotionCore] 多轴相对插补失败，错误码:" << ret
            << "轴数:" << axisCount;
    }
    else
    {
        qDebug() << "[MotionCore] 多轴相对插补启动，轴数:" << axisCount;
    }

    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_VMoveSingle(int axis, int direction, float speed)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接";
        return -1;
    }

    ZAux_Direct_SetSpeed(m_handle, axis, speed);

    // ZAux_Direct_Single_Vmove 是连续运动指令，不给距离，直到遇到停止指令或限位
    int32 ret = ZAux_Direct_Single_Vmove(m_handle, axis, direction);
    if (ret != 0)
    {
        qDebug() << "[MotionCore] 轴" << axis << "Vmove失败，错误码:" << ret;
    }
    else
    {
        qDebug() << "[MotionCore] 轴" << axis << "Vmove启动";
    }

    // 注意：Vmove通常是按照轴配置的默认速度运行，
    // 如果API支持带速度，需先调用 ZAux_Direct_SetSpeed(m_handle, axis, speed) 设置速度
    // 假设这里先设置速度再运动：
    // 

    return (ret == 0) ? 0 : (int)ret;
}

int MotionCore::MotionCore_StopSingle(int axis, int mode)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接，请先调用 MotionCore_Init() 初始化设备";
        return -1;
    }

    // 单轴减速停止 mode: 0=减速停, 1=急停
    int32 ret = ZAux_Direct_Single_Cancel(m_handle, axis, mode);
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

int MotionCore::MotionCore_GetInput(int ionum)
{
    if (!m_isConnected || m_handle == nullptr)
    {
        qDebug() << "[MotionCore] 设备未连接";
        return -1;
    }

    uint32 value = 0;
    int32 ret = ZAux_Direct_GetIn(m_handle, ionum, &value);
    if (ret != 0)
    {
        qDebug() << "[MotionCore] 读取 IN" << ionum << "失败，错误码:" << ret;
        return -1;
    }

    qDebug() << "[MotionCore] 读取 IN" << ionum << "成功，值:" << value;
    return (value != 0) ? 1 : 0;
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

