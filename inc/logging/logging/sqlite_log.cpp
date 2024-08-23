//
// Created by  on 2019/6/13.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <vector>
#include "logging/sqlite_log.h"
#include "common/util.h"
#include "io/io_director.h"
#include <thread>
#include <iterator>
#include <iostream>

#if defined(_WIN32)
#include <io.h>
#else
#include <sys/stat.h>
#endif

#define print_info 1

using smp::logging::sqlite_log;
using smp::logging::log_level;
using smp::io::file_info;

sqlite_log::sqlite_log()
    :m_Database(nullptr)
    ,m_ABNo(0),
    m_MaxLogLevel(log_level::All),
    m_LogExpired(24*60)
{
    m_DayLatest =  std::chrono::time_point_cast<day_type>(std::chrono::system_clock::now());
    m_HourLatest = std::chrono::time_point_cast<hour_type>(std::chrono::system_clock::now());
}

sqlite_log::~sqlite_log()
{
	if (m_Database!= nullptr)
    {
		flush();
        try{
            delete m_Database;
            m_Database = nullptr;
        }catch(std::exception& e){
            std::cout << "delete Database SQLite exception: " << e.what() << std::endl;
        }
	}

	m_ABNo = 0;
	m_LogCacheA.clear();
	m_LogCacheB.clear();
}

