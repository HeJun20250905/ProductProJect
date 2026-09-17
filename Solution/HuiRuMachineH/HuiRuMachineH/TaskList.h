#pragma once
#include <QQueue>
#include <QMutex>

// 单个贴片任务结构体（独立文件，方便全工程复用）
struct PickPlace
{
	QString PickName;				// 元件位号 (如 "R1")
	QString LibraryName;			// 元件型号 (用于去全局元件库查数据)
	float PickX;					// X 坐标
	float PickY;					// Y 坐标
    float PickZ;					// Z 坐标
	float PickR;					// 贴装角度R
};

class TaskList
{
public:
	// 获取单例实例
	static TaskList& instance();

	// 禁止赋值和拷贝
	TaskList(const TaskList&) = delete;
	TaskList& operator=(const TaskList&) = delete;

	// 禁用移动构造和移动赋值
	TaskList(TaskList&&) = delete;
	TaskList& operator=(TaskList&&) = delete;

	//初始化核心组件
	int TaskList_Init(QString& outFilePath);

	// 添加任务
	int TaskList_AddTask(PickPlace task);

	// 获取任务
    int TaskList_GetTask(PickPlace* task);

	// 卸载/清理资源 (安全释放DLL内部内存)
	int TaskList_Uninit();

    // 获取任务队列
	const QQueue<PickPlace>& TaskList_GetQueue() const;

private:
	// 构造函数和析构函数私有化
	explicit TaskList();
	~TaskList();

	QQueue<PickPlace> m_gTaskQueue;

	mutable QMutex m_mutex;               // 【关键】多线程互斥锁 (mutable允许在const函数中加锁)
};

