#include "WorkThread.h"
#include <QDebug>
#include <QThread>

#include <cmath> 

#include "TaskList.h"
#include "FeederPos.h"

#include "../../../Include/MotionCore_Export.h"

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\MotionCore.lib")

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

WorkThread::WorkThread(QObject* parent)
	: QObject(parent)
{
    m_stopTimer = new QTimer(this);
    connect(m_stopTimer, &QTimer::timeout, this, &WorkThread::TimerSlot);
}

WorkThread::~WorkThread()
{
    if (m_currentTask != nullptr) 
    {
        delete m_currentTask;
        m_currentTask = nullptr;
    }
}

void WorkThread::doWork()
{
    // 初始化轴参数（ATYPE, UNITS, SPEED等）
    Motion_InitAxis(AXIS_X);
    Motion_InitAxis(AXIS_Y);
    Motion_InitAxis(AXIS_Z);
    Motion_InitAxis(AXIS_R);
    
    // X轴使能，1使能, 0关闭使能
    Motion_Enable(-1, 1);
    qDebug() << "[WorkThread] doWork: 使能全部XYZR轴";

    m_stopTimer->start(50);
}

int WorkThread::IsAtPosition(int axis, float targetPos, float* currentPos)
{
    // 安全检查：防止传入空指针导致程序崩溃
    if (currentPos == nullptr) 
    {
        qDebug() << "[WorkThread] IsAtPosition: 传入的指针为空！";
        return 1;
    }

    // 获取当前实际位置
    Motion_GetDpos(axis, currentPos);

    // 【核心判断】如果 目标位置与实际位置的差值 小于 允许误差，说明到位了！
    if (m_hasStarted && std::abs(*currentPos - targetPos) <= m_tolerance)
    {
        qDebug() << "[WorkThread] IsAtPosition: 轴位置确认到位！最终位置:" << *currentPos;
        return 0;
    }
    else
    {
        qDebug() << "[WorkThread] IsAtPosition: 轴未到位！"
            << "目标位置:" << targetPos
            << "当前位置:" << *currentPos
            << "设定容差:" << m_tolerance;

        return 1;
    }
}

