#pragma once
#include <QMutex>

#include "../../../Include/MVS/MvCameraControl.h"

// Windows API
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

typedef void (*VisionImageCallback)(unsigned char* pData, 
									int nLen, 
									int nWidth, 
									int nHeight, 
									int nPixelType, 
									void* pUser
);

class VisionCore
{
public:
	// 获取单例实例
	static VisionCore& instance();

	// 禁止赋值和拷贝
	VisionCore(const VisionCore&) = delete;
	VisionCore& operator=(const VisionCore&) = delete;

	// 禁用移动构造和移动赋值
	VisionCore(VisionCore&&) = delete;
	VisionCore& operator=(VisionCore&&) = delete;

	//初始化核心组件
	int VisionCore_Init(HWND hWndDisplay);

	// 卸载/清理资源 (安全释放DLL内部内存)
	int VisionCore_Uninit();

	// 设置回调
	void VisionCore_SetImageCallback(VisionImageCallback callback, void* pUser);
	
	// 清除回调
	void VisionCore_ClearImageCallback();

	// 开始采集图像
    int VisionCore_Start();

	// 停止采集图像
    int VisionCore_Stop();

	// 曝光时间 (单位: 微秒 μs)
	int VisionCore_SetExposureTime(float exposureUs);
	int VisionCore_GetExposureTime(float& exposureUs);

	// 自动曝光 (0=Off, 1=On, 2=Once)
	int VisionCore_SetExposureAuto(int mode);
	int VisionCore_GetExposureAuto(int& mode);

	// 增益 (单位: dB)
	int VisionCore_SetGain(float gainDb);
	int VisionCore_GetGain(float& gainDb);

	// 自动增益 (0=Off, 1=On, 2=Once)
	int VisionCore_SetGainAuto(int mode);
	int VisionCore_GetGainAuto(int& mode);

	// 亮度 (0~255)
	int VisionCore_SetBrightness(int brightness);
	int VisionCore_GetBrightness(int& brightness);

	// 对比度 (范围因相机而异，先Get获取当前范围)
	int VisionCore_SetContrast(int contrast);
	int VisionCore_GetContrast(int& contrast);

	// 伽马使能
	int VisionCore_SetGammaEnable(bool enable);
	int VisionCore_GetGammaEnable(bool& enable);

	// 伽马值 (0.1~5.0)
	int VisionCore_SetGamma(float gamma);
	int VisionCore_GetGamma(float& gamma);

	// 自动白平衡 (0=Off, 1=On, 2=Once)
	int VisionCore_SetWhiteBalanceAuto(int mode);
	int VisionCore_GetWhiteBalanceAuto(int& mode);

	// 采集帧率 (单位: Hz)
	int VisionCore_SetAcquisitionFrameRate(float frameRate);
	int VisionCore_GetAcquisitionFrameRate(float& frameRate);

	// 像素格式 (枚举值，如 PixelType_Gvsp_Mono8)
	int VisionCore_SetPixelFormat(unsigned int pixelFormat);
	int VisionCore_GetPixelFormat(unsigned int& pixelFormat);

	// 触发模式 (0=Off连续, 1=On触发)
	int VisionCore_SetTriggerMode(int mode);
	int VisionCore_GetTriggerMode(int& mode);

	// 触发源
	int VisionCore_SetTriggerSource(int& source);
	int VisionCore_GetTriggerSource(int& sourceValue);

	// 回调相关的成员变量
	VisionImageCallback m_callback = nullptr;			// 外部传入的回调函数指针
	void* m_pCallbackUser = nullptr;					// 外部传入的用户上下文指针

	// 以下成员变量放在public这里是因为需要在静态全局函数中调用

	HWND m_display = nullptr;							// 图像显示窗口句柄

	void* m_handle = nullptr;							// 相机设备句柄

	bool m_isGrabbing = false;							// 是否正在采集标志

	unsigned char* m_frameData = nullptr;				// 存图像数据的内存

	MV_FRAME_OUT_INFO_EX m_frameInfo = { 0 };			// 图像的宽高、像素格式等信息

	unsigned int m_nFrameDataSize = 0;					// 【新增】记录当前分配的内存大小

	mutable QMutex m_mutex;								// 【关键】多线程互斥锁 (mutable允许在const函数中加锁)

private:
	// 构造函数和析构函数私有化
	explicit VisionCore();
	~VisionCore();
	
	MV_CC_DEVICE_INFO_LIST m_deviceList;				// 枚举到的设备列表

	MV_CC_DEVICE_INFO* m_deviceInfo = nullptr;			// 相机设备信息

	int m_packetSize = 0;								// 网络数据包尺寸大小

    void* m_grabThread = nullptr;						// 采集线程句柄
};

