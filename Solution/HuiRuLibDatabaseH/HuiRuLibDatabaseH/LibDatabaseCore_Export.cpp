#include "../../../Include/LibDatabaseCore_Export.h"

#include <QCoreApplication>
#include <QTranslator>
#include <QDebug>

#include "LibDatabaseCore.h"

#include <cstring> // 用于 memset

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
            return LIBDATABASE_RESULT_ERR_QT_ENV;
        }

        // 仅在需要中文时加载
        if (g_LanguageFlag == LanguageFlag::Chinese)
        {
            g_translator = new QTranslator(app);
            if (g_translator->load(":/Translator/m_zh_CN.qm"))
            {
                app->installTranslator(g_translator);
                qDebug() << "[LibDatabaseCore API] 翻译器安装成功";
            }
            else
            {
                delete g_translator;
                g_translator = nullptr;
                qDebug() << "[LibDatabaseCore API] 翻译器加载失败，请检查路径";
            }
        }
        return LIBDATABASE_RESULT_OK;
    }

    // 智能获取或创建App实例
    QCoreApplication* EnsureApp()
    {
        // 获取宿主程序已经创建的实例
        QCoreApplication* coreApp = QCoreApplication::instance();

        if (coreApp)
        {
            qDebug() << "[LibDatabaseCore API] 检测到宿主Qt环境，复用现有QApplication";
        }
        else
        {
            // 宿主非Qt程序，DLL内部接管创建
            qDebug() << "[LibDatabaseCore API] 未检测到宿主Qt环境，DLL内部创建QApplication";

            // 避免重复创建
            if (!g_qtApp)
            {
                static int argc = 1;
                static char appName[] = "LibDatabaseCore_Export.exe";
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
    LIBDATABASE_API int LibDatabase_InitDevice()
    {
        if (g_initState == InitState::Initialized || g_initState == InitState::Initializing)
        {
            qDebug() << "[LibDatabaseCore API] 设备已初始化，跳过重复初始化";
            return LIBDATABASE_RESULT_OK;
        }

        // QApplication 实例
        QCoreApplication* app = EnsureApp();
        if (!app)
        {
            qDebug() << "[LibDatabaseCore API] 致命错误: 无法初始化设备，QApplication 实例为空";
            return LIBDATABASE_RESULT_ERR_QT_ENV;
        }

        // 翻译家
        EnsureCoreTranslator(app);

        // 更新初始化状态
        g_initState = InitState::Initialized;

        // 初始化核心URL组件，将拼接好的完整 URL 传入
        LibDatabaseCore::instance().LibDatabaseCore_Init();

        qDebug() << "[LibDatabaseCore API] 设备初始化完成，QApplication 实例已就绪";

        return LIBDATABASE_RESULT_OK;
    }

    // 增加元件库数据
    LIBDATABASE_API int LibDatabase_InsertLibraryItem(const LibraryItem* item)
    {
        // 安全检查：防止外部传入空指针导致崩溃
        if (!item)
        {
            return LIBDATABASE_RESULT_ERR_INVALID_PARAM;
        }

        // 注意：这里传入 *item（解引用），将其转换为 C++ 核心层需要的引用类型
        return LibDatabaseCore::instance().LibDatabaseCoreInsertLibraryItem(*item);
    }

    // 删除元件库数据
    LIBDATABASE_API int LibDatabase_DeleteLibraryItem(const char* libraryName)
    {
        if (!libraryName)
        {
            return LIBDATABASE_RESULT_ERR_INVALID_PARAM;
        }

        // 将 C 风格字符串转换为 QString 传给内部核心
        return LibDatabaseCore::instance().LibDatabaseCoreDeleteLibraryItem(QString::fromLocal8Bit(libraryName));
    }

    // 修改元件库数据
    LIBDATABASE_API int LibDatabase_UpdateLibraryItem(const LibraryItem* item)
    {
        if (!item)
        {
            return LIBDATABASE_RESULT_ERR_INVALID_PARAM;
        }

        return LibDatabaseCore::instance().LibDatabaseCoreUpdateLibraryItem(*item);
    }

    // 查询元件库数据
    LIBDATABASE_API int LibDatabase_QueryLibraryItem(LibraryItem* item, const char* libraryName)
    {
        // 安全检查：查询时，item 指针和 libraryName 都不能为空
        if (!item || !libraryName)
        {
            return LIBDATABASE_RESULT_ERR_INVALID_PARAM;
        }

        // 内部核心查询完成后，会将数据写入 *item 指向的内存中
        return LibDatabaseCore::instance().LibDatabaseCoreQueryLibraryItem(*item, QString::fromLocal8Bit(libraryName));
    }

    LIBDATABASE_API int LibDatabase_OnInitDataStruct(LibraryItem* data)
    {
        // 将 name 数组的 128 个字节全部置为 0
        memset(data->name, 0, sizeof(data->name));

        // 【核心逻辑】：将所有物理尺寸、高度、速度等数值型参数全部归零
        data->width = 0.0f;
        data->height = 0.0f;
        data->length = 0.0f;
        data->pickZ = 0.0f;
        data->placeZ = 0.0f;
        data->speed = 0.0f;

        // 【核心逻辑】：吸嘴编号归 1（工业安全默认值，防止调用 0 号不存在的吸嘴）
        data->nozzleId = 1;

        qDebug() << "[LibDatabaseCore] LibraryItem 数据结构重置/初始化成功";
        return 0;
    }

    LIBDATABASE_API int LibDatabase_UninitDevice()
    {
        // 防止重复卸载
        if (g_initState != InitState::Initialized)
        {
            qDebug() << "[LibDatabaseCore API] 设备未初始化或已卸载，跳过重复卸载";

            return LIBDATABASE_RESULT_ERR_NOT_INIT;
        }

        qDebug() << "[LibDatabaseCore API] 开始清理设备与Qt资源...";

        // 卸载核心组件资源
        LibDatabaseCore::instance().LibDatabaseCore_Uninit();

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
            qDebug() << "[LibDatabaseCore API] 翻译器已卸载并释放";
        }

        // 当DLL自己创建的App时
        if (g_qtApp)
        {
            qDebug() << "[LibDatabaseCore API] 检测到DLL内部创建的QApplication，正在安全销毁...";

            // 处理事件队列中残留的事件，防止析构时的警告或崩溃
            //QCoreApplication::processEvents();

            delete g_qtApp;
            g_qtApp = nullptr;
        }
        else
        {
            qDebug() << "[LibDatabaseCore API] 宿主程序提供的QApplication，DLL跳过销毁操作";
        }

        // 重置初始化状态
        g_initState = InitState::Uninitialized;

        qDebug() << "[LibDatabaseCore API] 设备清理完成";


        return LIBDATABASE_RESULT_OK;
    }


}

