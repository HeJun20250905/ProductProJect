#pragma once
#include <QMutex>
#include <QString>

struct LibraryItem;
class QSqlQuery;

class LibDatabaseCore
{
public:
	// 获取单例实例
	static LibDatabaseCore& instance();

	// 禁止赋值和拷贝
	LibDatabaseCore(const LibDatabaseCore&) = delete;
	LibDatabaseCore& operator=(const LibDatabaseCore&) = delete;

	// 禁用移动构造和移动赋值
	LibDatabaseCore(LibDatabaseCore&&) = delete;
	LibDatabaseCore& operator=(LibDatabaseCore&&) = delete;

	//初始化核心组件
	int LibDatabaseCore_Init();

	// 增加元件库数据
	int LibDatabaseCoreInsertLibraryItem(const LibraryItem& item);

	// 删除元件库数据
	int LibDatabaseCoreDeleteLibraryItem(const QString& libraryName);

	// 更新元件库数据
	int LibDatabaseCoreUpdateLibraryItem(const LibraryItem& item);

	// 查询元件库数据
	int LibDatabaseCoreQueryLibraryItem(LibraryItem& item, const QString& libraryName);

	// 卸载/清理资源 (安全释放DLL内部内存)
	int LibDatabaseCore_Uninit();

private:
	// 构造函数和析构函数私有化
	explicit LibDatabaseCore();
	~LibDatabaseCore();

	mutable QMutex m_mutex;               // 【关键】多线程互斥锁 (mutable允许在const函数中加锁)

	// 将高频 SQL 语句定义为常量，避免每次调用都创建 QString 对象
	static const QString SQL_INSERT;
	static const QString SQL_UPDATE;
	static const QString SQL_DELETE;
	static const QString SQL_QUERY;

	// 预编译的 Query 对象（作为成员变量）
	QSqlQuery* m_insertQuery = nullptr;
	QSqlQuery* m_updateQuery = nullptr;
	QSqlQuery* m_deleteQuery = nullptr;
	QSqlQuery* m_selectQuery = nullptr;
};

