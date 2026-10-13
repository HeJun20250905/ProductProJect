#pragma once

#ifdef MOTIONCTRL_EXPORTS
#define MOTIONCTRL_API __declspec(dllexport)
#else
#define MOTIONCTRL_API __declspec(dllimport)
#endif

// 轴定义宏（X Y Z R H1 H2）
#define AXIS_X  0
#define AXIS_Y  1
#define AXIS_Z  2
#define AXIS_R  3
#define AXIS_H1 4
#define AXIS_H2 5

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

    // 初始化轴参数
    MOTIONCTRL_API int Motion_InitAxis(int axis, double unit, double speed, double accel, double decel);

    // 单轴使能
    MOTIONCTRL_API int Motion_Enable(int axis, int enableState);

    // 单轴回零
    MOTIONCTRL_API int Motion_Home(int axis, float speed, float creep, float accel, float decel);

    // 单轴定位
    MOTIONCTRL_API int Motion_MoveAbsSingle(int axis, float position);
    MOTIONCTRL_API int Motion_MoveRelSingle(int axis, float distance);
    
    // 多轴定位
    MOTIONCTRL_API int Motion_MoveAbsMulti(int axisCount, const int* axes, const float* targets);
    MOTIONCTRL_API int Motion_MoveRelMulti(int axisCount, const int* axes, const float* distances);

    // 单轴连续运动
    MOTIONCTRL_API int Motion_VMoveSingle(int axis, int direction, float speed);

    // 单轴停止运动
    MOTIONCTRL_API int Motion_StopSingle(int axis, int mode = 1);

    // 状态获取
    MOTIONCTRL_API int Motion_GetDpos(int axis, float* outPosition);
    MOTIONCTRL_API int Motion_GetAxisStatus(int axis, int* outStatus);

    // 输入IO状态获取
    MOTIONCTRL_API int Motion_GetInput(int ionum);

    // 卸载/清理资源 (安全释放DLL内部内存)
    MOTIONCTRL_API int Motion_UninitDevice();

#ifdef __cplusplus
}
#endif
