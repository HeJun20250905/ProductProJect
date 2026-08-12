#pragma once
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>

class LoggerH : public QObject
{
	Q_OBJECT
public:
	// 获取单例实例
	static LoggerH& instance();

	// 禁止赋值和拷贝
	LoggerH(const LoggerH&) = delete;
	LoggerH& operator=(const LoggerH&) = delete;

	// 禁用移动构造和移动赋值
	LoggerH(LoggerH&&) = delete;
	LoggerH& operator=(LoggerH&&) = delete;

	// 初始化日志文件
	void Init(const QString& logPath = "logs/app.log");

	// 核心写入函数
	void WriteLog(QtMsgType type, const QString& msg);

private:
	// 构造函数和析构函数私有化
	explicit LoggerH();
	~LoggerH();

	// 成员变量
	QFile m_file;
	QTextStream m_stream;
	mutable QMutex m_mutex;
};

