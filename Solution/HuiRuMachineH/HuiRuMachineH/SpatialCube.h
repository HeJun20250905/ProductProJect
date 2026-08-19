#pragma once
#include <QQueue>
#include <QMutex>

class SpatialCube
{
public:
	// 获取单例实例
	static SpatialCube& instance();

	// 禁止赋值和拷贝
	SpatialCube(const SpatialCube&) = delete;
	SpatialCube& operator=(const SpatialCube&) = delete;

	// 禁用移动构造和移动赋值
	SpatialCube(SpatialCube&&) = delete;
	SpatialCube& operator=(SpatialCube&&) = delete;

	// 初始化核心组件
	int SpatialCube_Init();

	// 世界坐标系原点标定
	int SpatialCube_CalibrateWorldOrigin(float x, float y);

	// 转换 pcb 坐标系到世界坐标系
	int SpatialCube_TransformPcbToWorld(float& pcb_x, float& pcb_y);
	
	// 转换 feeder 坐标系到世界坐标系
	int SpatialCube_TransformFeederToWorld(float& feeder_x, float& feeder_y);

	// 卸载/清理资源 (安全释放DLL内部内存)
	int SpatialCube_Uninit();

public:
	float m_worldZeroOffsetX = 0.0;						// 世界坐标系原点X电机偏移量
	float m_worldZeroOffsetY = 0.0;						// 世界坐标系原点Y电机偏移量
	float m_worldZeroOffsetZ = 0.0;						// 世界坐标系原点Z电机偏移量

	float m_pcbZeroOffsetX = 0.0;						// PCB坐标系原点X电机偏移量
	float m_pcbZeroOffsetY = 0.0;						// PCB坐标系原点Y电机偏移量
    float m_pcbZeroOffsetZ = 0.0;						// PCB坐标系原点Z电机偏移量

	float m_feederZeroOffsetX = 0.0;					// Feeder坐标系原点X电机偏移量
	float m_feederZeroOffsetY = 0.0;					// Feeder坐标系原点Y电机偏移量
    float m_feederZeroOffsetZ = 0.0;					// Feeder坐标系原点Z电机偏移量

private:
	// 构造函数和析构函数私有化
	explicit SpatialCube();
	~SpatialCube();

	mutable QMutex m_mutex;               // 【关键】多线程互斥锁 (mutable允许在const函数中加锁)
};

