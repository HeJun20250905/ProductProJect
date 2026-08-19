#include "../../../Include/VisionCore_Export.h"

#include <QCoreApplication>
#include <QTranslator>
#include <QDebug>

#include "VisionCore.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

namespace
{
    // 语言标记
    enum class LanguageFlag
    {
        English = 0,           // 英文
        Chinese = 2            // 中文
    };

    // 初始化状态
    enum class InitState
    {
        Uninitialized,          // 未初始化
        Initializing,           // 初始化中
        Initialized             // 已初始化
    };

    // 语言状态标记
    static LanguageFlag g_LanguageFlag = LanguageFlag::English;

    // 初始化状态标记
    static InitState g_initState = InitState::Uninitialized;

    // Qt实例对象
    static QCoreApplication* g_qtApp = nullptr;

    // Qt翻译器对象
    static QTranslator* g_translator = nullptr;

    // 翻译器加载
    int EnsureCoreTranslator(QCoreApplication* app)
    {
        if (!app || g_translator)
        {
            return VISION_RESULT_ERR_QT_ENV;
        }

        // 仅在需要中文时加载
        if (g_LanguageFlag == LanguageFlag::Chinese)
        {
            g_translator = new QTranslator(app);
            if (g_translator->load(":/Translator/m_zh_CN.qm"))
            {
                app->installTranslator(g_translator);
                qDebug() << "[VisionCore API] 翻译器安装成功";
            }
            else
            {
                delete g_translator;
                g_translator = nullptr;
                qDebug() << "[VisionCore API] 翻译器加载失败，请检查路径";
            }
        }
        return VISION_RESULT_OK;
    }

    // 智能获取或创建App实例
    QCoreApplication* EnsureApp()
    {
        // 获取宿主程序已经创建的实例
        QCoreApplication* coreApp = QCoreApplication::instance();

        if (coreApp)
        {
            qDebug() << "[VisionCore API] 检测到宿主Qt环境，复用现有QApplication";
        }
        else
        {
            // 宿主非Qt程序，DLL内部接管创建
            qDebug() << "[VisionCore API] 未检测到宿主Qt环境，DLL内部创建QApplication";

            // 避免重复创建
            if (!g_qtApp)
            {
                static int argc = 1;
                static char appName[] = "VisionCore.exe";
                static char* argv[] = { appName, nullptr };

                g_qtApp = new QCoreApplication(argc, argv);
            }
            coreApp = g_qtApp;
        }

        return coreApp;
    }
}

