#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cstdarg>

#include <iostream>
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>

#include <algorithm>
#include <thread>
#include <pthread.h>
#include <functional>


#ifdef _WIN32f
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
//#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
#include <limits.h>
#include <time.h>
#if defined( __MINGW32__) || defined(__linux__) || defined(__linux)
#include <sys/time.h>
#endif

#include "common/util.h"

#ifdef _WIN32
LARGE_INTEGER system_frequence = { 0 };
#endif

void util_mttime_init()
{
#ifdef _WIN32
	QueryPerformanceFrequency(&system_frequence);
#else
	//TODO
#endif
}

float util_mttime_diff_ms(uint64_t ts)
{
#ifdef _WIN32
	LARGE_INTEGER timeNow;
	QueryPerformanceCounter(&timeNow);
	return (timeNow.QuadPart - ts) *1000.0 / system_frequence.QuadPart;
#else
	return util_gettimestamp() - ts;
#endif
}

uint64_t util_mttime_tick_count()
{
#ifdef _WIN32
	LARGE_INTEGER timeNow;
	QueryPerformanceCounter(&timeNow);
	return timeNow.QuadPart;
#else
	return util_gettimestamp();
#endif
}

int util_gettimeofday(struct timeval *tv, struct timezone *tz)
{

#if defined( __MINGW32__) || defined (__linux__) || defined(__linux)
    gettimeofday(tv,tz);
    return 0;
#endif

#ifdef _MSC_VER
#define U64_LITERAL(n) n##ui64
#else
#define U64_LITERAL(n) n##llu
#endif

#ifdef _MSC_VER

	/* Conversion logic taken from Tor, which in turn took it
	* from Perl.  GetSystemTimeAsFileTime returns its value as
	* an unaligned (!) 64-bit value containing the number of
	* 100-nanosecond intervals since 1 January 1601 UTC. */
#define EPOCH_BIAS U64_LITERAL(116444736000000000)
#define UNITS_PER_SEC U64_LITERAL(10000000)
#define USEC_PER_SEC U64_LITERAL(1000000)
#define UNITS_PER_USEC U64_LITERAL(10)
	union {
		FILETIME ft_ft;
	uint64_t ft_64;
	} ft;

	if (tv == nullptr)
		return -1;

	GetSystemTimeAsFileTime(&ft.ft_ft);

	if (UTIL_UNLIKELY(ft.ft_64 < EPOCH_BIAS)) {
		/* Time before the unix epoch. */
		return -1;
	}
	ft.ft_64 -= EPOCH_BIAS;
	tv->tv_sec = (long)(ft.ft_64 / UNITS_PER_SEC);
	tv->tv_usec = (long)((ft.ft_64 / UNITS_PER_USEC) % USEC_PER_SEC);
#endif
	return 0;
}


#define MAX_SECONDS_IN_MSEC_LONG \
	(((LONG_MAX) - 999) / 1000)

long util_tv_to_msec(const struct timeval *tv)
{
	if (tv->tv_usec > 1000000 || tv->tv_sec > MAX_SECONDS_IN_MSEC_LONG)
		return -1;

	return (tv->tv_sec * 1000) + ((tv->tv_usec + 999) / 1000);
}

int64_t util_gettimestamp(){
	// struct timeval tv;
	// if (util_gettimeofday(&tv, nullptr) == 0){
	// 	return ((int64_t)tv.tv_sec * 1000 ) + (tv.tv_usec + 999) / 1000 ;
	// }
	// return 0;

	auto now = std::chrono::system_clock::now();
    auto time_stamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return time_stamp;
}

int64_t get_current_timestamp(){return util_gettimestamp();}

time_t get_timestamp_time_t(int64_t timestamp)
{
	if(timestamp <= 0)
	{
		auto now = std::chrono::system_clock::now();
    	auto tt = std::chrono::system_clock::to_time_t(now);
		return tt;
	}
	auto mTime = std::chrono::milliseconds(timestamp);
	auto tp = std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds>(mTime);
	auto tt = std::chrono::system_clock::to_time_t(tp);
	return tt;
}

std::string get_current_time_string(){
	auto now_c = get_timestamp_time_t(0);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %X");
    return ss.str();
}



std::string get_timestamp_string(int64_t timestamp)
{

	auto tt = get_timestamp_time_t(timestamp);
	std::stringstream ss;
    ss << std::put_time(std::localtime(&tt), "%Y-%m-%d %X");
    return ss.str();
}

std::string get_timestamp_string2(int64_t timestamp)
{
	auto tt = get_timestamp_time_t(timestamp);
	std::stringstream ss;
    ss << std::put_time(std::localtime(&tt), "%Y%m%d%H%M%S");
    return ss.str();
}

