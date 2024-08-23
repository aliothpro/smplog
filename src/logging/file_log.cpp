//
// Created by sxc on 2013/3/20.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <vector>
#include <list>
#include "logging/file_log.h"
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

#include <filesystem>

#include <chrono>
#include <iomanip>

#include <unistd.h>

using smp::logging::file_log;
using smp::logging::log_level;
using smp::io::file_info;

file_log::file_log():m_File(nullptr),m_CurrentFileSize(0),m_ABNo(0),
m_MaxLogLevel(log_level::All),
m_LogExpired(3*24*60)
{
}

file_log::~file_log() {
	if (m_File != nullptr) {
		flush();
		fclose(m_File);
	}
	m_ABNo = 0;
	m_LogCacheA.clear();
	m_LogCacheB.clear();
}

bool file_log::open(const std::string& module_name) {


	m_module_name = module_name;
	// std::cout << "file log,path:" << m_Path << " module:" << module_name << std::endl;
	// int64_t tv = util_gettimestamp();
	// char strTv[1024] = {0};
	// util_gettimestampString2(tv, strTv);
	if (m_Path.empty() == false) {
		m_FileName = m_Path;

		
        try {
			std::filesystem::path path(m_Path);
            if (!std::filesystem::exists(path)) 
            {
                std::filesystem::create_directory(path);
                std::cout << "log folder [" << m_Path << "] created successfully!" << std::endl;
            } 
			// else 
            // {
            //     std::cout << "log folder [" << m_Path << "] already exists..." << std::endl;
            // }
        } catch (std::filesystem::filesystem_error& e) {
            std::cout << "log  create dir [" << m_Path << "] error:" << e.what() << std::endl;
        }
	}

	m_FileName.append("/");
	m_FileName.append(module_name);
	// m_FileName.append(strTv);
	m_FileName.append(".log");
	close();
	m_File = fopen(m_FileName.c_str(), "a+");
	// std::string ok = m_File == nullptr ? "failed" : "ok";
	// std::cout << "file log:log open :" << m_module_name << ";" << m_FileName << " ;" << ok << "; error no:" << errno << std::endl;
	if (m_File == nullptr) {
		return false;
	}
	
	return true;
}

void file_log::rotate()
{
	int64_t tv = util_gettimestamp();
	char strTv[1024] = {0};
	util_gettimestampString2(tv, strTv);
	std::string n_fname;
	if (m_Path.empty() == false) {
		n_fname = m_Path;
	}
	n_fname.append("/");
	n_fname.append(m_module_name);
	n_fname.append(strTv);
	n_fname.append(".log");

	try
	{
		std::filesystem::rename(m_FileName, n_fname);

		/*
		std::lock_guard<std::mutex> guards(m_CachedLocker);
		fflush(m_File); // 刷新文件缓冲区
		std::filesystem::copy_file(m_FileName, n_fname, std::filesystem::copy_options::overwrite_existing);
		
		try
		{
			// //清空文件
			int fd = fileno(m_File); // 获取文件描述符
			ftruncate(fd, 0);
			// //从新跳转位置到开头--重要
			lseek(fd,0,SEEK_SET);
			
			// // //清空文件内容
    		// fseek(m_File, 0L, SEEK_SET); // 设置文件位置为起始位置
    		// fwrite("\0", sizeof(char), 1, m_File); // 向文件写入空字符
			// fflush(m_File); // 刷新文件缓冲区
			// fseek(m_File, 0L, SEEK_SET); // 设置文件位置为起始位置
			m_CurrentFileSize = 0;
			
		}catch(std::exception & ex)
		{
			std::cout<< "error happened:" << ex.what() << std::endl;
		}
		*/
	}
	catch (std::filesystem::filesystem_error& e) {
        std::cerr << "log  rotate ("<< m_FileName << " -> " << n_fname << ") error:" << e.what() << std::endl;
    }

	// m_File = fopen(m_FileName.c_str(), "a+");
	// if (m_File == nullptr) {
	// 	return false;
	// }
	// return true;
}

void file_log::set_path(const std::string& path) {
	m_Path = path;
}

