#pragma once
#include <QObject>
#include <QTimer>
#include <QMutex>

class ShellModule : public QObject
{
    Q_OBJECT
public:
	// 获取单例实例
	static ShellModule& instance();

	// 禁止赋值和拷贝
	ShellModule(const ShellModule&) = delete;
	ShellModule& operator=(const ShellModule&) = delete;

	// 禁用移动构造和移动赋值
	ShellModule(ShellModule&&) = delete;
	ShellModule& operator=(ShellModule&&) = delete;

	//初始化核心组件
	int ShellModule_Init();

	// 开启运行逻辑
	void StartShellModule();

	// 停止运行逻辑
	void StopShellModule();

	// 卸载/清理资源 (安全释放DLL内部内存)
	int ShellModule_Uninit();

private:
	// 构造函数和析构函数私有化
	explicit ShellModule();
	~ShellModule();

	QTimer* m_shellTimer = nullptr;					// 这个是进料口的定时器
	QTimer* m_shellTimer2 = nullptr;					// 这个是平台上的定时器		

	mutable QMutex m_mutex;							// 多线程互斥锁 (mutable允许在const函数中加锁)

	bool m_isShell = false;							// 标记是否有有工件 					

private slots:
	void onShellTimer();
	void onShellTimer2();
};


