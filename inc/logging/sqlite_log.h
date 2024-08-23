//
// Created by  on 2019/6/13.
//
#ifndef SMP_IO_LOG_XXSQLLITELOG_H
#define SMP_IO_LOG_XXSQLLITELOG_H
#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <chrono>

#include "common/global.h"
#include "logging/ilog.h"
#include "SQLiteCpp/SQLiteCpp.h"

/*
    提供一种SQLLite日志的方式
    分配策略:
        1.按小时分表;
        2.按天分分库;
*/

namespace smp{namespace logging{
struct SqlLine
{
	std::string level;
	std::string ts;
	std::string msg;
#define MSG_STRING 1
#define MSG_BINARY 2
    int msgType;
};

class SMP_UTIL_API sqlite_log :public ilog {
public:
	sqlite_log();
	~sqlite_log();
	sqlite_log(const sqlite_log&) = delete;
	sqlite_log&operator=(const sqlite_log&) = delete;

public:
	bool open(const std::string& module_name);
	void set_path(const std::string&);
	void log(log_level level, const char* title,...);
	void log(log_level level, const std::string& title);
	void log(std::vector<std::string>& record){};
	void table_head(std::vector<std::string>& head){};
	bool is_need_rotate();/*  这里应该是检查是否需要分库  */
	void flush();
	void close();
	void rotate();
	void set_level(log_level logLevel);
	log_level get_level();
	void set_expired(int minutes);
	int get_expired() const;
	void clear_expired_log();
    void log_binary(log_level level, const std::string& data);
private:
	void LogContent(log_level level, const std::string& content, int contentType);
    bool IfTableNeedSplit();
	bool IsDataBaseOutOfDate(const std::string& fileName);
    bool IsMyDataBase(const std::string& fileName);
	bool IsTableOutOfDate(const std::string& tableName);
    bool IsTimeExpired(const std::string& timeString);
	void SetTableName();

    SQLite::Database *m_Database; /*  运行时可能会换库，保存当前的数据库指针  */
    std::mutex m_DatabaseLocker;

    std::string m_DatabaseName; /*  按天分库    */
	std::string m_TableName;	/*	按小时分表	*/
	std::string m_Path; /*  库保存的路径 */

    typedef std::chrono::duration <int, std::ratio<60 * 60 * 24>> day_type;
    typedef std::chrono::duration <int, std::ratio<60 * 60>> hour_type;

    std::chrono::time_point<std::chrono::system_clock, day_type> m_DayLatest;
    std::chrono::time_point<std::chrono::system_clock, hour_type> m_HourLatest;
	int32_t		m_ABNo;
	std::vector<SqlLine>  m_LogCacheA;
	std::vector<SqlLine>  m_LogCacheB;
	std::mutex        m_CachedLocker;

	log_level m_MaxLogLevel;
	std::string m_ModuleName;
	int32_t 	m_LogExpired; /*    过期时间(单位分钟)  */
	const std::string m_LevelMap[8] = {"Off","Fatal","Error","Warn","Info","Debug","Trace","All"};
};
}}

#endif // SMP_IO_LOG_XXSQLLITELOG_H
