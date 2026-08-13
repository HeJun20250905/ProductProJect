#pragma once
#ifdef VISIONCTRL_EXPORTS
#define VISIONCTRL_API __declspec(dllexport)
#else
#define VISIONCTRL_API __declspec(dllimport)
#endif

// 返回状态码
typedef enum VISION_RESULT_STATUS
{
	VISION_RESULT_OK					= 0,						// 成功
	VISION_RESULT_ERR_NOT_INIT			= -1,						// 未初始化或已卸载
	VISION_RESULT_ERR_QT_ENV			= -2,						// Qt环境创建/获取失败
	VISION_RESULT_ERR_TRANS				= -3,						// 翻译器加载失败
	VISION_RESULT_ERR_INVALID_PARAM		= -4,						// 无效参数
	VISION_RESULT_ERR_UNKNOWN			= -99						// 未知错误
} VISION_RESULT_STATUS;

#ifdef __cplusplus
extern "C"
{
#endif

	// //初始化设备引擎
	VISIONCTRL_API int Vision_InitDevice(void* hWndDisplay);

	// 开始抓图
    VISIONCTRL_API int Vision_StartGrabbing();

    // 停止抓图
    VISIONCTRL_API int Vision_StopGrabbing();

	// 卸载/清理资源 (安全释放DLL内部内存)
	VISIONCTRL_API int Vision_UninitDevice();

#ifdef __cplusplus
}
#endif
