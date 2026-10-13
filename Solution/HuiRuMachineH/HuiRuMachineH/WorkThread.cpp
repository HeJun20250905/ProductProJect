#include "WorkThread.h"
#include <QDebug>
#include <QThread>

#include <cmath> 

#include "TaskList.h"
#include "FeederPos.h"
#include "SpatialCube.h"

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
    m_stepTimer = new QTimer(this);
    connect(m_stepTimer, &QTimer::timeout, this, &WorkThread::TimerSlot);

    // 这个是用安全的计时器
    m_safeTimer = new QElapsedTimer();

    m_xAxisPosition_VisionPCB = SpatialCube::instance().m_xAxisPosition_VisionPCB;
    m_yAxisPosition_VisionPCB = SpatialCube::instance().m_yAxisPosition_VisionPCB;
    m_zAxisPosition_VisionPCB = SpatialCube::instance().m_zAxisPosition_VisionPCB;

    m_xAxisPosition_VisionShell = SpatialCube::instance().m_xAxisPosition_VisionShell;
    m_yAxisPosition_VisionShell = SpatialCube::instance().m_yAxisPosition_VisionShell;
    m_zAxisPosition_VisionShell = SpatialCube::instance().m_zAxisPosition_VisionShell;

    m_xAxisPosition_PCB1 = SpatialCube::instance().m_xAxisPosition_PCB1;
    m_yAxisPosition_PCB1 = SpatialCube::instance().m_yAxisPosition_PCB1;
    m_zAxisPosition_PCB1 = SpatialCube::instance().m_zAxisPosition_PCB1;

    m_xAxisPosition_PCB2 = SpatialCube::instance().m_xAxisPosition_PCB2;
    m_yAxisPosition_PCB2 = SpatialCube::instance().m_yAxisPosition_PCB2;
    m_zAxisPosition_PCB2 = SpatialCube::instance().m_zAxisPosition_PCB2;

    m_xAxisPosition_Shell1 = SpatialCube::instance().m_xAxisPosition_Shell1;
    m_yAxisPosition_Shell1 = SpatialCube::instance().m_yAxisPosition_Shell1;
    m_zAxisPosition_Shell1 = SpatialCube::instance().m_zAxisPosition_Shell1;

    m_xAxisPosition_Shell2 = SpatialCube::instance().m_xAxisPosition_Shell2;
    m_yAxisPosition_Shell2 = SpatialCube::instance().m_yAxisPosition_Shell2;
    m_zAxisPosition_Shell2 = SpatialCube::instance().m_zAxisPosition_Shell2;

    for (int i = 0; i < 4; i++)
    {
        m_laserPoints[i] = SpatialCube::instance().m_laserPoints[i];
    }
}

WorkThread::~WorkThread()
{
    //if (m_currentTask != nullptr) 
    //{
    //    delete m_currentTask;
    //    m_currentTask = nullptr;
    //}
}

void WorkThread::doWork(Step startStep)
{
    // 轴使能，1使能, 0关闭使能（-1使能所有轴）
    Motion_Enable(0, 1);
    Motion_Enable(1, 1);
    Motion_Enable(2, 1);
    Motion_Enable(3, 1);
    Motion_Enable(4, 1);
    Motion_Enable(5, 1);
    qDebug() << "[WorkThread] doWork: 使能全部XYZR轴";

    // 起始步骤
    m_step = startStep;

    if (startStep != Step::Pipeline)
    { m_doWorkSingle = true; }
    else
    { m_step = Step::Stacking; }

    m_stepTimer->start(50);
}

int WorkThread::IsAtPosition(int axis, float targetPos)
{
    float currentPos = 0.0;

    // 获取当前实际位置
    Motion_GetDpos(axis, &currentPos);

    // 【核心判断】如果 目标位置与实际位置的差值 小于 允许误差，说明到位了！
    if (m_hasStarted && std::abs(currentPos - targetPos) <= m_tolerance)
    {
        qDebug() << "[WorkThread] IsAtPosition: 轴位置确认到位！最终位置:" << currentPos;
        return 0;
    }
    else
    {
        qDebug() << "[WorkThread] IsAtPosition: 轴未到位！"
            << "目标位置:" << targetPos
            << "当前位置:" << currentPos
            << "设定容差:" << m_tolerance;

        return 1;
    }
}

