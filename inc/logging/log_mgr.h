//
// Created by sxc on 2013/3/20.
//

#ifndef SMP_IO_LOG_XXlog_mgr_H
#define SMP_IO_LOG_XXlog_mgr_H
#include <map>
#include <atomic>
#include <thread>
#include <mutex>
#include <memory>
#include "common/global.h"
#include "logging/ilog.h"
namespace smp{namespace logging{

class SMP_UTIL_API log_mgr {
public:
	static log_mgr& get_instance();
	~log_mgr();
private:
	log_mgr();
	log_mgr(const log_mgr&) = delete;
	log_mgr& operator=(const log_mgr&) = delete;
public:
	bool start();
	void stop();
	std::shared_ptr<ilog> get_logger(const std::string& module_name, const std::string& path = ".", log_format format = log_format::Normal);

	void set_level(log_level level);
	void set_level(const std::string& log_level);

	void log(const std::string& module_name,log_level level, const char* title...);
	void log(const std::string& module_name,log_level level, const std::string& title);

	void log(std::shared_ptr<ilog> logger, log_level level, const char* title,...);
	void log(std::shared_ptr<ilog> logger, log_level level, const std::string& title);

	bool    get_logging_flag();
	int32_t get_interval();
	void    set_interval(int32_t msecs);

	void 	set_expired(int minutes);
	int32_t get_expired(std::shared_ptr<ilog> logger) const;

    void    set_log_on_one_file(bool log_on_one_file);
	void 	clear_all_expired_log();
protected:
	typedef std::map<std::string, std::shared_ptr<ilog>> SMPLogMap;
	static void do_logging(log_mgr* log_mgr);
	static void do_clear_expired_log(log_mgr* log_mgr);

	void set_logging_flag(bool flag);
private:
	SMPLogMap 			m_log_map;
	std::atomic_bool  	m_logging_flag;
	int32_t  			m_log_interval;//msecs
	std::mutex 			m_locker;
	std::thread 		*m_tlogging;
	std::thread 		*m_tclear_expired_log;
	log_level 			m_log_level;
    bool 				m_log_one_file;
};

}}
#endif // SMP_IO_LOG_XXlog_mgr_H
