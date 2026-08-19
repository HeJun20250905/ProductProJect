#pragma once
#include <QObject>
#include <QTimer>

#include <atomic> // 必须包含这个头文件

struct PickPlace;

enum class Step
{
    idle = 0,                      // 空闲：去队列取任务

    move_xy_feeder,                // 步骤1：XY轴移动
    wait_xy_feeder,                // 步骤2：等XY停稳
    move_z_down_feeder,            // 步骤3：Z轴下降取料
    wait_z_down_feeder,            // 步骤4：等Z下降到位
    move_z_up_feeder,              // 步骤5：Z轴上升
    wait_z_up_feeder,              // 步骤6：等Z上升

    move_xy_pcb,                   // 步骤7：XY轴移动
    wait_xy_pcb,                   // 步骤8：等XY停稳
    move_z_down_pcb,               // 步骤9：Z轴下降取料
    wait_z_down_pcb,               // 步骤10：等Z下降到位
    move_z_up_pcb,                 // 步骤11：Z轴上升
    wait_z_up_pcb                  // 步骤12：等Z上升
};

class WorkThread : public QObject
{
    Q_OBJECT

public:
    WorkThread(QObject* parent = nullptr);
    ~WorkThread();

    void doWork();

private:
    std::atomic<bool> m_stopFlag{ false };        // 紧急停止标志

    QTimer* m_stopTimer = nullptr;

    float m_targetPosX = 0.0;                    // X目标位置
    float m_speedX = 500.0;                      // X速度
    float m_currentPosX = 0.0;                   // X当前位置

    float m_targetPosY = 0.0;                    // Y目标位置
    float m_speedY = 500.0;                      // Y速度
    float m_currentPosY = 0.0;                   // Y当前位置

    float m_targetPosZ = 0.0;                    // Z目标位置
    float m_speedZ = 500.0;                      // Z速度
    float m_currentPosZ = 0.0;                   // Z当前位置

    float m_targetPosR = 0.0;                    // R目标位置
    float m_speedR = 500.0;                      // R速度
    float m_currentPosR = 0.0;                   // R当前位置

    float m_feederZeroOffsetX = 0.0;             // 飞达位置X
    float m_feederZeroOffsetY = 0.0;             // 飞达位置Y
    float m_feederZeroOffsetZ = 0.0;             // 飞达位置Z

    float m_tolerance = 0.05;                   // 0.05个单位误差

    bool m_hasStarted = false;                   // 防止起步瞬间误判

    Step m_step = Step::idle;                    // 当前任务步骤
    PickPlace* m_currentTask = nullptr;          // 当前任务

    // 工具函数：检查轴是否到位
    int IsAtPosition(int axis, float targetPos, float* currentPos);

private slots:
    void TimerSlot();

signals:
    void workFinished();
};

