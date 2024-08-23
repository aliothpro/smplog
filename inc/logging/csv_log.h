//
// Created by sxc on 2013/3/20.
//
#ifndef SMP_IO_LOG_XXCSVLOG_H
#define SMP_IO_LOG_XXCSVLOG_H

#include <string>
#include <vector>
#include <mutex>
#include "common/global.h"
#include "logging/ilog.h"

namespace smp{namespace logging{
    
#define LogCSVLength 31457280 //30M excel锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷[row < 1040k, col < 16k]
class SMP_UTIL_API csv_log :public ilog {
public:
	csv_log();
	~csv_log();
	csv_log(const csv_log&) = delete;
	csv_log&operator=(const csv_log&) = delete;
public:
	bool open(const std::string& module_name);
	void set_path(const std::string&);
	void log(log_level level, const char* title, ...);
	void log(log_level level, const std::string& title);
	void log(std::vector<std::string>& record);
	void table_head(std::vector<std::string>& head);
	bool is_need_rotate();
	void flush();
	void close();
	void rotate();
	void set_level(log_level log_level);
	log_level get_level();

	void set_expired(int minutes);
	int get_expired() const;
	void clear_expired_log();
    void log_binary(log_level level, const std::string& data){};
private:
	std::string m_FileName;
	std::string m_Path;
	FILE		*m_File;
	uint64_t	m_CurrentFileSize;
	int32_t		m_ABNo;
	std::vector<std::vector<std::string>>  m_LogCacheA;
	std::vector<std::vector<std::string>>  m_LogCacheB;
	std::mutex                m_CachedLocker;
	std::string m_ModuleName;
	uint32_t m_LogExpired;
};

}}

#endif // SMP_IO_LOG_XXCSVLOG_H
