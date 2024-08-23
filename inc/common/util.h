#ifndef SMPIO_UTIL_H
#define SMPIO_UTIL_H
#include <stdint.h>
#include <string>
#include "common/global.h"

#include <locale>
#include <thread>
#include <pthread.h>
#include <functional>


/* Evaluates to the same boolean value as 'p', and hints to the compiler that
* we expect this value to be false. */
#if defined(__GNUC__) && __GNUC__ >= 3         /* gcc 3.0 or later */
#define UTIL_UNLIKELY(p) __builtin_expect(!!(p),0)
#else
#define UTIL_UNLIKELY(p) (p)
#endif

SMP_UTIL_API void util_mttime_init();
SMP_UTIL_API float util_mttime_diff_ms(uint64_t ts);
SMP_UTIL_API uint64_t util_mttime_tick_count();

struct timezone;
SMP_UTIL_API int util_gettimeofday(struct timeval *tv, struct timezone *tz);
SMP_UTIL_API long util_tv_to_msec(const struct timeval *tv);

SMP_UTIL_API int64_t util_gettimestamp();
SMP_UTIL_API int64_t get_current_timestamp();
// #if defined(c_plus_plus) || defined(__cplusplus) || defined(__cplusplus)
time_t get_timestamp_time_t(int64_t timestamp=0);
std::string get_current_time_string();
std::string get_timestamp_string(int64_t timestamp=0);
std::string get_timestamp_string2(int64_t timestamp=0);
std::string get_timestamp_string_with_ms(int64_t timestamp=0);
void sprintf_std_string(std::string & destination, const char * format, ...);
std::string bytes2hex_string(uint8_t* input, int datasize);

// find substring (case insensitive)
template<typename T>
int ci_find_substr( const T& str1, const T& str2, const std::locale& loc = std::locale() );
void str_toupper( std::string& str, const std::locale& loc = std::locale() );
void set_thread_name(std::thread* thread, const std::string& t_name);
void set_current_thread_name(const std::string& t_name);
std::string get_thread_id_string(std::thread::id tid = std::this_thread::get_id());
void conditional_sleep(
  std::function<int()> func
  ,const std::chrono::milliseconds& n_sleep
  ,const std::chrono::milliseconds& n_interval = std::chrono::milliseconds(25) );

// #endif
// "yyyy-MM-dd HH:mm:ss"
SMP_UTIL_API void util_gettimestampString(int64_t timestamp, char *date_string);
// "yyyyMMddHHmmss"
SMP_UTIL_API void util_gettimestampString2(int64_t timestamp, char *date_string);
// "yyyy-MM-dd HH:mm:ss.zzz"
SMP_UTIL_API void util_gettimestampWithMSString(int64_t timestamp, char *date_string);

SMP_UTIL_API uint16_t util_flipbytes(uint16_t value);
SMP_UTIL_API uint32_t util_flipbytes(uint32_t value);
SMP_UTIL_API uint64_t util_flipbytes(uint64_t value);

SMP_UTIL_API void util_get_vertion(std::string &version);

SMP_UTIL_API void sprintf_std_string(std::string & destination, const char * format, ...);

#ifdef _WIN32
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#define TM_YEAR_BASE 1900
/*
 * We do not implement alternate representations. However, we always
 * check whether a given modifier is allowed for a certain conversion.
 */
#define ALT_E     0x01
#define ALT_O     0x02
#define LEGAL_ALT(x)    { if (alt_format & ~(x)) return (0); }
char * strptime(const char *buf, const char *fmt, struct tm *tm);
static  int conv_num(const char **, int *, int, int);
static const char *day[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
static const char *abday[7] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
static const char *mon[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
static const char *abmon[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static const char *am_pm[2] = { "AM", "PM" };
#endif

#endif //ASHDETECTOR_UTIL_H
