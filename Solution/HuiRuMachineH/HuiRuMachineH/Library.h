#pragma once
#include <QQueue>
#include <QMutex>

struct LibraryItem
{
	// 核心标识（必须有）
	QString name;					// 元件型号 (如 "0402_10K", "SOP8")，作为全局元件库 QHash 的 Key

	// 物理尺寸
	float length;					// 元件长度 (mm)
	float width;					// 元件宽度 (mm)
	float height;					// 元件高度/厚度 (mm)

	// 贴装工艺参数
	float pickZ;					// 取料高度 (mm)：吸嘴下探到料带/托盘吸取的深度
	float placeZ;					// 贴装高度 (mm)：吸嘴下压到 PCB 板上的贴装深度
	int nozzleId;					// 吸嘴编号 (如 1, 2, 3)：告诉机器换哪个吸嘴来吸取这个元件
	float speed;					// 贴装速度：不同大小、重量的元件，下压的速度要求不同
};

class Library
{
public:
	// 获取单例实例
	static Library& instance();

	// 禁止赋值和拷贝
	Library(const Library&) = delete;
	Library& operator=(const Library&) = delete;

	// 禁用移动构造和移动赋值
	Library(Library&&) = delete;
	Library& operator=(Library&&) = delete;

	//初始化核心组件
	int Library_Init();

	// 卸载/清理资源 (安全释放DLL内部内存)
	int Library_Uninit();

private:
	// 构造函数和析构函数私有化
	explicit Library();
	~Library();

	QQueue<LibraryItem> m_library;
	mutable QMutex m_mutex;               // 【关键】多线程互斥锁 (mutable允许在const函数中加锁)
};

