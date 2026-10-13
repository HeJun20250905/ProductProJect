#pragma once
#include <QQueue>
#include <QMutex>
#include <QPointF>

struct MeasurePoint
{
	float x = 0.0f;
	float y = 0.0f;
};

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
	float m_zAxisPosition_Safe = 0.0;			// Z轴安全高度

	float m_hAxisPosition = 0.0;				// h轴目标位置

	float m_xAxisPosition_VisionPCB = 0.0;			// x轴相机拍照目标位置
	float m_yAxisPosition_VisionPCB = 0.0;			// y轴相机拍照目标位置
	float m_zAxisPosition_VisionPCB = 0.0;			// z轴相机拍照目标位置

	float m_xAxisPosition_VisionShell = 0.0;			// x轴相机拍照目标位置
	float m_yAxisPosition_VisionShell = 0.0;			// y轴相机拍照目标位置
	float m_zAxisPosition_VisionShell = 0.0;			// z轴相机拍照目标位置

	float m_xAxisVisionOffset = 0.0;			// x视觉结果补偿
    float m_yAxisVisionOffset = 0.0;			// y视觉结果补偿
    float m_zAxisVisionOffset = 0.0;			// z视觉结果补偿
    float m_rAxisVisionOffset = 0.0;			// r视觉结果补偿
	
	bool m_visionRunSign = false;				// 视觉任务运行标志

	float m_xAxisPosition_PCB1 = 0.0;			// x轴PCB1目标位置
	float m_yAxisPosition_PCB1 = 0.0;			// y轴PCB1目标位置
	float m_zAxisPosition_PCB1 = 0.0;			// z轴PCB1目标位置

	float m_xAxisPosition_PCB2 = 0.0;			// x轴PCB2目标位置
	float m_yAxisPosition_PCB2 = 0.0;			// y轴PCB2目标位置
	float m_zAxisPosition_PCB2 = 0.0;			// z轴PCB2目标位置

	float m_xAxisPosition_Shell1 = 0.0;			// x轴Shell1目标位置
	float m_yAxisPosition_Shell1 = 0.0;			// y轴Shell1目标位置
	float m_zAxisPosition_Shell1 = 0.0;			// z轴Shell1目标位置

	float m_xAxisPosition_Shell2 = 0.0;			// x轴Shell2目标位置
	float m_yAxisPosition_Shell2 = 0.0;			// y轴Shell2目标位置
	float m_zAxisPosition_Shell2 = 0.0;			// z轴Shell2目标位置

	QPointF m_laserPoints[4];					// 4 个点

private:
	// 构造函数和析构函数私有化
	explicit SpatialCube();
	~SpatialCube();

	mutable QMutex m_mutex;               // 【关键】多线程互斥锁 (mutable允许在const函数中加锁)
};

