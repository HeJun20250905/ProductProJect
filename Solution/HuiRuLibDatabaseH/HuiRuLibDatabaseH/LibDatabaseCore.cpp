#include "LibDatabaseCore.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "../../../Include/LibDatabaseCore_Export.h"

// Windows API
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <cstring> 

#if defined(_MSC_VER) && (_MSC_VER >= 1600)    
# pragma execution_character_set("utf-8")    
#endif

const QString LibDatabaseCore::SQL_INSERT = R"(
    INSERT INTO library_database 
    (name, width, height, length, pick_z, place_z, nozzle_id, speed) 
    VALUES (?, ?, ?, ?, ?, ?, ?, ?)
)";

const QString LibDatabaseCore::SQL_UPDATE = R"(
    UPDATE library_database 
    SET width = ?, height = ?, length = ?, pick_z = ?, 
        place_z = ?, nozzle_id = ?, speed = ?
    WHERE name = ?
)";

const QString LibDatabaseCore::SQL_DELETE = R"(
    DELETE FROM library_database WHERE name = ?
)";

const QString LibDatabaseCore::SQL_QUERY = R"(
    SELECT name, width, height, length, pick_z, place_z, nozzle_id, speed
    FROM library_database WHERE name = ? LIMIT 1
)";

LibDatabaseCore& LibDatabaseCore::instance()
{
    // 全局单例对象
    static LibDatabaseCore instance;

    return instance;
}

int LibDatabaseCore::LibDatabaseCore_Init()
{
    // 添加并打开 SQLite 数据库连接
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "LIBRARY_DATABASE");

    // 【核心修改】：获取 .exe 所在的绝对路径，并拼接数据库文件名
    QString appDir = QCoreApplication::applicationDirPath();
    QString dbPath = QDir(appDir).filePath("library_database.db");

    db.setDatabaseName(dbPath);

    // 【强烈建议】：在控制台打印出真实路径，方便日后排查问题
    qDebug() << "[LibDatabaseCore] Library数据库实际存储路径:" << dbPath;

    if (!db.open())
    {
        qDebug() << "[LibDatabaseCore] 打开Library数据库失败:" << db.lastError().text();
        return -1; // 返回错误码
    }

    // 自动检查并创建元件库表 (元件库模块)
    QSqlQuery query(db);
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS library_database (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            width REAL DEFAULT 0.0,
            height REAL DEFAULT 0.0,
            length REAL DEFAULT 0.0,
            pick_z REAL DEFAULT 0.0,
            place_z REAL DEFAULT 0.0,
            nozzle_id INTEGER DEFAULT 1,
            speed REAL DEFAULT 100.0
        )
    )";

    if (!query.exec(createTableSql))
    {
        qDebug() << "[LibDatabaseCore] 创建 library_database 表失败:" << query.lastError().text();
        return -2; // 【修正】建表失败也返回错误码
    }
    else
    {
        qDebug() << "[LibDatabaseCore] Library数据库初始化成功，library_database 表已就绪";
    }

    // 核心优化：在数据库打开后，预编译所有高频 SQL 语句
    m_insertQuery = new QSqlQuery(db);
    m_insertQuery->prepare(SQL_INSERT);

    m_updateQuery = new QSqlQuery(db);
    m_updateQuery->prepare(SQL_UPDATE);

    m_deleteQuery = new QSqlQuery(db);
    m_deleteQuery->prepare(SQL_DELETE);

    m_selectQuery = new QSqlQuery(db);
    m_selectQuery->prepare(SQL_QUERY);

    qDebug() << "[LibDatabaseCore] 所有 SQL 语句预编译完成";

    return 0;
}

int LibDatabaseCore::LibDatabaseCoreInsertLibraryItem(const LibraryItem& item)
{
    QMutexLocker locker(&m_mutex);

    // 检查预编译状态
    if (!m_insertQuery)
    {
        qDebug() << "[LibDatabaseCore] InsertQuery 未预编译";
        return -1;
    }

    // 清除上一次的执行状态和绑定参数，防止参数叠加
    m_insertQuery->finish();

    // 绑定参数
    m_insertQuery->addBindValue(item.name);
    m_insertQuery->addBindValue(item.width);
    m_insertQuery->addBindValue(item.height);
    m_insertQuery->addBindValue(item.length);
    m_insertQuery->addBindValue(item.pickZ);
    m_insertQuery->addBindValue(item.placeZ);
    m_insertQuery->addBindValue(item.nozzleId);
    m_insertQuery->addBindValue(item.speed);

    if (!m_insertQuery->exec())
    {
        qDebug() << "[LibDatabaseCore] Insert元件失败 (可能已存在):" << m_insertQuery->lastError().text();
        return -2;
    }

    qDebug() << "[LibDatabaseCore] 元件数据Insert成功:" << item.name;
    return 0;
}

