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
}