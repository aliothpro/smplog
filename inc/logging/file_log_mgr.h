//
// Created by sxc on 2013/10/30.
//
#ifndef SMP_IO_LOG_XXFILELOG_MGR_H
#define SMP_IO_LOG_XXFILELOG_MGR_H
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include "common/global.h"
#include "logging/ilog.h"
#include "logging/file_log.h"

namespace smp{namespace logging{
#define LogFileLength 20971520 //20M
class SMP_UTIL_API file_log_mgr :public ilog {
public:
	file_log_mgr();
	~file_log_mgr();
	file_log_mgr(const file_log_mgr&) = delete;
	file_log_mgr&operator=(const file_log_mgr&) = delete;
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
	std::string m_Path;
    std::mutex  m_Locker;
	log_level  m_MaxLogLevel;
	std::string m_ModuleName;
    std::shared_ptr<ilog> trace_logger ;
    std::shared_ptr<ilog> debug_logger ;
    std::shared_ptr<ilog> info_logger  ;
    std::shared_ptr<ilog> warn_logger  ;
    std::shared_ptr<ilog> error_logger ;
    std::shared_ptr<ilog> fatal_logger ;
};
}}

#endif // SMP_IO_LOG_XXFILELOG_H
