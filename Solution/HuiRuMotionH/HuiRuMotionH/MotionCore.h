#pragma once
#include <QString>

// 临时设置的轴数
#define MAX_INTERP_AXES  4

class MotionCore
{
public:
	// 获取单例实例
	static MotionCore& instance();

	// 禁止赋值和拷贝
	MotionCore(const MotionCore&) = delete;
	MotionCore& operator=(const MotionCore&) = delete;

	// 禁用移动构造和移动赋值
	MotionCore(MotionCore&&) = delete;
	MotionCore& operator=(MotionCore&&) = delete;

	//初始化核心组件（连接运控卡）
	int MotionCore_Init(const QString& ipAddress);

	// 轴初始化
    int MotionCore_InitAxis(int axis, double unit, double speed, double accel, double decel);

	// 轴使能
	int MotionCore_SetEnable(int axis, int enableState);

	// 轴回零
	int MotionCore_Home(int axis, float speed, float creep, float accel, float decel);

	// 单轴运动到绝对位置
	int MotionCore_MoveAbsSingle(int axis, float position);

    // 单轴运动到相对位置
	int MotionCore_MoveRelSingle(int axis, float distance);

    // 多轴运动到绝对位置
	int MotionCore_MoveAbsMulti(int axisCount, const int* axes, const float* targets);

	// 多轴运动到相对位置
	int MotionCore_MoveRelMulti(int axisCount, const int* axes, const float* distances);

	// 单轴连续运动
	int MotionCore_VMoveSingle(int axis, int direction, float speed);

	// 单轴停止运动
	int MotionCore_StopSingle(int axis, int mode = 1);

    // 获取轴位置
	int MotionCore_GetDpos(int axis, float* outPosition);

    // 获取轴状态
	int MotionCore_GetAxisStatus(int axis, int* outStatus);

	// 获取IO点状态
    int MotionCore_GetInput(int ionum);

	// 卸载/清理资源 (安全释放DLL内部内存)
	int MotionCore_UninitDevice();

private:
	// 构造函数和析构函数私有化
	explicit MotionCore();
	~MotionCore();

	void* m_handle = nullptr;			// 核心：正运动控制器句柄
	bool m_isConnected = false;			// 连接状态标记

	float m_axis0 = 0.0f;				// 极限值
	float m_axis1 = 0.0f;				// 极限值
	float m_axis2 = 0.0f;				// 极限值
	float m_axis3 = 0.0f;				// 极限值
	float m_axis4 = 0.0f;				// 极限值
	float m_axis5 = 0.0f;				// 极限值
};