void file_log::log(log_level level, const char* fmt,...) {
	if (level > m_MaxLogLevel)
	{
		return;
	}
	std::lock_guard<std::mutex> guards(m_CachedLocker);
	// char tmp_time[100] = {0};
	char tmp_buf[4096] = {0};
	// int64_t ts = util_gettimestamp();
	// util_gettimestampWithMSString(ts, tmp_time);
	std::string tmp_time = get_timestamp_string_with_ms();
	std::string logMsg;//(tmp_time);
	logMsg.append(tmp_time);
	char strLevel[512] = {0};
	std::string tid_str = get_thread_id_string(std::this_thread::get_id());
	snprintf(strLevel, sizeof(strLevel)-1 , " [tid:%s][%s]:", tid_str.c_str() , m_LevelMap[(int)level].c_str());
	logMsg.append(strLevel);

	va_list args;
	va_start(args, fmt);
	vsnprintf(tmp_buf, sizeof(tmp_buf) - 1, fmt, args);
	va_end(args);
	logMsg.append(tmp_buf);
	logMsg.append("\n");
	if (m_ABNo == 0) {
		m_LogCacheB.push_back(logMsg);
	}
	else if (m_ABNo == 1) {
		m_LogCacheA.push_back(logMsg);
	}
	else {
		printf("Log error. ABNo = %d LogMsg : %s\n",m_ABNo,logMsg.c_str());
		return;
	}
}

void file_log::log(log_level level, const std::string& title) {
	if (level > m_MaxLogLevel)
	{
		return;
	}
	// char tmp_time[100] = {0};
	// int64_t ts = util_gettimestamp();
	// util_gettimestampWithMSString(ts, tmp_time);
	auto tmp_time = get_timestamp_string_with_ms();
	std::string logMsg;//(tmp_time);
	logMsg.append(tmp_time);

	char strLevel[512] = {0};
	std::string tid_str = get_thread_id_string(std::this_thread::get_id());
	snprintf(strLevel,sizeof(strLevel) - 1, " [tid:%s][%s]:", tid_str.c_str() , m_LevelMap[(int)level].c_str());
	logMsg.append(strLevel);
	logMsg.append(title);
	logMsg.append("\n");
	// if(m_MaxLogLevel >= log_level::Debug)
	// {
	// 	std::cout << logMsg ;
	// }
	std::lock_guard<std::mutex> guards(m_CachedLocker);
	if (m_ABNo == 0) {
		m_LogCacheB.push_back(logMsg);
	}
	else if (m_ABNo == 1) {
		m_LogCacheA.push_back(logMsg);
	}
	else {
		printf("Log error. ABNo = %d logMsg : %s\n",m_ABNo,logMsg.c_str());
		return;
	}
}

void file_log::log(std::vector<std::string>& record)
{
	return;
}

void file_log::table_head(std::vector<std::string>& head) {
	return;
}

bool file_log::is_need_rotate() {
	try
	{
		m_CurrentFileSize = std::filesystem::file_size(m_FileName);
		return m_CurrentFileSize >= LogFileLength;
	}
	catch (std::filesystem::filesystem_error& e) {
        std::cerr << "log is need rotate ("<< m_FileName << ") error:" << e.what() << std::endl;
    }
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	
// #if defined(_WIN32)
// 	m_CurrentFileSize = _filelengthi64(_fileno(m_File));
// #else
// 	struct stat statbuff;
//     if(stat(m_FileName.c_str(), &statbuff) < 0){
//         return false;
//     }else{
//         m_CurrentFileSize = statbuff.st_size;
//     }
// #endif
	return false;
}

void file_log::flush() {
	std::vector<std::string>logCache;
	{
		std::lock_guard<std::mutex> guards(m_CachedLocker);
		std::vector < std::string>* currentCache = nullptr;
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
			printf("Flush error\n");
		}

		if (nullptr != currentCache)
		{
			std::copy(currentCache->begin(), currentCache->end(), std::inserter(logCache, logCache.begin()));
			currentCache->clear();
		}
	}
	for (auto &line : logCache)
	{
		fwrite(line.c_str(), 1, line.length(), m_File);
	}
	fflush(m_File);
}

void file_log::close() {
	std::lock_guard<std::mutex> guards(m_CachedLocker);
	std::cout << "file log:start close file:" << m_module_name << std::endl;
	if (m_File != nullptr)
	{
		fclose(m_File);
		m_File = nullptr;
	}
	std::cout << "file log: close file:" << m_module_name << std::endl;
}

void file_log::set_level(log_level logLevel)
{
	m_MaxLogLevel = logLevel;
}

log_level file_log::get_level()
{
	return m_MaxLogLevel;
}

// template<typename ...Args>
// void file_log::log(log_level level,Args ...args)
// {
// 	std::string ctt = log_helper::mk_log_string(args);
// 	Log(level,ctt);	
// }


//////////////////////////////////////
using smp::logging::log_item;
log_item::log_item() :
	//m_ProcessName()
	m_module_name("SMPLOG")
	, m_log_level(log_level::Off)
	, m_timestamp(0)
	, m_content()
{
}

log_item::~log_item() {}


std::string log_item::get_module_name()const {
	return m_module_name;
}

