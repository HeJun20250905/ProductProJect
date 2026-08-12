#pragma once
#include <QQueue>
#include <QMutex>

class FeederPos
{
public:
	// 获取单例实例
	static FeederPos& instance();

	// 禁止赋值和拷贝
	FeederPos(const FeederPos&) = delete;
	FeederPos& operator=(const FeederPos&) = delete;

	// 禁用移动构造和移动赋值
	FeederPos(FeederPos&&) = delete;
	FeederPos& operator=(FeederPos&&) = delete;

	//初始化核心组件
	int FeederPos_Init();

	// 获取FeederPos
    int FeederPos_GetFeederPos(float& x, float& y, float& z, float& r);

	// 卸载/清理资源 (安全释放DLL内部内存)
	int FeederPos_Uninit();

private:
	// 构造函数和析构函数私有化
	explicit FeederPos();
	~FeederPos();

	float m_feederPos_x = 50.0;
	float m_feederPos_y = 50.0;
	float m_feederPos_z = 50.0;
	float m_feederPos_r = 50.0;

	mutable QMutex m_mutex;               // 【关键】多线程互斥锁 (mutable允许在const函数中加锁)
};