int LibDatabaseCore::LibDatabaseCoreDeleteLibraryItem(const QString& libraryName)
{
    QMutexLocker locker(&m_mutex);

    // 检查预编译状态
    if (!m_deleteQuery)
    {
        qDebug() << "[LibDatabaseCore] DeleteQuery 未预编译";
        return -1;
    }

    // 清除旧状态
    m_deleteQuery->finish();

    m_deleteQuery->addBindValue(libraryName);

    if (!m_deleteQuery->exec())
    {
        qDebug() << "[LibDatabaseCore] Delete元件失败:" << m_deleteQuery->lastError().text();
        return -2;
    }

    if (m_deleteQuery->numRowsAffected() == 0)
    {
        qDebug() << "[LibDatabaseCore] 未找到该元件，Delete未执行:" << libraryName;
        return 1;
    }

    qDebug() << "[LibDatabaseCore] 元件数据Delete成功:" << libraryName;
    return 0;
}

int LibDatabaseCore::LibDatabaseCoreUpdateLibraryItem(const LibraryItem& item)
{
    QMutexLocker locker(&m_mutex);

    // 检查预编译状态
    if (!m_updateQuery)
    {
        qDebug() << "[LibDatabaseCore] UpdateQuery 未预编译";
        return -1;
    }

    // 清除旧状态
    m_updateQuery->finish();

    // 重新绑定参数（注意顺序与之前一致）
    m_updateQuery->addBindValue(item.width);
    m_updateQuery->addBindValue(item.height);
    m_updateQuery->addBindValue(item.length);
    m_updateQuery->addBindValue(item.pickZ);
    m_updateQuery->addBindValue(item.placeZ);
    m_updateQuery->addBindValue(item.nozzleId);
    m_updateQuery->addBindValue(item.speed);
    m_updateQuery->addBindValue(item.name);

    if (!m_updateQuery->exec())
    {
        qDebug() << "[LibDatabaseCore] Update元件失败:" << m_updateQuery->lastError().text();
        return -2;
    }

    if (m_updateQuery->numRowsAffected() == 0)
    {
        return 1; // 未找到目标数据
    }

    qDebug() << "[LibDatabaseCore] 元件数据Update成功:" << item.name;
    return 0;
}

int LibDatabaseCore::LibDatabaseCoreQueryLibraryItem(LibraryItem& item, const QString& libraryName)
{
    QMutexLocker locker(&m_mutex);

    // 检查预编译状态
    if (!m_selectQuery)
    {
        qDebug() << "[LibDatabaseCore] SelectQuery 未预编译";
        return -1;
    }

    // 清除旧状态
    m_selectQuery->finish();

    // 直接使用成员变量 m_selectQuery
    m_selectQuery->addBindValue(libraryName);

    if (!m_selectQuery->exec())
    {
        qDebug() << "[LibDatabaseCore] 查询元件失败:" << m_selectQuery->lastError().text();
        return -2;
    }

    if (m_selectQuery->next())
    {
        // 将 QString 转换为 C 风格字符串，并安全拷贝到 char 中
        QString nameStr = m_selectQuery->value(0).toString();
        strncpy_s(item.name, nameStr.toLocal8Bit().constData(), sizeof(item.name) - 1);
        item.name[sizeof(item.name) - 1] = '\0'; // 确保字符串以 \0 结尾，防止溢出
        item.width = m_selectQuery->value(1).toFloat();
        item.height = m_selectQuery->value(2).toFloat();
        item.length = m_selectQuery->value(3).toFloat();
        item.pickZ = m_selectQuery->value(4).toFloat();
        item.placeZ = m_selectQuery->value(5).toFloat();
        item.nozzleId = m_selectQuery->value(6).toInt();
        item.speed = m_selectQuery->value(7).toFloat();
        return 0;
    }

    return 1; // 未找到数据
}

int LibDatabaseCore::LibDatabaseCore_Uninit()
{
    qDebug() << "[LibDatabaseCore] 开始安全停止连接...";

    QSqlDatabase db = QSqlDatabase::database("LIBRARY_DATABASE");
    if (db.isOpen())
    {
        db.close();
        qDebug() << "[LibDatabaseCore] 数据库连接已安全关闭";
    }

    return 0;
}

LibDatabaseCore::LibDatabaseCore()
{
    qDebug() << "[LibDatabaseCore] 开始初始化";

}

LibDatabaseCore::~LibDatabaseCore()
{
    // 在析构函数这里不调用关闭函数，会有逻辑问题，在外部显式调用
    //LibDatabaseCore_Uninit();
}

