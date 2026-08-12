#pragma once
#include <QString>

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
    int MotionCore_InitAxis(int axis);

	// 核心运动控制接口
	int MotionCore_SetEnable(int axis, int enableState);
	int MotionCore_MoveAbs(int axis, float position, float speed);
	int MotionCore_MoveRel(int axis, float distance, float speed);
	int MotionCore_MoveAbsXY(int x, int y, float targetX, float targetY, float speed);
    int MotionCore_MoveRelXY(int x, int y, float distX, float distY, float speed);
	int MotionCore_Stop(int axis);
	int MotionCore_GetDpos(int axis, float* outPosition);
	int MotionCore_GetAxisStatus(int axis, int* outStatus);

	// 卸载/清理资源 (安全释放DLL内部内存)
	int MotionCore_UninitDevice();

private:
	// 构造函数和析构函数私有化
	explicit MotionCore();
	~MotionCore();

	void* m_handle = nullptr;		// 核心：正运动控制器句柄
	bool m_isConnected = false;			// 连接状态标记
};

