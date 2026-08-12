#include "HuiRuLoggerH.h"

#include <QFileInfo>
#include <QDir>
#include <QDateTime>

// UTF-8
#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

LoggerH& LoggerH::instance()
{
	// 全局单例对象
	static LoggerH instance;

	return instance;
}

void LoggerH::Init(const QString& logPath)
{
	// 确保目录存在
	QFileInfo fileInfo(logPath);
	QDir dir = fileInfo.absoluteDir();
	if (!dir.exists())
	{
		dir.mkpath(".");
	}

	m_file.setFileName(logPath);

	// Append模式：在原有文件末尾追加，不覆盖
	if (m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
	{
		m_stream.setDevice(&m_file);
		m_stream.setCodec("UTF-8"); // 防止中文乱码
	}
}

void LoggerH::WriteLog(QtMsgType type, const QString& msg)
{
	// 使用QMutexLocker自动加锁/解锁，保证线程安全
	QMutexLocker locker(&m_mutex);

	// 格式化日志内容
	QString typeStr;
	switch (type)
	{
	case QtDebugMsg: typeStr = "[DEBUG]"; break;
	case QtInfoMsg: typeStr = "[INFO]"; break;
	case QtWarningMsg: typeStr = "[WARN]"; break;
	case QtCriticalMsg: typeStr = "[CRIT]"; break;
	case QtFatalMsg: typeStr = "[FATAL]"; break;
	default: typeStr = "[UNKN]"; break;
	}

	QString logLine = QString("%1 %2 %3\n")
		.arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
		.arg(typeStr)
		.arg(msg);

	// 写入文件
	if (m_file.isOpen())
	{
		m_stream << logLine;
		m_stream.flush(); // 立即写入磁盘
	}

	// 如果是Debug模式，同时也输出到控制台
#ifdef QT_WIDGETS_LIB
	fprintf(stdout, "%s", logLine.toStdString().c_str());
	fflush(stdout);
#endif
}

LoggerH::LoggerH()
{

}

LoggerH::~LoggerH()
{
	// 析构时关闭文件
	if (m_file.isOpen())
	{
		m_file.close();
	}
}