std::string get_timestamp_string_with_ms(int64_t timestamp)
{
	auto tt = get_timestamp_time_t(timestamp);
	auto ts = timestamp;
	if(ts == 0)
	{
		ts = get_current_timestamp();
	}
	std::string ms = std::to_string(ts % 1000);
	std::stringstream ss;
    ss << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S");
	ss << ".";
	ss << std::setfill('0') << std::setw(3) << ms;
	return ss.str();

	// std::tm* now = std::gmtime(&tt);
	// std::string year = std::to_string(now->tm_year + 1900);
	// std::string month = std::to_string(now->tm_mon + 1);
	// std::string day = std::to_string(now->tm_mday);
	// std::string hour = std::to_string(now->tm_hour);
	// std::string minute = std::to_string(now->tm_min);
	// std::string second = std::to_string(now->tm_sec);
	// std::string ms = std::to_string(timestamp % 1000);
	// return year.append("-").append(month).append("-").append(day).append(" ")
	// 	.append(hour).append(":").append(minute).append(":").append(second).append(".").append(ms);
}



void util_gettimestampString(int64_t timestamp,char *date_string){
	if(date_string)
	{
		auto tstr = get_timestamp_string(timestamp);
		strncpy(date_string,tstr.c_str(),tstr.length());
	}
}

void util_gettimestampWithMSString(int64_t timestamp,char *date_string){
	if(date_string)
	{
		auto tstr = get_timestamp_string_with_ms(timestamp);
		strncpy(date_string,tstr.c_str(),tstr.length());
	}
}

void util_gettimestampString2(int64_t timestamp,char *date_string){
	if(date_string)
	{
		auto tstr = get_timestamp_string2(timestamp);
		strncpy(date_string,tstr.c_str(),tstr.length());
	}
}

uint16_t util_flipbytes(uint16_t value)
{
	return (((value >> 8) & 0x00FF) | (value << 8) | 0xFF00);
}

uint32_t util_flipbytes(uint32_t value)
{
	uint16_t hi = uint16_t(value >> 16);
	uint16_t lo = uint16_t(value & 0xFFFF);

	return uint32_t(util_flipbytes(hi)) | uint32_t(util_flipbytes(lo)) << 16;
}

uint64_t util_flipbytes(uint64_t value)
{
	uint32_t hi = uint32_t(value >> 32);
	uint32_t lo = uint32_t(value & 0xFFFFFFFF);

	return uint64_t(util_flipbytes(hi)) | uint64_t(util_flipbytes(lo)) << 32;
}

void util_get_vertion(std::string &version)
{
	version = "mt_util_V1.3.5";
}

void sprintf_std_string(std::string & destination, const char * format, ...)
{
	va_list args0, args1;
	va_start(args0, format);
	va_copy(args1, args0);
	size_t num_of_chars = std::vsnprintf(nullptr, 0, format, args0);
	va_end(args0);
	destination.resize(num_of_chars + 1, '\0');
	std::vsnprintf(const_cast<char*>(destination.data()), destination.size(), format, args1);
	destination.resize(num_of_chars);
	va_end(args1);
}

std::string bytes2hex_string(uint8_t* input, int datasize)
{
	static std::string emp_str = "";
	if(input == nullptr || datasize <= 0)
	{
	return emp_str;
	}
	std::stringstream ss;
	ss << std::setbase(16) << std::setfill('0');
	for (int i = 0; i < datasize; i++)
	{
	ss << std::setw(2) << (uint32_t)input[i] ;
	ss << " ";
	}
	
	return ss.str();
}


// templated version of my_equal so it could work with both char and wchar_t
template<typename charT>
struct upper_equal {
    upper_equal( const std::locale& loc ) : loc_(loc) {}
    bool operator()(charT ch1, charT ch2) {
        return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
    }
private:
    const std::locale& loc_;
};
 
// find substring (case insensitive)
template<typename T>
int ci_find_substr( const T& str1, const T& str2, const std::locale& loc )
{
    typename T::const_iterator it = std::search( str1.begin(), str1.end(), 
        str2.begin(), str2.end(), upper_equal<typename T::value_type>(loc) );
    if ( it != str1.end() ) return it - str1.begin();
    else return -1; // not found
}


void str_toupper( std::string& str, const std::locale& loc )
{
    std::transform( str.begin(), str.end(), str.begin(), [loc](char ch) { return std::toupper(ch, loc); } );
    // std::transform( str.begin(), str.end(), 
    //     [loc](char ch) { return std::toupper(ch, loc); } );
}

