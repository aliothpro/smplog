//
// Created by sxc on 2013/3/20.
//
#include <stdarg.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <string.h>
#include "logging/log_mgr.h"
#include "logging/ilog.h"
#include "logging/file_log.h"
#include "logging/file_log_mgr.h"
#include "logging/csv_log.h"
#include "logging/sqlite_log.h"
#include "common/global.h"
#include "common/util.h"


using smp::logging::log_mgr;
using smp::logging::ilog;
using smp::logging::file_log;
using smp::logging::file_log_mgr;
using smp::logging::log_level;
using smp::logging::log_format;
using smp::logging::csv_log;
using smp::logging::sqlite_log;

log_mgr::log_mgr()
: m_log_map()
,m_logging_flag(0)
,m_log_interval(100)//msecs
,m_locker()
,m_tlogging(nullptr)
,m_tclear_expired_log(nullptr)
,m_log_level(log_level::All)
,m_log_one_file(false)
{

}

log_mgr& log_mgr::get_instance()
{
	static log_mgr log_mgr;
	return log_mgr;
}

log_mgr::~log_mgr()
{
	stop();
	SMPLogMap::iterator it;
	for (it = m_log_map.begin(); it != m_log_map.end(); ++it) {
		if (it->second) {
			//delete it->second;
			it->second = nullptr;
		}
	}
	m_log_map.clear();
}

bool log_mgr::start()
{
	//ClearAllExpiredLog();
	if (get_logging_flag()) { return true; }
	set_logging_flag(true);
	m_tlogging = new std::thread(log_mgr::do_logging, this);
	m_tclear_expired_log = new std::thread(log_mgr::do_clear_expired_log, this);
	auto handle = m_tlogging->native_handle();//获取原生句柄
	#if (!defined(__APPLE__) && !defined(WIN32) && !defined(WIN64))
   	pthread_setname_np(handle,"mtutil_logging");
	#endif

	auto handle_clear = m_tclear_expired_log->native_handle();//获取原生句柄
	#if (!defined(__APPLE__) && !defined(WIN32) && !defined(WIN64))
   	pthread_setname_np(handle_clear,"mtutil_log_clear");
	#endif

	return true;
}

void log_mgr::stop()
{
	set_logging_flag(false);
	std::cout << "Stop logging" << std::endl;
    SMPLogMap::iterator it;
    for (it = m_log_map.begin(); it != m_log_map.end(); ++it) {
        if (it->second) {
            std::shared_ptr<ilog> logger = it->second;
            logger->flush();
        }
    }

	std::cout << "Stop logging: before join" << std::endl;
	try{
		if(m_tlogging != nullptr &&m_tlogging->joinable())
		{
			m_tlogging->join();
		}
		if(m_tclear_expired_log != nullptr && m_tclear_expired_log->joinable())
		{
			m_tclear_expired_log->join();
		}
		
	}catch(...)
	{
		std::cout << "Stop logging: join exception" << std::endl;
	}

	std::cout << "Stop logging: join done" << std::endl;
	if(m_tlogging != nullptr)
	{
		delete m_tlogging;
	}
	if(m_tclear_expired_log != nullptr)
	{
		delete m_tclear_expired_log;
	}
	m_tlogging = nullptr;
	m_tclear_expired_log = nullptr;
	std::cout << "Stop logging: stopped" << std::endl;
}


void log_mgr::set_level(log_level log_level){
	m_log_level = log_level;
	SMPLogMap::iterator it;
	for (it = m_log_map.begin(); it != m_log_map.end(); ++it) {
		if (it->second) {
			std::shared_ptr<ilog> logger = it->second;
			logger->set_level(m_log_level);
		}
	}
}

void to_upper_string(std::string& str)
{
    transform(str.begin(), str.end(), str.begin(), (int (*)(int))toupper);
}
/*
    Off = 0,
	Fatal,
	Error,
	Warn,
	Info,
	Debug,
	Trace,
	All
*/
void log_mgr::set_level(const std::string& level){
	log_level ll = get_log_level_by_name(level);
	set_level(ll);
}

std::shared_ptr<ilog> log_mgr::get_logger(const std::string& module_name, const std::string& path, log_format format) {
	SMPLogMap::iterator it = m_log_map.find(module_name);
	if (it == m_log_map.end()) {
		std::shared_ptr<ilog> logger = nullptr;
		switch (format){
			case log_format::Normal :
			case log_format::File:
				logger = std::make_shared<file_log_mgr>();
				logger->set_level(m_log_level);
				break;
			case log_format::CSV:
				logger = std::make_shared<csv_log>();
				//logger->set_level(m_log_level);
				break;
            case log_format::Sqlite:
				logger = std::make_shared<sqlite_log>();
				break;
			default:
				logger = std::make_shared<file_log_mgr>();
				logger->set_level(m_log_level);
				break;
		}
		logger->set_path(path);
		logger->open(module_name);
		m_log_map.insert(std::make_pair(module_name, logger));
		return logger;
	}
	return it->second;
}