extern "C"
{
    VISIONCTRL_API int Vision_InitDevice(void* hWndDisplay)
    {
        if (g_initState == InitState::Initialized || g_initState == InitState::Initializing)
        {
            qDebug() << "[VisionCore API] 设备已初始化，跳过重复初始化";
            return VISION_RESULT_OK;
        }

        // QApplication 实例
        QCoreApplication* app = EnsureApp();
        if (!app)
        {
            qDebug() << "[VisionCore API] 致命错误: 无法初始化设备，QApplication 实例为空";
            return VISION_RESULT_ERR_QT_ENV;
        }

        // 检测显示窗口句柄
        if (hWndDisplay == nullptr)
        {
            qDebug() << "[VisionCore API] 致命错误: 传入的显示窗口句柄(hWndDisplay)为空";
            return VISION_RESULT_ERR_UNKNOWN;
        }

        // 翻译家
        EnsureCoreTranslator(app);

        // 更新初始化状态
        g_initState = InitState::Initialized;

        // 获取显示窗口句柄
        HWND hDisplay = static_cast<HWND>(hWndDisplay);

        // 初始化核心URL组件，将拼接好的完整 URL 传入
        VisionCore::instance().VisionCore_Init(hDisplay);

        qDebug() << "[VisionCore API] 设备初始化完成，QApplication 实例已就绪";

        return VISION_RESULT_OK;
    }

    VISIONCTRL_API int Vision_StartGrabbing()
    {
        if (VisionCore::instance().VisionCore_Start() != 0)
        {
            qDebug() << "[VisionCore API] 设备启动失败";
            return VISION_RESULT_ERR_UNKNOWN;
        }
        return VISION_RESULT_OK;
    }

    VISIONCTRL_API int Vision_StopGrabbing()
    {
        if (VisionCore::instance().VisionCore_Stop() != 0)
        {
            qDebug() << "[VisionCore API] 设备停止失败";
            return VISION_RESULT_ERR_UNKNOWN;
        }
        return VISION_RESULT_OK;
    }

    VISIONCTRL_API int Vision_UninitDevice()
    {
        // 防止重复卸载
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] 设备未初始化或已卸载，跳过重复卸载";

            return VISION_RESULT_ERR_NOT_INIT;
        }

        qDebug() << "[VisionCore API] 开始清理设备与Qt资源...";

        // 卸载核心组件资源
        VisionCore::instance().VisionCore_Uninit();

        // 安全卸载翻译器
        if (g_translator)
        {
            QCoreApplication* app = QCoreApplication::instance();
            if (app)
            {
                app->removeTranslator(g_translator);
            }
            delete g_translator;
            g_translator = nullptr;
            qDebug() << "[VisionCore API] 翻译器已卸载并释放";
        }

        // 当DLL自己创建的App时
        if (g_qtApp)
        {
            qDebug() << "[VisionCore API] 检测到DLL内部创建的QApplication，正在安全销毁...";

            // 处理事件队列中残留的事件，防止析构时的警告或崩溃
            //QCoreApplication::processEvents();

            delete g_qtApp;
            g_qtApp = nullptr;
        }
        else
        {
            qDebug() << "[VisionCore API] 宿主程序提供的QApplication，DLL跳过销毁操作";
        }

        // 重置初始化状态
        g_initState = InitState::Uninitialized;

        qDebug() << "[VisionCore API] 设备清理完成";


        return VISION_RESULT_OK;
    }

    // 注册回调
    VISIONCTRL_API int Vision_RegisterImageCallback(VisionImageCallback callback, void* pUser)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] 设备未初始化，无法注册回调";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (callback == nullptr)
        {
            return VISION_RESULT_ERR_INVALID_PARAM;
        }

        VisionCore::instance().VisionCore_SetImageCallback(callback, pUser);
        return VISION_RESULT_OK;
    }

    // 注销回调
    VISIONCTRL_API int Vision_UnregisterImageCallback()
    {
        VisionCore::instance().VisionCore_ClearImageCallback();
        return VISION_RESULT_OK;
    }

    VISIONCTRL_API int Vision_SetExposureTime(float exposureUs)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetExposureTime 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        
        if (VisionCore::instance().VisionCore_SetExposureTime(exposureUs) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetExposureTime 设置曝光成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetExposureTime 曝光设置失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetExposureTime(float* exposureUs)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetExposureTime 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!exposureUs)
        {
            qDebug() << "[VisionCore API] GetExposureTime 传入的参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }
        
        if (VisionCore::instance().VisionCore_GetExposureTime(*exposureUs) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetExposureTime 获取曝光成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetExposureTime 获取曝光失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetExposureAuto(int mode)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetExposureAuto 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if(VisionCore::instance().VisionCore_SetExposureAuto(mode) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetExposureAuto 设置自动曝光成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetExposureAuto 曝光自动设置失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetExposureAuto(int* mode)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetExposureAuto 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!mode)
        {
            qDebug() << "[VisionCore API] GetExposureAuto 获取参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }
        
        if (VisionCore::instance().VisionCore_GetExposureAuto(*mode) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetExposureAuto 获取自动曝光成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetExposureAuto 获取自动曝光失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetGain(float gainDb)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetGain 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (VisionCore::instance().VisionCore_SetGain(gainDb) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetGain 设置增益成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetGain 增益设置失败";
        return  VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetGain(float* gainDb)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetGain 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!gainDb)
        {
            qDebug() << "[VisionCore API] GetGain 获取参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }
        
        if (VisionCore::instance().VisionCore_GetGain(*gainDb) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetGain 获取增益成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetGain 获取增益失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetGainAuto(int mode)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetGainAuto 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (VisionCore::instance().VisionCore_SetGainAuto(mode) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetGainAuto 获取增益成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetGainAuto 增益自动设置失败";
        return  VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetGainAuto(int* mode)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetGainAuto 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!mode)
        {
            qDebug() << "[VisionCore API] GetGainAuto 获取参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }

        if (VisionCore::instance().VisionCore_GetGainAuto(*mode) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetGainAuto 获取增益成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetGainAuto 获取增益失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetBrightness(int brightness)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetBrightness 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (VisionCore::instance().VisionCore_SetBrightness(brightness) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetBrightness 设置亮度成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetBrightness 亮度设置失败";
        return  VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetBrightness(int* brightness)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetBrightness 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!brightness)
        {
            qDebug() << "[VisionCore API] GetBrightness 获取参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }
        
        if (VisionCore::instance().VisionCore_GetBrightness(*brightness) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetBrightness 获取亮度成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetBrightness 获取亮度失败";
        return  VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetContrast(int contrast)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetContrast 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (VisionCore::instance().VisionCore_SetContrast(contrast) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetContrast 设置对比度成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetContrast 对比度设置失败";
        return  VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetContrast(int* contrast)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetContrast 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!contrast)
        {
            qDebug() << "[VisionCore API] GetContrast 获取参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }
        
        if (VisionCore::instance().VisionCore_GetContrast(*contrast) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetContrast 获取对比度成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetContrast 获取对比度失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetGammaEnable(int enable)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetGammaEnable 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (VisionCore::instance().VisionCore_SetGammaEnable(enable != 0) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetGammaEnable 设置Gamma成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetGammaEnable Gamma设置失败";
        return  VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetGammaEnable(int* enable)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetGammaEnable 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!enable)
        {
            qDebug() << "[VisionCore API] GetGammaEnable 获取参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }
        
        bool bEnable = false;
        int ret = VisionCore::instance().VisionCore_GetGammaEnable(bEnable);
        if (bEnable) 
            *enable = 1;
        else 
            *enable = 0;

        if (ret == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetGammaEnable 获取Gamma成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetGammaEnable 获取Gamma失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetGamma(float gamma)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetGamma 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (VisionCore::instance().VisionCore_SetGamma(gamma) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetGamma 设置Gamma成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetGamma 设置Gamma失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetGamma(float* gamma)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetGamma 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!gamma)
        {
            qDebug() << "[VisionCore API] GetGamma 获取参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }
        
        if (VisionCore::instance().VisionCore_GetGamma(*gamma) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetGamma 获取Gamma成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetGamma 获取Gamma失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetWhiteBalanceAuto(int mode)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetWhiteBalanceAuto 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (VisionCore::instance().VisionCore_SetWhiteBalanceAuto(mode) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetWhiteBalanceAuto 设置白平衡成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetWhiteBalanceAuto 白平衡设置失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetWhiteBalanceAuto(int* mode)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetWhiteBalanceAuto 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!mode)
        {
            qDebug() << "[VisionCore API] GetWhiteBalanceAuto 获取参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }
        
        if (VisionCore::instance().VisionCore_GetWhiteBalanceAuto(*mode) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetWhiteBalanceAuto 获取白平衡成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetWhiteBalanceAuto 获取白平衡失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetAcquisitionFrameRate(float frameRate)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetAcquisitionFrameRate 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (VisionCore::instance().VisionCore_SetAcquisitionFrameRate(frameRate) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetAcquisitionFrameRate 设置帧率成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetAcquisitionFrameRate 帧率设置失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetAcquisitionFrameRate(float* frameRate)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetAcquisitionFrameRate 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!frameRate)
        {
            qDebug() << "[VisionCore API] GetAcquisitionFrameRate 获取参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }
        
        if (VisionCore::instance().VisionCore_GetAcquisitionFrameRate(*frameRate) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetAcquisitionFrameRate 获取帧率成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetAcquisitionFrameRate 获取帧率失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetPixelFormat(unsigned int pixelFormat)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetPixelFormat 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (VisionCore::instance().VisionCore_SetPixelFormat(pixelFormat) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetPixelFormat 设置像素格式成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetPixelFormat 设置像素格式失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetPixelFormat(unsigned int* pixelFormat)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetPixelFormat 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        // 判断指针本身是否为空
        if (!pixelFormat)
        {
            qDebug() << "[VisionCore API] GetPixelFormat 参数错误";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }

        // 将解引用后的值传给内部函数
        if (VisionCore::instance().VisionCore_GetPixelFormat(*pixelFormat) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetPixelFormat 获取像素格式成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetPixelFormat 获取像素格式失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetTriggerMode(int mode)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetTriggerMode 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (VisionCore::instance().VisionCore_SetTriggerMode(mode) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetTriggerMode 设置触发模式成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetTriggerMode 触发模式设置失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetTriggerMode(int* mode)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetTriggerMode 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!mode)
        {
            qDebug() << "[VisionCore API] GetTriggerMode 获取参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }
        
        if (VisionCore::instance().VisionCore_GetTriggerMode(*mode) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetTriggerMode 获取触发模式成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetTriggerMode 获取触发模式失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_SetTriggerSource(int* sourceValue)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] SetTriggerSource 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }

        if (!sourceValue)
        {
            qDebug() << "[VisionCore API] SetTriggerSource 获取参数为空";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }

        if (VisionCore::instance().VisionCore_SetTriggerSource(*sourceValue) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] SetTriggerSource 设置触发源成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] SetTriggerSource 触发源设置失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }

    VISIONCTRL_API int Vision_GetTriggerSource(int* sourceValue)
    {
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[VisionCore API] GetTriggerSource 设备未初始化，请先初始化设备";
            return VISION_RESULT_ERR_NOT_INIT;
        }
        if (!sourceValue)
        {
            qDebug() << "[VisionCore API] GetTriggerSource 参数错误";
            return VISION_RESULT_ERR_INVALID_PARAM;
        }
        
        if (VisionCore::instance().VisionCore_GetTriggerSource(*sourceValue) == VISION_RESULT_OK)
        {
            qDebug() << "[VisionCore API] GetTriggerSource 获取触发源成功";
            return VISION_RESULT_OK;
        }

        qDebug() << "[VisionCore API] GetTriggerSource 获取触发源失败";
        return VISION_RESULT_ERR_UNKNOWN;
    }
}