void set_thread_name(std::thread* thread, const std::string& t_name)
{
   std::string nm = t_name;
   if(nm.size() > 15)
   {
      nm = nm.substr(0,15);
   }
   auto handle = thread->native_handle();//获取原生句柄
   #if (!defined(__APPLE__) && !defined(WIN32) && !defined(WIN64))
   pthread_setname_np(handle,nm.c_str());
   #endif
}


void set_current_thread_name(const std::string& t_name)
{
   std::string nm = t_name;
   if(nm.size() > 15)
   {
      nm = nm.substr(0,15);
   }
   auto handle = pthread_self();//获取原生句柄
   #if (!defined(__APPLE__) && !defined(WIN32) && !defined(WIN64))
   pthread_setname_np(handle,nm.c_str());
   #endif
}

std::string get_thread_id_string(std::thread::id tid)
{
    std::ostringstream oss;
    oss << tid;
    return oss.str();
}


void conditional_sleep(
  std::function<int()> func
  ,const std::chrono::milliseconds& n_sleep
  ,const std::chrono::milliseconds& n_interval )
{ 
  static std::chrono::milliseconds n_zero(0);
  auto ns = n_sleep; 
  int64_t bt = util_gettimestamp();
  bool b_continue = func();
  int64_t et = util_gettimestamp();
  ns -= std::chrono::milliseconds(et - bt);
  while(b_continue && ns >= n_zero )
  {
      bt = util_gettimestamp();
      std::this_thread::sleep_for(n_interval);
      et = util_gettimestamp();
      auto cost = std::chrono::milliseconds(et - bt);
      auto x = cost > n_interval ? cost : n_interval;
      ns -= x;
      if(ns <= n_zero)
      {
        break;
      }
      bt = util_gettimestamp();
      b_continue = func();
      et = util_gettimestamp();
      ns -= std::chrono::milliseconds(et - bt);
      //std::cout << "sleep cost:" << cost.count() << ";cost2:" << x.count() << ";func() cost:" << et - bt << ";b_r:" << b_continue << std::endl;
  }
}


