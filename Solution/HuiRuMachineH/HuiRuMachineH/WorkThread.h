#pragma once
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QPointF>

#include <atomic>       // 必须包含这个头文件

struct PickPlace;

enum class Step
{
    Idle,                   // 空闲状态
    Stacking,               // 码垛阶段
    VisionPCB,              // 视觉识别PCB信号
    FetchShell,             // 取外壳阶段
    VisionShell,            // 视觉识别外壳阶段
    PickPCB1,                // 抓手取PCB阶段
    PickPCB2,                // 抓手取PCB阶段
    PickPCB3,                // 抓手取PCB阶段
    PlacePCB1,               // 抓手放PCB阶段
    PlacePCB2,               // 抓手放PCB阶段
    PlacePCB3,               // 抓手放PCB阶段
    LaserConfirm1,           // 激光位移确认阶段
    LaserConfirm2,           // 激光位移确认阶段
    LaserConfirm3,           // 激光位移确认阶段
    LaserConfirm4,           // 激光位移确认阶段
    ShellPlace,             // 外壳模组放下阶段
    Executing,              // 动作执行等待状态
    Pipeline               // 执行全部的动作
};

// 视觉任务种类标记
enum VisionTask
{
    VISION_PCB = 1,         // 识别PCB
    VISION_SHELL = 2,       // 识别SHELL
};

class WorkThread : public QObject
{
    Q_OBJECT

public:
    WorkThread(QObject* parent = nullptr);
    ~WorkThread();

    void doWork(Step startStep = Step::Executing);

    bool m_doWorkSingle = false;                  // 执行单步标记

private:
    std::atomic<bool> m_stopFlag{ false };        // 紧急停止标志

    QTimer* m_stepTimer = nullptr;
    QElapsedTimer* m_safeTimer = nullptr;

    float m_xAxisPosition_Safe = 0.0;             // 安全高度X轴位置
    float m_yAxisPosition_Safe = 0.0;             // 安全高度Y轴位置
    float m_zAxisPosition_Safe = 0.0;             // 安全高度Z轴位置

    float m_xAxisPosition_VisionPCB = 0.0;             // 相机视觉X轴位置
    float m_yAxisPosition_VisionPCB = 0.0;             // 相机视觉Y轴位置
    float m_zAxisPosition_VisionPCB = 0.0;             // 相机视觉Z轴位置

    float m_xAxisPosition_VisionShell = 0.0;             // 相机视觉X轴位置
    float m_yAxisPosition_VisionShell = 0.0;             // 相机视觉Y轴位置
    float m_zAxisPosition_VisionShell = 0.0;             // 相机视觉Z轴位置

    float m_xAxisPosition_PCB1 = 0.0;             // PCB1位置
    float m_yAxisPosition_PCB1 = 0.0;             // PCB1位置
    float m_zAxisPosition_PCB1 = 0.0;             // PCB1位置

    float m_xAxisPosition_PCB2 = 0.0;             // PCB2位置
    float m_yAxisPosition_PCB2 = 0.0;             // PCB2位置
    float m_zAxisPosition_PCB2 = 0.0;             // PCB2位置

    float m_xAxisPosition_Shell1 = 0.0;             // Shell位置
    float m_yAxisPosition_Shell1 = 0.0;             // Shell位置
    float m_zAxisPosition_Shell1 = 0.0;             // Shell位置

    float m_xAxisPosition_Shell2 = 0.0;             // Shell位置
    float m_yAxisPosition_Shell2 = 0.0;             // Shell位置
    float m_zAxisPosition_Shell2 = 0.0;             // Shell位置

    QPointF m_laserPoints[4];                   // 4 个测量点

    float m_tolerance = 0.05;                   // 0.05个单位误差

    bool m_hasStarted = false;                   // 防止起步瞬间误判
    
    Step m_step = Step::Idle;                    // 当前任务步骤

    bool m_moving = false;                      // 是否正在运动

    bool m_emitSiglePCB = false;
    bool m_emitSingleShell = false;

    int m_fetchShellCount = 0;              // 已检测次数（0~6）
    int m_fetchShellTick = 0;               // 50ms 累加器（0~19）

    // 工具函数：检查轴是否到位
    int IsAtPosition(int axis, float targetPos);

    // 工具函数：回到安全高度
    void ReturnToSafeHeight();

    // 工具函数：单步判定
    int SingleStepJudge();

    void on_stop_stepTimer();

private slots:
    void TimerSlot();

signals:
    void workFinished();
    void visionStart(int taskId);
    void scanStart();
};