bool sqlite_log::open(const std::string& module_name)
{
	m_ModuleName = module_name;
	int64_t tv = util_gettimestamp();
	char strTv[1024] = {0};
	util_gettimestampString2(tv, strTv);
	if (m_Path.empty() == false)
    {
		m_DatabaseName = m_Path;
	}

	m_DatabaseName.append("/");
	m_DatabaseName.append(module_name);
	m_DatabaseName.append(strTv);
	m_DatabaseName.append(".db3");

#ifdef print_info
	std::cout<<"Open DataBase "<<m_DatabaseName.c_str()<<std::endl;
#endif

    if (nullptr != m_Database)
    {
        delete m_Database;
        m_Database = nullptr;
    }

	try {
    	m_Database = new SQLite::Database(m_DatabaseName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
	} catch(std::exception& e) {
		std::cout << "create Database SQLite exception: " << e.what() << std::endl;
	}

	return m_Database != nullptr;
}

void sqlite_log::set_path(const std::string& path)
{
	m_Path = path;
}

void sqlite_log::log(log_level level, const char* fmt,...)
{
	if (level > m_MaxLogLevel)
	{
		return;
	}

	std::lock_guard<std::mutex> guards(m_CachedLocker);
	// char tmp_time[100] = {0};
	char tmp_buf[4096] = {0};
	// int64_t ts = util_gettimestamp();
	// util_gettimestampWithMSString(ts, tmp_time);
	auto tmp_time = get_timestamp_string_with_ms();

	va_list args;
	va_start(args, fmt);
	vsnprintf(tmp_buf, sizeof(tmp_buf) - 1, fmt, args);
	va_end(args);

	SqlLine line;
	line.level = m_LevelMap[(int)level];
	line.ts = tmp_time;
	line.msg = tmp_buf;
    line.msgType = MSG_STRING;

	if (m_ABNo == 0)
    {
		m_LogCacheB.push_back(line);
	}
	else if (m_ABNo == 1)
    {
		m_LogCacheA.push_back(line);
	}
	else
    {
		return;
	}
}

void sqlite_log::log(log_level level, const std::string& title)
{
    LogContent(level, title, MSG_STRING);
}

void sqlite_log::log_binary(log_level level, const std::string& data)
{
    LogContent(level, data, MSG_BINARY);
}

void sqlite_log::LogContent(log_level level, const std::string& content, int contentType)
{
	if (level > m_MaxLogLevel)
	{
		return;
	}
	// char tmp_time[100] = {0};
	// int64_t ts = util_gettimestamp();
	// util_gettimestampWithMSString(ts, tmp_time);

	auto tmp_time = get_timestamp_string_with_ms();


	SqlLine line;
	line.level = m_LevelMap[(int)level];
	line.ts = tmp_time;
	line.msg = content;
    line.msgType = contentType;

	std::lock_guard<std::mutex> guards(m_CachedLocker);
	if (m_ABNo == 0)
    {
		m_LogCacheB.push_back(line);
	}
	else if (m_ABNo == 1)
    {
		m_LogCacheA.push_back(line);
	}
	else
    {
		return;
	}
}
/*  如果运行时间超过了一天了，分库  */
bool sqlite_log::is_need_rotate()
{
    std::chrono::time_point<std::chrono::system_clock, day_type> today = std::chrono::time_point_cast<day_type>(std::chrono::system_clock::now());
    day_type days = today - m_DayLatest;
    if ( days.count() > 1)
    {
        m_DayLatest = today;
        return true;
    }
    else
    {
        return false;
    }
}

void sqlite_log::flush()
{
	std::vector<SqlLine>logCache;
	{
		std::lock_guard<std::mutex> guards(m_CachedLocker);
		std::vector <SqlLine>* currentCache = nullptr;
		if (m_ABNo == 0)
		{
			currentCache = &m_LogCacheA;
			m_ABNo++;
		}
		else if (m_ABNo == 1)
		{
			currentCache = &m_LogCacheB;
			m_ABNo--;
		}
		else
		{
			std::cout<<"Flush error"<<std::endl;
		}

		if (nullptr != currentCache)
		{
			std::copy(currentCache->begin(), currentCache->end(), std::inserter(logCache, logCache.begin()));
			currentCache->clear();
		}
	}


    /*  考察下是否需要分表  */
    {
        std::lock_guard<std::mutex> guard(m_DatabaseLocker);
		bool create = IfTableNeedSplit();
        SetTableName();
		if (create)
		{
			if (nullptr != m_Database)
			{
				std::string createSQL;
				sprintf_std_string(createSQL, "CREATE TABLE %s (id INTEGER PRIMARY KEY, level TEXT, ts TEXT, msg BLOB, model TEXT)", m_TableName.c_str());
                try{
                    SQLite::Transaction transaction(*m_Database);
                    m_Database->exec(createSQL);
                    transaction.commit();
                }catch(std::exception& e){
                    std::cout << "create table SQLite exception: " << e.what() << std::endl;
                    std::cout << "create SQL "<< createSQL.c_str()<< std::endl;
                }

			}
		}

		std::string insertSQL;
        try{
            SQLite::Transaction transaction(*m_Database);
            for (auto &line : logCache)
            {
                if (line.msgType == MSG_STRING)
                {
                    sprintf_std_string(insertSQL, "INSERT INTO %s VALUES (nullptr, \"%s\", \"%s\", \"%s\", \"%s\")",
                            m_TableName.c_str(), line.level.c_str(), line.ts.c_str(), line.msg.c_str(), m_ModuleName.c_str());
                    m_Database->exec(insertSQL);
                }
                else if (line.msgType == MSG_BINARY)
                {
                    sprintf_std_string(insertSQL, "INSERT INTO %s VALUES (nullptr, \"%s\", \"%s\", ?, \"%s\")",
                            m_TableName.c_str(), line.level.c_str(), line.ts.c_str(), m_ModuleName.c_str());
                    SQLite::Statement query(*m_Database, insertSQL.c_str());
                    query.bind(1, line.msg.c_str(), line.msg.length());
                    query.exec();
                }
            }
            transaction.commit();
        }catch(std::exception& e){
            std::cout<< "insert logs SQLite exception: "<< e.what() <<std::endl;
			std::cout<< "Insert SQL "<< insertSQL.c_str() << std::endl;
        }
    }
}

void sqlite_log::SetTableName()
{
    time_t t = std::chrono::system_clock::to_time_t(m_HourLatest);
    std::tm tm = *localtime(&t);
    char buffer[64] = {0};
    strftime(buffer, 64, "%Y%m%d%H%M%S", &tm);
    m_TableName = "t";
    m_TableName.append(buffer);
}

bool sqlite_log::IfTableNeedSplit()
{
    std::chrono::time_point<std::chrono::system_clock, hour_type> hour = std::chrono::time_point_cast<hour_type>(std::chrono::system_clock::now());
    hour_type hours = hour - m_HourLatest;
    if ( hours.count() > 1)
    {
        m_HourLatest = hour;
		return true;
    }
	else
	{
        return m_TableName.empty();
	}

}

void sqlite_log::close()
{
	if (m_Database != nullptr)
	{
		std::lock_guard<std::mutex> guards(m_DatabaseLocker);
        delete m_Database;
        m_Database = nullptr;

        try {
            m_Database = new SQLite::Database(m_DatabaseName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        } catch(std::exception& e) {
            std::cout << "create data base SQLite exception: " << e.what() << std::endl;
        }
	}
}

void sqlite_log::rotate()
{
	
}

void sqlite_log::set_level(log_level logLevel)
{
	m_MaxLogLevel = logLevel;
}

log_level sqlite_log::get_level()
{
	return m_MaxLogLevel;
}

void sqlite_log::set_expired(int minutes)
{
	m_LogExpired = minutes;
}

int sqlite_log::get_expired() const
{
	return  m_LogExpired;
}
/*	删除过期的数据库文件
	删除过期的数据库表
	删除当前表中的过期数据,这里先不做了，影响性能，清理暂时不这么严格执行
*/

void sqlite_log::clear_expired_log()
{

#ifdef print_info
		std::cout<<"clear_expired_log "<<std::endl;
#endif

	std::vector<file_info> fileArray =  traverse_files(m_Path, "db3");
	for (auto& file: fileArray)
	{

#ifdef print_info
		std::cout<<"Finding file "<< file.get_name().c_str()<< std::endl;
#endif
		if (IsDataBaseOutOfDate(file.get_name()))
		{
			if (0 != remove(file.get_name().c_str()))
            {
				std::cout<<"Fail to remove "<< file.get_name().c_str()<< std::endl;
			}
			else
			{
#ifdef print_info
				std::cout<<"Succeed to remove "<< file.get_name().c_str()<< std::endl;
#endif
			}
		}
		else
		{
			// drop tables
			std::vector<std::string> tableNames;
            SQLite::Database dataBase(file.get_name(), SQLite::OPEN_READWRITE);
            try{
                SQLite::Statement query(dataBase, "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;");
                while (query.executeStep())
                {
                    tableNames.push_back(query.getColumn(0));
                }
            }catch(std::exception& e){
                std::cout<<"get table names SQLite exception:"<< e.what()<< std::endl;
            }
#ifdef print_info
            std::cout<<"threre are "<<tableNames.size()<<"tables in database "<<file.get_name().c_str()<<std::endl;
#endif
			std::string dropSQL;
            try{
                SQLite::Transaction transaction(dataBase);
                for (auto& tableName: tableNames)
                {
					if (IsTableOutOfDate(tableName))
					{
                    	sprintf_std_string(dropSQL, "DROP TABLE IF EXISTS %s", tableName.c_str());
                    	dataBase.exec(dropSQL.c_str());
#ifdef print_info
                        std::cout<< "Drop Table "<< tableName.c_str() << std::endl;
#endif
					}
                }
                transaction.commit();
            }catch(std::exception& e){
                std::cout << "drop table SQLite exception: " << e.what() << std::endl;
            }

		}

	}
}

bool sqlite_log::IsDataBaseOutOfDate(const std::string& fileName)
{
    if (!IsMyDataBase(fileName))
    {
        return false;
    }

    int length = fileName.length();
    int timeAndSuffixLength = 18;
    int timeLength = 14;
    if (length <= timeAndSuffixLength)
        return false;

    std::string timeFile = fileName.substr(length - timeAndSuffixLength, timeLength);
    return IsTimeExpired(timeFile);
}

bool sqlite_log::IsMyDataBase(const std::string& fileName)
{
    return fileName.find(m_ModuleName) != std::string::npos;
}

bool sqlite_log::IsTableOutOfDate(const std::string& tableName)
{
    int length = tableName.length();
    int timeLength = 14;
    if (length < timeLength)
        return false;

    std::string timeTable = tableName.substr(length - timeLength,  timeLength);
    return IsTimeExpired(timeTable);
}

bool sqlite_log::IsTimeExpired(const std::string& timeString)
{
    int expiredHours = (m_LogExpired%60 == 0?(m_LogExpired / 60):(m_LogExpired /60 + 1));
	std::tm tm = {};
	strptime(timeString.c_str(),"%Y%m%d%H%M%S",&tm);
	auto tp = std::chrono::system_clock::from_time_t(mktime(&tm));
	auto tpHour =  std::chrono::time_point_cast<hour_type>(tp);
	hour_type hours = tpHour - m_HourLatest;
	return hours.count() >= expiredHours;
}