#ifdef _WIN32
char * strptime(const char *buf, const char *fmt, struct tm *tm)
{
	char c;
	const char *bp;
	size_t len = 0;
	int alt_format, i, split_year = 0;

	bp = buf;

	while ((c = *fmt) != '\0') {
		/* Clear `alternate' modifier prior to new conversion. */
		alt_format = 0;

		/* Eat up white-space. */
		if (isspace(c)) {
			while (isspace(*bp))
				bp++;

			fmt++;
			continue;
		}

		if ((c = *fmt++) != '%')
			goto literal;


	again:    switch (c = *fmt++) {
	case '%': /* "%%" is converted to "%". */
		literal :
			if (c != *bp++)
				return (0);
		break;

		/*
		 * "Alternative" modifiers. Just set the appropriate flag
		 * and start over again.
		 */
	case 'E': /* "%E?" alternative conversion modifier. */
		LEGAL_ALT(0);
		alt_format |= ALT_E;
		goto again;

	case 'O': /* "%O?" alternative conversion modifier. */
		LEGAL_ALT(0);
		alt_format |= ALT_O;
		goto again;

		/*
		 * "Complex" conversion rules, implemented through recursion.
		 */
	case 'c': /* Date and time, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%x %X", tm)))
			return (0);
		break;

	case 'D': /* The date as "%m/%d/%y". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%m/%d/%y", tm)))
			return (0);
		break;

	case 'R': /* The time as "%H:%M". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%H:%M", tm)))
			return (0);
		break;

	case 'r': /* The time in 12-hour clock representation. */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%I:%M:%S %p", tm)))
			return (0);
		break;

	case 'T': /* The time as "%H:%M:%S". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%H:%M:%S", tm)))
			return (0);
		break;

	case 'X': /* The time, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%H:%M:%S", tm)))
			return (0);
		break;

	case 'x': /* The date, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%m/%d/%y", tm)))
			return (0);
		break;

		/*
		 * "Elementary" conversion rules.
		 */
	case 'A': /* The day of week, using the locale's form. */
	case 'a':
		LEGAL_ALT(0);
		for (i = 0; i < 7; i++) {
			/* Full name. */
			len = strlen(day[i]);
			if (strncmp(day[i], bp, len) == 0)
				break;

			/* Abbreviated name. */
			len = strlen(abday[i]);
			if (strncmp(abday[i], bp, len) == 0)
				break;
		}

		/* Nothing matched. */
		if (i == 7)
			return (0);

		tm->tm_wday = i;
		bp += len;
		break;

	case 'B': /* The month, using the locale's form. */
	case 'b':
	case 'h':
		LEGAL_ALT(0);
		for (i = 0; i < 12; i++) {
			/* Full name. */
			len = strlen(mon[i]);
			if (strncmp(mon[i], bp, len) == 0)
				break;

			/* Abbreviated name. */
			len = strlen(abmon[i]);
			if (strncmp(abmon[i], bp, len) == 0)
				break;
		}

		/* Nothing matched. */
		if (i == 12)
			return (0);

		tm->tm_mon = i;
		bp += len;
		break;

	case 'C': /* The century number. */
		LEGAL_ALT(ALT_E);
		if (!(conv_num(&bp, &i, 0, 99)))
			return (0);

		if (split_year) {
			tm->tm_year = (tm->tm_year % 100) + (i * 100);
		}
		else {
			tm->tm_year = i * 100;
			split_year = 1;
		}
		break;

	case 'd': /* The day of month. */
	case 'e':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_mday, 1, 31)))
			return (0);
		break;

	case 'k': /* The hour (24-hour clock representation). */
		LEGAL_ALT(0);
		/* FALLTHROUGH */
	case 'H':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_hour, 0, 23)))
			return (0);
		break;

	case 'l': /* The hour (12-hour clock representation). */
		LEGAL_ALT(0);
		/* FALLTHROUGH */
	case 'I':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_hour, 1, 12)))
			return (0);
		if (tm->tm_hour == 12)
			tm->tm_hour = 0;
		break;

	case 'j': /* The day of year. */
		LEGAL_ALT(0);
		if (!(conv_num(&bp, &i, 1, 366)))
			return (0);
		tm->tm_yday = i - 1;
		break;

	case 'M': /* The minute. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_min, 0, 59)))
			return (0);
		break;

	case 'm': /* The month. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &i, 1, 12)))
			return (0);
		tm->tm_mon = i - 1;
		break;

	case 'p': /* The locale's equivalent of AM/PM. */
		LEGAL_ALT(0);
		/* AM? */
		if (strcmp(am_pm[0], bp) == 0) {
			if (tm->tm_hour > 11)
				return (0);

			bp += strlen(am_pm[0]);
			break;
		}
		/* PM? */
		else if (strcmp(am_pm[1], bp) == 0) {
			if (tm->tm_hour > 11)
				return (0);

			tm->tm_hour += 12;
			bp += strlen(am_pm[1]);
			break;
		}

		/* Nothing matched. */
		return (0);

	case 'S': /* The seconds. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_sec, 0, 61)))
			return (0);
		break;

	case 'U': /* The week of year, beginning on sunday. */
	case 'W': /* The week of year, beginning on monday. */
		LEGAL_ALT(ALT_O);
		/*
		 * XXX This is bogus, as we can not assume any valid
		 * information present in the tm structure at this
		 * point to calculate a real value, so just check the
		 * range for now.
		 */
		if (!(conv_num(&bp, &i, 0, 53)))
			return (0);
		break;

	case 'w': /* The day of week, beginning on sunday. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_wday, 0, 6)))
			return (0);
		break;

	case 'Y': /* The year. */
		LEGAL_ALT(ALT_E);
		if (!(conv_num(&bp, &i, 0, 9999)))
			return (0);

		tm->tm_year = i - TM_YEAR_BASE;
		break;

	case 'y': /* The year within 100 years of the epoch. */
		LEGAL_ALT(ALT_E | ALT_O);
		if (!(conv_num(&bp, &i, 0, 99)))
			return (0);

		if (split_year) {
			tm->tm_year = ((tm->tm_year / 100) * 100) + i;
			break;
		}
		split_year = 1;
		if (i <= 68)
			tm->tm_year = i + 2000 - TM_YEAR_BASE;
		else
			tm->tm_year = i + 1900 - TM_YEAR_BASE;
		break;

		/*
		 * Miscellaneous conversions.
		 */
	case 'n': /* Any kind of white-space. */
	case 't':
		LEGAL_ALT(0);
		while (isspace(*bp))
			bp++;
		break;


	default:  /* Unknown/unsupported conversion. */
		return (0);
	}


	}

	/* LINTED functional specification */
	return ((char *)bp);
}


static int conv_num(const char **buf, int *dest, int llim, int ulim)
{
	int result = 0;

	/* The limit also determines the number of valid digits. */
	int rulim = ulim;

	if (**buf < '0' || **buf > '9')
		return (0);

	do {
		result *= 10;
		result += *(*buf)++ - '0';
		rulim /= 10;
	} while ((result * 10 <= ulim) && rulim && **buf >= '0' && **buf <= '9');

	if (result < llim || result > ulim)
		return (0);

	*dest = result;
	return (1);
}
#endif
