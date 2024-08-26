#include "logging/ilog.h"
#include "logging/log_mgr.h"
#include <atomic>
#include <string.h>
#include <thread>
#include "common/util.h"
using smp::logging::log_mgr;
using smp::logging::ilog;
using smp::logging::log_level;
using smp::logging::log_format;

void Mylog(char* data, int len) {
	char *tmp = (char*)malloc(len * 4);//{0};
	memset(tmp, 0x00, len * 4);
	int i = 0;
	for (i = 0;i< len;++i) {
		snprintf(tmp + 3 * i, 4, "%02x ", (unsigned char)data[i]);
	}
	tmp[3 * i] = 0x00;
	log_mgr::get_instance().get_logger("LogExample")->log(log_level::Trace,"%s",tmp);
	printf("log [%d|%d]:%s\n", len, (int32_t)strlen(tmp), tmp);
	free(tmp);
}

#define THREAD_NUM 10
std::shared_ptr<ilog> logger = nullptr;
std::thread* ThreadVec[THREAD_NUM] = {0};

struct XData
{
	uint64_t id;
	char content[32];
};

void DoLogging() {
	char data[808] = {0x00, 0x00, 0x00, 0x29, 0x76, 0x11, 0x40, 0x7f, 0x00, 0x00};
	for (size_t i = 8;i<sizeof(data);++i) {
		data[i] = i % 0xff;
	}
	static std::atomic_int32_t counter = 0;
	char bufferCtx[4] = {0};
	std::string logMsg;
	for (int j = 0; j < 808; j++) {
		snprintf(bufferCtx,sizeof(bufferCtx), "%02x ", (unsigned char)data[j]);
		logMsg.append(bufferCtx, 3);
	}

	XData xdata;
	strcpy(xdata.content, "test data");

	for (int i = 0; i < 20000; i++) {
		xdata.id = i;
		std::string xx((char*)&xdata, sizeof(xdata));
		logger->log_binary(log_level::Debug, xx);
		logger->log(log_level::Debug, "log debug counter:(",counter.load(),")");
		counter++;
	}		
}

int main()
{
	int64_t timestamp = util_gettimestamp();
	std::string s1 =  get_timestamp_string();
	std::string s2 = get_timestamp_string( timestamp);
	std::string s3 = get_timestamp_string2( timestamp);
	std::string s4 = get_timestamp_string_with_ms( timestamp);

	std::cout << "s1:" << s1 << std::endl;
	std::cout << "s2:" << s2 << std::endl;
	std::cout << "s3:" << s3 << std::endl;
	std::cout << "s4:" << s4 << std::endl;

	//std::thread::id tid = std::this_thread::get_id();
	//printf("Main thread id = [%d]\n", tid);
	log_mgr::get_instance().start();
	printf("LogExample start.\n");
	logger = log_mgr::get_instance().get_logger("LogExample", "./log", log_format::File);
	// logger->set_path("./log/");
	logger->set_level(log_level::Debug);
	logger->set_expired(1);

	//char data[808] = {0x00, 0x00, 0x00, 0x29, 0x76, 0x11, 0x40, 0x7f, 0x00, 0x00};
	//for (int i = 8;i<sizeof(data);++i) {
	//	data[i] = i % 0xff;
	//}
	//char bufferCtx[4] = {0};
	//std::string logMsg;
	//for (int j = 0; j < 808; j++) {
	//	sprintf(bufferCtx, "%02x ", (unsigned char)data[j]);
	//	logMsg.append(bufferCtx, 3);
	//}
	for (int i = 0; i < THREAD_NUM; i++) {
		ThreadVec[i] = new std::thread(DoLogging);
	}
	
	static int joinableNum = 0;
	while (true) {
		for (int i = 0; i < THREAD_NUM; i++) {
			if (ThreadVec[i]->joinable()) {
				joinableNum++;
			}
		}
		if (joinableNum == 10) {
			break;
		}
		else {
			joinableNum = 0;
		}
	}
	
	for (int i = 0; i < THREAD_NUM; i++) {
		ThreadVec[i]->join();
		delete ThreadVec[i];
		ThreadVec[i] = 0;
	}
	printf("LogExample finish.\n"
	"Press any key to exit.");
	getchar();
	log_mgr::get_instance().stop();
	return getchar();
}

//#include "Logging/CSVLog.h"
//using SMP::Util::Logging::CSVLog;
//using SMP::IO::Logging::LogFormat;
//int main() {
//	LogMgr::GetInstance().Start();
//	printf("Log start.\n");
//	ILog* log = LogMgr::GetInstance().GetLogger("LogCSVExample", LogFormat::CSV);
//
//	std::vector<std::string> record;
//	record.push_back("FileName");
//	record.push_back("Angle");
//	log->TableHead(record);
//	
//	for (int i = 0; i < 1000; i++) {
//		record.clear();
//		char filename[256] = {0};
//		sprintf(filename, "%d.bmp", i);
//		record.push_back(filename);
//		float d = (float)rand() / RAND_MAX;
//		char fval[256] = {0};
//		sprintf(fval,"%f",d);
//		record.push_back(fval);
//		log->Log(record);
//	}
//	
//	printf("Log finish.\n");
//	getchar();
//	LogMgr::GetInstance().Stop();
//	return getchar();
//}