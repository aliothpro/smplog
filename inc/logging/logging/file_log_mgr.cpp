#include "logging/file_log_mgr.h"
#include "io/io_director.h"

using smp::logging::file_log_mgr;
using smp::logging::file_log;
using smp::logging::log_level;
using smp::io::file_info;

	file_log_mgr::file_log_mgr():
    m_Path("./")
    , m_Locker()
	, m_MaxLogLevel(log_level::Info)
	, m_ModuleName("module")
    , trace_logger(std::make_shared<file_log>())
    , debug_logger(std::make_shared<file_log>())
    , info_logger (std::make_shared<file_log>())
    , warn_logger (std::make_shared<file_log>())
    , error_logger(std::make_shared<file_log>())
    , fatal_logger(std::make_shared<file_log>())
    {

    }


	file_log_mgr::~file_log_mgr()
    {
        close();
    }
        




	bool file_log_mgr::open(const std::string& module_name)
    {
        std::lock_guard<std::mutex> lk(m_Locker);
        m_ModuleName = module_name;
        if(trace_logger != nullptr) { trace_logger ->set_level(log_level::Trace);}
        if(debug_logger != nullptr) { debug_logger ->set_level(log_level::Debug);}
        if(info_logger  != nullptr) { info_logger  ->set_level(log_level::Info) ;}
        if(warn_logger  != nullptr) { warn_logger  ->set_level(log_level::Warn) ;}
        if(error_logger != nullptr) { error_logger ->set_level(log_level::Error);}
        if(fatal_logger != nullptr) { fatal_logger ->set_level(log_level::Fatal);}

        if(trace_logger != nullptr) { trace_logger ->set_path(m_Path) ; }
        if(debug_logger != nullptr) { debug_logger ->set_path(m_Path) ; }
        if(info_logger  != nullptr) { info_logger  ->set_path(m_Path) ; }
        if(warn_logger  != nullptr) { warn_logger  ->set_path(m_Path) ; }
        if(error_logger != nullptr) { error_logger ->set_path(m_Path) ; }
        if(fatal_logger != nullptr) { fatal_logger ->set_path(m_Path) ; }

        bool opened = false;

        if(trace_logger != nullptr) {  opened =           trace_logger ->open(module_name+"_"+"trace") ; }
        if(debug_logger != nullptr) {  opened = opened && debug_logger ->open(module_name+"_"+"debug") ; }
        if(info_logger  != nullptr) {  opened = opened && info_logger  ->open(module_name+"_"+"info")  ; }
        if(warn_logger  != nullptr) {  opened = opened && warn_logger  ->open(module_name+"_"+"warn")  ; }
        if(error_logger != nullptr) {  opened = opened && error_logger ->open(module_name+"_"+"error") ; }
        if(fatal_logger != nullptr) {  opened = opened && fatal_logger ->open(module_name+"_"+"fatal") ; }

        return opened;
    }

	void file_log_mgr::set_path(const std::string& path)
    {
        m_Path = path;
        if(trace_logger != nullptr) { trace_logger ->set_path(m_Path); }
        if(debug_logger != nullptr) { debug_logger ->set_path(m_Path); }
        if(info_logger  != nullptr) { info_logger  ->set_path(m_Path); }
        if(warn_logger  != nullptr) { warn_logger  ->set_path(m_Path); }
        if(error_logger != nullptr) { error_logger ->set_path(m_Path); }
        if(fatal_logger != nullptr) { fatal_logger ->set_path(m_Path); }
    }

    #define logger_fmt_log(logger,level,fmt, ...) {logger->log(level,fmt,__VA_ARGS__);}

	void file_log_mgr::log(log_level level, const char* title,...)
    {
        static char buff[20480] = { 0x00 };
        if (title == nullptr) {
            return;
        }
        va_list ap;
        memset(buff, 0x00, sizeof(buff));
        va_start(ap, title);
        snprintf(buff,sizeof(buff)-1, title, ap);
        va_end(ap);
		
        switch (level)
        {
        case log_level::Trace :
            trace_logger->log(level,buff);
            break;
        case log_level::Debug :
            debug_logger->log(level,buff);
            break;
        case log_level::Info :
            info_logger->log(level,buff);
            break;
        case log_level::Warn :
            warn_logger->log(level,buff);
            break;
        case log_level::Error :
            error_logger->log(level,buff);
            break;
        case log_level::Fatal :
            fatal_logger->log(level,buff);
            break;
        default:
            break;
        }
    }
	void file_log_mgr::log(log_level level, const std::string& title)
    {
        switch (level)
        {
        case log_level::Trace :
            trace_logger->log(level,title);
            break;
        case log_level::Debug :
            debug_logger->log(level,title);
            break;
        case log_level::Info :
            info_logger->log(level,title);
            break;
        case log_level::Warn :
            warn_logger->log(level,title);
            break;
        case log_level::Error :
            error_logger->log(level,title);
            break;
        case log_level::Fatal :
            fatal_logger->log(level,title);
            break;
        default:
            break;
        }
    }
	void file_log_mgr::log(std::vector<std::string>& record)
    {
        /*not implement*/
    }

	void file_log_mgr::table_head(std::vector<std::string>& head){/*not implement*/}
	bool file_log_mgr::is_need_rotate()
    {
        std::lock_guard<std::mutex> lk(m_Locker);
        bool rst = false;
        if(trace_logger != nullptr) {  rst =        trace_logger ->is_need_rotate() ; }
        if(debug_logger != nullptr) {  rst = rst || debug_logger ->is_need_rotate() ; }
        if(info_logger  != nullptr) {  rst = rst || info_logger  ->is_need_rotate() ; }
        if(warn_logger  != nullptr) {  rst = rst || warn_logger  ->is_need_rotate() ; }
        if(error_logger != nullptr) {  rst = rst || error_logger ->is_need_rotate() ; }
        if(fatal_logger != nullptr) {  rst = rst || fatal_logger ->is_need_rotate() ; }
        return rst;    
    }
	void file_log_mgr::flush()
    {
        std::lock_guard<std::mutex> lk(m_Locker);
        if(trace_logger != nullptr) {  trace_logger ->flush() ; }
        if(debug_logger != nullptr) {  debug_logger ->flush() ; }
        if(info_logger  != nullptr) {  info_logger  ->flush() ; }
        if(warn_logger  != nullptr) {  warn_logger  ->flush() ; }
        if(error_logger != nullptr) {  error_logger ->flush() ; }
        if(fatal_logger != nullptr) {  fatal_logger ->flush() ; }
    }

	void file_log_mgr::close()
    {
        // std::cout << "file log close:" << m_ModuleName << std::endl;
        std::lock_guard<std::mutex> lk(m_Locker);
        if(trace_logger != nullptr) {  trace_logger ->close() ; }
        if(debug_logger != nullptr) {  debug_logger ->close() ; }
        if(info_logger  != nullptr) {  info_logger  ->close() ; }
        if(warn_logger  != nullptr) {  warn_logger  ->close() ; }
        if(error_logger != nullptr) {  error_logger ->close() ; }
        if(fatal_logger != nullptr) {  fatal_logger ->close() ; }
    }

	void file_log_mgr::rotate()
    {

        std::lock_guard<std::mutex> lk(m_Locker);
        if(trace_logger != nullptr && trace_logger ->is_need_rotate() ) {  /*std::cout << "file log rotate trace:" << m_ModuleName << std::endl;*/ trace_logger ->rotate() ; }
        if(debug_logger != nullptr && debug_logger ->is_need_rotate() ) {  /*std::cout << "file log rotate debug:" << m_ModuleName << std::endl;*/ debug_logger ->rotate() ; }
        if(info_logger  != nullptr && info_logger  ->is_need_rotate() ) {  /*std::cout << "file log rotate info :" << m_ModuleName << std::endl;*/ info_logger  ->rotate() ; }
        if(warn_logger  != nullptr && warn_logger  ->is_need_rotate() ) {  /*std::cout << "file log rotate warn :" << m_ModuleName << std::endl;*/ warn_logger  ->rotate() ; }
        if(error_logger != nullptr && error_logger ->is_need_rotate() ) {  /*std::cout << "file log rotate error:" << m_ModuleName << std::endl;*/ error_logger ->rotate() ; }
        if(fatal_logger != nullptr && fatal_logger ->is_need_rotate() ) {  /*std::cout << "file log rotate fatal:" << m_ModuleName << std::endl;*/ fatal_logger ->rotate() ; }
        // std::cout << "file log rotate completed!!!" << std::endl;
    }

	void file_log_mgr::set_level(log_level logLevel)
    {
        std::lock_guard<std::mutex> lk(m_Locker);
        if(trace_logger != nullptr) {  trace_logger ->set_level(logLevel) ; }
        if(debug_logger != nullptr) {  debug_logger ->set_level(logLevel) ; }
        if(info_logger  != nullptr) {  info_logger  ->set_level(logLevel) ; }
        if(warn_logger  != nullptr) {  warn_logger  ->set_level(logLevel) ; }
        if(error_logger != nullptr) {  error_logger ->set_level(logLevel) ; }
        if(fatal_logger != nullptr) {  fatal_logger ->set_level(logLevel) ; }

        m_MaxLogLevel = logLevel;
        
    }
	log_level file_log_mgr::get_level()
    {
        std::lock_guard<std::mutex> lk(m_Locker);
        return m_MaxLogLevel;
    }        