void log_mgr::do_logging(log_mgr* mgr) {
	//static bool need_clear = false;
	static int clear_check_interval = 60 * 1000;//1 minute

	static int64_t last_clear_time = 0;

	while (mgr->get_logging_flag()) {
		int logging_interval = mgr->get_interval();
		//need_clear = (sleep_counter >= );
		//sleep_counter = need_clear ? 0 : sleep_counter + logging_interval;
		int64_t bt = util_gettimestamp();
		SMPLogMap::iterator it;
		for (it = mgr->m_log_map.begin(); it != mgr->m_log_map.end(); ++it) {
			if (it->second) {
				std::shared_ptr<ilog> logger = it->second;
				logger->flush();
				if(!mgr->get_logging_flag())
				{
					break;
				}
				if (!mgr->m_log_one_file&&logger->is_need_rotate()) {
					logger->close();
					logger->rotate();
					logger->open(it->first);
				}
                //logger->clear_expired_log();
			}
		}
		int64_t et = util_gettimestamp();
		et = et - bt;
		et = logging_interval - et;
		
		if (et <= 0) {
			et = 1;
		}
		if(!mgr->get_logging_flag())
		{
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(et));
	}
}

void log_mgr::clear_all_expired_log()
{
	SMPLogMap::iterator it;
	for (it = m_log_map.begin(); it != m_log_map.end(); ++it)
	{
		if (it->second) {
			std::shared_ptr<ilog> logger = it->second;
			logger->clear_expired_log();
		}
	}
}

void log_mgr::do_clear_expired_log(log_mgr* mgr) {
	static int clear_check_interval = 60 * 1000;//1 minute
	static int64_t last_clear_time = 0;

	while (mgr->get_logging_flag()) {
		if(!mgr->get_logging_flag())
		{
			break;
		}
		int64_t bt = util_gettimestamp();
		int64_t span = bt - last_clear_time;
		if (span >= clear_check_interval) {
			mgr->clear_all_expired_log();
			last_clear_time = bt;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void log_mgr::set_logging_flag(bool flag){
	// std::lock_guard<std::mutex> locker(m_locker);
	m_logging_flag = flag ? 1 : 0;
}

void log_mgr::log(const std::string& module_name, log_level level, const char *title,...)
{
	std::shared_ptr<ilog> logger = get_logger(module_name);
	if (logger && title != nullptr) {
		va_list ap;
		char buff[2048] = { 0x00 };
		memset(buff, 0x00, sizeof(buff));
		va_start(ap, title);
		snprintf(buff,sizeof(buff)-1, title, ap);
		va_end(ap);

		logger->log(level, buff);
	}
}

void log_mgr::log(const std::string& module_name, log_level level, const std::string & title)
{
	std::shared_ptr<ilog> logger = get_logger(module_name);
	log(logger, level, title);
}

void log_mgr::log(std::shared_ptr<ilog> logger, log_level level, const char *title, ...)
{
	if (logger != nullptr) {
		if (title != nullptr) {
			va_list ap;
			char buff[2048] = { 0x00 };
			memset(buff, 0x00, sizeof(buff));
			va_start(ap, title);
			snprintf(buff, sizeof(buff) - 1, title, ap);
			va_end(ap);

			logger->log(level, buff);
		}


	}
}

void log_mgr::log(std::shared_ptr<ilog> logger, log_level level, const std::string & title)
{
	if (logger != nullptr) {
		logger->log(level, title);
	}
}

bool log_mgr::get_logging_flag()
{
	return m_logging_flag;
}

int32_t log_mgr::get_interval()
{
	return m_log_interval;
}

void log_mgr::set_interval(int32_t msecs)
{
	m_log_interval = msecs;
}


void log_mgr::set_expired(int minutes){

	SMPLogMap::iterator it;
	for (it = m_log_map.begin(); it != m_log_map.end(); ++it) {
		if (it->second) {
			std::shared_ptr<ilog> logger = it->second;
			logger->set_expired(minutes);
		}
	}
}

int log_mgr::get_expired(std::shared_ptr<ilog> logger) const{
	int expired = 0;
	if(logger != nullptr){
		return logger->get_expired();
	}
	return expired;
}


void log_mgr::set_log_on_one_file(bool log_on_one_file)
{
    m_log_one_file = log_on_one_file;
}
