#include "../../../Include/MotionCore_Export.h"
#include <QCoreApplication>
#include <QTranslator>
#include <QDebug>

#include "MotionCore.h"

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
            return MOTION_RESULT_ERR_QT_ENV;
        }

        // 仅在需要中文时加载
        if (g_LanguageFlag == LanguageFlag::Chinese)
        {
            g_translator = new QTranslator(app);
            if (g_translator->load(":/Translator/m_zh_CN.qm"))
            {
                app->installTranslator(g_translator);
                qDebug() << "[MotionCore API] 翻译器安装成功";
            }
            else
            {
                delete g_translator;
                g_translator = nullptr;
                qDebug() << "[MotionCore API] 翻译器加载失败，请检查路径";
            }
        }
        return MOTION_RESULT_OK;
    }

    // 智能获取或创建App实例
    QCoreApplication* EnsureApp()
    {
        // 获取宿主程序已经创建的实例
        QCoreApplication* coreApp = QCoreApplication::instance();

        if (coreApp)
        {
            qDebug() << "[MotionCore API] 检测到宿主Qt环境，复用现有QApplication";
        }
        else
        {
            // 宿主非Qt程序，DLL内部接管创建
            qDebug() << "[MotionCore API] 未检测到宿主Qt环境，DLL内部创建QApplication";

            // 避免重复创建
            if (!g_qtApp)
            {
                static int argc = 1;
                static char appName[] = "MotionCore_Export.exe";
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
	MOTIONCTRL_API int Motion_InitDevice(const char* ipAddress)
	{
        if (g_initState == InitState::Initialized || g_initState == InitState::Initializing)
        {
            qDebug() << "[MotionCore API] 设备已初始化，跳过重复初始化";
            return MOTION_RESULT_OK;
        }

        if (ipAddress == NULL)
        {
            qDebug() << "[MotionCore API] 无效参数: IP 不能为空";
            return MOTION_RESULT_ERR_INVALID_PARAM;
        }

        // QApplication 实例
        QCoreApplication* app = EnsureApp();
        if (!app)
        {
            qDebug() << "[MotionCore API] 致命错误: 无法初始化设备，QApplication 实例为空";
            return MOTION_RESULT_ERR_QT_ENV;
        }

        // 翻译家
        EnsureCoreTranslator(app);

        // 更新初始化状态
        g_initState = InitState::Initialized;

        // 拼接ip字符串
        QString ipStr = QString(ipAddress);

        // 初始化核心URL组件，将拼接好的完整 URL 传入
        MotionCore::instance().MotionCore_Init(ipStr);

        qDebug() << "[MotionCore API] 设备初始化完成，QApplication 实例已就绪";

		return MOTION_RESULT_OK;
	}

    MOTIONCTRL_API int Motion_InitAxis(int axis)
    {
        if (axis < AXIS_X || axis > AXIS_R)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;
        }

        // 调用核心单例的方法
        return MotionCore::instance().MotionCore_InitAxis(axis);
    }

    MOTIONCTRL_API int Motion_Enable(int axis, int enableState)
    {
        if (axis < AXIS_X || axis > AXIS_R)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;
        }

        return MotionCore::instance().MotionCore_SetEnable(axis, enableState);
    }

    MOTIONCTRL_API int Motion_MoveAbs(int axis, float position, float speed)
    {
        if (axis < AXIS_X || axis > AXIS_R) 
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;
        }

        // 补全：调用单例方法
        return MotionCore::instance().MotionCore_MoveAbs(axis, position, speed);
    }

    MOTIONCTRL_API int Motion_MoveRel(int axis, float distance, float speed)
    {
        if (axis < AXIS_X || axis > AXIS_R)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;
        }

        // 补全：调用单例方法
        return MotionCore::instance().MotionCore_MoveRel(axis, distance, speed);
    }

    MOTIONCTRL_API int Motion_MoveAbsXY(int x, int y, float targetX, float targetY, float speed)
    { 
        if (x < AXIS_X || x > AXIS_Y || y < AXIS_X || y > AXIS_Y)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;
        }
        if (x == y)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;  // 不能是同一个轴
        }
        if (speed <= 0.0)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;  // 速度必须大于0
        }
        return MotionCore::instance().MotionCore_MoveAbsXY(x, y, targetX, targetY, speed);
    }

    MOTIONCTRL_API int Motion_MoveRelXY(int x, int y, float distX, float distY, float speed)
    {
        if (x < AXIS_X || x > AXIS_Y || y < AXIS_X || y > AXIS_Y)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;
        }
        if (x == y)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;  // 不能是同一个轴
        }
        if (speed <= 0.0)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;  // 速度必须大于0
        }
        return MotionCore::instance().MotionCore_MoveRelXY(x, y, distX, distY, speed);
    }

    MOTIONCTRL_API int Motion_Stop(int axis)
    {
        if (axis < AXIS_X || axis > AXIS_R)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;
        }

        // 补全：调用单例方法
        return MotionCore::instance().MotionCore_Stop(axis);
    }

    MOTIONCTRL_API int Motion_GetDpos(int axis, float* outPosition)
    {
        if (axis < AXIS_X || axis > AXIS_R)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;
        }

        float tempPos = 0.0f;
        // 补全：调用单例方法并转换精度
        int ret = MotionCore::instance().MotionCore_GetDpos(axis, &tempPos);
        if (ret == 0) *outPosition = tempPos;
        return ret;
    }

    MOTIONCTRL_API int Motion_GetAxisStatus(int axis, int* outStatus)
    {
        if (axis < AXIS_X || axis > AXIS_R)
        {
            return MOTION_RESULT_ERR_INVALID_PARAM;
        }

        // 补全：调用单例方法
        return MotionCore::instance().MotionCore_GetAxisStatus(axis, outStatus);
    }

    MOTIONCTRL_API int Motion_UninitDevice()
    {
        // 防止重复卸载
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[MotionCore API] 设备未初始化或已卸载，跳过重复卸载";

            return MOTION_RESULT_ERR_NOT_INIT;
        }

        qDebug() << "[MotionCore API] 开始清理设备与Qt资源...";

        // 卸载核心组件资源
        MotionCore::instance().MotionCore_UninitDevice();

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
            qDebug() << "[MotionCore API] 翻译器已卸载并释放";
        }

        // 当DLL自己创建的App时
        if (g_qtApp)
        {
            qDebug() << "[MotionCore API] 检测到DLL内部创建的QApplication，正在安全销毁...";

            // 处理事件队列中残留的事件，防止析构时的警告或崩溃
            //QCoreApplication::processEvents();

            delete g_qtApp;
            g_qtApp = nullptr;
        }
        else
        {
            qDebug() << "[MotionCore API] 宿主程序提供的QApplication，DLL跳过销毁操作";
        }

        // 重置初始化状态
        g_initState = InitState::Uninitialized;

        qDebug() << "[MotionCore API] 设备清理完成";


        return MOTION_RESULT_OK;
    }
}

