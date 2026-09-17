#pragma once
#include <QMutex>
#include <QVector>

enum class VisionWorkFlow
{
    None = 0,           // 无操作/初始状态
    GaussBlur,          // 预处理：高斯滤波
    Threshold,          // 预处理：二值化
    FindContour,        // 特征提取：寻找轮廓
    FilterContour,      // 核心筛选：面积/长宽比过滤
    Locate,             // 精确定位：计算中心点/角度
    Draw,               // 结果绘制：在原图上画框或十字
    StepCount           // 总步骤数
};						// 备注：这里的枚举值一定要和UI上面的算法顺序严格一致

struct VisionWorkPara
{
    // 高斯模糊参数
    int m_gaussCoreX = 5;                               // 高斯核X
    int m_gaussCoreY = 5;                               // 高斯核Y
    double m_gaussSigmaX = 1.5;                         // 高斯标准差X
    double m_gaussSigmaY = 1.5;                         // 高斯标准差Y

    // 二值化参数
    int m_thresholdValue = 127;                         // 二值化的阈值

    // 筛选轮廓参数

    double m_minMarkArea = 1500.0;                      // 设置最小面积阈值
    double m_maxMarkArea = 5000.0;                      // 设置最大面积阈值
 
	// 画图参数
	int m_lineWidX = 2;                                 // 线宽X
	int m_lineWidY = 2;                                 // 线宽Y
	int m_lineLenX = 50;                                // 线距X
	int m_lineLenY = 50;                                // 线距Y
	int m_lineColorR = 0;                               // 线色R
	int m_lineColorG = 0;                               // 线色G
	int m_lineColorB = 0;                               // 线色B
};

namespace cv
{ class Mat; }

class VisionFrame
{
public:
	// 获取单例实例
	static VisionFrame& instance();

	// 禁止赋值和拷贝
	VisionFrame(const VisionFrame&) = delete;
	VisionFrame& operator=(const VisionFrame&) = delete;

	// 禁用移动构造和移动赋值
	VisionFrame(VisionFrame&&) = delete;
	VisionFrame& operator=(VisionFrame&&) = delete;

	//初始化核心组件
	int VisionFrame_Init();

	// 卸载/清理资源 (安全释放DLL内部内存)
	int VisionFrame_Uninit();


	// ########################## 视觉视频帧处理 ##########################

	cv::Mat* m_visionFrame = nullptr;               // 视觉算法源图
	QMutex m_frameMutex;                            // 视觉安全锁     
	bool m_newFrame = false;						// 视频帧是否更新

	// 更新视频帧
	void UpdateVisionFrame(unsigned char* pData, int nLen);

	// 获取视频帧
	cv::Mat GetVisionFrame() const;

	// ########################## 视觉算法工作流 ##########################

	QVector<VisionWorkFlow> m_visionWorkFlow;					// 视觉算法工作流

    // 更新视觉工作流
	void UpdateVisionWorkFlow(const QVector<VisionWorkFlow>& workFlow);

    // 获取视觉工作流
	QVector<VisionWorkFlow> GetVisionWorkFlow() const;

	// ########################## 视觉算法工作参数 ##########################

	VisionWorkPara m_visionWorkPara;							// 视觉算法工作参数

	// 更新视觉算法工作参数
	void UpdateVisionWorkPara(const VisionWorkPara& para);

	// 获取视觉算法工作参数
	VisionWorkPara GetVisionWorkPara() const;

private:
	// 构造函数和析构函数私有化
	explicit VisionFrame();
	~VisionFrame();

	mutable QMutex m_visionFrameMutex;							// 多线程互斥锁 (mutable允许在const函数中加锁)
	mutable QMutex m_visionWorkFlowMutex;						// 多线程互斥锁 (mutable允许在const函数中加锁)
	mutable QMutex m_visionWorkParaMutex;						// 多线程互斥锁 (mutable允许在const函数中加锁)
};