void log_item::set_module_name(const std::string& module_name) {
	m_module_name = module_name;
}

log_level log_item::get_level() const {
	return m_log_level;
}

void log_item::set_level(const log_level& level) {
	m_log_level = level;
}

int64_t log_item::get_timestamp() const {
	return m_timestamp;
}

void log_item::set_timestamp(int64_t ts) {
	m_timestamp = ts;
}

std::string log_item::get_content() const {
	return m_content;
}

void log_item::set_content(const std::string& content) {
	m_content = content;
}


void file_log::set_expired(int minutes){
	m_LogExpired = minutes;
}

int file_log::get_expired() const{
	return  m_LogExpired;
}

using std::chrono::time_point_cast;
void file_log::list_expired_files(const std::string& path,int32_t expired_seconds,std::list<std::string>& expired_files)
{
	std::filesystem::path pathpath(path);
    if (!std::filesystem::exists(pathpath))
    {
        return;
    }
    if (!std::filesystem::is_directory(pathpath))
    {
        return;
    }
    std::filesystem::directory_entry direntry(pathpath);
    if (direntry.status().type() == std::filesystem::file_type::directory)
    {
        //TODO something....
    }
    std::filesystem::directory_iterator dirite(pathpath);
    for (auto& ite : dirite)
    {
        if (ite.status().type() == std::filesystem::file_type::directory) //is folder
        {
            std::string strsubdir = ite.path().string();
            list_expired_files(strsubdir,expired_seconds,expired_files);
        }
        else if (ite.status().type() == std::filesystem::file_type::regular) //is file
        {
            std::string strfile = ite.path().filename().string();
			auto now = std::chrono::system_clock::now();
			auto ts_now = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

			auto last_wr_t = ite.last_write_time();
			auto fsz = ite.file_size();

			// get the ticks and subtract the file time epoch adjustment
			auto ftea = 116444736000000000ull;
			const auto ticks = last_wr_t.time_since_epoch().count() - ftea;

			// create a time_point from ticks
			const auto tp = std::chrono::system_clock::time_point(std::chrono::system_clock::time_point::duration(ticks));

			// finaly convert to time_t
			std::time_t cftime = std::chrono::system_clock::time_point::clock::to_time_t(tp);

			// auto cftime = time_point_cast(last_wr_t - std::filesystem::file_time_type::clock::now()
            //   + std::chrono::system_clock::now());

			//std::time_t cftime = decltype(last_wr_t)::clock::to_time_t(last_wr_t); // assuming system_clock
			auto diff = ts_now - cftime;
			if(diff > expired_seconds && fsz >= LogFileLength*0.9) 
			{
				expired_files.push_back(strfile);
			}

        }
    }
}

void file_log::clear_expired_log(){
	std::list<std::string> expired_files;
	try{
		list_expired_files(m_Path,m_LogExpired*60,expired_files);
	}catch(std::exception& e)
	{
		std::cerr << "list_expired_files error: " << e.what() << std::endl;
	}
	if(!expired_files.empty())
	{
		for(auto& fname : expired_files)
		{
			try
			{
				std::filesystem::remove(fname);
			}catch(std::exception& e)
			{
				std::cerr << "remove file: " << fname << "  error: " << e.what() << std::endl;
			}
		}
	}
	return;
	std::vector<file_info> fileArray =  traverse_files(m_Path);
	std::vector<file_info>::const_iterator cit;
	int64_t time_now = util_gettimestamp();
	uint32_t time_now_sec = (uint32_t)( time_now / 1000 );
	for(cit = fileArray.begin();cit != fileArray.end();++cit){
		uint32_t time_write = cit->get_write_time();
		//printf("passed %d seconds and expired time is %d seconds.\n",(time_now_sec - time_write),(m_LogExpired * 60));
		if((time_now_sec - time_write) > (uint32_t)(m_LogExpired*60)){
			//delete file
			// 删除文件，成功返回0，否则返回-1
			// if (0 == remove(cit->get_name().c_str()))
			// {
			// 	//printf("Success to remove %s\n",cit->get_name().c_str());
			// }else{
			// 	//printf("failed to remove %s\n",cit->get_name().c_str());
			// }
			try{
				int fsz = std::filesystem::file_size(cit->get_name());
				int max_sz = LogFileLength;
				//std::cout << "log  remove expired ("<< cit->get_name()<< ") size:" << fsz << std::endl;
				if(fsz >= LogFileLength )
				{
					std::filesystem::remove(cit->get_name());
				}
			}	
			catch (std::filesystem::filesystem_error& e) 
			{
				std::cerr << "log  remove expired ("<< cit->get_name()<< ") error:" << e.what() << std::endl;
			}
		}
	}
}
