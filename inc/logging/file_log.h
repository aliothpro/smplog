//
// Created by sxc on sxc/3/20.
//
#ifndef SMP_IO_LOG_XXFILELOG_H
#define SMP_IO_LOG_XXFILELOG_H
#include <string>
#include <vector>
#include <list>
#include <mutex>
#include "common/global.h"
#include "logging/ilog.h"

namespace smp{namespace logging{
#define LogFileLength 20971520 //20M
class SMP_UTIL_API file_log :public ilog {
public:
	file_log();
	~file_log();
	file_log(const file_log&) = delete;
	file_log&operator=(const file_log&) = delete;
public:
	bool open(const std::string& module_name);
	void set_path(const std::string&);
	void log(log_level level, const char* title,...);
	void log(log_level level, const std::string& title);
	void log(std::vector<std::string>& record);
	void table_head(std::vector<std::string>& head);
	bool is_need_rotate();
	void flush();
	void close();
	void rotate();
	void set_level(log_level logLevel);
	log_level get_level();

	void set_expired(int minutes);
	int get_expired() const;
	void clear_expired_log();
    void log_binary(log_level level, const std::string& data){};
	// template<typename ...Args>
	// virtual void log(log_level level,Args ...args);
private:
	std::string m_FileName;
	std::string m_Path;
	FILE		*m_File;
	uint64_t	m_CurrentFileSize;
	int32_t		m_ABNo;
	std::vector<std::string>  m_LogCacheA;
	std::vector<std::string>  m_LogCacheB;
	std::mutex                m_CachedLocker;
	log_level m_MaxLogLevel;
	std::string m_module_name;
	int32_t 	m_LogExpired;// minutes
	const std::string m_LevelMap[8] = {"Off","Fatal","Error","Warn","Info","Debug","Trace","All"};

private:

	void list_expired_files(const std::string& path,int32_t expired_seconds,std::list<std::string>& expired_files);
};
}}

#endif // SMP_IO_LOG_XXFILELOG_H