void WorkThread::TimerSlot()
{
    // 紧急停止标志
    if (m_stopFlag) 
    {
        Motion_Stop(AXIS_X);
        Motion_Enable(AXIS_X, 0);
        Motion_Stop(AXIS_Y);
        Motion_Enable(AXIS_Y, 0);
        Motion_Stop(AXIS_Z);
        Motion_Enable(AXIS_Z, 0);
        Motion_Stop(AXIS_R);
        Motion_Enable(AXIS_R, 0);

        m_stopTimer->stop();

        emit workFinished();

        return;
    }

    // 如果手里没活（IDLE），去队列里取一个
    if (m_step == Step::idle) 
    {
        // 【关键】如果指针为空，先在堆上分配内存
        if (m_currentTask == nullptr) 
        {
            m_currentTask = new PickPlace();
        }

        // 尝试从队列取任务（这里调用你TaskList里的出队函数）
        int result = TaskList::instance().TaskList_GetTask(m_currentTask);

        if (result == 0)
        {
            m_step = Step::move_xy_feeder;          // 切换状态：准备开始移动XY

            m_targetPosX = m_currentTask->PickX;
            m_targetPosY = m_currentTask->PickY;
            m_targetPosZ = m_currentTask->PickZ;

            FeederPos::instance().FeederPos_GetFeederPos(m_feederPosX, m_feederPosY, m_feederPosZ, m_feederPosR);

            qDebug() << "[WorkThread] 领取新任务:" 
                << m_currentTask->PickName 
                << "目标X:" 
                << m_currentTask->PickX 
                << "目标Y:"
                << m_currentTask->PickY
                << "目标Z:"
                << m_currentTask->PickZ
                << "飞达:"
                << m_feederPosX 
                << m_feederPosY 
                << m_feederPosZ 
                ;
        }
        else 
        {
            // 队列空了
            m_stopTimer->stop();
            emit workFinished();
            qDebug() << "[WorkThread] 任务队列已空，结束工作";
            return;
        }
    }

    // 根据当前步骤，执行对应的动作
    switch (m_step)
    {
    case Step::move_xy_feeder:
        // 发起移动指令
        Motion_MoveAbsXY(AXIS_X, AXIS_Y, m_feederPosX, m_feederPosY, 500.0);
        qDebug() << "[WorkThread] FEEDER XY 指令发送";
        m_step = Step::wait_xy_feeder;
        // 标志位
        m_hasStarted = false;
        break;

    case Step::wait_xy_feeder:
        // 等待一个定时器周期
        if (!m_hasStarted)
        {
            m_hasStarted = true;
            break; // 直接跳出，50ms硬件反应时间
        }
        // 检查XY轴是否到位
        if (IsAtPosition(AXIS_X, m_feederPosX, &m_currentPosX) == 0 && IsAtPosition(AXIS_Y, m_feederPosY, &m_currentPosY) == 0)
        {
            qDebug() << "[WorkThread] FEEDER X Y 到位";
            m_step = Step::move_z_down_feeder;
        }
        break;

    case Step::move_z_down_feeder:
        // Z轴下降到取料高度
        Motion_MoveAbs(AXIS_Z, m_feederPosZ, 500.0);
        qDebug() << "[WorkThread] FEEDER Z 下降指令发送";
        m_step = Step::wait_z_down_feeder;
        // 标志位
        m_hasStarted = false;
        break;

    case Step::wait_z_down_feeder:
        // 等待一个定时器周期
        if (!m_hasStarted)
        {
            m_hasStarted = true;
            break; // 直接跳出，50ms硬件反应时间
        }
        if (IsAtPosition(AXIS_Z, m_feederPosZ, &m_currentPosZ) == 0)
        {
            qDebug() << "[WorkThread] FEEDER Z 轴下降到位";

            // 这里可以加一点延时或者IO操作（如打开气阀）
            m_step = Step::move_z_up_feeder;
        }
        break;

    case Step::move_z_up_feeder:
        // Z轴回到安全高度
        Motion_MoveAbs(AXIS_Z, 0.0, 500.0);
        qDebug() << "[WorkThread] FEEDER Z 上升指令发送";
        m_step = Step::wait_z_up_feeder; 
        // 标志位
        m_hasStarted = false;
        break;
    
    case Step::wait_z_up_feeder:
        // 等待一个定时器周期
        if (!m_hasStarted)
        {
            m_hasStarted = true;
            break; // 直接跳出，50ms硬件反应时间
        }
        if (IsAtPosition(AXIS_Z, 0.0, &m_currentPosZ) == 0)
        {
            qDebug() << "[WorkThread] FEEDER Z 轴上升到位";
            m_step = Step::move_xy_pcb;
        }
        break;

    case Step::move_xy_pcb:
        // 发起移动指令
        Motion_MoveAbsXY(AXIS_X, AXIS_Y, m_targetPosX, m_targetPosY, 500.0);
        qDebug() << "[WorkThread] PCB X Y 指令发送";
        m_step = Step::wait_xy_pcb;
        // 标志位
        m_hasStarted = false;
        break;

    case Step::wait_xy_pcb:
        // 等待一个定时器周期
        if (!m_hasStarted)
        {
            m_hasStarted = true;
            break; // 直接跳出，50ms硬件反应时间
        }
        if (IsAtPosition(AXIS_X, m_targetPosX, &m_currentPosX) == 0 && IsAtPosition(AXIS_Y, m_targetPosY, &m_currentPosY) == 0)
        {
            qDebug() << "[WorkThread] PCB X Y 到位";
            m_step = Step::move_z_down_pcb;
        }
        break;

    case Step::move_z_down_pcb:
        Motion_MoveAbs(AXIS_Z, -50.0, 500.0);
        qDebug() << "[WorkThread] PCB Z 下降指令";
        m_step = Step::wait_z_down_pcb;
        // 标志位
        m_hasStarted = false;
        break;

    case Step::wait_z_down_pcb:
        // 等待一个定时器周期
        if (!m_hasStarted)
        {
            m_hasStarted = true;
            break; // 直接跳出，50ms硬件反应时间
        }
        if (IsAtPosition(AXIS_Z, -50.0, &m_currentPosZ) == 0)
        {
            qDebug() << "[WorkThread] PCB Z 轴下降到位";

            // 这里可以加一点延时或者IO操作（如打开气阀）
            m_step = Step::move_z_up_pcb;
        }
        break;

    case Step::move_z_up_pcb:
        // Z轴回到安全高度
        Motion_MoveAbs(AXIS_Z, 0.0, 500.0);
        qDebug() << "[WorkThread] PCB Z 上升指令发送";
        m_step = Step::wait_z_up_pcb; 
        // 标志位
        m_hasStarted = false;
        break;

    case Step::wait_z_up_pcb:
        // 等待一个定时器周期
        if (!m_hasStarted)
        {
            m_hasStarted = true;
            break; // 直接跳出，50ms硬件反应时间
        }
        if (IsAtPosition(AXIS_Z, 0.0, &m_currentPosZ) == 0)
        {
            qDebug() << "[WorkThread] PCB Z轴上升到位，任务完成";

            qDebug() << "[WorkThread] 单次贴装完成，等待下一任务";
            // 下次定时器进来时，m_step又是IDLE，会自动去取下一个任务
            m_step = Step::idle; 
        }
        break;

    default:
        break;
    }

}