void file_log_mgr::set_expired(int minutes)
{
    std::lock_guard<std::mutex> lk(m_Locker);
    if(trace_logger != nullptr) {  trace_logger ->set_expired(minutes) ; }
    if(debug_logger != nullptr) {  debug_logger ->set_expired(minutes) ; }
    if(info_logger  != nullptr) {  info_logger  ->set_expired(minutes) ; }
    if(warn_logger  != nullptr) {  warn_logger  ->set_expired(minutes) ; }
    if(error_logger != nullptr) {  error_logger ->set_expired(minutes) ; }
    if(fatal_logger != nullptr) {  fatal_logger ->set_expired(minutes) ; }
}

int file_log_mgr::get_expired() const
{
    int log_expired = 0;
    // std::lock_guard<std::mutex> lk(m_Locker);
    if(trace_logger != nullptr) {  log_expired = trace_logger ->get_expired() ; }
    if(debug_logger != nullptr) {  log_expired = debug_logger ->get_expired() ; }
    if(info_logger  != nullptr) {  log_expired = info_logger  ->get_expired() ; }
    if(warn_logger  != nullptr) {  log_expired = warn_logger  ->get_expired() ; }
    if(error_logger != nullptr) {  log_expired = error_logger ->get_expired() ; }
    if(fatal_logger != nullptr) {  log_expired = fatal_logger ->get_expired() ; }

    return log_expired;
}

void file_log_mgr::clear_expired_log()
{
    std::lock_guard<std::mutex> lk(m_Locker);
    if(trace_logger != nullptr) { trace_logger ->clear_expired_log() ; }
    if(debug_logger != nullptr) { debug_logger ->clear_expired_log() ; }
    if(info_logger  != nullptr) { info_logger  ->clear_expired_log() ; }
    if(warn_logger  != nullptr) { warn_logger  ->clear_expired_log() ; }
    if(error_logger != nullptr) { error_logger ->clear_expired_log() ; }
    if(fatal_logger != nullptr) { fatal_logger ->clear_expired_log() ; }
}