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


	// 初始化核心组件（世界坐标原点标定）
	int SpatialCube_Init();

	// 转换 pcb 坐标系到世界坐标系
	int TransformPcbToWorld(float& pcb_x, float& pcb_y, float& pcb_z, float& pcb_r);
	
	// 转换 feeder 坐标系到世界坐标系
	int TransformFeederToWorld(float& feeder_x, float& feeder_y, float& feeder_z, float& feeder_r);

	// 卸载/清理资源 (安全释放DLL内部内存)
	int SpatialCube_Uninit();

private:
	// 构造函数和析构函数私有化
	explicit SpatialCube();
	~SpatialCube();

	float m_world_xZeroOff = 0.0;					// 世界坐标系原点X偏移
    float m_world_yZeroOff = 0.0;					// 世界坐标系原点Y偏移
    float m_world_zZeroOff = 0.0;					// 世界坐标系原点Z偏移
    float m_world_rZeroOff = 0.0;					// 世界坐标系原点R偏移

	float m_pcb_xZeroOff = 0.0;
    float m_pcb_yZeroOff = 0.0;
    float m_pcb_zZeroOff = 0.0;
    float m_pcb_rZeroOff = 0.0;

    float m_feeder_xZeroOff = 0.0;
	float m_feeder_yZeroOff = 0.0;
    float m_feeder_zZeroOff = 0.0;
    float m_feeder_rZeroOff = 0.0;
    

	mutable QMutex m_mutex;               // 【关键】多线程互斥锁 (mutable允许在const函数中加锁)
};

