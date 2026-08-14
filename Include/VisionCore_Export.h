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

	VISIONCTRL_API int Vision_InitDevice(void* hWndDisplay);                        // 初始化设备引擎
    VISIONCTRL_API int Vision_StartGrabbing();                                      // 开始抓图
    VISIONCTRL_API int Vision_StopGrabbing();                                       // 停止抓图
	VISIONCTRL_API int Vision_UninitDevice();                                       // 卸载/清理资源 (安全释放DLL内部内存)

    VISIONCTRL_API int Vision_SetExposureTime(float exposureUs);                    // 设置曝光时间
    VISIONCTRL_API int Vision_GetExposureTime(float* exposureUs);                   // 获取曝光时间
    VISIONCTRL_API int Vision_SetExposureAuto(int mode);                            // 设置曝光自动模式
    VISIONCTRL_API int Vision_GetExposureAuto(int* mode);                           // 获取曝光自动模式

    VISIONCTRL_API int Vision_SetGain(float gainDb);                                // 设置增益
    VISIONCTRL_API int Vision_GetGain(float* gainDb);                               // 获取增益
    VISIONCTRL_API int Vision_SetGainAuto(int mode);                                // 设置增益自动模式
    VISIONCTRL_API int Vision_GetGainAuto(int* mode);                               // 获取增益自动模式

    VISIONCTRL_API int Vision_SetBrightness(int brightness);                        // 设置亮度
    VISIONCTRL_API int Vision_GetBrightness(int* brightness);                       // 获取亮度
    VISIONCTRL_API int Vision_SetContrast(int contrast);                            // 设置对比度
    VISIONCTRL_API int Vision_GetContrast(int* contrast);                           // 获取对比度

    VISIONCTRL_API int Vision_SetGammaEnable(int enable);                           // 设置Gamma使能
    VISIONCTRL_API int Vision_GetGammaEnable(int* enable);                          // 获取Gamma使能
    VISIONCTRL_API int Vision_SetGamma(float gamma);                                // 设置Gamma
    VISIONCTRL_API int Vision_GetGamma(float* gamma);                               // 获取Gamma

    VISIONCTRL_API int Vision_SetWhiteBalanceAuto(int mode);                        // 设置白平衡自动模式
    VISIONCTRL_API int Vision_GetWhiteBalanceAuto(int* mode);                       // 获取白平衡自动模式

    VISIONCTRL_API int Vision_SetAcquisitionFrameRate(float frameRate);             // 设置帧率
    VISIONCTRL_API int Vision_GetAcquisitionFrameRate(float* frameRate);            // 获取帧率
    VISIONCTRL_API int Vision_SetPixelFormat(unsigned int pixelFormat);             // 设置像素格式
    VISIONCTRL_API int Vision_GetPixelFormat(unsigned int* pixelFormat);            // 获取像素格式

    VISIONCTRL_API int Vision_SetTriggerMode(int mode);                             // 设置触发模式
    VISIONCTRL_API int Vision_GetTriggerMode(int* mode);                            // 获取触发模式
    VISIONCTRL_API int Vision_SetTriggerSource(int* sourceValue);                   // 设置触发源
    VISIONCTRL_API int Vision_GetTriggerSource(int* sourceValue);                   // 获取触发源

#ifdef __cplusplus
}
#endif
