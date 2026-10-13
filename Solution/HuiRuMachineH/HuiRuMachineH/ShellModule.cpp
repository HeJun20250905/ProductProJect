#include "ShellModule.h"
#include <QDebug>

// Windows API
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include "../../../Include/MotionCore_Export.h"

#ifdef _DEBUG
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Debug"
#else
#define  LIB_PATH     "..\\..\\..\\Library\\Win32\\Release"
#endif

#pragma comment(lib,  LIB_PATH"\\MotionCore.lib")

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

ShellModule& ShellModule::instance()
{
    // 全局单例对象
    static ShellModule instance;

    return instance;
}

int ShellModule::ShellModule_Init()
{
    return 0;
}

void ShellModule::StartShellModule()
{
    // 上升第一部分


    QTimer::singleShot(3000, this, [this]()
    {
        // 上升第二部分


        QTimer::singleShot(2000, this, [this]()
        {
            // 下降第一部分

            // 这里再开启第二个定时器检查另一个感应器状态
            m_shellTimer2->start(1000);
        });

    });


}

void ShellModule::StopShellModule()
{
    // 当检测到没有工件了,就下降第二部分,同时开启第一个定时器
    int result = Motion_GetInput(0);
    if (result == 0)
    {
        // Motion_MoveTo(m_zAxisPosition_Safe, 0.0, 0.0);
        m_shellTimer2->stop();

        m_shellTimer->start(1000);
    }
}


int ShellModule::ShellModule_Uninit()
{
    qDebug() << "[ShellModule] 开始安全停止连接...";

    return 0;
}

ShellModule::ShellModule()
{
    qDebug() << "[ShellModule] 开始初始化";

    m_shellTimer = new QTimer(this);
    connect(m_shellTimer, &QTimer::timeout, this, &ShellModule::onShellTimer);
    m_shellTimer->start(1000);

    m_shellTimer2 = new QTimer(this);
    connect(m_shellTimer2, &QTimer::timeout, this, &ShellModule::onShellTimer2);
}

ShellModule::~ShellModule()
{
    ShellModule_Uninit();
}

void ShellModule::onShellTimer2()
{
    this->StopShellModule();
}

void ShellModule::onShellTimer()
{
    // qDebug() << "[ShellModule] 检测到Shell定时器触发";

    int result = Motion_GetInput(0);
    if (result == 1)
    {
        qDebug() << "[ShellModule] 检测到Shell触发";

        m_shellTimer->stop();

        QTimer::singleShot(2000, this, [this]() 
        {
            StartShellModule();
        });
    }
}