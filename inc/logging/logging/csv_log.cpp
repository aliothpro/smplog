//
// Created by sxc on 2013/3/20.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "logging/csv_log.h"
#include "common/util.h"

#include "io/io_director.h"
using smp::io::file_info;
using smp::io::file_attribute;

#if defined(_WIN32)
#include <io.h>
#else
#include <sys/stat.h>
#endif

using smp::logging::csv_log;
using smp::logging::log_level;

csv_log::csv_log() :m_File(nullptr), m_CurrentFileSize(0), m_ABNo(0)
,m_LogExpired(24*60)
{
}

csv_log::~csv_log() {
	if (m_File != nullptr) {
		flush();
		fclose(m_File);
	}
	m_ABNo = 0;
	m_LogCacheA.clear();
	m_LogCacheB.clear();
}

bool csv_log::open(const std::string& module_name) {
	m_ModuleName = module_name;
	int64_t tv = util_gettimestamp();
	char strTv[1024] = {0};
	util_gettimestampString2(tv, strTv);
	if (m_Path.empty() == false) {
		m_FileName = m_Path;
	}
	m_FileName.append(module_name);
	m_FileName.append(strTv);
	m_FileName.append(".csv");
	m_File = fopen(m_FileName.c_str(), "a+");
	if (m_File == nullptr) {
		return false;
	}
	return true;
}

void csv_log::set_path(const std::string& path) {
	m_Path = path;
}

void csv_log::log(log_level level, const char* fmt, ...) {
	return;
}

void csv_log::log(log_level level, const std::string& title) {
	return;
}

void csv_log::table_head(std::vector<std::string>& head) {
	head.insert(head.begin(), "time");
	if (m_ABNo == 0) {
		m_LogCacheB.push_back(head);
	}
	else if (m_ABNo == 1) {
		m_LogCacheA.push_back(head);
	}
	else {
		printf("Log error. ABNo = %d", m_ABNo);
		return;
	}
}

void csv_log::log(std::vector<std::string>& record) {
	uint64_t now = util_gettimestamp();
	char nowTime[256] = {0};
	util_gettimestampString(now, nowTime);
	record.insert(record.begin(), nowTime);
	if (m_ABNo == 0) {
		m_LogCacheB.push_back(record);
	}
	else if (m_ABNo == 1) {
		m_LogCacheA.push_back(record);
	}
	else {
		printf("Log error. ABNo = %d", m_ABNo);
		return;
	}
}

bool csv_log::is_need_rotate() {
#if defined(_WIN32)
	m_CurrentFileSize = _filelengthi64(_fileno(m_File));
#else
	struct stat statbuff;
	if (stat(m_FileName.c_str(), &statbuff) < 0) {
		return false;
	}
	else {
		m_CurrentFileSize = statbuff.st_size;
	}
#endif
	return m_CurrentFileSize > LogCSVLength;
}

void csv_log::flush() {
	std::lock_guard<std::mutex> guards(m_CachedLocker);
	char endToken = '\n';
	char seperator = ',';
	if (m_ABNo == 0) //锟斤拷B刷A
	{
		for (uint64_t i = 0; i < m_LogCacheA.size(); i++) {
			for (uint64_t j = 0; j < m_LogCacheA[i].size(); j++) {
				fwrite(m_LogCacheA[i][j].c_str(), 1, m_LogCacheA[i][j].length(), m_File);
				if (j == (m_LogCacheA[i].size()-1)) {
					fwrite(&endToken, 1, 1, m_File);
				}
				else {
					fwrite(&seperator, 1, 1, m_File);
				}
			}
		}
		m_LogCacheA.clear();
		m_ABNo++;
	}
	else if (m_ABNo == 1)//锟斤拷A刷B
	{
		for (uint64_t i = 0; i < m_LogCacheB.size(); i++) {
			for (uint64_t j = 0; j < m_LogCacheB[i].size(); j++) {
				fwrite(m_LogCacheB[i][j].c_str(), 1, m_LogCacheB[i][j].length(), m_File);
				if (j == (m_LogCacheB[i].size()-1)) {
					fwrite(&endToken, 1, 1, m_File);
				}
				else {
					fwrite(&seperator, 1, 1, m_File);
				}
			}
		}
		m_LogCacheB.clear();
		m_ABNo--;
	}
	else {
		printf("Flush error/n");
	}
	fflush(m_File);
}

void csv_log::close() {
	if (m_File != nullptr)
	{
		std::lock_guard<std::mutex> guards(m_CachedLocker);
		fclose(m_File);
	}
}

void csv_log::rotate()
{
}

void csv_log::set_level(log_level logLevel)
{
	return;
}

log_level csv_log::get_level()
{
	return log_level::All;
}


void csv_log::set_expired(int minutes){
	m_LogExpired = minutes;
}

int csv_log::get_expired() const{
	return  m_LogExpired;
}

void csv_log::clear_expired_log(){
	std::vector<file_info> fileArray =  traverse_files(m_Path);
	std::vector<file_info>::const_iterator cit;
	int64_t time_now = util_gettimestamp();
	uint32_t time_now_sec = (uint32_t)( time_now / 1000 );
	for(cit = fileArray.begin();cit != fileArray.end();++cit){
		uint32_t time_write = cit->get_write_time();
		//expired
		if((time_now_sec - time_write) > (m_LogExpired * 1000)){
			//delete file
			// 鍒犻櫎鏂囦欢锛屾垚鍔熻繑鍥�0锛屽惁鍒欒繑鍥�-1
			if (0 == remove(cit->get_name().c_str()))
			{

			}else{
				//failed

			}
		}
	}
}
