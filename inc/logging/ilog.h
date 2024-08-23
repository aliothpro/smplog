//
// Created by sxc on 2013/3/20.
//
#ifndef SMP_IO_LOG_IXXLOG_H
#define SMP_IO_LOG_IXXLOG_H
#include "common/global.h"
#include <stdint.h>
#include <string>
#include <vector>

#include <libgen.h>
#include <initializer_list>
#include <iostream>
#include <sstream>

#include <mutex>
#include <thread>

#include <algorithm>
#include <string.h>
#include <stdarg.h>

namespace smp{namespace logging{

enum class log_level {
	Off = 0,
	Fatal,
	Error,
	Warn,
	Info,
	Debug,
	Trace,
	All
};

enum class log_format {
	Normal = 0,
	File = 1,
	Sqlite = 2,
	CSV = 3
};

static const std::string __log_file_fullname__ (__FILE__);
static log_level get_log_level_by_name(const std::string& level_str)
{
	std::string str_level = level_str;
	transform(str_level.begin(), str_level.end(), str_level.begin(), (int (*)(int))toupper);
	log_level level = log_level::Info;
	if(str_level == "OFF"){
		level = log_level::Off;
	}else if(str_level == "FATAL"){
		level = log_level::Fatal;
	}else if(str_level == "ERROR"){
		level = log_level::Error;
	}else if(str_level == "WARN"){
		level = log_level::Warn;
	}else if(str_level == "INFO"){
		level = log_level::Info;
	}else if(str_level == "DEBUG"){
		level = log_level::Debug;
	}else if(str_level == "TRACE"){
		level = log_level::Trace;
	}else if(str_level == "ALL"){
		level = log_level::All;
	}
	return level;
}
class log_helper
{
	public:
	template<typename T>
	static void pv(std::ostringstream& ostr,T &&v)
	{
		ostr << v << " ";
	}

	
	template<typename ...Args>
	static std::string mk_log_string(Args ...args)
	{
		std::ostringstream ostr;
		std::initializer_list<int>{ (pv(ostr,args),0)... };
		std::string ov = ostr.str();
		return ov;
	}

	template<const std::string* p=&__log_file_fullname__,unsigned l=__LINE__,typename ...Args>
	static std::string mk_log_string(const char* fmt,Args ...args)
	{
		size_t size = snprintf( nullptr, 0, fmt, args... ) + 1; 
        std::unique_ptr<char[]> buf( new char[ size ] ); 
        snprintf( buf.get(), size, fmt, args... );
        std::string ctt = std::string( buf.get(), buf.get() + size - 1 ); 
		return ctt;
	}
};

#define inner_n_log(level, ... ) log(level , __VA_ARGS__);
#define inner_log(fmt,level, ... ) log(level , log_helper::mk_log_string(fmt,__VA_ARGS__));

class SMP_UTIL_API ilog {
public:


public:
	ilog() {}
	virtual ~ilog() {}
private:
	ilog(const ilog&);
	ilog& operator=(const ilog&);
	private:
	std::mutex mtx_arglog;
public:
	virtual bool open(const std::string& module_name) = 0;
	virtual void set_path(const std::string&) = 0;
	virtual void log(log_level level,const char* title,...) = 0;
	virtual void log(log_level level, const std::string& title) {}
	virtual void log(std::vector<std::string>& record) = 0;
	virtual void table_head(std::vector<std::string>& record) = 0;
	virtual bool is_need_rotate() = 0;
	virtual void flush() = 0;
	virtual void close() = 0;
	virtual void rotate() = 0;
	virtual void set_level(log_level level) = 0;
	virtual log_level get_level() = 0;

	virtual void set_expired(int minutes) = 0;
	virtual int  get_expired() const = 0;
	virtual void clear_expired_log() = 0;
    virtual void log_binary(log_level level, const std::string& data) = 0;


	template<typename ...Args>
	void log(log_level level,Args ...args) 
	{
		std::unique_lock<std::mutex> lock(mtx_arglog);
		std::ostringstream ostr;
		std::initializer_list<int>{ (log_helper::pv(ostr,args),0)... };
		std::string ctt = ostr.str();
		log(level,ctt);	
	}
	

	template<typename ...Args> void FFatal(const char* fmt,Args ...args) {inner_log(log_level::Fatal , fmt ,args...);}
	template<typename ...Args> void FError(const char* fmt,Args ...args) {inner_log(log_level::Error , fmt ,args...);}
	template<typename ...Args> void FWarn (const char* fmt,Args ...args) {inner_log(log_level::Warn  , fmt ,args...);}
	template<typename ...Args> void FInfo (const char* fmt,Args ...args) {inner_log(log_level::Info  , fmt ,args...);}
	template<typename ...Args> void FDebug(const char* fmt,Args ...args) {inner_log(log_level::Debug , fmt ,args...);}
	template<typename ...Args> void FTrace(const char* fmt,Args ...args) {inner_log(log_level::Trace , fmt ,args...);}

	template<typename ...Args> void FFatal(const std::string& fmt,Args ...args) {inner_log(log_level::Fatal , fmt ,args...);}
	template<typename ...Args> void FError(const std::string& fmt,Args ...args) {inner_log(log_level::Error , fmt ,args...);}
	template<typename ...Args> void FWarn (const std::string& fmt,Args ...args) {inner_log(log_level::Warn  , fmt ,args...);}
	template<typename ...Args> void FInfo (const std::string& fmt,Args ...args) {inner_log(log_level::Info  , fmt ,args...);}
	template<typename ...Args> void FDebug(const std::string& fmt,Args ...args) {inner_log(log_level::Debug , fmt ,args...);}
	template<typename ...Args> void FTrace(const std::string& fmt,Args ...args) {inner_log(log_level::Trace , fmt ,args...);}

	template<typename ...Args> void Fatal(Args ...args) {inner_n_log(log_level::Fatal , args...);}
	template<typename ...Args> void Error(Args ...args) {inner_n_log(log_level::Error , args...);}
	template<typename ...Args> void Warn (Args ...args) {inner_n_log(log_level::Warn  , args...);}
	template<typename ...Args> void Info (Args ...args) {inner_n_log(log_level::Info  , args...);}
	template<typename ...Args> void Debug(Args ...args) {inner_n_log(log_level::Debug , args...);}
	template<typename ...Args> void Trace(Args ...args) {inner_n_log(log_level::Trace , args...);}

};



class log_item {
public:
	log_item();
	~log_item();
private:
	std::string m_module_name;
	log_level  m_log_level;
	int64_t    m_timestamp;
	std::string m_content;

public:
	std::string get_module_name()const;
	void set_module_name(const std::string& module_name);

	log_level get_level() const;
	void set_level(const log_level& level);

	int64_t get_timestamp() const;
	void set_timestamp(int64_t ts);

	std::string get_content() const;
	void set_content(const std::string& content);


};



}}

#endif // SMP_IO_LOG_IXXLOG_H
