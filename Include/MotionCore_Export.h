#pragma once

#ifdef MOTIONCTRL_EXPORTS
#define MOTIONCTRL_API __declspec(dllexport)
#else
#define MOTIONCTRL_API __declspec(dllimport)
#endif

// 轴定义宏（XYZR）
#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2
#define AXIS_R 3

// 返回状态码
typedef enum MOTION_RESULT_STATUS
{
	MOTION_RESULT_OK                   =   0,							// 成功
	MOTION_RESULT_ERR_NOT_INIT         =  -1,							// 未初始化或已卸载
	MOTION_RESULT_ERR_QT_ENV           =  -2,							// Qt环境创建/获取失败
	MOTION_RESULT_ERR_TRANS            =  -3,							// 翻译器加载失败
	MOTION_RESULT_ERR_INVALID_PARAM    =  -4,						    // 无效参数
	MOTION_RESULT_ERR_UNKNOWN          =  -99							// 未知错误
} MOTION_RESULT_STATUS;

#ifdef __cplusplus
extern "C"
{
#endif

    // //初始化设备引擎
    MOTIONCTRL_API int Motion_InitDevice(const char* ipAddress);

    // 【新增】初始化轴参数
    MOTIONCTRL_API int Motion_InitAxis(int axis);

    // 单轴使能
    MOTIONCTRL_API int Motion_Enable(int axis, int enableState);

    // 单轴定位与速度控制
    MOTIONCTRL_API int Motion_MoveAbs(int axis, float position, float speed);
    MOTIONCTRL_API int Motion_MoveRel(int axis, float distance, float speed);
    
    // 多轴定位与速度控制
    MOTIONCTRL_API int Motion_MoveAbsXY(int x, int y, float targetX, float targetY, float speed);
    MOTIONCTRL_API int Motion_MoveRelXY(int x, int y, float distX, float distY, float speed);

    // 单轴停止
    MOTIONCTRL_API int Motion_Stop(int axis);

    // 状态获取
    MOTIONCTRL_API int Motion_GetDpos(int axis, float* outPosition);
    MOTIONCTRL_API int Motion_GetAxisStatus(int axis, int* outStatus);

    // 卸载/清理资源 (安全释放DLL内部内存)
    MOTIONCTRL_API int Motion_UninitDevice();

#ifdef __cplusplus
}
#endif
