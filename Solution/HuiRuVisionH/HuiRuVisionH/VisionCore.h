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

	// 开始采集图像
    int VisionCore_Start();

	// 停止采集图像
    int VisionCore_Stop();

	// 卸载/清理资源 (安全释放DLL内部内存)
	int VisionCore_Uninit();


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