void WorkThread::ReturnToSafeHeight()
{
    float zAxisPosition = SpatialCube::instance().m_zAxisPosition_Safe;
    int result = Motion_MoveAbsSingle(2, zAxisPosition);
    if (result == 0)
    {
        qDebug() << "[WorkThread] 回到安全高度";
    }
}

int WorkThread::SingleStepJudge()
{
    if (m_doWorkSingle)
    {
        this->on_stop_stepTimer();
    }

    return 0;
}

void WorkThread::on_stop_stepTimer()
{
    m_stepTimer->stop();
    emit workFinished();
}

void WorkThread::TimerSlot()
{
    // 紧急停止标志
    if (m_stopFlag) 
    {
        Motion_StopSingle(AXIS_X);
        Motion_Enable(AXIS_X, 0);

        Motion_StopSingle(AXIS_Y);
        Motion_Enable(AXIS_Y, 0);

        Motion_StopSingle(AXIS_Z);
        Motion_Enable(AXIS_Z, 0);

        Motion_StopSingle(AXIS_R);
        Motion_Enable(AXIS_R, 0);

        Motion_StopSingle(AXIS_H1);
        Motion_Enable(AXIS_H1, 0);

        Motion_StopSingle(AXIS_H2);
        Motion_Enable(AXIS_H2, 0);

        this->on_stop_stepTimer();

        qDebug() << "[WorkThread] 紧急停止";

        return;
    }

    int result = -100;

    // 根据当前步骤，执行对应的动作
    switch (m_step)
    {
    case Step::Idle:          // 空闲状态
        this->SingleStepJudge();
        break;

    case Step::Stacking:      // 码垛阶段
        if (!m_moving)
        {
            // 人工放码垛进去，按下开启按钮

            // 感应到了感应器位置，
            result = Motion_GetInput(0);  // 暂时用0号输入，根据现场调整
            if (result != 1)
            {
                qDebug() << "[WorkThread] 码垛阶段: 未检测到人工放码垛进入";
                break;
            }

            // H1轴缓慢上升，
            result = Motion_VMoveSingle(4, 0, 0);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 码垛阶段: H1轴缓慢上升";
            }
            else
            {
                qDebug() << "[WorkThread] 码垛阶段: H1轴缓慢上升失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }

            m_moving = true;
        }
        
        // 直到托盘感应器感应到就马上停止
        if (Motion_GetInput(0) == 1)
        {
            result = Motion_StopSingle(4, 1);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 码垛阶段: H1轴停止, result = " << result;
                m_moving = false;

                m_step = Step::VisionPCB;
                this->SingleStepJudge();
            }
        }
        
        break;

    case Step::VisionPCB:     // 视觉识别PCB信号
        if (!m_moving)
        {
            // 视觉模组移动到板位置，识别PCB信息，确认非空
            int axis[3] = { AXIS_X, AXIS_Y, AXIS_Z };
            float pos[3] = { m_xAxisPosition_VisionPCB, m_yAxisPosition_VisionPCB, m_zAxisPosition_VisionPCB };
            result = Motion_MoveAbsMulti(3, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 视觉识别PCB阶段: 正在移动到板位置";
            }
            else
            {
                qDebug() << "[WorkThread] 视觉识别PCB阶段: 移动到板位置失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            if (1)  // 这里是检测视觉识别完成PCB信息
            {
                if (!m_emitSiglePCB)
                {
                    if (IsAtPosition(AXIS_X, m_xAxisPosition_VisionPCB) &&
                        IsAtPosition(AXIS_Y, m_yAxisPosition_VisionPCB) &&
                        IsAtPosition(AXIS_Z, m_zAxisPosition_VisionPCB))
                    {
                        emit visionStart(VISION_PCB);
                        m_emitSiglePCB = true;
                        
                    }
                }     
            }
            else
            {
                // 这里是假设检测if (1)视觉识别完成PCB信息才进入到下一步骤
                m_step = Step::FetchShell;
                this->SingleStepJudge();
                m_moving = false;
                m_emitSiglePCB = false;
            }
        }
        
        break;

    case Step::FetchShell:    // 取外壳阶段
        // 外壳模组运行取到一个外壳,这里通过感应器确定有无外壳在平台上

        // 50ms 一次，累加到 20 次 = 1 秒
        m_fetchShellTick++;
        if (m_fetchShellTick < 20)
        {
            break;                      // 还没到 1 秒，本轮不检测
        }
            
        m_fetchShellTick = 0;    // 1 秒到，重置 tick
        m_fetchShellCount++;     // 第 N 次检测

        result = Motion_GetInput(0);
        if (result == 0)
        {
            qDebug() << "[WorkThread] 取外壳阶段: 检测到外壳, 第" << m_fetchShellCount << "次";
            m_fetchShellCount = 0;
            m_fetchShellTick = 0;
            m_step = Step::VisionShell;
            this->SingleStepJudge();
        }
        
        if (m_fetchShellCount >= 6)   // 6 次都没检测到
        {
            qDebug() << "[WorkThread] 取外壳阶段: 检测 6 次仍未检测到外壳, 报警";
            m_fetchShellCount = 0;
            m_fetchShellTick = 0;

            this->on_stop_stepTimer();
        }
        

        break;

    case Step::VisionShell:   // 视觉识别外壳阶段
        // 视觉模组移动到外壳位置，确认信息
        if (!m_moving)
        {
            // 视觉模组移动到板位置，识别PCB信息，确认非空
            int axis[3] = { AXIS_X, AXIS_Y, AXIS_Z };
            float pos[3] = { m_xAxisPosition_VisionShell, m_yAxisPosition_VisionShell, m_zAxisPosition_VisionShell };
            result = Motion_MoveAbsMulti(3, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 视觉识别外壳阶段: 视觉模组正在移动到板位置";
            }
            else
            {
                qDebug() << "[WorkThread] 视觉识别外壳阶段: 视觉模组移动到板位置失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            if (1)  // 这里是检测视觉识别完成Shell信息
            {
                if (!m_emitSingleShell)
                {
                    if (IsAtPosition(AXIS_X, m_xAxisPosition_VisionShell) &&
                        IsAtPosition(AXIS_Y, m_yAxisPosition_VisionShell) &&
                        IsAtPosition(AXIS_Z, m_zAxisPosition_VisionShell))
                    {
                        emit visionStart(VISION_SHELL);
                        m_emitSingleShell = true;
                    }
                }
            }
            else
            {
                // 这里是假设检测if (1)视觉识别完成Shell信息才进入到下一步骤
                m_step = Step::PickPCB1;
                this->SingleStepJudge();
                m_moving = false;
                m_emitSingleShell = false;
            }
        }

        break;

    case Step::PickPCB1:       // 抓手取PCB阶段
        // 先移动到PCB位置1
        if (!m_moving)
        {
            // 抓手模组移动到PCB位置1
            int axis[3] = { AXIS_X, AXIS_Y, AXIS_Z };
            float pos[3] = { m_xAxisPosition_PCB1, m_yAxisPosition_PCB1, m_zAxisPosition_PCB1 };
            result = Motion_MoveAbsMulti(3, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 抓手取PCB1阶段: 抓手模组正在移动到PCB位置1";
            }
            else
            {
                qDebug() << "[WorkThread] 抓手取PCB1阶段: 抓手模组移动到PCB位置1失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            if (IsAtPosition(AXIS_X, m_xAxisPosition_PCB1) &&
                IsAtPosition(AXIS_Y, m_yAxisPosition_PCB1) &&
                IsAtPosition(AXIS_Z, m_zAxisPosition_PCB1))
            {
                m_step = Step::PickPCB2;
                this->SingleStepJudge();
                m_moving = false;
            }
        }

        break;

    case Step::PickPCB2:       // 抓手取PCB阶段
        // 到PCB位置2取一个PCB板子
        if (!m_moving)
        {
            // 抓手模组移动到PCB位置1
            int axis[3] = { AXIS_X, AXIS_Y, AXIS_Z };
            float pos[3] = { m_xAxisPosition_PCB2, m_yAxisPosition_PCB2, m_zAxisPosition_PCB2 };
            result = Motion_MoveAbsMulti(3, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 抓手取PCB2阶段: 抓手模组正在移动到PCB位置2";
            }
            else
            {
                qDebug() << "[WorkThread] 抓手取PCB2阶段: 抓手模组移动到PCB位置2失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            if (IsAtPosition(AXIS_X, m_xAxisPosition_PCB2) &&
                IsAtPosition(AXIS_Y, m_yAxisPosition_PCB2) &&
                IsAtPosition(AXIS_Z, m_zAxisPosition_PCB2))
            {
                // 这里打开真空吸嘴

                m_step = Step::PickPCB3;
                m_moving = false;
            }
        }
        this->SingleStepJudge();

        break;

    case Step::PickPCB3:       // 抓手取PCB3阶段
        // 回到PCB位置1
        if (!m_moving)
        {
            // 抓手模组移动到PCB位置1
            int axis[3] = { AXIS_X, AXIS_Y, AXIS_Z };
            float pos[3] = { m_xAxisPosition_PCB1, m_yAxisPosition_PCB1, m_zAxisPosition_PCB1 };
            result = Motion_MoveAbsMulti(3, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 抓手取PCB3阶段: 抓手模组正在移动到PCB位置1";
            }
            else
            {
                qDebug() << "[WorkThread] 抓手取PCB3阶段: 抓手模组移动到PCB位置1失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            if (IsAtPosition(AXIS_X, m_xAxisPosition_PCB1) &&
                IsAtPosition(AXIS_Y, m_yAxisPosition_PCB1) &&
                IsAtPosition(AXIS_Z, m_zAxisPosition_PCB1))
            {
                m_step = Step::PlacePCB1;
                this->SingleStepJudge();
                m_moving = false;
            }
        }

        break;

    case Step::PlacePCB1:      // 抓手放PCB1阶段
        // 抓手模组移动到外壳位置1
        if (!m_moving)
        {
            // 抓手模组移动到PCB位置1
            int axis[3] = { AXIS_X, AXIS_Y, AXIS_Z };
            float pos[3] = { m_xAxisPosition_Shell1, m_yAxisPosition_Shell1, m_zAxisPosition_Shell1 };
            result = Motion_MoveAbsMulti(3, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 抓手放Shell 1阶段: 抓手模组正在移动到放Shell位置1";
            }
            else
            {
                qDebug() << "[WorkThread] 抓手放Shell 1阶段: 抓手模组移动到放Shell位置1失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            if (IsAtPosition(AXIS_X, m_xAxisPosition_Shell1) &&
                IsAtPosition(AXIS_Y, m_yAxisPosition_Shell1) &&
                IsAtPosition(AXIS_Z, m_zAxisPosition_Shell1))
            {
                m_step = Step::PlacePCB2;
                this->SingleStepJudge();
                m_moving = false;
            }
        }

        break;

    case Step::PlacePCB2:      // 抓手放PCB2阶段
        // 抓手模组放板子到外壳位置2上去
        if (!m_moving)
        {
            // 抓手模组移动到PCB位置1
            int axis[3] = { AXIS_X, AXIS_Y, AXIS_Z };
            float pos[3] = { m_xAxisPosition_Shell2, m_yAxisPosition_Shell2, m_zAxisPosition_Shell2 };
            result = Motion_MoveAbsMulti(3, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 抓手放Shell 2阶段: 抓手模组正在移动到放Shell位置2";
            }
            else
            {
                qDebug() << "[WorkThread] 抓手放Shell 2阶段: 抓手模组移动到放Shell位置2失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            if (IsAtPosition(AXIS_X, m_xAxisPosition_Shell2) &&
                IsAtPosition(AXIS_Y, m_yAxisPosition_Shell2) &&
                IsAtPosition(AXIS_Z, m_zAxisPosition_Shell2))
            {
                // 这里关闭真空吸嘴

                m_step = Step::PlacePCB3;
                this->SingleStepJudge();
                m_moving = false;
            }
        }
        
        break;

    case Step::PlacePCB3:      // 抓手放PCB3阶段
        // 抓手模组回到外壳位置1
        if (!m_moving)
        {
            // 抓手模组移动到PCB位置1
            int axis[3] = { AXIS_X, AXIS_Y, AXIS_Z };
            float pos[3] = { m_xAxisPosition_Shell1, m_yAxisPosition_Shell1, m_zAxisPosition_Shell1 };
            result = Motion_MoveAbsMulti(3, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 抓手放Shell 3阶段: 抓手模组正在移动到放Shell位置1";
            }
            else
            {
                qDebug() << "[WorkThread] 抓手放Shell 3阶段: 抓手模组移动到放Shell位置1失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            if (IsAtPosition(AXIS_X, m_xAxisPosition_Shell1) &&
                IsAtPosition(AXIS_Y, m_yAxisPosition_Shell1) &&
                IsAtPosition(AXIS_Z, m_zAxisPosition_Shell1))
            {
                m_step = Step::PlacePCB1;
                this->SingleStepJudge();
                m_moving = false;
            }
        }

        break;

    case Step::LaserConfirm1:  // 激光位移确认阶段
        // 激光位移测距确认正常
        if (!m_moving)
        {
            int axis[2] = { AXIS_X, AXIS_Y };
            float pos[2] = { m_laserPoints[0].x(), m_laserPoints[0].y() };
            result = Motion_MoveAbsMulti(2, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移正在移动到1点";
            }
            else
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移移动到1点失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            // 激光位移确认
            result = Motion_GetInput(0);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移到1点已确认";
                m_moving = false;
                m_step = Step::LaserConfirm2;
                this->SingleStepJudge();
            }
            else
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移到1点未确认, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
        }
        

        break;

    case Step::LaserConfirm2:  // 激光位移确认阶段
        // 激光位移测距确认正常
        if (!m_moving)
        {
            int axis[2] = { AXIS_X, AXIS_Y };
            float pos[2] = { m_laserPoints[1].x(), m_laserPoints[1].y() };
            result = Motion_MoveAbsMulti(2, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移正在移动到2点";
            }
            else
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移移动到2点失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            // 激光位移确认
            result = Motion_GetInput(0);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移到2点已确认";
                m_moving = false;
                m_step = Step::LaserConfirm3;
                this->SingleStepJudge();
            }
            else
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移到2点未确认, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
        }

        break;

    case Step::LaserConfirm3:  // 激光位移确认阶段
        // 激光位移测距确认正常
        if (!m_moving)
        {
            int axis[2] = { AXIS_X, AXIS_Y };
            float pos[2] = { m_laserPoints[2].x(), m_laserPoints[2].y() };
            result = Motion_MoveAbsMulti(2, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移正在移动到3点";
            }
            else
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移移动到3点失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            // 激光位移确认
            result = Motion_GetInput(0);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移到3点已确认";
                m_moving = false;
                m_step = Step::LaserConfirm4;
                this->SingleStepJudge();
            }
            else
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移到3点未确认, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
        }

        break;

    case Step::LaserConfirm4:  // 激光位移确认阶段
        // 激光位移测距确认正常
        if (!m_moving)
        {
            int axis[2] = { AXIS_X, AXIS_Y };
            float pos[2] = { m_laserPoints[3].x(), m_laserPoints[3].y() };
            result = Motion_MoveAbsMulti(2, axis, pos);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移正在移动到4点";
            }
            else
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移移动到4点失败, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
            m_moving = true;
        }
        else
        {
            // 激光位移确认
            result = Motion_GetInput(0);
            if (result == 0)
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移到4点已确认";
                m_moving = false;
                m_step = Step::ShellPlace;
                this->SingleStepJudge();
            }
            else
            {
                qDebug() << "[WorkThread] 激光位移确认阶段: 激光位移到4点未确认, result = " << result;
                this->on_stop_stepTimer();
                break;
            }
        }

        break;

    case Step::ShellPlace:    // 外壳模组放下阶段
        // 外壳模组放下外壳

        m_step = Step::ShellPlace;
        this->SingleStepJudge();

        break;

    case Step::Executing:     // 一个动作执行后的等待时间

        m_step = Step::Executing;
        this->SingleStepJudge();

        break;

    default:
        break;
    }
}

